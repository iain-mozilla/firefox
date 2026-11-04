/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "PWAManager.h"
#include "ConnectionError.h"
#include "PWAParent.h"

#include "mozilla/ipc/IOThread.h"
#include "mozilla/ipc/ProtocolUtils.h"
#include "mozilla/pwa/PPWAWindow.h"
#include "mozilla/pwa/PWAConnectionUtils.h"
#include "chrome/common/chrome_switches.h"
#include "chrome/common/process_watcher.h"
#include "chrome/common/mach_ipc_mac.h"
#include "mozilla/MozPromise.h"
#include "mozilla/UniquePtr.h"
#include "nsAppRunner.h"
#include "nsThreadUtils.h"
#include "nsIWidget.h"
#include "nsIObserverService.h"
#include "nsCRT.h"

#include <mach/message.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <servers/bootstrap.h>
#include <cstdint>

using namespace mozilla;

namespace mozilla {
namespace pwa {

NS_IMPL_ISUPPORTS(PWAProcessHost, nsIObserver)
void PWAProcessHost::Init() {

}

PWAProcessHost::~PWAProcessHost() {

}

void PWAProcessHost::OnChannelConnected(int32_t peer_pid) {
  printf("*** OnChannelConnected %d\n", peer_pid);
  GeckoChildProcessHost::OnChannelConnected(peer_pid);

  // FIXME: RDDProcessHost & GPUProcessHost have a liveToken.
  // Find out what that is and if we need it
  NS_DispatchToMainThread(NS_NewRunnableFunction(
    "PWAProcessHost::OnChannelConnected", [this]() {
      InitAfterConnect(true);
    }));
}

void PWAProcessHost::InitAfterConnect(bool aSucceeded) {
  MOZ_ASSERT(NS_IsMainThread());
  MOZ_ASSERT(!mPWAParent);

  if (!aSucceeded) {
    return;
  }

  mPWAParent = MakeRefPtr<PWAParent>(this);
  DebugOnly<bool> rv = TakeInitialEndpoint().Bind(mPWAParent.get());
  MOZ_ASSERT(rv);

  if (!mPWAParent->Init()) {
    printf("Failed to init\n");
  }
}

// Shared memory handles are turned into name,handle,size tuples (unlike for processes
// that use commandline args where the name is the command line argument). ":" is chosen
// to match GeckoArgs.cpp, but it means that @name cannot contain ":", which shouldn't
// be a problem, but calling it out here to make sure.
nsCString SerializeSharedMemoryHandle(
  const nsCString& name,
  ipc::ReadOnlySharedMemoryHandle handle,
  std::vector<UniqueMachSendRight>* sendRights) {
  auto size = handle.Size();
  auto platformHandle = std::move(handle).TakePlatformHandle();
  MOZ_ASSERT(platformHandle, "shmem platform handle is invalid");

  sendRights->push_back(std::move(platformHandle));
  uint32_t position = sendRights->size() - 1;

  nsCString arg = nsPrintfCString("%s:%d:%llu", name.get(), position, size);

  return arg;
}

RefPtr<ProcessConnectionPromise> PWAProcessHost::SetupConnection() {
  geckoargs::ChildProcessArgs args;

  MOZ_ASSERT(NS_IsMainThread());

  if (mConnectionPromise) {
    return mConnectionPromise;
  }

  mPrefSerializer = MakeUnique<ipc::SharedPreferenceSerializer>();
  if (!mPrefSerializer->SerializeToSharedMemory(GeckoProcessType_PWA, {})) {
    return ProcessConnectionPromise::CreateAndReject(-1, __func__);;
  }

  // Get the preferences handles
  auto prefsHandle = mPrefSerializer->GetPrefsHandle().Clone();
  MOZ_RELEASE_ASSERT(prefsHandle, "failed to clone prefs handle");
  auto prefMapHandle = mPrefSerializer->GetPrefMapHandle().Clone();
  MOZ_RELEASE_ASSERT(prefMapHandle, "failed to clone pref map handle");

  auto prefsStr = SerializeSharedMemoryHandle("prefsHandle"_ns, std::move(prefsHandle), &mSendRights);
  auto prefMapStr = SerializeSharedMemoryHandle("prefMapHandle"_ns, std::move(prefMapHandle), &mSendRights);
  mSharedMemoryHandles.push_back(std::move(prefsStr));
  mSharedMemoryHandles.push_back(std::move(prefMapStr));

  mConnectionPromise = mozilla::InvokeAsync(
    XRE_GetAsyncIOEventTarget(), this, __func__,
    &PWAProcessHost::InitializeChannel)
      -> Then(
        XRE_GetAsyncIOEventTarget(), __func__,
        [](bool aResult) {
          return ProcessConnectionPromise::CreateAndResolve(aResult, __func__);
        },
        [](int32_t aError) {
          return ProcessConnectionPromise::CreateAndReject(aError, __func__);
        }
      );

  return mConnectionPromise;
}

// We need to follow the launch sequence to set up everything
// but instead of forking and using command line arguments, we use a unix socket
RefPtr<ProcessConnectionPromise> PWAProcessHost::InitializeChannel() {
  ipc::AssertIOThread();

  mClientHandle.emplace<mozilla::UniqueFileHandle>(mChildConnection);

  if (!InitializeChannelWithHandle(&mClientHandle)) {
    return ProcessConnectionPromise::CreateAndReject(-1, __func__);
  }

  GetInitialChannelId().ToProvidedString(mInitialChannelIdString);
  printf("*** channel id: %s\n", mInitialChannelIdString);

  mParentPid = base::GetCurrentProcId();

  // Need the uuid to make the mach service name
  nsCString uuid;
  if (PWAConnectionUtils::ReadString(mChildConnection, uuid) == -1) {
    perror("Reading uuid");
    return ProcessConnectionPromise::CreateAndReject(-1, __func__);
  }

  printf("Read uuid: %s\n", uuid.get());

  std::string machServiceName(
    StringPrintf("org.mozilla.machname.%s.%d", uuid.get(),
      base::RandInt(0, std::numeric_limits<int>::max())));
  MOZ_ASSERT(machServiceName.size() < BOOTSTRAP_MAX_NAME_LEN);

  // At this point we've gathered all the information we need to send
  // to the child process. Usually it would be put into the argv passed
  // to the child after fork, but the child is already running.

  // bootstrap the mach port we're sending to the child
  // This might need to use XPC rather than mach port
  const char* machServiceNameCStr = machServiceName.c_str();
  kern_return_t kr =
        bootstrap_check_in(bootstrap_port, machServiceNameCStr,
                           getter_Transfers(mParentRecvPort));
  if (kr != KERN_SUCCESS) {
    printf("*** parent bootstrap_check_in failed: %s\n", mach_error_string(kr));
    return ProcessConnectionPromise::CreateAndReject(-2, __func__);
  }

  // Instead of forking to get the childPid, read it from the socket
  base::ProcessId childPid;

  // if (read(childConnection, &childPid, sizeof(childPid)) == -1) {
  if (PWAConnectionUtils::ReadProcessId(mChildConnection, &childPid) == -1) {
    perror("Reading child pid");
    return ProcessConnectionPromise::CreateAndReject(-3, __func__);
  }

  printf("*** Child pid: %d\n", childPid);
  mChildPid = childPid;

  // Write all the information the child needs to the socket
  // Data sent to the child
  // Process ID - ProcessId (int)
  // Service name length (+1 for nul terminator) - uint32
  // Service name - char[Service name length]
  // child id - int32_t
  // channel id length (+ 1 for nul terminator) - uint32
  // channel id - char[channel id length]
  // build id length (+1 for nul terminator) - uint32
  // build id - char[build id length]
  // number of shared memory handles - uint32
  //    for each shared memory handle:
  //        handle length - uint32
  //        handle - char[handle length]

  if (PWAConnectionUtils::WriteProcessId(mChildConnection, mParentPid) == -1) {
    perror("Sending pid");
    return ProcessConnectionPromise::CreateAndReject(-4, __func__);
  }

  if (PWAConnectionUtils::WriteCString(mChildConnection, machServiceNameCStr) == -1) {
    perror("Sending service name");
    return ProcessConnectionPromise::CreateAndReject(-5, __func__);
  }

  if (PWAConnectionUtils::WriteUInt32(mChildConnection, GetChildID()) == -1) {
    perror("Sending child id");
    return ProcessConnectionPromise::CreateAndReject(-12, __func__);
  }
  
  if (PWAConnectionUtils::WriteCString(mChildConnection, mInitialChannelIdString) == -1) {
    perror("Sending channel Id");
    return ProcessConnectionPromise::CreateAndReject(-6, __func__);
  }

  nsCString parentBuildId(mozilla::PlatformBuildID());
  if (PWAConnectionUtils::WriteString(mChildConnection, parentBuildId) == -1) {
    perror("Sending build id");
    return ProcessConnectionPromise::CreateAndReject(-7, __func__);
  }

  uint32_t handleCount = mSharedMemoryHandles.size();
  if (PWAConnectionUtils::WriteUInt32(mChildConnection, handleCount) == -1) {
    perror("Sending handle count");
    return ProcessConnectionPromise::CreateAndReject(-8, __func__);
  }

  for(uint32_t id = 0; id < handleCount; id++) {
    nsCString handle = mSharedMemoryHandles[id];
    if (PWAConnectionUtils::WriteString(mChildConnection, handle) == -1) {
      perror("Sending handle");
      return ProcessConnectionPromise::CreateAndReject(-9, __func__);
    }
  }

  // Set up machport receiver to get the child's task_t
  return MachHandleProcessCheckIn(
                  std::move(mParentRecvPort),
                  mChildPid,
                  mozilla::TimeDuration::FromSeconds(10),
                  std::move(mSendRights),
                  std::move(mReceiveRights))
          ->Then(
              XRE_GetAsyncIOEventTarget(), __func__,
              [this](task_t aTask) mutable {
                  mChildTask = aTask;

                  if (mNodeChannel) {
                    mNodeChannel->SetOtherPid(base::GetProcId(this->mChildPid));
                    mNodeChannel->SetMachTaskPort(this->mChildTask);
                  }

                  MonitorAutoLock lock(mMonitor);
                  // The OnChannel{Connected,Error} may have already advanced
                  // the state.
                  if (mProcessState < PROCESS_CREATED) {
                    mProcessState = PROCESS_CREATED;
                  }
                  lock.Notify();

                  return ProcessConnectionPromise::CreateAndResolve(true, __func__);
                },
                [](base::LaunchError aError) {
                  return ProcessConnectionPromise::CreateAndReject(-10, __func__);
          });
}

NS_IMETHODIMP
PWAProcessHost::Observe(nsISupports* aSubject,
                        char const* aTopic,
                        char16_t const* aData) {
  return NS_OK;
}

bool PWAProcessHost::MakeBridgeAndResolve() {
  printf("Make bridge and resolve\n");

  ipc::Endpoint<PPWAWindowParent> parent;
  ipc::Endpoint<PPWAWindowChild> child;

  base::ProcessId parentPid = getpid();
  ipc::EndpointProcInfo other = mPWAParent->OtherEndpointProcInfo();

  printf("Other endpoint: %d, %d\n", other.mPid, other.mChildID);
  nsresult rv = PPWAWindow::CreateEndpoints(ipc::EndpointProcInfo::Current(),
                                            mPWAParent->OtherEndpointProcInfo(),
                                            &parent, &child);

  if (NS_WARN_IF(NS_FAILED(rv))) {
    NS_WARNING("Failed to create PPWAWindow endpoints");
    return false;
  }

  if (!mPWAParent) {
    NS_WARNING("Cannot SendCreatePWAWindowChild. Actor is already destroyed");
    return false;
  }

  if (!mPWAParent->SendCreatePWAWindowChild(std::move(child))) {
    NS_WARNING("Failed to SendCreatePWAWindowChild");
    return false;
  }

  mParentEndpoint = std::move(parent);
  return true;
}

RefPtr<PWAManager> sManager;

NS_IMPL_ISUPPORTS(PWAManager, nsIObserver)

PWAManager::PWAManager() : mLastId(0) {
  StartListen();

  nsCOMPtr<nsIObserverService> observerService =
    mozilla::services::GetObserverService();
  if (!observerService) {
    return;
  }

  observerService->AddObserver(this, NS_XPCOM_SHUTDOWN_OBSERVER_ID, false);
}

PWAManager::~PWAManager() {
}

NS_IMETHODIMP
PWAManager::Observe(nsISupports* aSubject,
                    const char* aTopic,
                    const char16_t* aData) {
  if (!nsCRT::strcmp(aTopic, NS_XPCOM_SHUTDOWN_OBSERVER_ID)) {
    printf("*** XPCOM shutdown.\n");
    int socket = mListenerSocket;
    mListenerSocket = -1;
    close(socket);

    sManager = nullptr;

    for (unsigned long i = 0; i < mPWAs.Length(); i++) {
      mPWAs[i]->Shutdown();
    }

    mPWAs.Clear();
  }

  return NS_OK;
}

void
PWAManager::StartListen() {
  RefPtr<Runnable> listener = mozilla::NewRunnableMethod("PWAListener", this, &PWAManager::Listen);
  nsCOMPtr<nsIThread> thread;
  NS_NewNamedThread("PWAListener"_ns, getter_AddRefs(thread), listener);

  // Is this handled anywhere? Should it be?
  nsCOMPtr<nsIObserverService> os = mozilla::services::GetObserverService();
  os->AddObserver(this, "quit-application", false);
}

void
PWAManager::Listen() {
  nsCOMPtr<nsIFile> socketFile;

  // FIXME: Get the proper file path
  nsresult rv = NS_NewLocalFile(u"/Users/iholmes/Library/Caches/Firefox/pwa.sock"_ns,
                getter_AddRefs(socketFile));
  NS_ENSURE_SUCCESS_VOID(rv);

  socketFile->Remove(true);

  printf("*** Creating socket.\n");
  mListenerSocket = socket(AF_UNIX, SOCK_STREAM, 0);
  if (mListenerSocket == -1) {
    perror("Creating socket");
    return;
  }

  struct sockaddr_un addr;
  memset(&addr, 0, sizeof(struct sockaddr_un));
  addr.sun_family = AF_UNIX;
  strncpy(addr.sun_path, socketFile->NativePath().get(), sizeof(addr.sun_path) - 1);

  printf("*** binding\n");
  if (bind(mListenerSocket, (const sockaddr *)&addr, sizeof(addr)) == -1) {
    perror("Binding to socket");
    return;
  }

  if (listen(mListenerSocket, 5) == -1) {
    perror("Listening");
    return;
  }

  while (mListenerSocket > 0) {
    int childConnection = accept(mListenerSocket, NULL, NULL);
    if (childConnection == -1) {
      perror("Accepting connection");
      continue;
    }

    printf("Accepted connection,  creating host for Child\n");
    RefPtr<PWAProcessHost> host = MakeRefPtr<PWAProcessHost>(childConnection);

    mHosts.AppendElement(host);
    printf("Invoke async\n");
    mozilla::InvokeAsync(
      GetMainThreadSerialEventTarget(), host.get(), __func__,
      &PWAProcessHost::SetupConnection);
      /*
        ->Then(
        GetMainThreadSerialEventTarget(), __func__,
        [this, host = std::move(host)](bool result) {
          printf("SetupConnection->Then %d\n", this->mChildPid);
          MOZ_ASSERT(NS_IsMainThread());

          PWAParent* parent = new PWAParent(host);
          mPWAs.AppendElement(parent);
          // return ProcessConnectionPromise::CreateAndResolve(result, __func__);
          return mozilla::InvokeAsync(
            XRE_GetAsyncIOEventTarget(), parent, __func__,
            &PWAParent::Init);
        },
        [](int32_t aError) {
          printf("Got an error from somewhere %d\n", aError);
          return ProcessConnectionPromise::CreateAndReject(aError, __func__);
        }
      );*/
    printf("returning to listen\n");
  }
}

} // namespace pwa
} // namespace mozilla

void
NS_InitPWAManager() {
  if (mozilla::pwa::sManager) {
    return;
  }

  mozilla::pwa::sManager = new mozilla::pwa::PWAManager();
}