/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* vim:set ts=2 sw=2 sts=2 et cindent: */
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef AUTOFILE_H
#define AUTOFILE_H

#include "updatecommon.h"

// A simple stack based container for a FILE struct that closes the
// file descriptor from its destructor.
class AutoFile {
 public:
  explicit AutoFile(FILE* file = nullptr) : mFile(file) {}

  ~AutoFile() { close(); }

  AutoFile& operator=(FILE* file) {
    close();
    mFile = file;
    return *this;
  }

  operator FILE*() { return mFile; }

  FILE* get() { return mFile; }

 private:
  FILE* mFile;

  void close() {
    if (mFile != nullptr) {
      int rv = fclose(mFile);
      if (rv != 0) {
        LOG(("File close did not execute successfully"));
      }
      mFile = nullptr;
    }
  }
};

#endif
