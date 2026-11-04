/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "PWAWindow.h"

PWAWindow::PWAWindow() {
  mWindow = [[NSWindow alloc] initWithContentRect:NSMakeRect(0, 0, 1024, 500)
                                        styleMask:NSTitledWindowMask
                                          backing:NSBackingStoreBuffered
                                            defer:NO];
  [[mWindow contentView] setWantsLayer:YES];
  [mWindow setTitle:@"PWA"];
  [mWindow makeKeyAndOrderFront:nil];
}