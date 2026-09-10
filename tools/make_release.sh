#!/usr/bin/env bash
# Build a single merged flashable image at release/oregontrail-s3-full.bin
set -euo pipefail
cd "$(dirname "$0")/.."

pio run -e hosyond-s3

B=.pio/build/hosyond-s3
ESPTOOL=$(find ~/.platformio -name 'esptool.py' -path '*tool-esptoolpy*' | head -1)
BOOTAPP=$(find ~/.platformio/packages/framework-arduinoespressif32 \
            -name 'boot_app0.bin' | head -1)

mkdir -p release
python "$ESPTOOL" --chip esp32s3 merge_bin \
  -o release/oregontrail-s3-full.bin \
  --flash_mode dio --flash_freq 80m --flash_size 16MB \
  0x0     "$B/bootloader.bin" \
  0x8000  "$B/partitions.bin" \
  0xe000  "$BOOTAPP" \
  0x10000 "$B/firmware.bin"

echo "release/oregontrail-s3-full.bin  ($(du -h release/oregontrail-s3-full.bin | cut -f1))"
