# Задание 7. Ветвления и раскладка кода

!!! warning "WIP"
    Задание в работе: условия и код могут меняться.

> Модуль 7: [Ветвления, косвенные вызовы и раскладка кода](../modules/07-branches.md). Нужно: `gcc`, `perf` (аппаратные счетчики), Python с `matplotlib`.

## Цель

Измерить цену неверно предсказанного перехода, убрать ее и понять, когда branchless-код проигрывает.

## Часть A. Порог P

Полный стенд: три версии суммы и режимы запуска.

```c
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define N 1000000

static int32_t a[N];
static volatile int64_t sink;

static double now(void) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec + ts.tv_nsec * 1e-9;
}

static int64_t sum_branchy(const int32_t *v, size_t n, int p) {
    int64_t s = 0;
    for (size_t i = 0; i < n; i++)
        if (v[i] < p)
            s += v[i];
    return s;
}

static int64_t sum_cmov(const int32_t *v, size_t n, int p) {
    int64_t s = 0;
    for (size_t i = 0; i < n; i++)
        s += (v[i] < p) ? v[i] : 0;
    return s;
}

static int64_t sum_arith(const int32_t *v, size_t n, int p) {
    int64_t s = 0;
    for (size_t i = 0; i < n; i++)
        s += v[i] * (v[i] < p);
    return s;
}

static int cmp_int(const void *x, const void *y) {
    int32_t u = *(const int32_t *)x, v = *(const int32_t *)y;
    return (u > v) - (u < v);
}

int main(int argc, char **argv) {
    int p = argc > 1 ? atoi(argv[1]) : 50;
    int sorted = argc > 2 ? atoi(argv[2]) : 0;
    int variant = argc > 3 ? atoi(argv[3]) : 0;
    srand(42);
    for (size_t i = 0; i < N; i++) a[i] = rand() % 100;
    if (sorted) qsort(a, N, sizeof a[0], cmp_int);

    double t[5];
    for (int r = 0; r < 5; r++) {
        double t0 = now();
        if (variant == 0)      sink = sum_branchy(a, N, p);
        else if (variant == 1) sink = sum_cmov(a, N, p);
        else                   sink = sum_arith(a, N, p);
        t[r] = now() - t0;
    }
    for (int i = 0; i < 5; i++)
        for (int j = i + 1; j < 5; j++)
            if (t[j] < t[i]) { double z = t[i]; t[i] = t[j]; t[j] = z; }
    printf("%.6f %lld\n", t[2] * 1e9 / N, (long long)sink);
    return 0;
}

```

Соберите два бинарника: один с запретом if-conversion для branchy-версии, другой обычный для `cmov` и арифметики.

```bash
gcc -O2 -fno-tree-vectorize -fno-if-conversion -fno-if-conversion2 branch_bench.c -o bench_branchy
gcc -O2 -fno-tree-vectorize branch_bench.c -o bench_fast
```

Проверка:

```bash
./bench_branchy 50 0 0     # branchy, случайный массив
./bench_branchy 50 1 0     # branchy, отсортированный
./bench_fast   50 0 1     # cmov/?:, случайный
```

Третий аргумент: 0 - branchy, 1 - `?:`, 2 - арифметика. Второй: 0 - случайный порядок, 1 - отсортированный.

График времени от порога:

```python
import subprocess
import matplotlib.pyplot as plt

ps = list(range(0, 101, 5))

def run(binary, p, sorted_flag, variant):
    out = subprocess.check_output([binary, str(p), str(sorted_flag), str(variant)], text=True)
    return float(out.split()[0])

branchy = [run("./bench_branchy", p, 0, 0) for p in ps]
sorted_b = [run("./bench_branchy", p, 1, 0) for p in ps]
cmov = [run("./bench_fast", p, 0, 1) for p in ps]

plt.plot(ps, branchy, "o-", label="branchy, случайный")
plt.plot(ps, sorted_b, "s-", label="branchy, отсортированный")
plt.plot(ps, cmov, "^-", label="cmov/?:, случайный")
plt.xlabel("порог P")
plt.ylabel("нс на элемент")
plt.xticks(range(0, 101, 10))
plt.grid(True)
plt.legend()
plt.savefig("branch-time.png", dpi=150)
```

Снимите `branch-misses` для `p = 0, 50, 100`:

```bash
perf stat -e branch-misses ./bench_branchy 50 0 0
```

Оцените штраф:

$$
C_{miss} \approx \frac{T(P = 50) - T(P = 0)}{N_{miss}(P = 50)} \cdot f
$$

Проверьте ассемблер: компилятор мог сам превратить `?:` в переход, а branchy - в `cmov`. Ищите `cmov`, `setl`, `setb`.

## Часть B. Бинарный поиск

Полная программа: стандартный, branchless и prefetch-варианты.

