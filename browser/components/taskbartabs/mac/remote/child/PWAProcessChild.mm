/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "PWAProcessChild.h"
#include "PWAChild.h"
#include "mozilla/ipc/IOThread.h"

namespace mozilla {
namespace pwa {

PWAProcessChild::PWAProcessChild(IPC::Channel::ChannelHandle aClientHandle,
                                 ProcessId aParentPid,
                                 const nsID& aMessageChannelId)
      : ProcessChild(std::move(aClientHandle), aParentPid, aMessageChannelId) {
  printf("*** PWAProcessChild started.\n");
  mChild = new PWAChild(aParentPid);
}

bool
PWAProcessChild::Init(int aArgc, char* aArgv[]) {
  return true;
}

bool
PWAProcessChild::Init(const char* parentBuildId,
                      std::vector<nsCString> sharedMemoryHandles) {
  printf("*** PWAProcessChild Init\n");
  return mChild->Init(parentBuildId, std::move(sharedMemoryHandles),
                      TakeInitialEndpoint());
}

void
PWAProcessChild::CleanUp() {}

} // namespace pwa
} // namespace mozilla