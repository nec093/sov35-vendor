#
# Copyright (C) 2016 The CyanogenMod Project
# Copyright (C) 2017-2018 The LineageOS Project
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#

# Get non-open-source specific aspects
$(call inherit-product, vendor/sony/keyaki/keyaki-vendor.mk)

# Local overlays
DEVICE_PACKAGE_OVERLAYS += \
    $(LOCAL_PATH)/overlay \
    $(LOCAL_PATH)/overlay-lineage

# Inherit from tone-common
$(call inherit-product, device/sony/tone-common/tone.mk)

# Focus calibration

# Vendor properties
-include $(LOCAL_PATH)/vendor_prop.mk

# Camera Configuration

# Audio
PRODUCT_COPY_FILES += \
    $(LOCAL_PATH)/audio/audio_platform_info.xml:$(TARGET_COPY_OUT_VENDOR)/etc/audio_platform_info.xml \
    $(LOCAL_PATH)/audio/mixer_paths_tasha.xml:$(TARGET_COPY_OUT_VENDOR)/etc/mixer_paths_tasha.xml

# Device init scripts
PRODUCT_PACKAGES += \
    init.target.rc

# Camera Augmented Sensing Helper

# Input
PRODUCT_COPY_FILES += \
    $(LOCAL_PATH)/idc/clearpad.idc:$(TARGET_COPY_OUT_VENDOR)/usr/idc/clearpad.idc

# ro.hardware is keyaki, not qcom: init and fs_mgr look for the keyaki names
PRODUCT_COPY_FILES += \
    $(LOCAL_PATH)/rootdir/vendor/etc/init/hw/init.keyaki.rc:$(TARGET_COPY_OUT_VENDOR)/etc/init/hw/init.keyaki.rc \
    device/sony/tone-common/rootdir/etc/fstab.qcom:$(TARGET_COPY_OUT_VENDOR)/etc/fstab.keyaki

# Wi-Fi firmware for the mainline brcmfmac driver of the 5.4 kernel
# (same BCM4359 firmware and NVRAM the bcmdhd paths carry, under the names brcmfmac asks for)
PRODUCT_COPY_FILES += \
    $(LOCAL_PATH)/prebuilt/firmware/brcm/brcmfmac4359-pcie.bin:$(TARGET_COPY_OUT_VENDOR)/firmware/brcm/brcmfmac4359-pcie.bin \
    $(LOCAL_PATH)/prebuilt/firmware/brcm/brcmfmac4359-pcie.txt:$(TARGET_COPY_OUT_VENDOR)/firmware/brcm/brcmfmac4359-pcie.txt

# NFC

# Thermal Configuration
PRODUCT_COPY_FILES += \
    $(LOCAL_PATH)/rootdir/vendor/etc/thermanager.xml:$(TARGET_COPY_OUT_VENDOR)/etc/thermanager.xml
