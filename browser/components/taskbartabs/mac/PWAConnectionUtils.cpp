/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "mozilla/pwa/PWAConnectionUtils.h"

namespace mozilla {
namespace pwa {

// static
ssize_t PWAConnectionUtils::WriteUInt32(int socket, uint32_t value) {
  return write(socket, &value, sizeof(value));
}

// static
ssize_t PWAConnectionUtils::ReadUInt32(int socket, uint32_t* value) {
  return read(socket, value, sizeof(uint32_t));
}

// static
ssize_t PWAConnectionUtils::WriteString(int socket, nsCString value) {
  uint32_t length = strlen(value.get()) + 1;

  if (WriteUInt32(socket, length) == -1) {
    return -1;
  }

  return write(socket, value.BeginReading(), length);
}

// static
ssize_t PWAConnectionUtils::ReadString(int socket, nsCString value) {
  uint32_t length;
  if (read(socket, &length, sizeof(length)) == -1) {
    return -1;
  }

  value.SetLength(length);
  return read(socket, value.BeginWriting(), length);
}

// static
ssize_t PWAConnectionUtils::WriteCString(int socket, const char* value) {
  uint32_t length = strlen(value) + 1;

  if (WriteUInt32(socket, length) == -1) {
    return -1;
  }

  return write(socket, value, length);
}

// static
ssize_t PWAConnectionUtils::WriteProcessId(int socket, base::ProcessId value) {
  return write(socket, &value, sizeof(value));
}

// static
ssize_t PWAConnectionUtils::ReadProcessId(int socket, base::ProcessId* value) {
  return read(socket, value, sizeof(base::ProcessId));
}

} // namespace pwa
} // namespace mozilla
