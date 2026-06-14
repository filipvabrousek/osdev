#!/usr/bin/env python3
"""Headless smoke test for H7 OS.

Boots build/os.img in QEMU (no display), drives the PS/2 mouse over QMP
to drag the WELCOME window, and saves screenshots as build/shot1..3.png.
Uses only the Python standard library.
"""
import json
import os
import socket
import struct
import subprocess
import sys
import time
import zlib

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
IMG = os.path.join(ROOT, "build", "os.img")
QMP_PORT = 4455


def ppm_to_png(ppm_path, png_path):
    with open(ppm_path, "rb") as f:
        data = f.read()
    # parse P6 header (magic, width, height, maxval), tolerate comments
    fields, pos = [], 0
    while len(fields) < 4:
        while pos < len(data) and data[pos:pos + 1].isspace():
            pos += 1
        if data[pos:pos + 1] == b"#":
            while data[pos:pos + 1] not in (b"\n", b""):
                pos += 1
            continue
        start = pos
        while pos < len(data) and not data[pos:pos + 1].isspace():
            pos += 1
        fields.append(data[start:pos])
    pos += 1  # single whitespace after maxval
    assert fields[0] == b"P6", "not a P6 ppm"
    w, h = int(fields[1]), int(fields[2])
    raw = data[pos:pos + w * h * 3]

    lines = b"".join(
        b"\x00" + raw[y * w * 3:(y + 1) * w * 3] for y in range(h)
    )

    def chunk(tag, payload):
        c = struct.pack(">I", len(payload)) + tag + payload
        return c + struct.pack(">I", zlib.crc32(tag + payload) & 0xFFFFFFFF)

    png = (b"\x89PNG\r\n\x1a\n"
           + chunk(b"IHDR", struct.pack(">IIBBBBB", w, h, 8, 2, 0, 0, 0))
           + chunk(b"IDAT", zlib.compress(lines, 6))
           + chunk(b"IEND", b""))
    with open(png_path, "wb") as f:
        f.write(png)


class Qmp:
    def __init__(self, port):
        self.sock = socket.create_connection(("127.0.0.1", port), timeout=10)
        self.f = self.sock.makefile("rw")
        json.loads(self.f.readline())  # greeting
        self.cmd("qmp_capabilities")

    def cmd(self, name, **args):
        msg = {"execute": name}
        if args:
            msg["arguments"] = args
        self.sock.sendall((json.dumps(msg) + "\n").encode())
        while True:
            line = self.f.readline()
            if not line:
                raise RuntimeError("QMP connection closed")
            reply = json.loads(line)
            if "return" in reply:
                return reply["return"]
            if "error" in reply:
                raise RuntimeError(f"{name}: {reply['error']}")
            # else: async event, keep reading

    def mouse_move(self, dx, dy):
        self.cmd("input-send-event", events=[
            {"type": "rel", "data": {"axis": "x", "value": dx}},
            {"type": "rel", "data": {"axis": "y", "value": dy}},
        ])

    def mouse_button(self, down):
        self.cmd("input-send-event", events=[
            {"type": "btn", "data": {"down": down, "button": "left"}},
        ])

    def click(self):
        self.mouse_button(True)
        time.sleep(0.2)
        self.mouse_button(False)
        time.sleep(0.2)

    def type_keys(self, qcodes):
        for qc in qcodes:
            for down in (True, False):
                self.cmd("input-send-event", events=[
                    {"type": "key",
                     "data": {"down": down,
                              "key": {"type": "qcode", "data": qc}}},
                ])
            time.sleep(0.08)

    def move_steps(self, dx, dy, steps=8):
        for _ in range(steps):
            self.mouse_move(dx // steps, dy // steps)
            time.sleep(0.05)
        self.mouse_move(dx - (dx // steps) * steps,
                        dy - (dy // steps) * steps)
        time.sleep(0.2)

    def shot(self, name):
        ppm = os.path.join(ROOT, "build", name + ".ppm")
        png = os.path.join(ROOT, "build", name + ".png")
        self.cmd("screendump", filename=ppm)
        time.sleep(0.2)
        ppm_to_png(ppm, png)
        os.remove(ppm)
        print(f"saved build/{name}.png")


def main():
    qemu = subprocess.Popen([
        "qemu-system-i386",
        "-drive", f"format=raw,file={IMG}",
        "-vga", "std", "-m", "64",
        "-display", "none",
        "-qmp", f"tcp:127.0.0.1:{QMP_PORT},server,nowait",
    ])
    try:
        time.sleep(1.0)
        q = Qmp(QMP_PORT)
        time.sleep(2.5)                 # let it boot and settle

        q.shot("shot1")                 # initial desktop

        # cursor starts at screen center (320, 240); WELCOME title bar
        # center is around (185, 80). QEMU can drop a single large rel
        # jump right after boot, so approach in small steps.
        q.mouse_move(1, 0)              # warm-up event
        time.sleep(0.2)
        for _ in range(8):
            q.mouse_move(-17, -20)
            time.sleep(0.06)
        time.sleep(0.3)
        q.mouse_button(True)            # grab the title bar
        time.sleep(0.3)
        for _ in range(10):             # drag down-right in steps
            q.mouse_move(18, 14)
            time.sleep(0.06)
        q.mouse_button(False)
        time.sleep(0.3)

        q.shot("shot2")                 # window should have moved

        # cursor is now at ~(366, 222). Move to (520, 150) -- safely
        # inside NOTEPAD, clear of the moved WELCOME window -- then
        # click to focus and type on the keyboard
        q.move_steps(154, -72)
        q.click()
        q.type_keys(["h", "i", "spc", "o", "s", "1"])
        time.sleep(0.3)
        q.shot("shot3")                 # notepad shows "HI OS1"

        # close NOTEPAD via its title-bar X at ~(564, 100)
        q.move_steps(44, -50)
        q.click()
        time.sleep(0.3)
        q.shot("shot4")                 # notepad gone

        q.cmd("quit")
        qemu.wait(timeout=5)
        print("OK")
    finally:
        if qemu.poll() is None:
            qemu.kill()


if __name__ == "__main__":
    sys.exit(main())
