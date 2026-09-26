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

#define LOG_TAG "LightsService"

#include "Lights.h"

#include <android-base/logging.h>

namespace aidl {
namespace android {
namespace hardware {
namespace light {

namespace {

constexpr int kRampSize = 8;
constexpr int kRampStepDuration = 50;
constexpr int kBrightnessRamp[kRampSize] = {0, 12, 25, 37, 50, 72, 85, 100};
constexpr uint32_t kDefaultMaxBrightness = 255;

const std::string kLedsDir = "/sys/class/leds/";

uint32_t rgbToBrightness(const HwLightState& state) {
    uint32_t color = state.color & 0x00ffffff;
    return ((77 * ((color >> 16) & 0xff)) + (150 * ((color >> 8) & 0xff)) +
            (29 * (color & 0xff))) >> 8;
}

bool isLit(const HwLightState& state) {
    return state.color & 0x00ffffff;
}

std::string getScaledDutyPcts(int brightness) {
    std::string buf, pad;

    for (auto i : kBrightnessRamp) {
        buf += pad;
        buf += std::to_string(i * brightness / 255);
        pad = ",";
    }

    return buf;
}

HwLight makeLight(LightType type) {
    HwLight light;
    light.id = static_cast<int32_t>(type);
    light.ordinal = 0;
    light.type = type;
    return light;
}

}  // anonymous namespace

LedChannel::LedChannel(const std::string& color)
    : brightness(kLedsDir + color + "/brightness"),
      dutyPcts(kLedsDir + color + "/duty_pcts"),
      startIdx(kLedsDir + color + "/start_idx"),
      pauseLo(kLedsDir + color + "/pause_lo"),
      pauseHi(kLedsDir + color + "/pause_hi"),
      rampStepMs(kLedsDir + color + "/ramp_step_ms"),
      blink(kLedsDir + color + "/blink") {
    ok = brightness && dutyPcts && startIdx && pauseLo && pauseHi && rampStepMs && blink;
    if (!ok) {
        LOG(WARNING) << "LED channel " << color << " is incomplete, not using the RGB LED";
    }
}

Lights::Lights(std::ofstream&& backlight, uint32_t maxBacklight)
    : mLcdBacklight(std::move(backlight)),
      mMaxBacklight(maxBacklight),
      mRgbBlink(kLedsDir + "rgb/rgb_blink") {
    mHasRgb = mRed.ok && mGreen.ok && mBlue.ok && mRgbBlink;

    mLights.push_back(makeLight(LightType::BACKLIGHT));
    if (mHasRgb) {
        mLights.push_back(makeLight(LightType::BATTERY));
        mLights.push_back(makeLight(LightType::NOTIFICATIONS));
        mLights.push_back(makeLight(LightType::ATTENTION));
    }
}

ndk::ScopedAStatus Lights::setLightState(int32_t id, const HwLightState& state) {
    switch (static_cast<LightType>(id)) {
        case LightType::BACKLIGHT:
            setLcdBacklight(state);
            return ndk::ScopedAStatus::ok();
        case LightType::BATTERY:
        case LightType::NOTIFICATIONS:
        case LightType::ATTENTION:
            break;
        default:
            return ndk::ScopedAStatus::fromExceptionCode(EX_UNSUPPORTED_OPERATION);
    }

    if (!mHasRgb) {
        return ndk::ScopedAStatus::fromExceptionCode(EX_UNSUPPORTED_OPERATION);
    }

    std::lock_guard<std::mutex> lock(mLock);
    switch (static_cast<LightType>(id)) {
        case LightType::BATTERY:
            mBatteryState = state;
            break;
        case LightType::NOTIFICATIONS:
            mNotificationState = state;
            break;
        default:
            mAttentionState = state;
            break;
    }
    updateSpeakerLightLocked();

    return ndk::ScopedAStatus::ok();
}

ndk::ScopedAStatus Lights::getLights(std::vector<HwLight>* lights) {
    *lights = mLights;
    return ndk::ScopedAStatus::ok();
}

void Lights::setLcdBacklight(const HwLightState& state) {
    std::lock_guard<std::mutex> lock(mLock);

    uint32_t brightness = rgbToBrightness(state);

    // Scale linearly when the panel range is not the default 0-255.
    if (mMaxBacklight != kDefaultMaxBrightness) {
        brightness = brightness * mMaxBacklight / kDefaultMaxBrightness;
    }

    mLcdBacklight << brightness << std::endl;
}

void Lights::rgbOffLocked() {
    for (LedChannel* c : {&mRed, &mGreen, &mBlue}) {
        c->brightness << 0 << std::endl;
        c->blink << 0 << std::endl;
    }
}

void Lights::updateSpeakerLightLocked() {
    if (isLit(mNotificationState)) {
        setSpeakerLightLocked(mNotificationState);
    } else if (isLit(mAttentionState)) {
        setSpeakerLightLocked(mAttentionState);
    } else if (isLit(mBatteryState)) {
        setSpeakerLightLocked(mBatteryState);
    } else {
        rgbOffLocked();
    }
}

void Lights::setSpeakerLightLocked(const HwLightState& state) {
    // Extract brightness from AARRGGBB and scale the colors with it
    uint32_t alpha = (state.color >> 24) & 0xff;
    int red = (state.color >> 16) & 0xff;
    int green = (state.color >> 8) & 0xff;
    int blue = state.color & 0xff;

    if (alpha != 0xff) {
        red = (red * alpha) / 0xff;
        green = (green * alpha) / 0xff;
        blue = (blue * alpha) / 0xff;
    }

    int onMs = 0, offMs = 0;
    if (state.flashMode == FlashMode::TIMED) {
        onMs = state.flashOnMs;
        offMs = state.flashOffMs;
    }
    bool blink = onMs > 0 && offMs > 0;

    // Disable all blinking to start
    mRgbBlink << 0 << std::endl;

    if (blink) {
        int stepDuration = kRampStepDuration;
        int pauseHi = onMs - (stepDuration * kRampSize * 2);

        if (stepDuration * kRampSize * 2 > onMs) {
            stepDuration = onMs / (kRampSize * 2);
            pauseHi = 0;
        }

        int startIdx = 0;
        for (auto [c, value] : {std::pair<LedChannel*, int>{&mRed, red},
                                std::pair<LedChannel*, int>{&mGreen, green},
                                std::pair<LedChannel*, int>{&mBlue, blue}}) {
            c->startIdx << startIdx << std::endl;
            c->dutyPcts << getScaledDutyPcts(value) << std::endl;
            c->pauseLo << offMs << std::endl;
            c->pauseHi << pauseHi << std::endl;
            c->rampStepMs << stepDuration << std::endl;
            startIdx += kRampSize;
        }

        mRgbBlink << 1 << std::endl;
    } else {
        if (red == 0 && green == 0 && blue == 0) {
            rgbOffLocked();
            return;
        }
        mRed.brightness << red << std::endl;
        mGreen.brightness << green << std::endl;
        mBlue.brightness << blue << std::endl;
    }
}

}  // namespace light
}  // namespace hardware
}  // namespace android
}  // namespace aidl
