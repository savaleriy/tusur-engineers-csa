#!/usr/bin/env python3
"""Пределы и ulp для double и float32."""

import math
import sys

import numpy as np


def main():
    print("double")
    print("  epsilon:  ", sys.float_info.epsilon)
    print("  mant_dig: ", sys.float_info.mant_dig)
    print("  max:      ", sys.float_info.max)
    print("  min:      ", sys.float_info.min)
    print("  ulp(1.0): ", math.ulp(1.0))
    print("  nextafter(1.0, inf):", math.nextafter(1.0, math.inf))

    fi = np.finfo(np.float32)
    print()
    print("float32")
    print("  eps:      ", fi.eps)
    print("  nmant:    ", fi.nmant)
    print("  max:      ", fi.max)
    print("  tiny:     ", fi.tiny)
    print("  smallest_subnormal:", fi.smallest_subnormal)
    print("  spacing(1.0):", np.spacing(np.float32(1.0)))
    print("  spacing(2^24):", np.spacing(np.float32(2**24)))


if __name__ == "__main__":
    main()
