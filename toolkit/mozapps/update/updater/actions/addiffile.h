/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef ADDIFFILE_H
#define ADDIFFILE_H

#include "action.h"
#include "addfile.h"

class AddIfFile : public AddFile {
 public:
  AddIfFile(ActionContext* context) : AddFile(context) {}

  int Parse(NS_tchar* line) override;
  int Prepare() override;
  int Execute() override;
  void Finish(int status) override;

 protected:
  mozilla::UniquePtr<NS_tchar[]> mTestFile;
};

#endif