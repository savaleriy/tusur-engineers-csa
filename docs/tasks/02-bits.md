# Задание 2. Биты без ветвлений

> Модуль 2: [Целые числа и битовые операции](../modules/02-integers.md).

## Цель

Написать девять функций, в которых нет ни одного оператора управления и ни одного неопределенного поведения; увидеть, как компилятор заменяет ветвление арифметикой и инструкциями; сравнить x86-64 и RISC-V; измерить, сколько дает аппаратный `popcnt`.

## Правила

- В девяти функциях запрещены `if`, `else`, `switch`, `for`, `while`, `do`, `goto`, логические `&&` и `||`, а также любые `__builtin_*`.
- Сравнения (`x == 0`, `a < b`) разрешены: как выражение они дают 0 или 1. Побитовые `&`, `|`, `^`, `~` и сдвиги разрешены.
- `?:` разрешен, только если в ассемблере на месте этого выражения нет условного перехода. Проверьте по листингу; если переход появился, перепишите через маску.
- `cmov` и `setcc` разрешены: это условная пересылка и результат сравнения, а не переход.
- `main`, проверки и эталон могут использовать все что угодно, в том числе `__builtin_*`. Запреты относятся только к девяти функциям.
- Эталон в `main` - это не решение, а способ проверить свое решение.

## Шаблон файла

Скопируйте шаблон в `bits.c`. Тела девяти функций замените своими; `main` можно менять и дополнять.

