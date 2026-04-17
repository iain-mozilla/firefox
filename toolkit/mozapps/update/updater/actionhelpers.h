/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* vim:set ts=2 sw=2 sts=2 et cindent: */
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef ACTIONHELPERS_H
#define ACTIONHELPERS_H

#include <stdio.h>
#include "updatecommon.h"
#include "actioncontext.h"

#ifndef SSIZE_MAX
#  define SSIZE_MAX LONG_MAX
#endif

const NS_tchar kQuote[] = NS_T("\"");
size_t mmin(size_t a, size_t b);

NS_tchar* mstrtok(const NS_tchar* delims, NS_tchar** str);

NS_tchar* get_valid_path(NS_tchar** line, bool isdir = false);
NS_tchar* get_full_path(const NS_tchar* relpath, ActionContext* context);
void ensure_write_permissions(const NS_tchar* path);
int ensure_remove(const NS_tchar* path);
int ensure_parent_dir(const NS_tchar* path);
FILE* ensure_open(const NS_tchar* path, const NS_tchar* flags,
                  unsigned int options);

// FIXME: Different file?
int backup_create(const NS_tchar* path);
void backup_finish(const NS_tchar* path, const NS_tchar* relPath, int status,
                   const NS_tchar* deleteDirPath, bool stagedOrReplaced);
#endif
