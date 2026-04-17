/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef ACTIONCONTEXT_H
#define ACTIONCONTEXT_H

#include "archivereader.h"
// For NS_tchar
#include "readstrings.h"

struct ActionContext {
  ActionContext(bool stagedUpdate, bool replaceRequest,
                NS_tchar* workingDirPath, NS_tchar* installDirPath,
                ArchiveReader* mArchiveReader)
      : mStagedUpdate(stagedUpdate),
        mWorkingDirPath(workingDirPath),
        mInstallDirPath(installDirPath),
#ifdef XP_WIN
        mDeleteDirPath(nullptr),
        mCallbackRelPath(nullptr),
        mCallbackBackupPath(nullptr),
#endif
        mArchiveReader(mArchiveReader) {
  }

  bool mStagedUpdate;
  bool mReplaceRequest;
  bool stagedOrReplaced() { return mStagedUpdate || mReplaceRequest; }

  const NS_tchar* mWorkingDirPath;
  const NS_tchar* mInstallDirPath;
#ifdef XP_WIN
  const NS_tchar* mDeleteDirPath;
  const NS_tchar* mCallbackRelPath;
  const NS_tchar* mCallbackBackupPath;
#endif
  ArchiveReader* mArchiveReader;
};

#endif
