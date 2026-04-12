/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* vim:set ts=2 sw=2 sts=2 et cindent: */
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef REMOVEDIR_H
#define REMOVEDIR_H

#include "mozilla/UniquePtr.h"

#include "action.h"

class RemoveDir : public Action {
 public:
  RemoveDir(ActionContext* context) : mSkip(0), mContext(context) {}

  int Parse(NS_tchar* line) override;
  int Prepare() override;  // check that the source dir exists
  int Execute() override;
  void Finish(int status) override;

 private:
  mozilla::UniquePtr<NS_tchar[]> mDir;
  mozilla::UniquePtr<NS_tchar[]> mRelPath;
  int mSkip;
  ActionContext* mContext;
};

#endif