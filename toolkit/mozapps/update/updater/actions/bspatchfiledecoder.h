/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef BSPATCHFILEDECODER_H
#define BSPATCHFILEDECODER_H

#if defined(MOZ_BSPATCH)

#  include "bspatch.h"

#  include "patchfiledecoder.h"
class BSPatchFileDecoder : public PatchFileDecoder {
 public:
  ~BSPatchFileDecoder() override = default;

  unsigned int ComputeCrc32(const uint8_t* aBuf, size_t aBufSize) override;

  off_t SourceSize() override;

  off_t DestinationSize() override;

  unsigned int SourceCrc32() override;

  int Apply(const uint8_t* aSrcBuf, size_t aSrcBufSize,
            FILE* aDstFile) override;

 protected:  // Comply with PatchFileDecoder::TryLoadAs requirements
  BSPatchFileDecoder() = default;
  int Load(FILE* aPatchFile) override;
  friend class PatchFileDecoder;

 private:
  FILE* mPatchFile{};
  MBSPatchHeader mHeader{};
};

#endif /* MOZ_BSPATCH */
#endif
