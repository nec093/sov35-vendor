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
#include <android/binder_manager.h>
#include <android/binder_process.h>

#include <cerrno>
#include <cstring>

using aidl::android::hardware::light::Lights;

static const std::string kLcdBacklightPath = "/sys/class/leds/lcd-backlight/brightness";
static const std::string kLcdMaxBacklightPath = "/sys/class/leds/lcd-backlight/max_brightness";

int main() {
    ABinderProcess_setThreadPoolMaxThreadCount(0);

    std::ofstream lcdBacklight(kLcdBacklightPath);
    if (!lcdBacklight) {
        LOG(ERROR) << "Failed to open " << kLcdBacklightPath << ": " << strerror(errno);
        return EXIT_FAILURE;
    }

    uint32_t lcdMaxBrightness = 255;
    std::ifstream lcdMaxBacklight(kLcdMaxBacklightPath);
    if (lcdMaxBacklight) {
        lcdMaxBacklight >> lcdMaxBrightness;
    } else {
        LOG(WARNING) << "Failed to read " << kLcdMaxBacklightPath << ", assuming 255";
    }

    std::shared_ptr<Lights> lights =
            ndk::SharedRefBase::make<Lights>(std::move(lcdBacklight), lcdMaxBrightness);

    const std::string instance = std::string(Lights::descriptor) + "/default";
    binder_status_t status = AServiceManager_addService(lights->asBinder().get(), instance.c_str());
    CHECK(status == STATUS_OK) << "Failed to register " << instance;

    ABinderProcess_joinThreadPool();
    return EXIT_FAILURE;  // should not reach
}
