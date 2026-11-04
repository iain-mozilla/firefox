/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "PWAParent.h"
#include "PWAManager.h"
#include "nsAppRunner.h"
#include "mozilla/ipc/IOThread.h"

namespace mozilla {
namespace pwa {

PWAParent::PWAParent(PWAProcessHost* host) : mHost(host) {
}

bool PWAParent::Init() {
  printf("PWAParent::Init()\n");
  return true;
}

void PWAParent::ActorDestroy(ActorDestroyReason aWhy) {
  if (aWhy == AbnormalShutdown) {
    // GenerateCrashReport(OtherPid());
  }

  mHost->mPWAParent = nullptr;
}

PWAParent::~PWAParent() {
  printf("Deleting PWAParent\n");
  // Manager::GetInstance()->mWeakActors.RemoveElement(WeakPtr(this));
}

void PWAParent::Shutdown() {
}

ipc::IPCResult PWAParent::RecvChildConnected() {
  printf("*** Received child connected\n");

  if (!mHost->MakeBridgeAndResolve()) {
    printf("Error MakeBridgetAndResolve\n");
  }

  return IPC_OK();
}

} // namespace pwa
} // namespace mozilla
