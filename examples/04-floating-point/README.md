# Примеры к занятию 4: плавающая точка

Код к модулю 4: формат IEEE 754, особые значения, округление, суммы, FMA, субнормальные числа и Python-разбор битов.

## Быстрый старт

```bash
cd examples/04-floating-point
make            # собрать все C-программы
make run        # запустить все
make python     # запустить Python-скрипты
make fma        # базовый и -mfma варианты
make run-fma
make subnormal  # векторный и скалярный варианты
make run-subnormal
make riscv      # статические бинарники для RISC-V
make run-riscv  # запустить их в qemu-riscv64
make clean
```

Нужны `gcc`, `python3` и `numpy`. Для RISC-V - `riscv64-linux-gnu-gcc` и `qemu-riscv64`; без них соответствующие шаги пропускаются.

## Файлы C

| Файл | Что показывает |
|---|---|
| `src/01-intro.c` | `0.1 + 0.2`, сравнения `double` и `float` |
| `src/02-print-float.c` | `print_float`: знак, порядок, мантисса, `inf`, NaN, субнормальное |
| `src/03-rounding.c` | `2^24` и режимы `FE_UPWARD` |
| `src/04-close.c` | сравнение с относительным и абсолютным допуском |
| `src/05-harmonic.c` | прямой, обратный, попарный и Кэхэн для гармонического ряда |
| `src/06-fma.c` | `0x0p+0` без FMA и `0x1p-25` с FMA |
| `src/07-subnormal.c` | субнормальные: пять запусков, FTZ/DAZ (только x86: MXCSR) |
| `src/08-fp-examples.c` | `f2c`, `sqrt_demo`, `e_const`, `add_ld` |
| `src/09-long-double.c` | `sizeof(long double)`, `LDBL_MANT_DIG` |
| `src/10-special.c` | `1/+0`, `1/-0`, NaN, `fmax`, `FLT_TRUE_MIN`, `2^24` |

## Команды компиляции

Все C-программы:

```bash
cd examples/04-floating-point
make
```

Вручную:

```bash
gcc -std=gnu17 -O2 -Wall -Wextra -Wpedantic src/02-print-float.c -lm -o build/02-print-float
gcc -std=gnu17 -O2 -Wall -Wextra -Wpedantic src/05-harmonic.c -lm -o build/05-harmonic
gcc -std=gnu17 -O2 -Wall -Wextra -Wpedantic -D_POSIX_C_SOURCE=200809L src/07-subnormal.c -lm -o build/07-subnormal
gcc -std=gnu17 -O2 -Wall -Wextra -Wpedantic src/08-fp-examples.c -lm -o build/08-fp-examples
```

FMA:

```bash
gcc -std=gnu17 -O2 -Wall -Wextra -Wpedantic src/06-fma.c -lm -o build/06-fma
gcc -std=gnu17 -O2 -Wall -Wextra -Wpedantic -mfma -mavx2 src/06-fma.c -lm -o build/06-fma-mfma
./build/06-fma        # 0x0p+0
./build/06-fma-mfma   # 0x1p-25
```

Субнормальные:

```bash
gcc -std=gnu17 -O2 -D_POSIX_C_SOURCE=200809L src/07-subnormal.c -lm -o build/07-subnormal
gcc -std=gnu17 -O2 -D_POSIX_C_SOURCE=200809L -fno-tree-vectorize src/07-subnormal.c -lm -o build/07-subnormal-scalar
```

RISC-V:

```bash
make riscv
make run-riscv
```

Вручную:

```bash
riscv64-linux-gnu-gcc -O2 -march=rv64gc -static src/08-fp-examples.c -lm -o build/08-fp-examples-rv
qemu-riscv64 ./build/08-fp-examples-rv

riscv64-linux-gnu-gcc -O2 -march=rv64gc -static src/06-fma.c -lm -o build/06-fma-rv
qemu-riscv64 ./build/06-fma-rv
```

## Python

Скрипты лежат в `python/`:

| Файл | Что показывает |
|---|---|
| `fp_fields.py` | `struct`, знак/порядок/мантисса для `float32` и `float64` |
| `fp_exact.py` | `Decimal`, `Fraction`, `float.hex()`, `math.frexp` |
| `fp_float32.py` | `numpy.float32`: `0.1f+0.2f`, `0.3f-0.2f`, ulp |
| `fp_limits.py` | `sys.float_info`, `np.finfo`, `math.ulp`, `nextafter` |

Запуск:

```bash
make python
# или по отдельности
python3 python/fp_fields.py
python3 python/fp_exact.py
python3 python/fp_float32.py
python3 python/fp_limits.py
```

Если `numpy` нет:

```bash
python3 -m pip install numpy
```

## Что смотреть

- `02-print-float`: `1.0 = 0x3F800000`, `-6.25 = 0xC0C80000`, `0.1 = 0x3DCCCCCD`, `1e-40 = 0x000116C2`, `inf = 0x7F800000`, NaN = `0x7FC00000` или `0xFFC00000`.
- `03-rounding`: `default: 16777216`, `upward: 67108864`.
- `05-harmonic`: `15.403682709`, `16.686031342`, `16.695310593`, `16.695310593`, exact `16.695311366`.
- `06-fma`: `0x0p+0` против `0x1p-25`.
- `07-subnormal`: медиана и разброс пяти запусков; скалярная версия около 115x, векторная - около 41x.
- `08-fp-examples`: `f2c(212) = 100.000008`, `sqrt_demo(-1) = -1/-1`, `add_ld = 3`.

## Связь с заметкой

- `01-intro` - раздел 1.
- `02-print-float` - раздел 5.3.
- `03-rounding` - раздел 7.1.
- `04-close` - раздел 8.2.
- `05-harmonic` - разделы 9.1 и 10.1.
- `06-fma` - раздел 11.1.
- `07-subnormal` - раздел 12.2.
- `08-fp-examples` - раздел 14.1.
- Python-скрипты - практика 4: поля, точное значение, `float32`, пределы.

## Оговорки

- Python `float` - это `double`; для `float32` используйте `struct` или `numpy.float32`.
- `struct.pack(">f", x)` округляет `x` до `float32`; при переполнении бросает `OverflowError`.
- `1.0 / -0.0` в Python бросает `ZeroDivisionError`; для IEEE-поведения используйте `numpy.float64`.
- `08-fp-examples.c` использует `M_E`; поэтому в Makefile стоит `-std=gnu17`, а не строгий `-std=c17`.
