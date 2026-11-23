#!/usr/bin/env python3
"""Показать знак, порядок и мантиссу float32/float64 через struct."""

import math
import struct

FORMATS = {
    "f32": ("f", 8, 23, 127),
    "f64": ("d", 11, 52, 1023),
}


def bits(x, kind="f32"):
    fmt = FORMATS[kind][0]
    return int.from_bytes(struct.pack(">" + fmt, x), "big")


def fields(x, kind="f32"):
    _, eb, mb, bias = FORMATS[kind]
    u = bits(x, kind)
    total = 1 + eb + mb

    sign = u >> (total - 1)
    exp = (u >> mb) & ((1 << eb) - 1)
    mant = u & ((1 << mb) - 1)

    if exp == 0:
        cls = "subnormal" if mant else "zero"
        hidden = 0
        eff_exp = 1 - bias
        value = (-1) ** sign * (mant / 2**mb) * 2 ** (1 - bias)
    elif exp == (1 << eb) - 1:
        cls = "inf" if mant == 0 else "nan"
        hidden = None
        eff_exp = None
        value = math.inf if mant == 0 else math.nan
        if sign:
            value = -value
    else:
        cls = "normal"
        hidden = 1
        eff_exp = exp - bias
        value = (-1) ** sign * (1 + mant / 2**mb) * 2 ** (exp - bias)

    return u, sign, exp, eff_exp, mant, hidden, cls, value


def show(x, kind="f32"):
    u, s, e, eff, m, h, cls, val = fields(x, kind)
    _, eb, mb, _ = FORMATS[kind]
    width = (1 + eb + mb + 3) // 4
    mwidth = (mb + 3) // 4
    eff_s = "-" if eff is None else eff

    print(
        f"{x!r:>12} {kind} 0x{u:0{width}X} "
        f"s={s} e={e:4} exp={eff_s!s:>5} "
        f"m=0x{m:0{mwidth}X} m/2^mb={m / 2**mb:.9f} "
        f"h={h} {cls:9} value={val!r}"
    )


def main():
    for x, kind in [
        (1.0, "f32"),
        (-6.25, "f32"),
        (0.1, "f32"),
        (1e-40, "f32"),
        (float("inf"), "f32"),
        (float("nan"), "f32"),
        (-0.0, "f32"),
        (0.1, "f64"),
    ]:
        show(x, kind)

    print()
    print("0.1f bits:", hex(bits(0.1, "f32")))
    print("0.1f exact as double:", struct.unpack(">f", struct.pack(">f", 0.1))[0].hex())
    print("0.1 double hex:", (0.1).hex())
    print("frexp(0.1):", math.frexp(0.1))


if __name__ == "__main__":
    main()
