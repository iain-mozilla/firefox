/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "actionlist.h"
#include "progressui.h"
#include "updatecommon.h"
#include "updatererrors.h"

ActionList::~ActionList() {
  Action* a = mFirst;
  while (a) {
    Action* b = a;
    a = a->mNext;
    delete b;
  }
}

void ActionList::Append(Action* action) {
  if (mLast) {
    mLast->mNext = action;
  } else {
    mFirst = action;
  }

  mLast = action;
  mCount++;
}

int ActionList::Prepare() {
  // If the action list is empty then we should fail in order to signal that
  // something has gone wrong. Otherwise we report success when nothing is
  // actually done. See bug 327140.
  if (mCount == 0) {
    LOG(("empty action list"));
    return MAR_ERROR_EMPTY_ACTION_LIST;
  }

  Action* a = mFirst;
  int i = 0;
  while (a) {
    int rv = a->Prepare();
    if (rv) {
      return rv;
    }

    float percent = float(++i) / float(mCount);
    UpdateProgressUI(PROGRESS_PREPARE_SIZE * percent);

    a = a->mNext;
  }

  return OK;
}

int ActionList::Execute() {
  int currentProgress = 0, maxProgress = 0;
  Action* a = mFirst;
  while (a) {
    maxProgress += a->mProgressCost;
    a = a->mNext;
  }

  a = mFirst;
  while (a) {
    int rv = a->Execute();
    if (rv) {
      LOG(("### execution failed"));
      return rv;
    }

    currentProgress += a->mProgressCost;
    float percent = float(currentProgress) / float(maxProgress);
    UpdateProgressUI(PROGRESS_PREPARE_SIZE + PROGRESS_EXECUTE_SIZE * percent);

    a = a->mNext;
  }

  return OK;
}

bool ActionList::Finish(int status) {
  Action* a = mFirst;
  int i = 0;
  while (a) {
    a->Finish(status);

    float percent = float(++i) / float(mCount);
    UpdateProgressUI(PROGRESS_PREPARE_SIZE + PROGRESS_EXECUTE_SIZE +
                     PROGRESS_FINISH_SIZE * percent);

    a = a->mNext;
  }

  return (status == OK);
}
