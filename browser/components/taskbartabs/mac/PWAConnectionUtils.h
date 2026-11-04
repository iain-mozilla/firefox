/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef PWAConnectionUtils_h
#define PWAConnectionUtils_h

#include <unistd.h>

#include "nsString.h"
#include "base/process.h"

namespace mozilla {
namespace pwa {

struct PWAConnectionUtils final {
static ssize_t WriteUInt32(int socket, uint32_t value);
static ssize_t ReadUInt32(int socket, uint32_t* value);

static ssize_t WriteString(int socket, nsCString value);
static ssize_t ReadString(int socket, nsCString value);

static ssize_t WriteCString(int socket, const char* value);

static ssize_t WriteProcessId(int socket, base::ProcessId value);
static ssize_t ReadProcessId(int socket, base::ProcessId* value);
};
}
}

#endif