```c
#define _POSIX_C_SOURCE 200809L
#include <limits.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* ---- девять функций: реализуйте ---- */

uint32_t bswap32(uint32_t x) { (void)x; return 0; }
int      is_pow2(uint64_t x) { (void)x; return 0; }
uint32_t parity32(uint32_t x) { (void)x; return 0; }
uint32_t popcount32(uint32_t x) { (void)x; return 0; }
uint32_t ctz32(uint32_t x) { (void)x; return 0; }
uint32_t clz32(uint32_t x) { (void)x; return 0; }
uint64_t next_pow2(uint64_t x) { (void)x; return 0; }
int64_t  sign_extend(uint64_t x, unsigned b) { (void)x; (void)b; return 0; }
int32_t  sat_add32(int32_t a, int32_t b) { (void)a; (void)b; return 0; }

/* ---- проверки: main можно менять ---- */

static int fails;

static void check_u(const char *name, unsigned long long got, unsigned long long want) {
    if (got != want) {
        if (fails < 10)
            printf("FAIL %-12s got %llu, want %llu\n", name, got, want);
        fails++;
    }
}

static void check_s(const char *name, long long got, long long want) {
    if (got != want) {
        if (fails < 10)
            printf("FAIL %-12s got %lld, want %lld\n", name, got, want);
        fails++;
    }
}

static int64_t sign_extend_ref(uint64_t x, unsigned b) {
    unsigned sh = 64 - b;
    return (int64_t)(x << sh) >> sh;
}

static int32_t sat_add_ref(int32_t a, int32_t b) {
    long long s = (long long)a + b;
    if (s > INT32_MAX) return INT32_MAX;
    if (s < INT32_MIN) return INT32_MIN;
    return (int32_t)s;
}

static double now_sec(void) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (double)ts.tv_sec + (double)ts.tv_nsec * 1e-9;
}

static int cmp_double(const void *pa, const void *pb) {
    double a = *(const double *)pa;
    double b = *(const double *)pb;
    return (a > b) - (a < b);
}

static void bench(void) {
    enum { N = 1 << 22 };
    static uint32_t a[N];
    uint32_t s = 2463534242u;
    for (unsigned i = 0; i < N; i++) {
        s ^= s << 13;
        s ^= s >> 17;
        s ^= s << 5;
        a[i] = s;
    }

    double t[5];
    for (int r = 0; r < 5; r++) {
        unsigned long long acc = 0;
        double t0 = now_sec();
        for (unsigned i = 0; i < N; i++)
            acc += popcount32(a[i]);
        t[r] = now_sec() - t0;
        printf("run %d: %.6f s, sum %llu\n", r + 1, t[r], acc);
    }

    qsort(t, 5, sizeof t[0], cmp_double);
    printf("median %.6f s, min %.6f, max %.6f, ns/element %.2f\n",
           t[2], t[0], t[4], t[2] * 1e9 / N);
}

int main(int argc, char **argv) {
    if (argc > 1 && argv[1][0] == '-') {
        bench();
        return 0;
    }

    check_u("bswap32", bswap32(0x0A0B0C0Du), 0x0D0C0B0Au);
    check_u("is_pow2", (unsigned)is_pow2(0), 0);
    check_u("is_pow2", (unsigned)is_pow2(1), 1);
    check_u("is_pow2", (unsigned)is_pow2(1ull << 63), 1);
    check_u("parity32", parity32(7u), 1);
    check_u("parity32", parity32(3u), 0);
    check_u("popcount32", popcount32(0u), 0);
    check_u("popcount32", popcount32(0xFFFFFFFFu), 32);
    check_s("sign_extend", sign_extend(0xFu, 4), -1);
    check_s("sign_extend", sign_extend(0x7u, 4), 7);
    check_s("sign_extend", sign_extend(0x80u, 8), -128);
    check_s("sign_extend", sign_extend(~0ull, 64), -1);
    check_s("sat_add32", sat_add32(INT32_MAX, 1), INT32_MAX);
    check_s("sat_add32", sat_add32(INT32_MIN, -1), INT32_MIN);
    check_u("next_pow2", next_pow2(0), 1);
    check_u("next_pow2", next_pow2(8), 8);
    check_u("next_pow2", next_pow2(9), 16);

    uint32_t x = 1;
    for (int i = 0; i < 200000; i++) {
        x = x * 1664525u + 1013904223u;
        check_u("popcount32", popcount32(x), (unsigned)__builtin_popcount(x));
        check_u("parity32", parity32(x), (unsigned)__builtin_parity(x));
        check_u("bswap32", bswap32(x), (unsigned)__builtin_bswap32(x));
        if (x != 0) {
            check_u("ctz32", ctz32(x), (unsigned)__builtin_ctz(x));
            check_u("clz32", clz32(x), (unsigned)__builtin_clz(x));
        }
        check_u("is_pow2", (unsigned)is_pow2(x),
                (unsigned)((x != 0) && ((x & (x - 1)) == 0)));
    }

    for (unsigned b = 1; b <= 64; b++) {
        uint64_t v = 0x0F0F0F0F0F0F0F0Full;
        check_s("sign_extend", sign_extend(v, b), sign_extend_ref(v, b));
        check_s("sign_extend", sign_extend(~v, b), sign_extend_ref(~v, b));
    }

    int32_t edge[] = {INT32_MAX, INT32_MIN, -1, 0, 1, 12345, -12345};
    for (unsigned i = 0; i < sizeof edge / sizeof edge[0]; i++) {
        for (unsigned j = 0; j < sizeof edge / sizeof edge[0]; j++) {
            check_s("sat_add32", sat_add32(edge[i], edge[j]),
                    sat_add_ref(edge[i], edge[j]));
        }
    }

    uint64_t pw[] = {0, 1, 2, 3, 7, 8, 9,
                     (1ull << 31) - 1, 1ull << 31,
                     (1ull << 62) - 1, 1ull << 62, 1ull << 63};
    for (unsigned i = 0; i < sizeof pw / sizeof pw[0]; i++) {
        uint64_t p = next_pow2(pw[i]);
        int ok = p >= pw[i] && (p & (p - 1)) == 0 &&
                 (pw[i] <= 1 || p / 2 < pw[i]);
        if (!ok) {
            printf("FAIL next_pow2(%llu) = %llu\n",
                   (unsigned long long)pw[i], (unsigned long long)p);
            fails++;
        }
    }

    if (fails) {
        printf("FAILED: %d\n", fails);
        return 1;
    }
    puts("OK");
    return 0;
}
```

