/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef PWACHILD_H
#define PWACHILD_H

#include "mozilla/pwa/PPWAChild.h"
#include "mozilla/ipc/ProcessChild.h"
#include "mozilla/UniquePtr.h"

namespace mozilla {
namespace pwa {

class PWAProcessChild;
class PWAWindowChild;

class PWAChild: public PPWAChild {
  friend class PWAProcessChild;

 public:
  NS_INLINE_DECL_THREADSAFE_REFCOUNTING(PWAChild, override);

  PWAChild();

 private:
  friend class PPWAChild;

  explicit PWAChild(ProcessId aProcessId);
  bool Init(const char* aParentBuildID,
            std::vector<nsCString> sharedMemoryHandles,
            mozilla::ipc::UntypedEndpoint&& aEndpoint);

  void ActorDestroy(ActorDestroyReason aWhy) override {
    // mPWAWindowChild = nullptr;
  }

  mozilla::ipc::IPCResult RecvCreatePWAWindowChild(Endpoint<PPWAWindowChild>&& aEndpoint);

  ~PWAChild();

  ProcessId mParentPid;
  RefPtr<PWAWindowChild> mPWAWindowChild;
};

} // namespace pwa
} // namespace mozilla

#endif