```c
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define Q 1000000

static const int32_t *lb_std(const int32_t *a, size_t n, int32_t x) {
    size_t l = 0, r = n;
    while (l < r) {
        size_t m = (l + r) / 2;
        if (a[m] < x) l = m + 1;
        else          r = m;
    }
    return a + l;
}

static const int32_t *lb_branchless(const int32_t *a, size_t n, int32_t x) {
    while (n > 1) {
        size_t half = n / 2;
        a += (a[half - 1] < x) * half;
        n -= half;
    }
    return a + (*a < x);
}

static const int32_t *lb_prefetch(const int32_t *a, size_t n, int32_t x) {
    while (n > 1) {
        size_t half = n / 2;
        __builtin_prefetch(a + half / 2 - 1);
        __builtin_prefetch(a + half + half / 2 - 1);
        a += (a[half - 1] < x) * half;
        n -= half;
    }
    return a + (*a < x);
}

static double now(void) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec + ts.tv_nsec * 1e-9;
}

typedef const int32_t *(*lb_fn)(const int32_t *, size_t, int32_t);

static void bench(const char *name, lb_fn f, const int32_t *a, size_t n,
                  const int32_t *q) {
    double t[5];
    volatile intptr_t sink = 0;
    for (int r = 0; r < 5; r++) {
        intptr_t s = 0;
        double t0 = now();
        for (size_t i = 0; i < Q; i++) s += f(a, n, q[i]) - a;
        t[r] = now() - t0;
        sink ^= s;
    }
    for (int i = 0; i < 5; i++)
        for (int j = i + 1; j < 5; j++)
            if (t[j] < t[i]) { double z = t[i]; t[i] = t[j]; t[j] = z; }
    printf("%7zu %-10s %8.2f\n", n, name, t[2] * 1e9 / Q);
    (void)sink;
}

int main(void) {
    srand(42);
    static int32_t q[Q];
    for (size_t i = 0; i < Q; i++) q[i] = rand();

    for (int e = 10; e <= 24; e++) {
        size_t n = (size_t)1 << e;
        int32_t *a = malloc(n * sizeof *a);
        if (!a) return 1;
        for (size_t i = 0; i < n; i++) a[i] = (int32_t)(2 * i);
        for (size_t i = 0; i < Q; i++) q[i] %= (int32_t)n;

        printf("# n = %zu\n", n);
        bench("std", lb_std, a, n, q);
        bench("branchless", lb_branchless, a, n, q);
        bench("prefetch", lb_prefetch, a, n, q);
        free(a);
    }
    return 0;
}

```

Сборка и запуск:

```bash
gcc -O2 -fno-tree-vectorize bsearch.c -o bsearch
./bsearch > bsearch.csv
```

График:

```python
import matplotlib.pyplot as plt

rows = []
for line in open("bsearch.csv"):
    line = line.strip()
    if not line or line.startswith("#"):
        continue
    n, name, ns = line.split()
    rows.append((int(n), name, float(ns)))

for name in ["std", "branchless", "prefetch"]:
    xs = [r[0] for r in rows if r[1] == name]
    ys = [r[2] for r in rows if r[1] == name]
    plt.plot(xs, ys, "o-", label=name)

plt.xscale("log", base=2)
plt.xlabel("размер массива n")
plt.ylabel("нс на запрос")
plt.grid(True)
plt.legend()
plt.savefig("binary-search.png", dpi=150)
```

Размеры от $2^{10}$ до $2^{24}$; при желании добавьте $2^{26}$. В `prefetch` префетчатся обе возможные следующие середины: `a + half / 2 - 1` и `a + half + half / 2 - 1`.

Объясните, где branchless выигрывает, где проигрывает и почему. Дополнительно: `__builtin_prefetch` не всегда помогает - проверьте по графику, а не по названию.

## Продвинутый уровень

Раскладка Эйтцингера: `build_eytzinger` и `lb_eytzinger` с предвыборкой. Сравните со всеми версиями на всех размерах.

## Косвенные вызовы и layout

Соберите `switch` на восемь значений и вызов через указатель на функцию. Найдите в ассемблере таблицу переходов и `call *%rax` (x86) или `jalr` (RISC-V). Сравните, как предсказывается прямой и косвенный вызов. Затем сдвиньте целевую функцию на несколько байт (`-falign-functions=...`) и посмотрите, изменилось ли время.

## Критерии готовности

- `bench_branchy` и `bench_fast` собраны, три режима запускаются.
- Построен график `branch-time.png` для случайного и отсортированного массивов.
- Сняты `branch-misses`, оценен штраф $C_{miss}$.
- `bsearch` собран, график `binary-search.png` построен.
- Объяснено, где branchless проигрывает и почему.

## Вопросы

1. Почему время `branchy` зависит от `P`, а `cmov` - нет?
2. Почему на отсортированном массиве `branchy` быстрее, чем на случайном, даже при том же числе сравнений?
3. Почему branchless-поиск может проиграть обычному на маленьких массивах и выиграть на больших?
4. Сколько тактов стоит промах предсказания на вашей машине? Совпадает ли оценка с таблицей из модуля 6?
