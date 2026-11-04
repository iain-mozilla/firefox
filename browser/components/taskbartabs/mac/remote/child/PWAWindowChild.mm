/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#import <AppKit/AppKit.h>

#include "PWAWindowChild.h"
#include "PWAChild.h"
#include "PWAWindow.h"

namespace mozilla {
namespace pwa {

PWAWindowChild::PWAWindowChild() {
  MOZ_ASSERT(XRE_IsPWAProcess());
  printf("*** PWAWindowChild ctor\n");

  mWindow = new PWAWindow();
}

mozilla::ipc::IPCResult PWAWindowChild::RecvSetTitle(nsString title) {
  const unichar* uniTitle = reinterpret_cast<const unichar*>(title.get());
  // [mWindow setTitle:[NSString stringWithCharacters:uniTitle length:title.Length()]];

  return IPC_OK();
}

void PWAWindowChild::ActorDestroy(ActorDestroyReason aWhy) {
  // PWAChild::Shutdown();
}

} // namespace pwa
} // namespace mozilla
