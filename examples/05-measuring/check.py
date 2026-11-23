#!/usr/bin/env python3
"""Сравнить вывод heat с контрольными значениями.

Использование:
    python3 check.py expected/512x100.txt build/512x100.txt

Допуски: полная теплота - 1e-6 относительно, максимум - 1e-5
относительно, каждый столбец гистограммы - не больше 5 отсчетов.
Такие допуски пропускают разумные изменения реализации (например,
переход с float на double для сетки), но не пропускают ошибку в
накоплении суммы.
"""

import sys

HIST_BINS = 20
TOTAL_RTOL = 1e-6
MAX_RTOL = 1e-5
HIST_ATOL = 5


def read_output(path):
    try:
        with open(path, encoding="utf-8") as stream:
            lines = [line.strip() for line in stream if line.strip()]
    except OSError as error:
        raise SystemExit(f"не удалось прочитать {path}: {error}")

    if not lines:
        raise SystemExit(f"{path}: пустой файл")

    header = {}
    for field in lines[0].split():
        key, sep, value = field.partition("=")
        if not sep:
            raise SystemExit(f"{path}: неверная строка заголовка: {lines[0]}")
        try:
            header[key] = int(value)
        except ValueError:
            raise SystemExit(f"{path}: неверное значение {field}")

    rows = []
    for line in lines[1:]:
        if line.startswith("#"):
            continue
        fields = line.split()
        if len(fields) != 3 + HIST_BINS:
            raise SystemExit(f"{path}: ожидалось {3 + HIST_BINS} чисел: {line}")
        try:
            rows.append(
                (
                    int(fields[0]),
                    float(fields[1]),
                    float(fields[2]),
                    [int(value) for value in fields[3:]],
                )
            )
        except ValueError:
            raise SystemExit(f"{path}: неверная строка данных: {line}")

    return header, rows


def close(actual, expected, rtol):
    return abs(actual - expected) <= rtol * abs(expected)


def compare(expected_path, actual_path):
    expected_header, expected_rows = read_output(expected_path)
    actual_header, actual_rows = read_output(actual_path)
    errors = []

    for key in ("n", "steps", "seed"):
        if expected_header.get(key) != actual_header.get(key):
            errors.append(
                f"заголовок: {key} = {actual_header.get(key)}, "
                f"ожидалось {expected_header.get(key)}"
            )

    if len(expected_rows) != len(actual_rows):
        errors.append(
            f"строк статистики {len(actual_rows)}, ожидалось {len(expected_rows)}"
        )

    for index, (want, got) in enumerate(zip(expected_rows, actual_rows)):
        step, total, tmax, hist = want
        if got[0] != step:
            errors.append(f"строка {index + 1}: шаг {got[0]}, ожидался {step}")
            continue

        if not close(got[1], total, TOTAL_RTOL):
            errors.append(
                f"шаг {step}: total_heat {got[1]:.6f}, ожидалось {total:.6f} "
                f"(отличие {got[1] - total:+.6f})"
            )
        if not close(got[2], tmax, MAX_RTOL):
            errors.append(
                f"шаг {step}: max_temp {got[2]:.6f}, ожидалось {tmax:.6f} "
                f"(отличие {got[2] - tmax:+.6f})"
            )
        for bin_index, (actual_count, expected_count) in enumerate(
            zip(got[3], hist)
        ):
            if abs(actual_count - expected_count) > HIST_ATOL:
                errors.append(
                    f"шаг {step}: hist[{bin_index}] = {actual_count}, "
                    f"ожидалось {expected_count}"
                )

    if errors:
        print(f"FAIL: {actual_path} отличается от {expected_path}")
        for line in errors[:10]:
            print(f"  {line}")
        if len(errors) > 10:
            print(f"  ... еще {len(errors) - 10}")
        return 1

    print(f"OK: {actual_path} совпадает с {expected_path}")
    return 0


def main():
    if len(sys.argv) != 3:
        raise SystemExit("usage: check.py EXPECTED ACTUAL")
    return compare(sys.argv[1], sys.argv[2])


if __name__ == "__main__":
    sys.exit(main())
