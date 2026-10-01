#!/bin/bash
# Build boot.img from a sov35-kernel Image.gz-dtb.
# usage: mkboot.sh KERNEL RAMDISK OUT.img ["extra cmdline"]
#   KERNEL   arch/arm64/boot/Image.gz-dtb of sov35-kernel
#   RAMDISK  the ramdisk of your current (Magisk-patched) boot image, e.g. from
#            unpack_bootimg.py --boot_img boot.img --out unpacked  ->  unpacked/ramdisk
#   MKBOOTIMG  path to mkbootimg.py (default: system/tools/mkbootimg/mkbootimg.py
#              of a lineage-18.1 checkout in $LOS_ROOT, or mkbootimg on PATH)
set -e
K=${1:?kernel}; R=${2:?ramdisk}; OUT=${3:?output}; EXTRA=$4
MK=${MKBOOTIMG:-${LOS_ROOT:+$LOS_ROOT/system/tools/mkbootimg/mkbootimg.py}}
CMDLINE="androidboot.bootdevice=7464900.sdhci androidboot.selinux=permissive msm_rtb.filter=0x3F ehci-hcd.park=3 coherent_pool=8M sched_enable_power_aware=1 user_debug=31 cgroup.memory=nokmem printk.devkmsg=on kpti=0 androidboot.hardware=keyaki buildvariant=userdebug log_buf_len=4M firmware_class.path=/vendor/firmware"
run() { if [ -n "$MK" ]; then python3 "$MK" "$@"; else mkbootimg "$@"; fi; }
run --kernel "$K" --ramdisk "$R" \
  --header_version 0 --base 0x80000000 --kernel_offset 0x8000 --ramdisk_offset 0x2000000 \
  --tags_offset 0x1e00000 --pagesize 4096 --os_version 10.0.0 --os_patch_level 2020-03 \
  --cmdline "$CMDLINE $EXTRA" -o "$OUT"
ls -l "$OUT"
