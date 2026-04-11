/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* vim:set ts=2 sw=2 sts=2 et cindent: */
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef UMASKCONTEXT_OSX_H
#define UMASKCONTEXT_OSX_H

#include <sys/stat.h>

// Just a simple class that sets a umask value in its constructor and resets
// it in its destructor.
class UmaskContext {
 public:
  explicit UmaskContext(mode_t umaskToSet) {
    mPreviousUmask = umask(umaskToSet);
  }
  ~UmaskContext() { umask(mPreviousUmask); }

 private:
  mode_t mPreviousUmask;
};

#endif