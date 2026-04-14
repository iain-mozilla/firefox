/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef PATCHFILEDECODER_H
#define PATCHFILEDECODER_H

#include <stdio.h>

#include "mozilla/UniquePtr.h"
#include "updatererrors.h"

class PatchFileDecoder {
 public:
  // This method is the only valid way to create a PatchFileDecoder object,
  // which ensures that any such object has successfully called the Load()
  // method. Child classes must hide their constructors and declare the parent
  // class as a friend.
  template <typename ChildClass>
  static mozilla::UniquePtr<PatchFileDecoder> TryLoadAs(FILE* aPatchFile,
                                                        int* aReturnValue) {
    // We cannot use MakeUnique because child classes hide their constructors.
    mozilla::UniquePtr<ChildClass> ptr{new ChildClass()};
    fseek(aPatchFile, 0, SEEK_SET);
    int rv = ptr->Load(aPatchFile);
    if (rv != OK) {
      ptr.reset();
    }
    if (aReturnValue) {
      *aReturnValue = rv;
    }
    return ptr;
  }

  virtual ~PatchFileDecoder() = default;

  virtual unsigned int ComputeCrc32(const uint8_t* aBuf, size_t aBufSize) = 0;

  virtual off_t SourceSize() = 0;
  virtual off_t DestinationSize() = 0;
  virtual unsigned int SourceCrc32() = 0;

  // Applies the loaded patch to aCheckedSrcBuf, and writes the result to
  // aDstFile. aDstFile is never deleted, cleanup is up to the caller.
  // Assumes that the crc32 and size of aCheckedSrcBuf have been
  // checked by the caller.
  virtual int Apply(const uint8_t* aCheckedSrcBuf, size_t aCheckedSrcBufSize,
                    FILE* aDstFile) = 0;

 protected:
  virtual int Load(FILE* aPatchFile) = 0;
};

#endif
