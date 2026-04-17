/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include <sys/stat.h>

#include "../actionhelpers.h"
#include "removefile.h"
#include "updatererrors.h"

int RemoveFile::Parse(NS_tchar* line) {
  // format "<deadfile>"

  NS_tchar* validPath = get_valid_path(&line);
  if (!validPath) {
    return PARSE_ERROR;
  }

  mRelPath = mozilla::MakeUnique<NS_tchar[]>(MAXPATHLEN);
  NS_tstrcpy(mRelPath.get(), validPath);

  mFile.reset(get_full_path(validPath, mContext));
  if (!mFile) {
    return PARSE_ERROR;
  }

  return OK;
}

int RemoveFile::Prepare() {
  // Skip the file if it already doesn't exist.
  int rv = NS_taccess(mFile.get(), F_OK);
  if (rv) {
    mSkip = 1;
    mProgressCost = 0;
    return OK;
  }

  LOG(("PREPARE REMOVEFILE " LOG_S, mRelPath.get()));

  // Make sure that we're actually a file...
  struct NS_tstat_t fileInfo;
  rv = NS_tstat(mFile.get(), &fileInfo);
  if (rv) {
    LOG(("failed to read file status info: " LOG_S ", err: %d", mFile.get(),
         errno));
    return READ_ERROR;
  }

  if (!S_ISREG(fileInfo.st_mode)) {
    LOG(("path present, but not a file: " LOG_S, mFile.get()));
    return DELETE_ERROR_EXPECTED_FILE;
  }

  NS_tchar* slash = (NS_tchar*)NS_tstrrchr(mFile.get(), NS_T('/'));
  if (slash) {
    *slash = NS_T('\0');
    rv = NS_taccess(mFile.get(), W_OK);
    *slash = NS_T('/');
  } else {
    rv = NS_taccess(NS_T("."), W_OK);
  }

  if (rv) {
    LOG(("access failed: %d", errno));
    return WRITE_ERROR_FILE_ACCESS_DENIED;
  }

  return OK;
}

int RemoveFile::Execute() {
  if (mSkip) {
    return OK;
  }

  LOG(("EXECUTE REMOVEFILE " LOG_S, mRelPath.get()));

  // The file is checked for existence here and in Prepare since it might have
  // been removed by a separate instruction: bug 311099.
  int rv = NS_taccess(mFile.get(), F_OK);
  if (rv) {
    LOG(("file cannot be removed because it does not exist; skipping"));
    mSkip = 1;
    return OK;
  }

  if (mContext->mStagedUpdate) {
    // Staged updates don't need backup files so just remove it.
    rv = ensure_remove(mFile.get());
    if (rv) {
      return rv;
    }
  } else {
    // Rename the old file. It will be removed in Finish.
    rv = backup_create(mFile.get());
    if (rv) {
      LOG(("backup_create failed: %d", rv));
      return rv;
    }
  }

  return OK;
}

void RemoveFile::Finish(int status) {
  if (mSkip) {
    return;
  }

  LOG(("FINISH REMOVEFILE " LOG_S, mRelPath.get()));

  // Staged updates don't create backup files.
  if (!mContext->mStagedUpdate) {
    const NS_tchar* deleteDirPath = nullptr;
#ifdef XP_WIN
    deleteDirPath = mContext->mDeleteDirPath;
#endif
    backup_finish(mFile.get(), mRelPath.get(), status, deleteDirPath,
                  mContext->stagedOrReplaced());
  }
}
