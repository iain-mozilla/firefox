/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include <sys/stat.h>

#include "actionhelpers.h"
#include "action.h"
#include "updatererrors.h"

size_t mmin(size_t a, size_t b) { return (a > b) ? b : a; }

NS_tchar* mstrtok(const NS_tchar* delims, NS_tchar** str) {
  if (!*str || !**str) {
    *str = nullptr;
    return nullptr;
  }

  // skip leading "whitespace"
  NS_tchar* ret = *str;
  const NS_tchar* d;
  do {
    for (d = delims; *d != NS_T('\0'); ++d) {
      if (*ret == *d) {
        ++ret;
        break;
      }
    }
  } while (*d);

  if (!*ret) {
    *str = ret;
    return nullptr;
  }

  NS_tchar* i = ret;
  do {
    for (d = delims; *d != NS_T('\0'); ++d) {
      if (*i == *d) {
        *i = NS_T('\0');
        *str = ++i;
        return ret;
      }
    }
    ++i;
  } while (*i);

  *str = nullptr;
  return ret;
}

/**
 * Gets the platform specific path and performs simple checks to the path. If
 * the path checks don't pass nullptr will be returned.
 *
 * @param  line
 *         The line from the manifest that contains the path.
 * @param  isdir
 *         Whether the path is a directory path. Defaults to false.
 * @return valid filesystem path or nullptr if the path checks fail.
 */
NS_tchar* get_valid_path(NS_tchar** line, bool isdir) {
  NS_tchar* path = mstrtok(kQuote, line);
  if (!path) {
    LOG(("get_valid_path: unable to determine path: " LOG_S, *line));
    return nullptr;
  }

  // All paths must be relative from the current working directory
  if (path[0] == NS_T('/')) {
    LOG(("get_valid_path: path must be relative: " LOG_S, path));
    return nullptr;
  }

#ifdef XP_WIN
  // All paths must be relative from the current working directory
  if (path[0] == NS_T('\\') || path[1] == NS_T(':')) {
    LOG(("get_valid_path: path must be relative: " LOG_S, path));
    return nullptr;
  }
#endif

  if (isdir) {
    // Directory paths must have a trailing forward slash.
    if (path[NS_tstrlen(path) - 1] != NS_T('/')) {
      LOG(
          ("get_valid_path: directory paths must have a trailing forward "
           "slash: " LOG_S,
           path));
      return nullptr;
    }

    // Remove the trailing forward slash because stat on Windows will return
    // ENOENT if the path has a trailing slash.
    path[NS_tstrlen(path) - 1] = NS_T('\0');
  }

  // Don't allow relative paths that resolve to a parent directory.
  if (NS_tstrstr(path, NS_T("..")) != nullptr) {
    LOG(("get_valid_path: paths must not contain '..': " LOG_S, path));
    return nullptr;
  }

  return path;
}

/**
 * Converts a relative update path to a full path.
 *
 * @param  relpath
 *         The relative path to convert to a full path.
 * @return valid filesystem full path or nullptr if memory allocation fails.
 */
NS_tchar* get_full_path(const NS_tchar* relpath, ActionContext* context) {
  const NS_tchar* destpath = context->mStagedUpdate ? context->mWorkingDirPath
                                                    : context->mInstallDirPath;
  size_t lendestpath = NS_tstrlen(destpath);
  size_t lenrelpath = NS_tstrlen(relpath);
  NS_tchar* s = new NS_tchar[lendestpath + lenrelpath + 2];

  NS_tchar* c = s;

  NS_tstrcpy(c, destpath);
  c += lendestpath;
  NS_tstrcat(c, NS_T("/"));
  c++;

  NS_tstrcat(c, relpath);
  c += lenrelpath;
  *c = NS_T('\0');
  return s;
}

void ensure_write_permissions(const NS_tchar* path) {
#ifdef XP_WIN
  (void)_wchmod(path, _S_IREAD | _S_IWRITE);
#else
  struct stat fs;
  if (!stat(path, &fs) && !(fs.st_mode & S_IWUSR)) {
    (void)chmod(path, fs.st_mode | S_IWUSR);
  }
#endif
}

int ensure_remove(const NS_tchar* path) {
  ensure_write_permissions(path);
  int rv = NS_tremove(path);
  if (rv) {
    LOG(("ensure_remove: failed to remove file: " LOG_S ", rv: %d, err: %d",
         path, rv, errno));
  }
  return rv;
}

// Ensure that the directory containing this file exists.
int ensure_parent_dir(const NS_tchar* path) {
  int rv = OK;

  NS_tchar* slash = (NS_tchar*)NS_tstrrchr(path, NS_T('/'));
  if (slash) {
    *slash = NS_T('\0');
    rv = ensure_parent_dir(path);
    // Only attempt to create the directory if we're not at the root
    if (rv == OK && *path) {
      rv = NS_tmkdir(path, 0755);
      // If the directory already exists, then ignore the error.
      if (rv < 0 && errno != EEXIST) {
        LOG(("ensure_parent_dir: failed to create directory: " LOG_S ", "
             "err: %d",
             path, errno));
        rv = WRITE_ERROR;
      } else {
        rv = OK;
      }
    }
    *slash = NS_T('/');
  }
  return rv;
}

