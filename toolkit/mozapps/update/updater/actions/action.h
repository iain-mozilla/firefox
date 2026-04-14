/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef ACTION_H
#define ACTION_H

#include "../archivereader.h"
// For NS_tchar
#include "readstrings.h"

struct ActionContext {
  ActionContext(bool stagedUpdate, NS_tchar* workingDirPath,
                NS_tchar* installDirPath, ArchiveReader* mArchiveReader)
      : mStagedUpdate(stagedUpdate),
        mWorkingDirPath(workingDirPath),
        mInstallDirPath(installDirPath),
        mArchiveReader(mArchiveReader) {}

  bool mStagedUpdate;
  const NS_tchar* mWorkingDirPath;
  const NS_tchar* mInstallDirPath;
  ArchiveReader* mArchiveReader;
};

class Action {
 public:
  Action(ActionContext* context)
      : mProgressCost(1), mContext(context), mNext(nullptr) {}
  virtual ~Action() = default;

  virtual int Parse(NS_tchar* line) = 0;

  // Do any preprocessing to ensure that the action can be performed.  Execute
  // will be called if this Action and all others return OK from this method.
  virtual int Prepare() = 0;

  // Perform the operation.  Return OK to indicate success.  After all actions
  // have been executed, Finish will be called.  A requirement of Execute is
  // that its operation be reversable from Finish.
  virtual int Execute() = 0;

  // Finish is called after execution of all actions.  If status is OK, then
  // all actions were successfully executed.  Otherwise, some action failed.
  virtual void Finish(int status) = 0;

  int mProgressCost;

 protected:
  ActionContext* mContext;

 private:
  Action* mNext;

  friend class ActionList;
};

#endif
