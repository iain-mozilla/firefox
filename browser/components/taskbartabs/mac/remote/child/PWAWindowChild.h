/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef PWAWindowChild_h
#define PWAWindowChild_h

#include "mozilla/pwa/PPWAWindowChild.h"

class PWAWindow;

namespace mozilla {
namespace pwa {
/**
 * ITopLevelActor connected to the PWA process.
 * Instances are owned by IPDL and PWAChild
 */
class PWAWindowChild final : public PPWAWindowChild {
 public:
  NS_INLINE_DECL_THREADSAFE_REFCOUNTING(PWAWindowChild, override);

  PWAWindowChild();

 private:
  friend PPWAWindowChild;
  ~PWAWindowChild() = default;

  mozilla::ipc::IPCResult RecvSetTitle(nsString title);
  void ActorDestroy(ActorDestroyReason aWhy) override;

  PWAWindow* mWindow;
};

} // namespace pwa
} // namespace mozilla
#endif
