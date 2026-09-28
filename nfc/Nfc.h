/*
 * android.hardware.nfc@1.2 service for the Sony CXD224X (SOV35).
 *
 * Bridges the framework to Sony's legacy nfc_nci_device_t HAL module
 * (nfc_nci.cxd224x.msm8996.so), which predates HIDL and knows nothing of
 * the 1.1/1.2 additions.
 */
#pragma once

#include <android/hardware/nfc/1.2/INfc.h>
#include <android/hardware/nfc/1.2/types.h>
#include <hardware/hardware.h>
#include <hardware/nfc.h>
#include <hidl/Status.h>
#include <log/log.h>

namespace android {
namespace hardware {
namespace nfc {
namespace V1_2 {
namespace implementation {

using ::android::sp;
using ::android::wp;
using ::android::hardware::hidl_death_recipient;
using ::android::hardware::hidl_vec;
using ::android::hardware::Return;
using ::android::hardware::Void;
using ::android::hardware::nfc::V1_0::NfcStatus;

struct Nfc : public V1_2::INfc, public hidl_death_recipient {
    explicit Nfc(nfc_nci_device_t* device);

    // V1_0::INfc
    Return<NfcStatus> open(const sp<V1_0::INfcClientCallback>& clientCallback) override;
    Return<uint32_t> write(const hidl_vec<uint8_t>& data) override;
    Return<NfcStatus> coreInitialized(const hidl_vec<uint8_t>& data) override;
    Return<NfcStatus> prediscover() override;
    Return<NfcStatus> close() override;
    Return<NfcStatus> controlGranted() override;
    Return<NfcStatus> powerCycle() override;

    // V1_1::INfc
    Return<void> factoryReset() override;
    Return<NfcStatus> closeForPowerOffCase() override;
    Return<NfcStatus> open_1_1(const sp<V1_1::INfcClientCallback>& clientCallback) override;
    Return<void> getConfig(getConfig_cb _hidl_cb) override;

    // V1_2::INfc
    Return<void> getConfig_1_2(getConfig_1_2_cb _hidl_cb) override;

    void serviceDied(uint64_t cookie, const wp<::android::hidl::base::V1_0::IBase>& who) override;

  private:
    static void eventCallback(uint8_t event, uint8_t status);
    static void dataCallback(uint16_t data_len, uint8_t* p_data);
    static void fillConfig(V1_1::NfcConfig& config);

    static sp<V1_0::INfcClientCallback> mCallback;
    static sp<V1_1::INfcClientCallback> mCallback_1_1;
    nfc_nci_device_t* mDevice;
};

}  // namespace implementation
}  // namespace V1_2
}  // namespace nfc
}  // namespace hardware
}  // namespace android
