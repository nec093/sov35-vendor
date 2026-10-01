#!/bin/bash
# Copy a sov35-kernel build into device/sony/keyaki/prebuilt.
# usage: update-kernel.sh KERNEL_OUT [LOS_ROOT]
#   KERNEL_OUT  the kernel's O= directory (Image.gz-dtb + techpack/*_dlkm.ko)
#   LOS_ROOT    lineage-18.1 checkout (default: current directory)
set -e
O=$(realpath "${1:?kernel output directory}")
L=$(realpath "${2:-.}")
P=$L/device/sony/keyaki/prebuilt
[ -f "$O/arch/arm64/boot/Image.gz-dtb" ] || { echo "no Image.gz-dtb in $O" >&2; exit 1; }
STRIP=${STRIP:-aarch64-linux-gnu-strip}
cp "$O/arch/arm64/boot/Image.gz-dtb" "$P/Image.gz-dtb"
rm -f "$P"/modules/*.ko
mkdir -p "$P/modules"
find "$O/techpack" -name '*_dlkm.ko' | while read -r m; do
  "$STRIP" --strip-debug -o "$P/modules/$(basename "$m")" "$m"
done
strings "$O/vmlinux" 2>/dev/null | grep -m1 "Linux version" || true
ls "$P/modules"
