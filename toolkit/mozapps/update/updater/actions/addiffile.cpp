/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include <sys/stat.h>

#include "actionhelpers.h"
#include "addiffile.h"
#include "updatererrors.h"


int AddIfFile::Parse(NS_tchar* line) {
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

int AddIfFile::Prepare() {
  // If the test file does not exist, then skip this action.
  if (NS_taccess(mTestFile.get(), F_OK)) {
    mTestFile = nullptr;
    return OK;
  }

  return AddFile::Prepare();
}

int AddIfFile::Execute() {
  if (!mTestFile) {
    return OK;
  }

  return AddFile::Execute();
}

void AddIfFile::Finish(int status) {
  if (!mTestFile) {
    return;
  }

  AddFile::Finish(status);
}
