/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "mozilla/Bootstrap.h"
#import <Foundation/Foundation.h>
#import <Cocoa/Cocoa.h>

using namespace mozilla;

Bootstrap::UniquePtr gBootstrap;

int main(int argc, char * argv[]) {
    NSString* bundlePath = [[[NSBundle mainBundle] bundleURL] path];
    NSString* parentPath = [bundlePath stringByDeletingLastPathComponent];
    NSString* firefoxPath = [parentPath stringByAppendingPathComponent:@"NightlyDebug.app/Contents/MacOS/firefox"];

    const char* firefoxCStr = [firefoxPath cStringUsingEncoding:NSASCIIStringEncoding];

    NSLog(@"Using firefox at %s", firefoxCStr);
    auto bootstrapResult = GetBootstrap(firefoxCStr);
    if (bootstrapResult.isErr()) {
        fprintf(stderr, "Error bootstrapping");
        return -1;
    }

    gBootstrap = bootstrapResult.unwrap();
    gBootstrap->XRE_InitPWAProcess("test-pwa", argc, argv);
}