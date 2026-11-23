#!/usr/bin/env python3
"""Точное значение double и float32: Decimal, Fraction, hex."""

import math
import struct
from decimal import Decimal
from fractions import Fraction


def f32(x):
    return struct.unpack(">f", struct.pack(">f", x))[0]


def main():
    x = 0.1
    y = f32(0.1)

    print("double 0.1")
    print("  repr:   ", repr(x))
    print("  hex:    ", x.hex())
    print("  Decimal:", Decimal.from_float(x))
    print("  Fraction:", Fraction.from_float(x))

    print()
    print("float32 0.1f")
    print("  repr:   ", repr(y))
    print("  hex:    ", y.hex())
    print("  Decimal:", Decimal.from_float(y))
    print("  Fraction:", Fraction.from_float(y))

    print()
    print("Decimal('0.1') - это десятичная 0.1, а не double:")
    print("  ", Decimal("0.1"))

    print()
    print("frexp(0.1):", math.frexp(0.1))
    print("hex(0.1):  ", (0.1).hex())


if __name__ == "__main__":
    main()
