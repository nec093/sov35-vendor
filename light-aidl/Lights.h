/*
 * Copyright (C) 2021 The LineageOS Project
 * Copyright (C) 2026 The XZs 5.4 port
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#pragma once

#include <aidl/android/hardware/light/BnLights.h>

#include <fstream>
#include <mutex>
#include <string>
#include <vector>

namespace aidl {
namespace android {
namespace hardware {
namespace light {

// sysfs controls of one channel of the RGB notification LED (qpnp LPG)
struct LedChannel {
    std::ofstream brightness;
    std::ofstream dutyPcts;
    std::ofstream startIdx;
    std::ofstream pauseLo;
    std::ofstream pauseHi;
    std::ofstream rampStepMs;
    std::ofstream blink;
    bool ok = false;

    explicit LedChannel(const std::string& color);
};

class Lights : public BnLights {
  public:
    Lights(std::ofstream&& backlight, uint32_t maxBacklight);

    ndk::ScopedAStatus setLightState(int32_t id, const HwLightState& state) override;
    ndk::ScopedAStatus getLights(std::vector<HwLight>* lights) override;

  private:
    void setLcdBacklight(const HwLightState& state);
    void updateSpeakerLightLocked();
    void setSpeakerLightLocked(const HwLightState& state);
    void rgbOffLocked();

    std::ofstream mLcdBacklight;
    uint32_t mMaxBacklight;
    LedChannel mRed{"red"};
    LedChannel mGreen{"green"};
    LedChannel mBlue{"blue"};
    std::ofstream mRgbBlink;
    bool mHasRgb;

    HwLightState mAttentionState;
    HwLightState mBatteryState;
    HwLightState mNotificationState;

    std::vector<HwLight> mLights;
    std::mutex mLock;
};

}  // namespace light
}  // namespace hardware
}  // namespace android
}  // namespace aidl
