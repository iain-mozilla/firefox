/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef ACTIONLIST_H
#define ACTIONLIST_H

#include "actions/action.h"

class ActionList {
 public:
  ActionList() : mFirst(nullptr), mLast(nullptr), mCount(0) {}
  ~ActionList();

  void Append(Action* action);
  int Prepare();
  int Execute();
  bool Finish(int status);

 private:
  Action* mFirst;
  Action* mLast;
  int mCount;
};

#endif
