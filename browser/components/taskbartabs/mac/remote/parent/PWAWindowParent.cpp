/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "PWAWindowParent.h"
#include "PWAParent.h"
#include "mozilla/ipc/Endpoint.h"

namespace mozilla {
namespace pwa {

void PWAWindowParent::ActorDestroy(ActorDestroyReason aWhy) {
    printf("PWAWindowParent::ActorDestroy\n");
}

}
}
