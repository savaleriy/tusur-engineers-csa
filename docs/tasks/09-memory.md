# Задание 9. Латентность и пропускная способность памяти

!!! warning "WIP"
    Задание в работе: условия и код могут меняться.

> Модуль 9: [Иерархия памяти](../modules/09-memory.md). Нужно: Linux с `perf` (аппаратные счетчики), `gcc`, Python с `matplotlib`.

## Цель

Без документации, только микробенчмарками, восстановить устройство памяти машины: где кончается L1, где L2, где начинается DRAM, сколько стоит обращение и какая пропускная способность у памяти.

## Часть A. Латентность: pointer chasing

Полная программа для замера латентности:

```c
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

static uint64_t rng_state = 88172645463325252ull;
static uint64_t rng(void) {
    rng_state ^= rng_state << 13;
    rng_state ^= rng_state >> 7;
    rng_state ^= rng_state << 17;
    return rng_state;
}

static double now(void) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec + ts.tv_nsec * 1e-9;
}

int main(int argc, char **argv) {
    int e = argc > 1 ? atoi(argv[1]) : 20;
    size_t bytes = (size_t)1 << e;
    size_t n = bytes / sizeof(size_t);
    size_t *p = malloc(n * sizeof *p);
    if (!p) return 1;
    for (size_t i = 0; i < n; i++) p[i] = i;
    for (size_t i = n - 1; i > 0; i--) {
        size_t j = (size_t)(rng() % i);
        size_t t = p[i]; p[i] = p[j]; p[j] = t;
    }
    volatile size_t sink = 0;
    size_t i = 0;
    for (size_t k = 0; k < n; k++) i = p[i];
    sink ^= i;

    long steps = 1000000;
    double t[3];
    for (int r = 0; r < 3; r++) {
        i = 0;
        double t0 = now();
        for (long k = 0; k < steps; k++) i = p[i];
        t[r] = now() - t0;
        sink ^= i;
    }
    double best = t[0] < t[1] ? t[0] : t[1];
    if (t[2] < best) best = t[2];
    printf("%d %.3f\n", e, best * 1e9 / steps);
    free(p);
    return 0;
}

```

`p` - случайная циклическая перестановка индексов, построенная алгоритмом Саттоло: обычная перестановка распалась бы на несколько циклов, и вы ходили бы по маленькому из них.

Сборка и замеры:

```bash
gcc -O2 memlat.c -o memlat
for e in 12 14 16 18 20 22 24 26 28; do ./memlat $e; done > memlat.csv
```

График:

```python
import matplotlib.pyplot as plt

rows = [line.split() for line in open("memlat.csv")]
es = [int(r[0]) for r in rows]
ns = [float(r[1]) for r in rows]

plt.plot(es, ns, "o-")
plt.xlabel("размер массива, 2^N байт")
plt.ylabel("латентность, нс")
plt.xticks(es)
plt.grid(True)
plt.savefig("memory-latency.png", dpi=150)
```

1. Размеры от 4 КБ до 1 ГБ, шаг $\times 2$, плюс промежуточные точки около ожидаемых границ.
2. Перед замером один раз пройти весь массив (прогрев, page faults).
3. Число шагов подберите так, чтобы замер шел 0.2-1 секунду.
4. Результат - наносекунды и такты на обращение. Постройте график.

Найдите ступени: L1, L2, L3, DRAM. Сравните с `lscpu`.

## Часть B. Пропускная способность

Чтение (сумма массива) и запись (заполнение) для тех же размеров. Результат - ГБ/с.

```c
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define BYTES (256u * 1024u * 1024u)

static double now(void) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec + ts.tv_nsec * 1e-9;
}

int main(void) {
    uint8_t *a = malloc(BYTES);
    if (!a) return 1;
    volatile uint64_t sink = 0;
    double best_r = 1e30, best_w = 1e30;

    for (int r = 0; r < 5; r++) {
        uint64_t s0 = 0, s1 = 0, s2 = 0, s3 = 0;
        double t0 = now();
        for (size_t i = 0; i + 3 < BYTES; i += 4) {
            s0 += a[i]; s1 += a[i+1]; s2 += a[i+2]; s3 += a[i+3];
        }
        double t = now() - t0;
        if (t < best_r) best_r = t;
        sink ^= s0 ^ s1 ^ s2 ^ s3;
    }

    for (int r = 0; r < 5; r++) {
        double t0 = now();
        for (size_t i = 0; i < BYTES; i++) a[i] = (uint8_t)i;
        double t = now() - t0;
        if (t < best_w) best_w = t;
        sink ^= a[0];
    }

    printf("read %.2f GB/s\n", (double)BYTES / best_r / 1e9);
    printf("write %.2f GB/s\n", (double)BYTES / best_w / 1e9);
    printf("sink %llu\n", (unsigned long long)sink);
    free(a);
    return 0;
}

```

Сборка и запуск:

```bash
gcc -O3 -march=native bandwidth.c -o bandwidth
./bandwidth
```

Для чтения нужен цикл, который не упирается в латентность сложения: несколько аккумуляторов, `-O3 -march=native`.

## Итог: паспорт машины

Соберите таблицу:

```text
| Параметр                | Измерено | lscpu / спецификация |
|-------------------------|----------|----------------------|
| L1d: размер, латентность |          |                      |
| L2:  размер, латентность |          |                      |
| L3:  размер, латентность |          |                      |
| DRAM: латентность        |          |                      |
| Чтение, 1 ядро, ГБ/с     |          |                      |
| Запись, 1 ядро, ГБ/с     |          |                      |
```

Оцените по закону Литтла, сколько промахов одновременно держит ядро:

$$
N_{outstanding} \approx \frac{BW \cdot L_{mem}}{64 \text{ Б}}
$$

## Вопросы

1. Почему латентность L1 не зависит от размера массива, пока он в L1, а потом резко растет?
2. Почему аппаратный префетчер ломает замер латентности при последовательном обходе, и почему случайная перестановка это лечит?
3. Совпал ли ваш паспорт с `lscpu`? Где расхождение и почему?
4. Почему пропускная способность чтения выше пропускной способности записи (или наоборот) на вашей машине?
