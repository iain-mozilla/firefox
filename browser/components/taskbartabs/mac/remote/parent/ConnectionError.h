/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef ConnectionError_h
#define ConnectionError_h

#include "mozilla/StaticString.h"

namespace mozilla::pwa {
class ConnectionError {
 public:
  explicit ConnectionError(StaticString aFunction, StaticString aReason)
    : mFunction(aFunction), mReason(aReason) {}

 private:
  StaticString mFunction;
  StaticString mReason;
};

} // namespace mozilla::pwa
#endif