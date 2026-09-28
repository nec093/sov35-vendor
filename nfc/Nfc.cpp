#define LOG_TAG "android.hardware.nfc@1.2-service.cxd224x"

#include "Nfc.h"

namespace android {
namespace hardware {
namespace nfc {
namespace V1_2 {
namespace implementation {

using V1_1::Constant;
using V1_1::PresenceCheckAlgorithm;

sp<V1_0::INfcClientCallback> Nfc::mCallback = nullptr;
sp<V1_1::INfcClientCallback> Nfc::mCallback_1_1 = nullptr;

Nfc::Nfc(nfc_nci_device_t* device) : mDevice(device) {}

void Nfc::eventCallback(uint8_t event, uint8_t status) {
    if (mCallback_1_1 != nullptr) {
        auto ret = mCallback_1_1->sendEvent_1_1(static_cast<V1_1::NfcEvent>(event),
                                                static_cast<NfcStatus>(status));
        if (!ret.isOk()) ALOGW("sendEvent_1_1 failed");
    } else if (mCallback != nullptr) {
        auto ret = mCallback->sendEvent(static_cast<V1_0::NfcEvent>(event),
                                        static_cast<NfcStatus>(status));
        if (!ret.isOk()) ALOGW("sendEvent failed");
    }
}

void Nfc::dataCallback(uint16_t data_len, uint8_t* p_data) {
    hidl_vec<uint8_t> data;
    data.setToExternal(p_data, data_len);
    if (mCallback_1_1 != nullptr) {
        auto ret = mCallback_1_1->sendData(data);
        if (!ret.isOk()) ALOGW("sendData (1.1) failed");
    } else if (mCallback != nullptr) {
        auto ret = mCallback->sendData(data);
        if (!ret.isOk()) ALOGW("sendData failed");
    }
}

Return<NfcStatus> Nfc::open(const sp<V1_0::INfcClientCallback>& clientCallback) {
    mCallback = clientCallback;
    mCallback_1_1 = nullptr;
    if (mDevice == nullptr || mCallback == nullptr) return NfcStatus::FAILED;
    mCallback->linkToDeath(this, 0);
    return mDevice->open(mDevice, eventCallback, dataCallback) ? NfcStatus::FAILED
                                                               : NfcStatus::OK;
}

Return<NfcStatus> Nfc::open_1_1(const sp<V1_1::INfcClientCallback>& clientCallback) {
    mCallback_1_1 = clientCallback;
    mCallback = nullptr;
    if (mDevice == nullptr || mCallback_1_1 == nullptr) return NfcStatus::FAILED;
    mCallback_1_1->linkToDeath(this, 0);
    return mDevice->open(mDevice, eventCallback, dataCallback) ? NfcStatus::FAILED
                                                               : NfcStatus::OK;
}

Return<uint32_t> Nfc::write(const hidl_vec<uint8_t>& data) {
    if (mDevice == nullptr) return -1;
    return mDevice->write(mDevice, data.size(), &data[0]);
}

Return<NfcStatus> Nfc::coreInitialized(const hidl_vec<uint8_t>& data) {
    hidl_vec<uint8_t> copy = data;
    if (mDevice == nullptr || copy.size() == 0) return NfcStatus::FAILED;
    return mDevice->core_initialized(mDevice, &copy[0]) ? NfcStatus::FAILED : NfcStatus::OK;
}

Return<NfcStatus> Nfc::prediscover() {
    if (mDevice == nullptr) return NfcStatus::FAILED;
    return mDevice->pre_discover(mDevice) ? NfcStatus::FAILED : NfcStatus::OK;
}

Return<NfcStatus> Nfc::close() {
    if (mDevice == nullptr) return NfcStatus::FAILED;
    if (mCallback_1_1 != nullptr) {
        mCallback_1_1->unlinkToDeath(this);
    } else if (mCallback != nullptr) {
        mCallback->unlinkToDeath(this);
    } else {
        return NfcStatus::FAILED;
    }
    return mDevice->close(mDevice) ? NfcStatus::FAILED : NfcStatus::OK;
}

Return<NfcStatus> Nfc::controlGranted() {
    if (mDevice == nullptr) return NfcStatus::FAILED;
    return mDevice->control_granted(mDevice) ? NfcStatus::FAILED : NfcStatus::OK;
}

Return<NfcStatus> Nfc::powerCycle() {
    if (mDevice == nullptr) return NfcStatus::FAILED;
    return mDevice->power_cycle(mDevice) ? NfcStatus::FAILED : NfcStatus::OK;
}

// The legacy module has no factory reset notion.
Return<void> Nfc::factoryReset() {
    return Void();
}

// "If the device doesn't support power off use cases, this call should be
// same as close()."
Return<NfcStatus> Nfc::closeForPowerOffCase() {
    return close();
}

// The chip has no secure element / UICC routing and none of the
// proprietary RF protocols; report the documented "unsupported" values.
void Nfc::fillConfig(V1_1::NfcConfig& config) {
    const uint8_t unsupported = static_cast<uint8_t>(Constant::UNSUPPORTED_CONFIG);

    config.nfaPollBailOutMode = false;
    config.presenceCheckAlgorithm = PresenceCheckAlgorithm::DEFAULT;
    config.nfaProprietaryCfg.protocol18092Active = unsupported;
    config.nfaProprietaryCfg.protocolBPrime = unsupported;
    config.nfaProprietaryCfg.protocolDual = unsupported;
    config.nfaProprietaryCfg.protocol15693 = unsupported;
    config.nfaProprietaryCfg.protocolKovio = unsupported;
    config.nfaProprietaryCfg.protocolMifare = unsupported;
    config.nfaProprietaryCfg.discoveryPollKovio = unsupported;
    config.nfaProprietaryCfg.discoveryPollBPrime = unsupported;
    config.nfaProprietaryCfg.discoveryListenBPrime = unsupported;
    config.defaultOffHostRoute = 0x00;
    config.defaultOffHostRouteFelica = 0x00;
    config.defaultSystemCodeRoute = 0x00;
    config.defaultSystemCodePowerState = 0x00;
    config.defaultRoute = 0x00;
    config.offHostESEPipeId = 0x00;
    config.offHostSIMPipeId = 0x00;
    config.maxIsoDepTransceiveLength = 261;
    config.hostWhitelist = hidl_vec<uint8_t>();
}

Return<void> Nfc::getConfig(getConfig_cb _hidl_cb) {
    V1_1::NfcConfig config = {};
    fillConfig(config);
    _hidl_cb(config);
    return Void();
}

Return<void> Nfc::getConfig_1_2(getConfig_1_2_cb _hidl_cb) {
    V1_2::NfcConfig config = {};
    fillConfig(config.v1_1);
    config.offHostRouteUicc = hidl_vec<uint8_t>();
    config.offHostRouteEse = hidl_vec<uint8_t>();
    config.defaultIsoDepRoute = 0x00;
    _hidl_cb(config);
    return Void();
}

void Nfc::serviceDied(uint64_t /*cookie*/, const wp<::android::hidl::base::V1_0::IBase>& /*who*/) {
    close();
}

}  // namespace implementation
}  // namespace V1_2
}  // namespace nfc
}  // namespace hardware
}  // namespace android
