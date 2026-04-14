/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "../crctable.h"
#include "bspatchfiledecoder.h"

#if defined(MOZ_BSPATCH)

// This BZ2_crc32Table variable lives in libbz2. We just took the
// data structure from bz2 and created crctables.h
unsigned int BSPatchFileDecoder::ComputeCrc32(const uint8_t* aBuf,
                                              size_t aBufSize) {
  unsigned int crc = 0xffffffffL;

  const uint8_t* end = aBuf + aBufSize;
  for (; aBuf != end; ++aBuf)
    crc = (crc << 8) ^ BZ2_crc32Table[(crc >> 24) ^ *aBuf];

  crc = ~crc;
  return crc;
}

int BSPatchFileDecoder::Load(FILE* aPatchFile) {
  mPatchFile = aPatchFile;
  return MBS_ReadHeader(aPatchFile, &mHeader);
}

off_t BSPatchFileDecoder::SourceSize() {
  return static_cast<off_t>(mHeader.slen);
}

off_t BSPatchFileDecoder::DestinationSize() {
  return static_cast<off_t>(mHeader.dlen);
}

unsigned int BSPatchFileDecoder::SourceCrc32() { return mHeader.scrc32; }

int BSPatchFileDecoder::Apply(const uint8_t* aCheckedSrcBuf,
                              size_t aCheckedSrcBufSize, FILE* aDstFile) {
  return MBS_ApplyPatch(&mHeader, mPatchFile, aCheckedSrcBuf, aDstFile);
}
#endif  // defined(MOZ_BSPATCH)
