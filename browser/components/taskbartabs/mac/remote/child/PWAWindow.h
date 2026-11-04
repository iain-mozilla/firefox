/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef PWAWindow_h
#define PWAWindow_h

#import <Cocoa/Cocoa.h>

// FIXME: A wrapper class to get round objc++ in headers included by cpp files
class PWAWindow {
 public:
  PWAWindow();

 private:
  NSWindow* mWindow;
};

#endif