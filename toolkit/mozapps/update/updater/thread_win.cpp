/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* vim:set ts=2 sw=2 sts=2 et cindent: */
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "thread.h"

int Thread::Run(ThreadFunc func, void* param) {
  mThreadFunc = func;
  mThreadParam = param;

  unsigned int threadID;

  mThread = (HANDLE)_beginthreadex(nullptr, 0, ThreadMain, this, 0, &threadID);

  return mThread ? 0 : -1;
}

int Thread::Join() {
  WaitForSingleObject(mThread, INFINITE);
  CloseHandle(mThread);
  return 0;
}

static unsigned __stdcall Thread::ThreadMain(void* p) {
  Thread* self = (Thread*)p;
  self->mThreadFunc(self->mThreadParam);
  return 0;
}
