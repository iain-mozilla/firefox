/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include <sys/stat.h>
#include <fcntl.h>

#include "mozilla/UniquePtr.h"
#include "mozilla/UniquePtrExtensions.h"

#include "actionhelpers.h"
#include "patchfile.h"
#include "updatererrors.h"

#include "bspatchfiledecoder.h"
#include "zucchinipatchfiledecoder.h"

int PatchFile::sPatchIndex = 0;

PatchFile::~PatchFile() {
  // Make sure mPatchStream gets unlocked on Windows; the system will do that,
  // but not until some indeterminate future time, and we want determinism.
  // Normally this happens at the end of Execute, when we close the stream;
  // this call is here in case Execute errors out.
#ifdef XP_WIN
  if (mPatchStream) {
    UnlockFile((HANDLE)_get_osfhandle(fileno(mPatchStream)), 0, 0, -1, -1);
  }
#endif
  // Patch files are written to the <working_dir>/updating directory which is
  // removed after the update has finished so don't delete patch files here.
}

int PatchFile::LoadSourceFile(FILE* ofile) {
  struct stat os;
  int rv = fstat(fileno((FILE*)ofile), &os);
  if (rv) {
    LOG(("LoadSourceFile: unable to stat destination file: " LOG_S ", "
         "err: %d",
         mFileRelPath.get(), errno));
    return READ_ERROR;
  }

  off_t expectedSize = mPatchFileDecoder->SourceSize();
  if (os.st_size != expectedSize) {
    LOG(
        ("LoadSourceFile: destination file size %jd does not match expected "
         "size %jd",
         static_cast<intmax_t>(os.st_size),
         static_cast<intmax_t>(expectedSize)));
    return LOADSOURCE_ERROR_WRONG_SIZE;
  }

  mBufSize = os.st_size;
  mBuf = mozilla::MakeUniqueFallible<uint8_t[]>(mBufSize);
  if (!mBuf) {
    return UPDATER_MEM_ERROR;
  }

  size_t r = mBufSize;
  uint8_t* rb = mBuf.get();
  while (r) {
    const size_t count = mmin(SSIZE_MAX, r);
    size_t c = fread(rb, 1, count, ofile);
    if (c != count) {
      LOG(("LoadSourceFile: error reading destination file: " LOG_S,
           mFileRelPath.get()));
      return READ_ERROR;
    }

    r -= c;
    rb += c;
  }

  // Verify that the contents of the source file correspond to what we expect.

  unsigned int crc = mPatchFileDecoder->ComputeCrc32(mBuf.get(), mBufSize);
  unsigned int expectedCrc = mPatchFileDecoder->SourceCrc32();

  if (crc != expectedCrc) {
    LOG(
        ("LoadSourceFile: destination file crc %u does not match expected "
         "crc %u",
         crc, expectedCrc));
    return CRC_ERROR;
  }

  return OK;
}

int PatchFile::Parse(NS_tchar* line) {
  // format "<patchfile>" "<filetopatch>"

  // Get the path to the patch file inside of the mar
  mPatchFile = mstrtok(kQuote, &line);
  if (!mPatchFile) {
    return PARSE_ERROR;
  }

  // consume whitespace between args
  NS_tchar* q = mstrtok(kQuote, &line);
  if (!q) {
    return PARSE_ERROR;
  }

  NS_tchar* validPath = get_valid_path(&line);
  if (!validPath) {
    return PARSE_ERROR;
  }

  mFileRelPath = mozilla::MakeUnique<NS_tchar[]>(MAXPATHLEN);
  NS_tstrcpy(mFileRelPath.get(), validPath);

  mFile.reset(get_full_path(validPath, mContext));
  if (!mFile) {
    return PARSE_ERROR;
  }

  return OK;
}

