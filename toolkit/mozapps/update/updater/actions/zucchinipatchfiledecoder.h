/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef ZUCCHINIPATCHFILEDECODER_H
#define ZUCCHINIPATCHFILEDECODER_H

#if defined(MOZ_ZUCCHINI)
#  include "mozilla/moz_zucchini.h"

#  include "patchfiledecoder.h"

class ZucchiniPatchFileDecoder : public PatchFileDecoder {
 public:
  ~ZucchiniPatchFileDecoder() override = default;

  unsigned int ComputeCrc32(const uint8_t* aBuf, size_t aBufSize) override;

  off_t SourceSize() override;

  off_t DestinationSize() override;

  unsigned int SourceCrc32() override;

  int Apply(const uint8_t* aCheckedSrcBuf, size_t aCheckedSrcBufSize,
            FILE* aDstFile) override;

 protected:  // Comply with PatchFileDecoder::TryLoadAs requirements
  ZucchiniPatchFileDecoder() = default;
  int Load(FILE* aPatchFile) override;
  friend class PatchFileDecoder;

 private:
  zucchini::mozilla::MappedPatch mMappedPatch;
  uint32_t mSourceSize{};
  uint32_t mDestinationSize{};
  uint32_t mSourceCrc32{};
};

#endif  // MOZ_ZUCCHINI
#endif  // ZUCCHINIPATCHFILEDECODER_H
