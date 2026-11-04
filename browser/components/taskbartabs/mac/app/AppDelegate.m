/* vim: se cin sw=2 ts=2 et filetype=javascript :
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

 #import "AppDelegate.h"
#include <CoreFoundation/CFStream.h>

 @interface AppDelegate () {
    CFReadStreamRef readStream;
    CFWriteStreamRef writeStream;
 }

 @end

 @implementation AppDelegate

 - (void)applicationDidFinishLaunching:(NSNotification *)notification {

 }

 - (void)stream:(NSStream *)aStream handleEvent:(NSStreamEvent)eventCode {
 }

 - (void)applicationWillTerminate:(NSNotification *)notification {
 }

 @end