int PatchFile::Prepare() {
  LOG(("PREPARE PATCH " LOG_S, mFileRelPath.get()));

  // extract the patch to a temporary file
  mPatchIndex = sPatchIndex++;

  NS_tsnprintf(mPatchPath, sizeof(mPatchPath) / sizeof(mPatchPath[0]),
               NS_T("%s/updating/%d.patch"), mContext->mWorkingDirPath,
               mPatchIndex);

  // The removal of pre-existing patch files here is in case a previous update
  // crashed and left these files behind.
  if (NS_tremove(mPatchPath) && errno != ENOENT) {
    LOG(("failure removing pre-existing patch file: " LOG_S ", err: %d",
         mPatchPath, errno));
    return WRITE_ERROR;
  }

  mPatchStream = NS_tfopen(mPatchPath, NS_T("wb+"));
  if (!mPatchStream) {
    return WRITE_ERROR;
  }

#ifdef XP_WIN
  // Lock the patch file, so it can't be messed with between
  // when we're done creating it and when we go to apply it.
  if (!LockFile((HANDLE)_get_osfhandle(fileno(mPatchStream)), 0, 0, -1, -1)) {
    LOG(("Couldn't lock patch file: %lu", GetLastError()));
    return LOCK_ERROR_PATCH_FILE;
  }

  char sourcefile[MAXPATHLEN];
  if (!WideCharToMultiByte(CP_UTF8, 0, mPatchFile, -1, sourcefile, MAXPATHLEN,
                           nullptr, nullptr)) {
    LOG(("error converting wchar to utf8: %lu", GetLastError()));
    return STRING_CONVERSION_ERROR;
  }

  int rv =
      mContext->mArchiveReader->ExtractFileToStream(sourcefile, mPatchStream);
#else
  int rv =
      mContext->mArchiveReader->ExtractFileToStream(mPatchFile, mPatchStream);
#endif

  return rv;
}

