/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef LAUNCHCHILD_OSX_H
#define LAUNCHCHILD_OSX_H

void CleanupElevatedMacUpdate(bool aFailureOccurred);
bool IsOwnedByGroupAdmin(const char* aAppBundle);
bool IsRecursivelyWritable(const char* aPath);
void LaunchMacApp(int argc, const char** argv);
void LaunchMacPostProcess(const char* aAppBundle);
bool ObtainUpdaterArguments(int* aArgc, char*** aArgv,
                            MARChannelStringTable* aMARStrings);
bool ServeElevatedUpdate(int aArgc, const char** aArgv,
                         const char* aMARChannelID);
void SetGroupOwnershipAndPermissions(const char* aAppBundle);
bool PerformInstallationFromDMG(int argc, char** argv);

#endif  // LAUNCHCHILD_OSX_H
