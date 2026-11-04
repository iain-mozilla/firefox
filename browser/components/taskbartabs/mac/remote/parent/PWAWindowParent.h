/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef PWAWindowParent_h
#define PWAWindowParent_h

#include "mozilla/pwa/PPWAWindowParent.h"

namespace mozilla {
namespace pwa {

/**
 * ITopLevelActor for the PWA process in the main process
 */
class PWAWindowParent final : public PPWAWindowParent {
 public:
  NS_INLINE_DECL_THREADSAFE_REFCOUNTING(PWAWindowParent, override)

 private:
  friend class PPWAWindowParent;

  PWAWindowParent() = default;
  ~PWAWindowParent() = default;

  void ActorDestroy(ActorDestroyReason aWhy) override;
};

} // namespace pwa
} // namespace mozilla

#endif
