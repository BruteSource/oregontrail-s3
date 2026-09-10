# Flashing oregontrail-s3-full.bin

Single merged image for the Hosyond ESP32-S3 2.8" touchscreen board. Flash to
offset `0x0`.

## esptool

```sh
esptool --chip esp32s3 -p /dev/ttyACM0 -b 921600 write-flash 0x0 oregontrail-s3-full.bin
```

(older esptool: `write_flash`. If auto-reset into the bootloader misbehaves,
hold **BOOT**, tap **RESET**, release **BOOT**, then run the command.)

## ESP Web Flasher

Go to <https://esp.huhn.me>, connect the board, add `oregontrail-s3-full.bin` at
offset `0x0`, program.

## First boot

The screen runs a 4-corner touch calibration on first flash (tap each crosshair).
Recalibrate any time from **Settings → Recalibrate touch**.

Built from commit: see `git rev-parse HEAD` in the source tree.
