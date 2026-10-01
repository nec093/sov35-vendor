# sov35-vendor — LineageOS 18.1 Treble vendor for the Xperia XZs (SOV35)

A Treble `vendor` image for the Sony Xperia XZs (SOV35, au, `keyaki`,
MSM8996), built from LineageOS 18.1 (Android 11, `FCM target-level 5`) and
flashed to the **`oem` partition**, so that GSIs (Android 12+) run on top of
[sov35-kernel](https://github.com/nec093/sov35-kernel) (Linux 5.4).
GSI-specific fixes live in
[sov35-magisk-modules](https://github.com/nec093/sov35-magisk-modules).

## Branches

| branch | checked out at | what |
|---|---|---|
| `main` | — | this README, local manifests, scripts |
| `device-keyaki` | `device/sony/keyaki` | keyaki device tree, including the sov35-kernel prebuilts (`prebuilt/Image.gz-dtb`, `prebuilt/modules/*_dlkm.ko`) |
| `device-tone-common` | `device/sony/tone-common` | tone platform tree (shared with dora/kagura) |
| `hal-audio-msm8996` | `hardware/qcom-caf/msm8996/audio` | LineageOS `lineage-18.1-caf-msm8996` audio HAL + a build fix for the 5.4 audio-kernel uapi headers |

The proprietary files (`vendor/sony`) are in a separate, **private**
repository (`sov35-vendor-blobs`, see below).

## Status (2026-10-01, phh AOSP 12.1 GSI)

| works | not yet |
|---|---|
| display, touch, GPU (Adreno 530), CPU frequency/idle (incl. L2 power collapse), thermal/LMH | camera (kernel side probes; the HAL blobs do not match) |
| Wi-Fi (brcmfmac), storage, microSD, USB/adb | cellular modem (firmware authentication) |
| battery/charging, sensors (SLPI) | Bluetooth power-up is not verified |
| audio: speakers (stereo, Sony ACDB 124 tuning), headphones on `SLIMBUS_6_RX` up to 192 kHz / 24-bit, microphones | NFC HAL starts; tag reading not verified |
| hardware video decode/encode (Venus) | |

## Build

Prerequisites: a Linux machine set up for building LineageOS 18.1 (about
250 GB of disk), and a sov35-kernel build (see its README).

```sh
mkdir lineage-18.1 && cd lineage-18.1
repo init -u https://github.com/LineageOS/android.git -b lineage-18.1
mkdir -p .repo/local_manifests
curl -o .repo/local_manifests/sov35.xml \
  https://raw.githubusercontent.com/nec093/sov35-vendor/main/local_manifests/sov35.xml
# optional: drop projects a vendor-only build does not need
curl -o .repo/local_manifests/00-trim.xml \
  https://raw.githubusercontent.com/nec093/sov35-vendor/main/local_manifests/00-trim.xml
# with access to the private blobs repository:
curl -o .repo/local_manifests/sov35-blobs.xml \
  https://raw.githubusercontent.com/nec093/sov35-vendor/main/local_manifests/sov35-blobs.xml
repo sync -c -j8
```

Then:

1. (Optional) put your own kernel build into the device tree. `device-keyaki`
   already carries prebuilts from a sov35-kernel commit named in its log.

   ```sh
   scripts/update-kernel.sh /path/to/kernel-out .
   ```

2. Build the vendor image:

   ```sh
   scripts/build-vendor.sh .        # -> out/target/product/keyaki/vendor.img
   ```

3. Build the boot image. Use the ramdisk of your current (Magisk-patched)
   boot image, for example `unpack_bootimg.py --boot_img boot.img --out unpacked`:

   ```sh
   MKBOOTIMG=/path/to/mkbootimg.py scripts/mkboot.sh \
       /path/to/kernel-out/arch/arm64/boot/Image.gz-dtb unpacked/ramdisk boot.img
   ```

   The script works with the mkbootimg of lineage-18.1
   (`system/tools/mkbootimg`). AOSP's current mkbootimg produces the same image
   except for the header id hash.

(The `scripts/` directory is on the `main` branch of this repository; clone it
next to the checkout or download the scripts.)

## Flash

**Back up the `oem` partition first.** It holds Sony's carrier
configuration, and this vendor image replaces it:

```sh
adb shell su -c "dd if=/dev/block/bootdevice/by-name/oem of=/sdcard/oem_backup.img"
adb pull /sdcard/oem_backup.img
```

Then, from fastboot:

```sh
fastboot flash oem out/target/product/keyaki/vendor.img
fastboot flash boot boot.img
```

Flash a GSI to `system` as usual (an arm64 A/B "bgS"/"bvS" GSI, for example
phh AOSP 12.1).

## Proprietary files

`vendor/sony/{keyaki,tone-common}` contains Sony/Qualcomm binaries:

- the files listed in `device/sony/*/proprietary-files.txt`,
- the BCM4359 Wi-Fi firmware + NVRAM (`keyaki/proprietary/vendor/firmware/brcm/`),
- the GCam apk (`tone-common/prebuilt-apps/GCam/`).

They are not redistributed here. The set in `sov35-vendor-blobs` started from
the tone-common + keyaki blobs of
[sony-msm89xx-development](https://github.com/sony-msm89xx-development)
(`lineage-20.0`), with changes for this port: the Android 11 seccomp policies,
the libexidx shim for libfastcvopt, dropped camera/widevine blobs, the CXD224X
NFC blobs, the Wi-Fi firmware and GCam. Without access to
`sov35-vendor-blobs`, start from that set (or extract from a keyaki/tone
firmware with the device trees' `extract-files.sh` / `setup-makefiles.sh`) and
expect to adjust `proprietary-files.txt`.

---

## 日本語

SOV35（Xperia XZs）用の Treble vendor イメージです。LineageOS 18.1 からビルドし、`oem` パーティションに書き込みます。
[sov35-kernel](https://github.com/nec093/sov35-kernel)（Linux 5.4）と組み合わせて、GSI（Android 12 以降）を動かします。

- **ビルド手順**：lineage-18.1 を `repo init` し、`local_manifests/sov35.xml` を置いて `repo sync` します。
  その後、`scripts/build-vendor.sh` で vendor をビルドします。
- **書き込み前に必ず oem パーティションをバックアップしてください**（Sony のキャリア設定が上書きされます）。
- **proprietary ファイル**（vendor/sony）は、非公開リポジトリ（sov35-vendor-blobs）にあります。
  アクセスできない場合は、`extract-files.sh` で各自抽出してください。
- GSI ごとの修正は [sov35-magisk-modules](https://github.com/nec093/sov35-magisk-modules) にあります。