Сборка и запуск проверок:

```bash
gcc -std=c17 -O2 -Wall -Wextra -Wpedantic -fsanitize=undefined -o bits bits.c && ./bits
```

Шаблон намеренно не содержит массивных проверок всех 2^32 входов: 200 тысяч псевдослучайных значений и граничные случаи ловят почти все ошибки. Если хотите больше, увеличьте счетчик цикла.

## Часть A. Ядро

Шесть функций: `bswap32`, `is_pow2`, `parity32`, `popcount32`, `sign_extend`, `sat_add32`. Подробнее о приемах - в модуле, разделы *Условные операции без ветвления* и *Знак, модуль, минимум и максимум*.

Граничные случаи, которые обязаны проходить:

| Функция | Вход | Ожидание |
|---|---|---|
| `bswap32` | `0x0A0B0C0D` | `0x0D0C0B0A` |
| `is_pow2` | 0, 1, 2, `1ull << 63` | 0, 1, 1, 1 |
| `parity32` | 7, 3 | 1, 0 |
| `popcount32` | 0, `0xFFFFFFFF` | 0, 32 |
| `sign_extend` | `0xF, 4`; `0x7, 4`; `0x80, 8` | -1, 7, -128 |
| `sat_add32` | `INT32_MAX, 1`; `INT32_MIN, -1` | `INT32_MAX`, `INT32_MIN` |

Ветвящиеся версии. Соберите рядом с безветвевыми и сравните ассемблер:

```c
int is_pow2_branchy(uint64_t x) {
    if (x == 0) return 0;
    while (x > 1) {
        if (x & 1) return 0;
        x >>= 1;
    }
    return 1;
}

int32_t sat_add32_branchy(int32_t a, int32_t b) {
    long long s = (long long)a + b;
    if (s > INT32_MAX) return INT32_MAX;
    if (s < INT32_MIN) return INT32_MIN;
    return (int32_t)s;
}
```

Вопрос не в том, какая версия *правильнее*, а в том, что компилятор оставил от каждой на `-O2`. Для `is_pow2_branchy` он может свернуть цикл в `x & (x - 1)`, а может оставить переход. Для `sat_add32_branchy` на x86 обычно появляется `cmov`, а на RISC-V - ветка.

## Часть B. Сложное

Три функции: `ctz32`, `clz32`, `next_pow2`. Подробнее - в модуле, разделы *Старший и младший единичный бит* и *Следующая степень двойки*. Подсказки.

- `ctz32`: `x & (0u - x)` оставляет младшую единицу. Умножение на `0x077CB531` и сдвиг на 27 дают индекс 0-31; таблица на 32 байта переводит индекс в число нулей.
- `clz32`: двоичный поиск по старшей половине. Сравнение дает 0 или 1; сдвиг делается на `t << k`, а не оператором `if`. Пять шагов покрывают 32 бита.
- `next_pow2`: `x |= x >> 1; x |= x >> 2; ...` заполняет единицами все биты ниже старшего. Для `x == 0` схема ломается, поэтому сначала `x -= (x != 0)`.

Для `sign_extend` нужен отдельный случай `b == 64`: сдвиг на `64 - b` при `b == 64` равен нулю, и это законно; проверьте, что ваша формула не делает сдвиг на 64.

Для `next_pow2` область определения - `x <= 2^63`: при большем `x` ответ `2^64` не помещается в `uint64_t`.

## Часть C. Ассемблер

### C1. Команды и биты

Если ассемблер пока непривычен, держите под рукой [шпаргалку по ассемблеру](../appendix/asm.md): регистры, `cmp` и флаги, `jcc`, `setcc`, `cmov`.

Соберите на Godbolt (x86-64, `-O2`) функции:

```c
unsigned f_and(unsigned x) { return x & 0xff; }
unsigned f_or (unsigned x) { return x | 1; }
unsigned f_shl(unsigned x) { return x << 3; }
int      f_sar(int x)      { return x >> 3; }
unsigned f_xor(unsigned x) { return x ^ (x >> 16); }
```

