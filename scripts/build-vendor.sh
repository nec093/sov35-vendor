#!/bin/bash
# Build vendor.img for keyaki. usage: build-vendor.sh [LOS_ROOT]
set -e
cd "${1:-.}"
export ALLOW_MISSING_DEPENDENCIES=true   # only the vendor image is built
export LC_ALL=C
source build/envsetup.sh
lunch lineage_keyaki-userdebug
m -j"$(nproc)" vendorimage
ls -l out/target/product/keyaki/vendor.img