int PatchFile::Execute() {
  LOG(("EXECUTE PATCH " LOG_S, mFileRelPath.get()));

  int rv = UNEXPECTED_BSPATCH_ERROR;

  // zucchini patch files start with "Zucc" bytes, while bspatch patch files
  // start with "MBDIFF10" bytes. Since these bytes are checked, there is no
  // risk of a loader accepting a patch in the wrong format and we can safely
  // iterate over the formats.

#if defined(MOZ_BSPATCH)
  mPatchFileDecoder =
      PatchFileDecoder::TryLoadAs<BSPatchFileDecoder>(mPatchStream, &rv);
#endif  // defined(MOZ_BSPATCH)

#if defined(MOZ_ZUCCHINI)
  if (!mPatchFileDecoder) {
    mPatchFileDecoder = PatchFileDecoder::TryLoadAs<ZucchiniPatchFileDecoder>(
        mPatchStream, &rv);
  }
#endif  // defined(MOZ_ZUCCHINI)

  if (!mPatchFileDecoder) {
    return rv;
  }

  FILE* origfile = nullptr;
#ifdef XP_WIN
  if (NS_tstrcmp(mFileRelPath.get(), gCallbackRelPath) == 0) {
    // Read from the copy of the callback when patching since the callback can't
    // be opened for reading to prevent the application from being launched.
    origfile = NS_tfopen(gCallbackBackupPath, NS_T("rb"));
  } else {
    origfile = NS_tfopen(mFile.get(), NS_T("rb"));
  }
#else
  origfile = NS_tfopen(mFile.get(), NS_T("rb"));
#endif

  if (!origfile) {
    LOG(("unable to open destination file: " LOG_S ", err: %d",
         mFileRelPath.get(), errno));
    return READ_ERROR;
  }

  rv = LoadSourceFile(origfile);
  fclose(origfile);
  if (rv) {
    LOG(("LoadSourceFile failed"));
    return rv;
  }

  // Rename the destination file if it exists before proceeding so it can be
  // used to restore the file to its original state if there is an error.
  struct NS_tstat_t ss;
  rv = NS_tstat(mFile.get(), &ss);
  if (rv) {
    LOG(("failed to read file status info: " LOG_S ", err: %d",
         mFileRelPath.get(), errno));
    return READ_ERROR;
  }

  // Staged updates don't need backup files.
  if (!mContext->mStagedUpdate) {
    rv = backup_create(mFile.get());
    if (rv) {
      return rv;
    }
  }

  off_t dlen = mPatchFileDecoder->DestinationSize();

#if defined(HAVE_POSIX_FALLOCATE)
  AutoFile ofile(ensure_open(mFile.get(), NS_T("wb+"), ss.st_mode));
  posix_fallocate(fileno((FILE*)ofile), 0, dlen);
#elif defined(XP_WIN)
  bool shouldTruncate = true;

  // Creating the file, setting the size, and then closing the file handle
  // lessens fragmentation more than any other method tested. Other methods that
  // have been tested are:
  // 1. _chsize / _chsize_s reduced fragmentation though not completely.
  // 2. _get_osfhandle and then setting the size reduced fragmentation though
  //    not completely. There are also reports of _get_osfhandle failing on
  //    mingw.
  HANDLE hfile = CreateFileW(mFile.get(), GENERIC_WRITE, 0, nullptr,
                             CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);

  if (hfile != INVALID_HANDLE_VALUE) {
    if (SetFilePointer(hfile, dlen, nullptr, FILE_BEGIN) !=
            INVALID_SET_FILE_POINTER &&
        SetEndOfFile(hfile) != 0) {
      shouldTruncate = false;
    }
    CloseHandle(hfile);
  }

  AutoFile ofile(ensure_open(
      mFile.get(), shouldTruncate ? NS_T("wb+") : NS_T("rb+"), ss.st_mode));
#elif defined(XP_MACOSX)
  AutoFile ofile(ensure_open(mFile.get(), NS_T("wb+"), ss.st_mode));
  // Modified code from FileUtils.cpp
  fstore_t store = {F_ALLOCATECONTIG, F_PEOFPOSMODE, 0, dlen};
  // Try to get a continous chunk of disk space
  rv = fcntl(fileno((FILE*)ofile), F_PREALLOCATE, &store);
  if (rv == -1) {
    // OK, perhaps we are too fragmented, allocate non-continuous
    store.fst_flags = F_ALLOCATEALL;
    rv = fcntl(fileno((FILE*)ofile), F_PREALLOCATE, &store);
  }

  if (rv != -1) {
    ftruncate(fileno((FILE*)ofile), dlen);
  }
#else
  AutoFile ofile(ensure_open(mFile.get(), NS_T("wb+"), ss.st_mode));
#endif

  if (ofile == nullptr) {
    LOG(("unable to create new file: " LOG_S ", err: %d", mFileRelPath.get(),
         errno));
    return WRITE_ERROR_OPEN_PATCH_FILE;
  }

#ifdef XP_WIN
  if (!shouldTruncate) {
    fseek(ofile, 0, SEEK_SET);
  }
#endif

  // SAFETY: We have manually checked that the size and crc32 of mBuf match with
  // the patch in PatchFile::LoadSourceFile.
  rv = mPatchFileDecoder->Apply(mBuf.get(), mBufSize, ofile);

  // Go ahead and do a bit of cleanup now to minimize runtime overhead.
  // Make sure mPatchStream gets unlocked on Windows; the system will do that,
  // but not until some indeterminate future time, and we want determinism.
#ifdef XP_WIN
  UnlockFile((HANDLE)_get_osfhandle(fileno(mPatchStream)), 0, 0, -1, -1);
#endif
  // Set mPatchStream to nullptr to make AutoFile close the file,
  // so it can be deleted on Windows.
  mPatchStream = nullptr;
  // Patch files are written to the <working_dir>/updating directory which is
  // removed after the update has finished so don't delete patch files here.
  mPatchPath[0] = NS_T('\0');
  mBuf.reset();
  mBufSize = 0;

  return rv;
}

void PatchFile::Finish(int status) {
  LOG(("FINISH PATCH " LOG_S, mFileRelPath.get()));

  // Staged updates don't create backup files.
  if (!mContext->mStagedUpdate) {
    backup_finish(mFile.get(), mFileRelPath.get(), status);
  }
}