Для каждой найдите в ассемблере команду (`and`, `or`, `shl`, `sar`, `shr`, `xor`) и объясните, чем `sar` отличается от `shr`. Затем переключитесь на `RISC-V (64-bits) gcc` и найдите те же операции в RISC-V: как они называются там?

### C2. Деление и обмен

1. Соберите `int d7(int x) { return x / 7; }` и `int m7(int x) { return x % 7; }` при `-O2`. Найдите магическое умножение; замените 7 на переменную - появится `div`.
2. Проверьте обмен трех переменных: XOR-версию и арифметическую. Сравните с обменом через временную переменную: что сгенерировал компилятор в каждом случае?

### C3. Таблица функций

Заполните для девяти функций:

| Функция | x86-64 | x86-64-v3 | rv64gc | rv64gc_zbb | Какие инструкции появились |
|---|---:|---:|---:|---:|---|
| `bswap32` | | | | | |
| `is_pow2` | | | | | |
| `parity32` | | | | | |
| `popcount32` | | | | | |
| `ctz32` | | | | | |
| `clz32` | | | | | |
| `next_pow2` | | | | | |
| `sign_extend` | | | | | |
| `sat_add32` | | | | | |

Команды:

```bash
gcc -std=c17 -O2 -S -masm=intel -o bits.s bits.c
gcc -std=c17 -O2 -march=x86-64-v3 -S -masm=intel -o bits-v3.s bits.c
riscv64-linux-gnu-gcc -O2 -march=rv64gc -S -o bits-rv.s bits.c
riscv64-linux-gnu-gcc -O2 -march=rv64gc_zbb -S -o bits-rv-zbb.s bits.c
```

На Godbolt те же цели называются `x86-64 gcc` и `RISC-V (64-bits) gcc`. Число инструкций считайте вместе с `ret` и указывайте компилятор и версию: у GCC и Clang числа разные.

Что должно получиться и что нужно объяснить:

- `popcount32` на `x86-64-v3` сворачивается в `popcnt`, на `rv64gc_zbb` - в `cpop`.
- `bswap32` на x86-64 компилируется в `bswap` уже без `-march`: это базовая инструкция. На `rv64gc_zbb` появляется `rev8`.
- `clz32`, написанный двоичным поиском, в `lzcnt` не превращается. Объясните, почему компилятор не узнает эту идиому, и что было бы с `__builtin_clz`.
- `ctz32` через таблицу де Брёйна у GCC превращается в `tzcnt`; у Clang может появиться `test` и переход для `x == 0`. Объясните, откуда переход, если алгоритм безветвевый.
- На RISC-V в базе нет `cmov`, поэтому безветвевый исходник иногда компилируется в ветку. Найдите такой случай и объясните.


## Часть D. Замер

Программа в шаблоне умеет измерять `popcount32` на массиве из 4 миллионов чисел: пять запусков, медиана, минимум и максимум.

```bash
gcc -std=c17 -O2 -o bits bits.c
./bits --bench

gcc -std=c17 -O2 -march=x86-64-v3 -o bits-v3 bits.c
./bits-v3 --bench
```

Сравните два бинарника: первый считает SWAR, второй на машине с AVX2 использует `popcnt`. Запишите в таблицу медиану, минимум, максимум и наносекунды на элемент. Если процессор не поддерживает `-march=x86-64-v3`, второй бинарник может упасть с недопустимой инструкцией; тогда соберите с `-mpopcnt` и проверьте поддержку через `lscpu`.

| Сборка | Медиана, с | Минимум, с | Максимум, с | нс/элемент | Ускорение |
|---|---:|---:|---:|---:|---:|
| `-O2` | | | | | 1 |
| `-O2 -march=x86-64-v3` | | | | | |

Пять запусков уже внутри программы; если разброс большой, запустите программу три раза и возьмите медиану медиан.
