/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef REMOVEFILE_H
#define REMOVEFILE_H

#include "mozilla/UniquePtr.h"

#include "action.h"

class RemoveFile : public Action {
 public:
  explicit RemoveFile(ActionContext* context) : Action(context), mSkip(0) {}

  int Parse(NS_tchar* line) override;
  int Prepare() override;
  int Execute() override;
  void Finish(int status) override;

 private:
  mozilla::UniquePtr<NS_tchar[]> mFile;
  mozilla::UniquePtr<NS_tchar[]> mRelPath;
  int mSkip;
};

#endif
