/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* vim:set ts=2 sw=2 sts=2 et cindent: */
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include <sys/stat.h>

#include "actionhelpers.h"
#include "removedir.h"
#include "updatererrors.h"

int RemoveDir::Parse(NS_tchar* line) {
  // format "<deaddir>/"

  NS_tchar* validPath = get_valid_path(&line, true);
  if (!validPath) {
    return PARSE_ERROR;
  }

  mRelPath = mozilla::MakeUnique<NS_tchar[]>(MAXPATHLEN);
  NS_tstrcpy(mRelPath.get(), validPath);

  mDir.reset(get_full_path(validPath, mContext));
  if (!mDir) {
    return PARSE_ERROR;
  }

  return OK;
}

int RemoveDir::Prepare() {
  // We expect the directory to exist if we are to remove it.
  int rv = NS_taccess(mDir.get(), F_OK);
  if (rv) {
    mSkip = 1;
    mProgressCost = 0;
    return OK;
  }

  LOG(("PREPARE REMOVEDIR " LOG_S "/", mRelPath.get()));

  // Make sure that we're actually a dir.
  struct NS_tstat_t dirInfo;
  rv = NS_tstat(mDir.get(), &dirInfo);
  if (rv) {
    LOG(("failed to read directory status info: " LOG_S ", err: %d",
         mRelPath.get(), errno));
    return READ_ERROR;
  }

  if (!S_ISDIR(dirInfo.st_mode)) {
    LOG(("path present, but not a directory: " LOG_S, mRelPath.get()));
    return DELETE_ERROR_EXPECTED_DIR;
  }

  rv = NS_taccess(mDir.get(), W_OK);
  if (rv) {
    LOG(("access failed: %d, %d", rv, errno));
    return WRITE_ERROR_DIR_ACCESS_DENIED;
  }

  return OK;
}

int RemoveDir::Execute() {
  if (mSkip) {
    return OK;
  }

  LOG(("EXECUTE REMOVEDIR " LOG_S "/", mRelPath.get()));

  // The directory is checked for existence at every step since it might have
  // been removed by a separate instruction: bug 311099.
  int rv = NS_taccess(mDir.get(), F_OK);
  if (rv) {
    LOG(("directory no longer exists; skipping"));
    mSkip = 1;
  }

  return OK;
}

void RemoveDir::Finish(int status) {
  if (mSkip || status != OK) {
    return;
  }

  LOG(("FINISH REMOVEDIR " LOG_S "/", mRelPath.get()));

  // The directory is checked for existence at every step since it might have
  // been removed by a separate instruction: bug 311099.
  int rv = NS_taccess(mDir.get(), F_OK);
  if (rv) {
    LOG(("directory no longer exists; skipping"));
    return;
  }

  if (status == OK) {
    if (NS_trmdir(mDir.get())) {
      LOG(("non-fatal error removing directory: " LOG_S "/, rv: %d, err: %d",
           mRelPath.get(), rv, errno));
    }
  }
}
