# Задание 6. Конвейер, ILP и планирование

!!! warning "WIP"
    Задание в работе: условия и код могут меняться.

> Модуль 6: [Конвейер, ILP и планирование](../modules/06-pipeline-ilp.md). Нужно: `gcc`, `perf stat`, `llvm-mca`, [uops.info](https://uops.info).

## Цель

Измерить латентность и пропускную способность инструкций, сверить их со справочником и использовать эти числа, чтобы ускорить цикл, не меняя число операций.

## Часть A. Микробенчмарк инструкций

Ниже полный стенд для `imul`. Скопируйте его в `asm_bench.c`, соберите и запустите два режима: латентность и пропускную способность.

```c
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#define REP16(s) s; s; s; s; s; s; s; s; s; s; s; s; s; s; s; s

static uint64_t lat_imul(uint64_t x, long n) {
    for (long i = 0; i < n; i++)
        REP16(__asm__ volatile("imul %0, %0" : "+r"(x)));
    return x;
}

static uint64_t tp_imul(long n) {
    uint64_t x0 = 1, x1 = 2, x2 = 3, x3 = 4;
    uint64_t x4 = 5, x5 = 6, x6 = 7, x7 = 8;
    for (long i = 0; i < n; i++) {
        __asm__ volatile("imul %0, %0" : "+r"(x0));
        __asm__ volatile("imul %0, %0" : "+r"(x1));
        __asm__ volatile("imul %0, %0" : "+r"(x2));
        __asm__ volatile("imul %0, %0" : "+r"(x3));
        __asm__ volatile("imul %0, %0" : "+r"(x4));
        __asm__ volatile("imul %0, %0" : "+r"(x5));
        __asm__ volatile("imul %0, %0" : "+r"(x6));
        __asm__ volatile("imul %0, %0" : "+r"(x7));
        __asm__ volatile("imul %0, %0" : "+r"(x0));
        __asm__ volatile("imul %0, %0" : "+r"(x1));
        __asm__ volatile("imul %0, %0" : "+r"(x2));
        __asm__ volatile("imul %0, %0" : "+r"(x3));
        __asm__ volatile("imul %0, %0" : "+r"(x4));
        __asm__ volatile("imul %0, %0" : "+r"(x5));
        __asm__ volatile("imul %0, %0" : "+r"(x6));
        __asm__ volatile("imul %0, %0" : "+r"(x7));
    }
    return x0 ^ x1 ^ x2 ^ x3 ^ x4 ^ x5 ^ x6 ^ x7;
}

int main(int argc, char **argv) {
    long n = argc > 1 ? atol(argv[1]) : 1000000;
    int mode = argc > 2 ? atoi(argv[2]) : 0;   /* 1 - lat, 2 - tp */
    uint64_t r = 0;
    if (mode != 2) r ^= lat_imul(3, n);
    if (mode != 1) r ^= tp_imul(n);
    printf("%llu\n", (unsigned long long)r);
    return 0;
}

```

Сборка и замеры:

```bash
gcc -O2 asm_bench.c -o asm_bench
perf stat -e cycles ./asm_bench 1000000 1
perf stat -e cycles ./asm_bench 1000000 2
```

В первом режиме 16 инструкций образуют одну зависимую цепочку; во втором 16 инструкций независимы и идут через восемь регистров. В обоих случаях число инструкций равно $16n$, поэтому

$$
L \approx \frac{\text{cycles}}{16n}, \qquad
\tau \approx \frac{\text{cycles}}{16n}.
$$

Сравните с [uops.info](https://uops.info) для своего процессора. Чтобы измерить другую инструкцию, замените `imul` в ассемблере на `add`, `div`, `addss`, `mulss`, `vfmadd231ss` или `sqrtss`. Для SSE-регистров используйте ограничение `"+x"`.

## Часть B. Аккумуляторы и полином

Полная программа: четыре суммы, Горнер, Эстрин и Горнер на четырех точках. Скопируйте в `ilp.c`.

```c
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define N 1000000

static float a[N], x[N], y[N], c[17];

static double now(void) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec + ts.tv_nsec * 1e-9;
}

static float sum_k1(const float *v, size_t n) {
    float s = 0;
    for (size_t i = 0; i < n; i++) s += v[i];
    return s;
}

static float sum_k2(const float *v, size_t n) {
    float s0 = 0, s1 = 0;
    size_t i = 0;
    for (; i + 1 < n; i += 2) { s0 += v[i]; s1 += v[i + 1]; }
    for (; i < n; i++) s0 += v[i];
    return s0 + s1;
}

static float sum_k4(const float *v, size_t n) {
    float s0 = 0, s1 = 0, s2 = 0, s3 = 0;
    size_t i = 0;
    for (; i + 3 < n; i += 4) {
        s0 += v[i]; s1 += v[i + 1]; s2 += v[i + 2]; s3 += v[i + 3];
    }
    for (; i < n; i++) s0 += v[i];
    return (s0 + s1) + (s2 + s3);
}

static float sum_k8(const float *v, size_t n) {
    float s0 = 0, s1 = 0, s2 = 0, s3 = 0;
    float s4 = 0, s5 = 0, s6 = 0, s7 = 0;
    size_t i = 0;
    for (; i + 7 < n; i += 8) {
        s0 += v[i];     s1 += v[i + 1]; s2 += v[i + 2]; s3 += v[i + 3];
        s4 += v[i + 4]; s5 += v[i + 5]; s6 += v[i + 6]; s7 += v[i + 7];
    }
    for (; i < n; i++) s0 += v[i];
    return ((s0 + s1) + (s2 + s3)) + ((s4 + s5) + (s6 + s7));
}

static void poly_horner(const float *xx, float *yy, size_t n, const float *cc) {
    for (size_t i = 0; i < n; i++) {
        float p = cc[16];
        for (int k = 15; k >= 0; k--) p = p * xx[i] + cc[k];
        yy[i] = p;
    }
}

static void poly_estrin(const float *xx, float *yy, size_t n, const float *cc) {
    for (size_t i = 0; i < n; i++) {
        float t = xx[i], t2 = t * t, t4 = t2 * t2, t8 = t4 * t4, t16 = t8 * t8;
        float p0 = cc[0] + cc[1] * t;
        float p1 = cc[2] + cc[3] * t;
        float p2 = cc[4] + cc[5] * t;
        float p3 = cc[6] + cc[7] * t;
        float p4 = cc[8] + cc[9] * t;
        float p5 = cc[10] + cc[11] * t;
        float p6 = cc[12] + cc[13] * t;
        float p7 = cc[14] + cc[15] * t;
        float p8 = cc[16];
        float q0 = p0 + p1 * t2, q1 = p2 + p3 * t2, q2 = p4 + p5 * t2;
        float q3 = p6 + p7 * t2, q4 = p8;
        float r0 = q0 + q1 * t4, r1 = q2 + q3 * t4, r2 = q4;
        float s0 = r0 + r1 * t8;
        yy[i] = s0 + r2 * t16;
    }
}

static void poly_horner4(const float *xx, float *yy, size_t n, const float *cc) {
    size_t i = 0;
    for (; i + 3 < n; i += 4) {
        float p0 = cc[16], p1 = cc[16], p2 = cc[16], p3 = cc[16];
        for (int k = 15; k >= 0; k--) {
            p0 = p0 * xx[i] + cc[k];
            p1 = p1 * xx[i + 1] + cc[k];
            p2 = p2 * xx[i + 2] + cc[k];
            p3 = p3 * xx[i + 3] + cc[k];
        }
        yy[i] = p0; yy[i + 1] = p1; yy[i + 2] = p2; yy[i + 3] = p3;
    }
    for (; i < n; i++) {
        float p = cc[16];
        for (int k = 15; k >= 0; k--) p = p * xx[i] + cc[k];
        yy[i] = p;
    }
}

typedef float (*sum_fn)(const float *, size_t);
typedef void (*poly_fn)(const float *, float *, size_t, const float *);

static volatile float sink;

static double median5(double *t) {
    for (int i = 0; i < 5; i++)
        for (int j = i + 1; j < 5; j++)
            if (t[j] < t[i]) { double z = t[i]; t[i] = t[j]; t[j] = z; }
    return t[2];
}

static void bench_sum(const char *name, sum_fn f) {
    double t[5];
    for (int r = 0; r < 5; r++) {
        double t0 = now();
        sink = f(a, N);
        t[r] = now() - t0;
    }
    printf("sum  %-4s %8.3f ns/el\n", name, median5(t) * 1e9 / N);
}

static void bench_poly(const char *name, poly_fn f) {
    double t[5];
    for (int r = 0; r < 5; r++) {
        double t0 = now();
        f(x, y, N, c);
        t[r] = now() - t0;
    }
    printf("poly %-7s %8.3f ns/el\n", name, median5(t) * 1e9 / N);
}

int main(void) {
    for (size_t i = 0; i < N; i++) {
        a[i] = (float)(i % 1000) / 1000.0f;
        x[i] = 2.0f * (float)i / (float)N - 1.0f;
    }
    for (int k = 0; k <= 16; k++) c[k] = 1.0f / (float)(k + 1);

    bench_sum("k1", sum_k1);
    bench_sum("k2", sum_k2);
    bench_sum("k4", sum_k4);
    bench_sum("k8", sum_k8);

    bench_poly("horner", poly_horner);
    bench_poly("estrin", poly_estrin);
    bench_poly("horner4", poly_horner4);

    printf("sink = %g\n", (double)sink);
    return 0;
}

```

Сборка и запуск:

```bash
gcc -O2 -fno-tree-vectorize ilp.c -lm -o ilp
./ilp
```

`-fno-tree-vectorize` отключает SIMD: мы измеряем ILP, а не векторизацию. Программа делает по пять запусков и печатает медиану в наносекундах на элемент.

Посмотрите на `sum_k1` ... `sum_k8`: при каком $k$ время перестает падать? Сравните с $k = L / \tau$ из части A. Затем сравните `poly_horner`, `poly_estrin` и `poly_horner4`: у Горнера длина цепочки 16 FMA, у Эстрина - $O(\log 16)$, а `poly_horner4` использует независимые точки.

Для `llvm-mca` сохраните ассемблер и подайте его модели:

```bash
gcc -O2 -fno-tree-vectorize -S ilp.c -o ilp.s
llvm-mca -mcpu=native -iterations=100 < ilp.s
```

Сравните предсказание модели с замером. Модель не знает вашу частоту, турбо-режим и состояние кэша, поэтому совпадение не обязано быть точным.

## Критерии готовности

- `asm_bench` собран, оба режима замерены через `perf stat -e cycles`.
- Для `imul` получены $L$ и $\tau$ и сверены с uops.info.
- `ilp` собран с `-fno-tree-vectorize`, пять запусков, медианы записаны.
- Объяснено, при каком числе аккумуляторов рост прекращается.
- Сравнены Горнер, Эстрин и Горнер на четырех точках.

## Вопросы

1. Почему латентность `add` равна 1, а пропускная способность - 0.25? Что это значит для программы?
2. Почему сумма с одним аккумулятором упирается в латентность сложения, а с четырьмя - нет?
3. Почему `div` стоит десятки тактов и как это обходят компиляторы?
4. Почему `poly_estrin` быстрее `poly_horner`, хотя операций у него не меньше?
5. Где `llvm-mca` ошибается и почему его прогноз не заменяет замер?
