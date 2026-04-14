/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef ADDIFNOTFILE_H
#define ADDIFNOTFILE_H

#include "action.h"
#include "addfile.h"

class AddIfNotFile : public AddFile {
 public:
  using AddFile::AddFile;

  int Parse(NS_tchar* line) override;
  int Prepare() override;
  int Execute() override;
  void Finish(int status) override;

 protected:
  mozilla::UniquePtr<NS_tchar[]> mTestFile;
};


#endif