// Renames the specified file to the new file specified. If the destination file
// exists it is removed.
static int rename_file(const NS_tchar* spath, const NS_tchar* dpath,
                       bool allowDirs = false) {
  int rv = ensure_parent_dir(dpath);
  if (rv) {
    return rv;
  }

  struct NS_tstat_t spathInfo;
  rv = NS_tstat(spath, &spathInfo);
  if (rv) {
    LOG(("rename_file: failed to read file status info: " LOG_S ", "
         "err: %d",
         spath, errno));
    return READ_ERROR;
  }

  if (!S_ISREG(spathInfo.st_mode)) {
    if (allowDirs && !S_ISDIR(spathInfo.st_mode)) {
      LOG(("rename_file: path present, but not a file: " LOG_S ", err: %d",
           spath, errno));
      return RENAME_ERROR_EXPECTED_FILE;
    }
    LOG(("rename_file: proceeding to rename the directory"));
  }

  if (!NS_taccess(dpath, F_OK)) {
    if (ensure_remove(dpath)) {
      LOG(
          ("rename_file: destination file exists and could not be "
           "removed: " LOG_S,
           dpath));
      return WRITE_ERROR_DELETE_FILE;
    }
  }

  if (NS_trename(spath, dpath) != 0) {
    LOG(("rename_file: failed to rename file - src: " LOG_S ", "
         "dst:" LOG_S ", err: %d",
         spath, dpath, errno));
    return WRITE_ERROR;
  }

  return OK;
}

static bool is_read_only(const NS_tchar* flags) {
  size_t length = NS_tstrlen(flags);
  if (length == 0) {
    return false;
  }

  // Make sure the string begins with "r"
  if (flags[0] != NS_T('r')) {
    return false;
  }

  // Look for "r+" or "r+b"
  if (length > 1 && flags[1] == NS_T('+')) {
    return false;
  }

  // Look for "rb+"
  if (NS_tstrcmp(flags, NS_T("rb+")) == 0) {
    return false;
  }

  return true;
}

FILE* ensure_open(const NS_tchar* path, const NS_tchar* flags,
                  unsigned int options) {
  ensure_write_permissions(path);
  FILE* f = NS_tfopen(path, flags);
  if (is_read_only(flags)) {
    // Don't attempt to modify the file permissions if the file is being opened
    // in read-only mode.
    return f;
  }
  if (NS_tchmod(path, options) != 0) {
    if (f != nullptr) {
      fclose(f);
    }
    return nullptr;
  }
  struct NS_tstat_t ss;
  if (NS_tstat(path, &ss) != 0 || ss.st_mode != options) {
    if (f != nullptr) {
      fclose(f);
    }
    return nullptr;
  }
  return f;
}

// Create a backup of the specified file by renaming it.
int backup_create(const NS_tchar* path) {
  NS_tchar backup[MAXPATHLEN];
  NS_tsnprintf(backup, sizeof(backup) / sizeof(backup[0]),
               NS_T("%s") BACKUP_EXT, path);

  return rename_file(path, backup);
}

// Rename the backup of the specified file that was created by renaming it back
// to the original file.
int backup_restore(const NS_tchar* path, const NS_tchar* relPath) {
  NS_tchar backup[MAXPATHLEN];
  NS_tsnprintf(backup, sizeof(backup) / sizeof(backup[0]),
               NS_T("%s") BACKUP_EXT, path);

  NS_tchar relBackup[MAXPATHLEN];
  NS_tsnprintf(relBackup, sizeof(relBackup) / sizeof(relBackup[0]),
               NS_T("%s") BACKUP_EXT, relPath);

  if (NS_taccess(backup, F_OK)) {
    LOG(("backup_restore: backup file doesn't exist: " LOG_S, relBackup));
    return OK;
  }

  return rename_file(backup, path);
}

// Discard the backup of the specified file that was created by renaming it.
int backup_discard(const NS_tchar* path, const NS_tchar* relPath) {
  NS_tchar backup[MAXPATHLEN];
  NS_tsnprintf(backup, sizeof(backup) / sizeof(backup[0]),
               NS_T("%s") BACKUP_EXT, path);

  NS_tchar relBackup[MAXPATHLEN];
  NS_tsnprintf(relBackup, sizeof(relBackup) / sizeof(relBackup[0]),
               NS_T("%s") BACKUP_EXT, relPath);

  // Nothing to discard
  if (NS_taccess(backup, F_OK)) {
    return OK;
  }

  int rv = ensure_remove(backup);
#if defined(XP_WIN)
  if (rv && !sStagedUpdate && !sReplaceRequest) {
    LOG(("backup_discard: unable to remove: " LOG_S, relBackup));
    NS_tchar path[MAXPATHLEN + 1];
    GetUUIDTempFilePath(gDeleteDirPath, L"moz", path);
    if (rename_file(backup, path)) {
      LOG(("backup_discard: failed to rename file:" LOG_S ", dst:" LOG_S,
           relBackup, relPath));
      return WRITE_ERROR_DELETE_BACKUP;
    }
    // The MoveFileEx call to remove the file on OS reboot will fail if the
    // process doesn't have write access to the HKEY_LOCAL_MACHINE registry key
    // but this is ok since the installer / uninstaller will delete the
    // directory containing the file along with its contents after an update is
    // applied, on reinstall, and on uninstall.
    if (MoveFileEx(path, nullptr, MOVEFILE_DELAY_UNTIL_REBOOT)) {
      LOG(
          ("backup_discard: file renamed and will be removed on OS "
           "reboot: " LOG_S,
           relPath));
    } else {
      LOG(
          ("backup_discard: failed to schedule OS reboot removal of "
           "file: " LOG_S,
           relPath));
    }
  }
#else
  if (rv) {
    return WRITE_ERROR_DELETE_BACKUP;
  }
#endif

  return OK;
}

// Helper function for post-processing a temporary backup.
void backup_finish(const NS_tchar* path, const NS_tchar* relPath, int status) {
  if (status == OK) {
    backup_discard(path, relPath);
  } else {
    backup_restore(path, relPath);
  }
}
