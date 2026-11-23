#!/usr/bin/env python3
"""Арифметика float32 и ulp через numpy."""

import struct

import numpy as np


def b32(x):
    return int.from_bytes(struct.pack(">f", float(x)), "big")


def show_bits(name, x):
    print(f"{name:>16} = {x!r:>14}  0x{b32(x):08X}")


def main():
    a = np.float32(0.1)
    b = np.float32(0.2)
    c = np.float32(0.3)

    show_bits("0.1f", a)
    show_bits("0.2f", b)
    show_bits("0.3f", c)
    show_bits("0.1f + 0.2f", a + b)
    show_bits("0.3f - 0.2f", c - b)

    print()
    print("0.1f + 0.2f == 0.3f:", a + b == c)
    print("0.3f - 0.2f == 0.1f:", c - b == a)

    print()
    print("2^24 + 1 == 2^24:", np.float32(2**24) + np.float32(1) == np.float32(2**24))
    print("spacing(2^24):", np.spacing(np.float32(2**24)))
    print("spacing(1.0):", np.spacing(np.float32(1.0)))
    print("nextafter(1, 2):", np.nextafter(np.float32(1), np.float32(2)))

    fi = np.finfo(np.float32)
    print()
    print("float32 eps:    ", fi.eps)
    print("float32 nmant:  ", fi.nmant)
    print("float32 max:    ", fi.max)
    print("float32 tiny:   ", fi.tiny)
    print("float32 smallest_subnormal:", fi.smallest_subnormal)


if __name__ == "__main__":
    main()
