/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#if defined(MOZ_ZUCCHINI)

#include "zucchinipatchfiledecoder.h"
#include "updatecommon.h"

// Best-effort conversion from Zucchini status codes to updater status codes
int FromZucchiniStatus(zucchini::status::Code code) {
  int result = OK;
  switch (code) {
    case zucchini::status::kStatusSuccess:
      break;
    case zucchini::status::kStatusFileReadError:
    case zucchini::status::kStatusPatchReadError:
      result = READ_ERROR;
      break;
    case zucchini::status::kStatusFileWriteError:
      result = WRITE_ERROR;
      break;
    case zucchini::status::kStatusPatchWriteError:
      result = WRITE_ERROR_PATCH_FILE;
      break;
    case zucchini::status::kStatusInvalidOldImage:
    case zucchini::status::kStatusInvalidNewImage:
      result = CRC_ERROR;
      break;
    case zucchini::status::kStatusInvalidParam:
    case zucchini::status::kStatusDiskFull:
    case zucchini::status::kStatusIoError:
    case zucchini::status::kStatusFatal:
    default:
      result = UNEXPECTED_BSPATCH_ERROR;
      break;
  }
  if (result != OK) {
    LOG(
        ("FromZucchiniStatus: encountered zucchini error %d, converting to "
         "updater error %d",
         code, result));
  }
  return result;
}


unsigned int ZucchiniPatchFileDecoder::ComputeCrc32(const uint8_t* aBuf,
                                                    size_t aBufSize) {
  return zucchini::mozilla::ComputeCrc32(aBuf, aBufSize);
}

int ZucchiniPatchFileDecoder::Load(FILE* aPatchFile) {
  return FromZucchiniStatus(mMappedPatch.Load(
      aPatchFile, &mSourceSize, &mDestinationSize, &mSourceCrc32));
}

off_t ZucchiniPatchFileDecoder::SourceSize() {
  return static_cast<off_t>(mSourceSize);
}

off_t ZucchiniPatchFileDecoder::DestinationSize() {
  return static_cast<off_t>(mDestinationSize);
}

unsigned int ZucchiniPatchFileDecoder::SourceCrc32() {
  return static_cast<int>(mSourceCrc32);
}

int ZucchiniPatchFileDecoder::Apply(const uint8_t* aCheckedSrcBuf,
                                    size_t aCheckedSrcBufSize, FILE* aDstFile) {
  // SAFETY: The caller has already checked that the crc32 and size of
  // aCheckedSrcBuf match with the contents of the patch file as a requirement
  // of PatchFileDecoder::Apply.
  return FromZucchiniStatus(
      mMappedPatch.ApplyUnsafe(aCheckedSrcBuf, aCheckedSrcBufSize, aDstFile));
}
#endif  // defined(MOZ_ZUCCHINI)
