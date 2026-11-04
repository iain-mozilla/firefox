/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef PWAProcessChild_h
#define PWAProcessChild_h

#include "PWAChild.h"
#include "mozilla/ipc/ProcessChild.h"

namespace mozilla {
namespace pwa {

// ProcessChild that exists inside the separate process with a connection
// to PWAProcessHost inside Firefox
class PWAProcessChild final : public mozilla::ipc::ProcessChild {
 public:
  // explicit PWAProcessChild(ProcessId aParentPid, int aFd);
  explicit PWAProcessChild(IPC::Channel::ChannelHandle aClientHandle, ProcessId aParentPid, const nsID& aMessageChannelId);
  // PWAProcessChild(ProcessId aProcessId);

  bool Init(int aArgc, char* aArgv[]) override;
  bool Init(const char* parentBuildId, std::vector<nsCString>sharedMemoryHandles);
  void CleanUp() override;

 private:
  RefPtr<PWAChild> mChild;
  DISALLOW_COPY_AND_ASSIGN(PWAProcessChild);
};

} // namespace pwa
} // namespace mozilla

#endif