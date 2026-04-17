/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "../actionhelpers.h"
#include "addfile.h"
#include "updatererrors.h"

int AddFile::Parse(NS_tchar* line) {
  // format "<newfile>"

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

int AddFile::Prepare() {
  LOG(("PREPARE ADD " LOG_S, mRelPath.get()));

  return OK;
}

int AddFile::Execute() {
  LOG(("EXECUTE ADD " LOG_S, mRelPath.get()));

  int rv;

  // First make sure that we can actually get rid of any existing file.
  rv = NS_taccess(mFile.get(), F_OK);
  if (rv == 0) {
    if (mContext->mStagedUpdate) {
      // Staged updates don't need backup files so just remove it.
      rv = ensure_remove(mFile.get());
    } else {
      rv = backup_create(mFile.get());
    }
    if (rv) {
      return rv;
    }
  } else {
    rv = ensure_parent_dir(mFile.get());
    if (rv) {
      return rv;
    }
  }

#ifdef XP_WIN
  char sourcefile[MAXPATHLEN];
  if (!WideCharToMultiByte(CP_UTF8, 0, mRelPath.get(), -1, sourcefile,
                           MAXPATHLEN, nullptr, nullptr)) {
    LOG(("error converting wchar to utf8: %lu", GetLastError()));
    return STRING_CONVERSION_ERROR;
  }

  rv = mContext->mArchiveReader->ExtractFile(sourcefile, mFile.get());
#else
  rv = mContext->mArchiveReader->ExtractFile(mRelPath.get(), mFile.get());
#endif
  if (!rv) {
    mAdded = true;
  }
  return rv;
}

void AddFile::Finish(int status) {
  const NS_tchar* deleteDirPath = nullptr;
#ifdef XP_WIN
  deleteDirPath = mContext->mDeleteDirPath;
#endif

  LOG(("FINISH ADD " LOG_S, mRelPath.get()));
  // Staged updates don't create backup files.
  if (!mContext->mStagedUpdate) {
    // When there is an update failure and a file has been added it is removed
    // here since there might not be a backup to replace it.
    if (status && mAdded) {
      if (NS_tremove(mFile.get()) && errno != ENOENT) {
        LOG(("non-fatal error after update failure removing added file: " LOG_S
             ", err: %d",
             mFile.get(), errno));
      }
    }

    backup_finish(mFile.get(), mRelPath.get(), status, deleteDirPath,
                  mContext->stagedOrReplaced());
  }
}
