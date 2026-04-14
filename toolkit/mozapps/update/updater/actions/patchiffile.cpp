/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "actionhelpers.h"
#include "patchiffile.h"
#include "updatererrors.h"

int PatchIfFile::Parse(NS_tchar* line) {
  // format "<testfile>" "<patchfile>" "<filetopatch>"

  mTestFile.reset(get_full_path(get_valid_path(&line), mContext));
  if (!mTestFile) {
    return PARSE_ERROR;
  }

  // consume whitespace between args
  NS_tchar* q = mstrtok(kQuote, &line);
  if (!q) {
    return PARSE_ERROR;
  }

  return PatchFile::Parse(line);
}

int PatchIfFile::Prepare() {
  // If the test file does not exist, then skip this action.
  if (NS_taccess(mTestFile.get(), F_OK)) {
    mTestFile = nullptr;
    return OK;
  }

  return PatchFile::Prepare();
}

int PatchIfFile::Execute() {
  if (!mTestFile) {
    return OK;
  }

  return PatchFile::Execute();
}

void PatchIfFile::Finish(int status) {
  if (!mTestFile) {
    return;
  }

  PatchFile::Finish(status);
}
