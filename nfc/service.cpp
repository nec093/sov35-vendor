#define LOG_TAG "android.hardware.nfc@1.2-service.cxd224x"

#include <hidl/HidlTransportSupport.h>
#include <log/log.h>

#include "Nfc.h"

using android::sp;
using android::hardware::configureRpcThreadpool;
using android::hardware::joinRpcThreadpool;
using android::hardware::nfc::V1_2::implementation::Nfc;

int main() {
    const hw_module_t* module = nullptr;
    nfc_nci_device_t* device = nullptr;

    // Sony's module identifies itself as "nfc_nci.cxd224x" and libhardware
    // checks the id against the name it was asked for, so ask for exactly
    // that; the file, nfc_nci.cxd224x.msm8996.so, is found through
    // ro.board.platform.
    int ret = hw_get_module(NFC_NCI_HARDWARE_MODULE_ID ".cxd224x", &module);
    if (ret) {
        ALOGE("hw_get_module(%s.cxd224x) failed: %d", NFC_NCI_HARDWARE_MODULE_ID, ret);
        return 1;
    }
    ret = nfc_nci_open(module, &device);
    if (ret) {
        ALOGE("nfc_nci_open failed: %d", ret);
        return 1;
    }

    configureRpcThreadpool(1, true /* callerWillJoin */);
    sp<Nfc> nfc = new Nfc(device);
    if (nfc->registerAsService() != android::OK) {
        ALOGE("Could not register android.hardware.nfc@1.2::INfc");
        return 1;
    }
    joinRpcThreadpool();
    return 1;
}
