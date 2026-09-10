#!/usr/bin/env python3
"""Serial monitor + screenshot grabber over one connection.

    tools/mon.py [seconds] [--shot-every N] [--shot-dir DIR]

`pio device monitor` needs a TTY and can't run here. This connects once (the
board resets on connect — native USB-serial-JTAG), streams timestamped serial
lines, and, with --shot-every, sends 'S' every N seconds and decodes the
framebuffer dump the dev build prints back (see src/hw/Screenshot.h) to a PNG.
Doing it on one connection means an interactive nav test yields a screenshot
timeline without reconnecting (which would reset the board again).
"""
import argparse
import base64
import os
import struct
import sys
import time
import zlib

import serial

PORT = "/dev/ttyACM0"
_B64 = set("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/=")


def rgb565_be_to_png(path, w, h, raw):
    out = bytearray(w * h * 3)
    for i in range(w * h):
        v = (raw[2 * i] << 8) | raw[2 * i + 1]
        r, g, b = (v >> 11) & 0x1F, (v >> 5) & 0x3F, v & 0x1F
        out[3 * i] = (r << 3) | (r >> 2)
        out[3 * i + 1] = (g << 2) | (g >> 4)
        out[3 * i + 2] = (b << 3) | (b >> 2)

    def chunk(tag, data):
        return (struct.pack(">I", len(data)) + tag + data +
                struct.pack(">I", zlib.crc32(tag + data) & 0xFFFFFFFF))

    rows = bytearray()
    for y in range(h):
        rows.append(0)
        rows.extend(out[y * w * 3:(y + 1) * w * 3])
    png = b"\x89PNG\r\n\x1a\n"
    png += chunk(b"IHDR", struct.pack(">IIBBBBB", w, h, 8, 2, 0, 0, 0))
    png += chunk(b"IDAT", zlib.compress(bytes(rows), 9))
    png += chunk(b"IEND", b"")
    open(path, "wb").write(png)


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("secs", nargs="?", type=float, default=20.0)
    ap.add_argument("--shot-every", type=float, default=0.0)
    ap.add_argument("--shot-dir", default=".")
    args = ap.parse_args()

    os.makedirs(args.shot_dir, exist_ok=True)
    os.system(f"stty -F {PORT} -hupcl clocal 2>/dev/null")
    s = serial.Serial(PORT, 115200, timeout=0.2)
    s.reset_input_buffer()

    t0 = time.time()
    next_shot = args.shot_every if args.shot_every else 1e9
    buf = b""
    fb = None          # (w, h) while receiving a frame
    fb_b64 = []
    shot_n = 0

    while time.time() - t0 < args.secs:
        now = time.time() - t0
        if now >= next_shot:
            s.write(b"S")
            s.flush()
            next_shot += args.shot_every

        d = s.read(4096)
        if d:
            buf += d
        while b"\n" in buf:
            line, buf = buf.split(b"\n", 1)
            txt = line.decode("utf-8", "replace").rstrip()
            if txt.startswith("FRAMEBUF"):
                p = txt.split()
                fb, fb_b64 = (int(p[1]), int(p[2])), []
            elif txt.startswith("ENDFRAME") and fb:
                raw = base64.b64decode("".join(fb_b64))
                shot_n += 1
                name = os.path.join(args.shot_dir, f"shot-{shot_n:02d}.png")
                rgb565_be_to_png(name, fb[0], fb[1], raw)
                print(f"{now:6.1f}  >>> {name}")
                fb = None
            elif fb is not None and len(txt) >= 4 and set(txt) <= _B64:
                fb_b64.append(txt)
            elif txt:
                print(f"{now:6.1f}  {txt}")
            sys.stdout.flush()
    s.close()


if __name__ == "__main__":
    main()
