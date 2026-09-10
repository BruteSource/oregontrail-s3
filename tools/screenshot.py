#!/usr/bin/env python3
"""Grab a framebuffer screenshot from the board over USB serial.

Usage:
    tools/screenshot.py [--port /dev/ttyACM0] [--out shot.png]

Sends 'S' to the firmware, reads the base64 RGB565 dump it prints back
(see src/hw/Screenshot.h), and writes a PNG. No third-party deps.
"""
import argparse
import base64
import os
import struct
import sys
import time
import zlib

try:
    import serial  # pyserial
except ImportError:
    sys.exit("pyserial not found. Try: ~/.platformio/penv/bin/python tools/screenshot.py")


_B64SET = set("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/=")


def _is_b64(line):
    return len(line) >= 4 and set(line) <= _B64SET


def read_frame(port, baud, timeout):
    # Don't reset the board on open (native USB-serial-JTAG resets on a DTR/RTS
    # transition) — otherwise every screenshot bounces back to the boot screen.
    os.system(f"stty -F {port} -hupcl clocal 2>/dev/null")
    s = serial.Serial()
    s.port = port
    s.baudrate = baud
    s.timeout = 1
    s.dtr = False
    s.rts = False
    s.open()
    time.sleep(0.2)
    s.reset_input_buffer()
    s.write(b"S")
    s.flush()

    header = None
    b64 = []
    deadline = time.time() + timeout
    while time.time() < deadline:
        line = s.readline().decode("ascii", "replace").strip()
        if not line:
            continue
        if line.startswith("FRAMEBUF"):
            _, w, h, fmt, enc = line.split()
            header = (int(w), int(h))
        elif line.startswith("ENDFRAME"):
            break
        elif header is not None and _is_b64(line):
            b64.append(line)
    s.close()
    if header is None:
        sys.exit("no FRAMEBUF response — is the dev build flashed and idle?")
    raw = base64.b64decode("".join(b64))
    return header[0], header[1], raw


def rgb565_to_rgb888(raw, w, h):
    out = bytearray(w * h * 3)
    for i in range(w * h):
        # LovyanGFX stores 16bpp sprite bytes in display (big-endian) order.
        v = (raw[2 * i] << 8) | raw[2 * i + 1]
        r = (v >> 11) & 0x1F
        g = (v >> 5) & 0x3F
        b = v & 0x1F
        out[3 * i] = (r << 3) | (r >> 2)
        out[3 * i + 1] = (g << 2) | (g >> 4)
        out[3 * i + 2] = (b << 3) | (b >> 2)
    return bytes(out)


def write_png(path, w, h, rgb):
    def chunk(tag, data):
        return (struct.pack(">I", len(data)) + tag + data +
                struct.pack(">I", zlib.crc32(tag + data) & 0xFFFFFFFF))

    raw = bytearray()
    for y in range(h):
        raw.append(0)  # filter: none
        raw.extend(rgb[y * w * 3:(y + 1) * w * 3])
    png = b"\x89PNG\r\n\x1a\n"
    png += chunk(b"IHDR", struct.pack(">IIBBBBB", w, h, 8, 2, 0, 0, 0))
    png += chunk(b"IDAT", zlib.compress(bytes(raw), 9))
    png += chunk(b"IEND", b"")
    with open(path, "wb") as f:
        f.write(png)


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--port", default="/dev/ttyACM0")
    ap.add_argument("--baud", type=int, default=115200)
    ap.add_argument("--timeout", type=float, default=30.0)
    ap.add_argument("--out", default=None)
    args = ap.parse_args()

    w, h, raw = read_frame(args.port, args.baud, args.timeout)
    expect = w * h * 2
    if len(raw) != expect:
        print(f"warning: got {len(raw)} bytes, expected {expect}", file=sys.stderr)
    rgb = rgb565_to_rgb888(raw, w, h)
    out = args.out or time.strftime("shot-%Y%m%d-%H%M%S.png")
    write_png(out, w, h, rgb)
    print(out)


if __name__ == "__main__":
    main()
