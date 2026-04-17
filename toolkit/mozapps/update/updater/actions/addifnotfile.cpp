/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* vim:set ts=2 sw=2 sts=2 et cindent: */
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "../actionhelpers.h"
#include "addifnotfile.h"
#include "updatererrors.h"

int AddIfNotFile::Parse(NS_tchar* line) {
  // format "<testfile>" "<newfile>"

  mTestFile.reset(get_full_path(get_valid_path(&line), mContext));
  if (!mTestFile) {
    return PARSE_ERROR;
  }

  // consume whitespace between args
  NS_tchar* q = mstrtok(kQuote, &line);
  if (!q) {
    return PARSE_ERROR;
  }

  return AddFile::Parse(line);
}

int AddIfNotFile::Prepare() {
  // If the test file exists, then skip this action.
  if (!NS_taccess(mTestFile.get(), F_OK)) {
    mTestFile = nullptr;
    return OK;
  }

  return AddFile::Prepare();
}

int AddIfNotFile::Execute() {
  if (!mTestFile) {
    return OK;
  }

  return AddFile::Execute();
}

void AddIfNotFile::Finish(int status) {
  if (!mTestFile) {
    return;
  }

  AddFile::Finish(status);
}
