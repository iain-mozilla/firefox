/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef PWAPARENT_H
#define PWAPARENT_H

#include "PWAManager.h"
#include "mozilla/WeakPtr.h"
#include "mozilla/dom/MemoryReportRequest.h"
#include "mozilla/dom/PContentParent.h"
#include "mozilla/ipc/CrashReporterHelper.h"
#include "mozilla/pwa/PPWAParent.h"
#include "mozilla/pwa/PWAManager.h"

#include "nsTArray.h"

namespace mozilla {
namespace pwa {

class PWAProcessHost;

class PWAParent final : public PPWAParent,
                        public ipc::CrashReporterHelper<PWAParent>,
                        public SupportsWeakPtr {
 public:
  static constexpr GeckoProcessType PROCESS_TYPE = GeckoProcessType_PWA;

  NS_INLINE_DECL_THREADSAFE_REFCOUNTING(PWAParent, override)

  friend class UtilityProcessHost;
  explicit PWAParent(PWAProcessHost* host);
  bool Init();

  int GetChannelHandle() { return mChannelHandle; }
  const char* GetInitialChannelId() { return mInitialChannelIdString; }

  void Shutdown();

 private:
  friend PPWAParent;

  ~PWAParent();

  mozilla::ipc::IPCResult RecvChildConnected();

  RefPtr<PWAProcessHost> mHost;
  void ActorDestroy(ActorDestroyReason aWhy) override;
  int mChannelHandle;
  char mInitialChannelIdString[NSID_LENGTH];
};

} // namespace pwa
} // namespace mozilla

#endif
