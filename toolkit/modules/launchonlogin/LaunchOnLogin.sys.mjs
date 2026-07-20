/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import { AppConstants } from "resource://gre/modules/AppConstants.sys.mjs";

const lazy = {};

if (AppConstants.platform == "win") {
  ChromeUtils.defineESModuleGetters(lazy, {
    LaunchOnLoginImpl:
      "resource://gre/modules/launchonlogin/WindowsLaunchOnLogin.sys.mjs",
  });
} else if (AppConstants.platform == "macosx") {
  ChromeUtils.defineESModuleGetters(lazy, {
    LaunchOnLoginImpl:
      "resource://gre/modules/launchonlogin/MacOSLaunchOnLogin.sys.mjs",
  });
} else {
  lazy.LaunchOnLoginImpl = undefined;
}

export var LaunchOnLogin = {
  isSupported() {
    return lazy.LaunchOnLoginImpl != undefined;
  },

  async createLaunchOnLogin() {
    if (!this.isSupported()) {
      return await Promise.reject(new Error("unsupported"));
    }

    return await lazy.LaunchOnLoginImpl.createLaunchOnLogin();
  },

  async removeLaunchOnLogin() {
    if (!this.isSupported()) {
      return await Promise.reject(new Error("unsupported"));
    }

    return await lazy.LaunchOnLoginImpl.removeLaunchOnLogin();
  },

  async getLaunchOnLoginApproved() {
    if (!this.isSupported()) {
      return await Promise.reject(new Error("unsupported"));
    }

    return await lazy.LaunchOnLoginImpl.getLaunchOnLoginApproved();
  },

  async getLaunchOnLoginEnabled() {
    if (!this.isSupported()) {
      return await Promise.reject(new Error("unsupported"));
    }

    return await lazy.LaunchOnLoginImpl.getLaunchOnLoginEnabled();
  },

  async getLaunchOnLoginEnablementDetails() {
    if (!this.isSupported()) {
      return await Promise.reject(new Error("unsupported"));
    }

    return await lazy.LaunchOnLoginImpl.getLaunchOnLoginEnablementDetails();
  },
};
