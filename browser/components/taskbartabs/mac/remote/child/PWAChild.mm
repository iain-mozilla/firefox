/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "PWAChild.h"
#include "PWAWindowChild.h"

#include "mozilla/ipc/ProcessUtils.h"
#include "nsThreadManager.h"
#include "nsDebugImpl.h"
#include "mozilla/ipc/Endpoint.h"

namespace mozilla {
namespace pwa {

PWAChild::PWAChild(ProcessId aProcessId) : mParentPid(aProcessId) {}

bool PWAChild::Init(const char* parentBuildID,
                    std::vector<nsCString> sharedMemoryHandles,
                    mozilla::ipc::UntypedEndpoint&& aEndPoint) {
  // Not sure if the child needs preferences or xpcom yet.
  // if (!mozilla::ipc::ProcessChild::InitPrefs(aArgc, aArgv)) {
  //   printf("*** No prefs\n");
  //   return false;
  // }

  if (NS_WARN_IF(NS_FAILED(nsThreadManager::get().Init()))) {
    printf("*** No threads\n");
    return false;
  }

  nsDebugImpl::SetMultiprocessMode("PWA");
  mozilla::ipc::SetThisProcessName("PWA Process");

  if (NS_WARN_IF(!aEndPoint.Bind(this))) {
    return false;
  }

  MessageChannel* channel = GetIPCChannel();
  channel->SetAbortOnError(true);

  // Require that our code came from the same build as the main process
  if (channel && !channel->SendBuildIDsMatchMessage(parentBuildID)) {
    NS_WARNING("Failed to send BuildID message of BuildID for PWA "
    "process does not match parent process.");
    mozilla::ipc::ProcessChild::QuickExit();
  }

  // Start crash reporter
  // mozilla::ipc::CrashReporterClient::InitSingleton(this);

  /*
  if (NS_FAILED(NS_InitMinimalXPCOM())) {
    return false;
  }
*/

  printf("Sending hello\n");
  if (!SendChildConnected()) {
    printf("Failed to send ChildConnected\n");
    return false;
  }

  return true;
}

PWAChild::~PWAChild() {
#ifndef NS_FREE_PERMANENT_DATA
  mozilla::ipc::ProcessChild::QuickExit();
#endif
  // Shutdown crash reporter
  // mozilla::ipc::CreateReporterClient::DestroySingleton();

  XRE_ShutdownChildProcess();
}

mozilla::ipc::IPCResult PWAChild::RecvCreatePWAWindowChild(
    Endpoint<PPWAWindowChild>&& aEndPoint) {
  printf("RecvCreatePWAWindowChild\n");

  mPWAWindowChild = new PWAWindowChild();

  printf("Binding on %d, %d\n", base::GetCurrentProcId(), XRE_GetChildID());
  DebugOnly<bool> ok = aEndPoint.Bind(mPWAWindowChild);
  MOZ_ASSERT(ok);

  return IPC_OK();
}

} // namespace pwa
} // namespace mozilla