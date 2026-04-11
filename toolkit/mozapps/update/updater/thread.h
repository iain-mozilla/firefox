/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* vim:set ts=2 sw=2 sts=2 et cindent: */
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef THREAD_H
#define THREAD_H

#ifdef XP_WIN
#  include <process.h>
#elif defined(XP_UNIX)
#  include <pthread.h>
#else
#  error "Unsupported platform"
#endif

typedef void (*ThreadFunc)(void* param);

class Thread {
  public:
    int Run(ThreadFunc func, void* param);
    int Join();

  private:
#ifdef XP_WIN
  static unsigned __stdcall ThreadMain(void* p);
  HANDLE mThread;
  ThreadFunc mThreadFunc;
  void* mThreadParam;
#elif defined(XP_UNIX)
  pthread_t thr;
#endif
};

#endif /* THREAD_H */