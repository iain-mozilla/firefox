/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef PATCHFILE_H
#define PATCHFILE_H

#include "mozilla/UniquePtr.h"
#include "../autofile.h"
#include "action.h"
#include "patchfiledecoder.h"

class PatchFile : public Action {
 public:
  PatchFile(ActionContext* context)
    : mContext(context), mPatchFile(nullptr), mPatchIndex(-1), mBufSize(0) {}

  ~PatchFile() override;

  int Parse(NS_tchar* line) override;
  int Prepare() override;  // should check for patch file and for checksum here
  int Execute() override;
  void Finish(int status) override;

 protected:
  ActionContext* mContext;

 private:
  int LoadSourceFile(FILE* ofile);

  static int sPatchIndex;

  const NS_tchar* mPatchFile;
  mozilla::UniquePtr<NS_tchar[]> mFile;
  mozilla::UniquePtr<NS_tchar[]> mFileRelPath;
  int mPatchIndex;
  mozilla::UniquePtr<PatchFileDecoder> mPatchFileDecoder;
  mozilla::UniquePtr<uint8_t[]> mBuf;
  size_t mBufSize;
  NS_tchar mPatchPath[MAXPATHLEN];
  AutoFile mPatchStream;
};

#endif