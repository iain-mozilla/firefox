/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef PATCHIFFILE_H
#define PATCHIFFILE_H

#include "mozilla/UniquePtr.h"
#include "patchfile.h"

class PatchIfFile : public PatchFile {
 public:
  PatchIfFile(ActionContext* context) : PatchFile(context) {}
  
  int Parse(NS_tchar* line) override;
  int Prepare() override;  // should check for patch file and for checksum here
  int Execute() override;
  void Finish(int status) override;

 private:
  mozilla::UniquePtr<NS_tchar[]> mTestFile;
};

#endif