/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */
#ifndef PWAManager_h
#define PWAManager_h

#include "mozilla/ipc/GeckoChildProcessHost.h"
#include "mozilla/ipc/ProcessUtils.h"
#include "mozilla/pwa/ConnectionError.h"
#include "mozilla/pwa/PWAParent.h"
#include "base/rand_util.h"
#include "nsISupportsImpl.h"
#include "nsIObserver.h"

namespace mozilla {
namespace pwa {

// using ProcessConnectionPromise = mozilla::MozPromise<bool, int32_t, false>;
typedef mozilla::MozPromise<bool, int32_t, false>
  ProcessConnectionPromise;

class PWAProcessHost final : public mozilla::ipc::GeckoChildProcessHost,
                             public nsIObserver {
  friend class PWAParent;
 public:
  NS_DECL_THREADSAFE_ISUPPORTS
  NS_DECL_NSIOBSERVER

  PWAProcessHost(int childConnection)
    : GeckoChildProcessHost(GeckoProcessType_PWA, false),
      mChildConnection(childConnection) {}

  void OnChannelConnected(int32_t peer_pid) override;
  RefPtr<ProcessConnectionPromise> SetupConnection();
  void InitAfterConnect(bool aSucceeded);
  void Init();
  bool BuildProcessArgs(std::vector<std::string>* aExtraArgs);
  bool MakeBridgeAndResolve();

 private:
  ~PWAProcessHost();
  RefPtr<ProcessConnectionPromise> InitializeChannel();
  // RefPtr<ProcessConnectionPromise> SetupNewConnection(PWAProcessHost* self);
  // nsCString SerializeSharedMemoryHandle(const nsCString& name, ipc::ReadOnlySharedMemoryHandle handle);

  RefPtr<ProcessConnectionPromise> mConnectionPromise;
  int mChildConnection;
  IPC::Channel::ChannelHandle mClientHandle;
  base::ProcessId mParentPid;

  char mInitialChannelIdString[NSID_LENGTH];
  GeckoChildID mChildID;
  char mChildIDString[32];
  base::ProcessId mChildPid;

  UniqueMachReceiveRight mParentRecvPort;
  task_t mChildTask;
  UniquePtr<ipc::SharedPreferenceSerializer> mPrefSerializer;
  std::vector<mozilla::UniqueMachSendRight> mSendRights;
  std::vector<mozilla::UniqueMachReceiveRight> mReceiveRights;
  std::vector<nsCString> mSharedMemoryHandles;

  RefPtr<PWAParent> mPWAParent;
  ipc::Endpoint<PPWAWindowParent> mParentEndpoint;
};

class PWAManager final : public nsIObserver {
 public:
  NS_DECL_THREADSAFE_ISUPPORTS
  NS_DECL_NSIOBSERVER

  PWAManager();

  void InitParent(PWAParent* parent, int childConnection);
  already_AddRefed<PWAParent> GetPWA(uint32_t remoteId);

 private:
  ~PWAManager();

  void StartListen();
  void Listen();
  base::ProcessId mChildPid;
  nsTArray<RefPtr<PWAParent>> mPWAs;
  uint32_t mLastId;
  int mListenerSocket;
  nsTArray<RefPtr<PWAProcessHost>> mHosts;
};

} // namespace pwa
} // namespace mozilla

void NS_InitPWAManager();

#endif