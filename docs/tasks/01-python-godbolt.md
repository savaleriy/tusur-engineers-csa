# Задание 1. Python, C и Godbolt

> Модуль 1: [Производительность и железо](../modules/01-intro.md). Нужно: Godbolt и Python; для замеров C - локальный `gcc` (см. [Инструменты](../tools.md)).

## Цель

Увидеть, что одна и та же формула может считаться в сотни раз быстрее, и научиться это измерять: несколько запусков, медиана, разброс, график. Один запуск ничего не доказывает.

## Часть 1. Устройство компьютера

1. В Linux выполните `lscpu` (или `cat /proc/cpuinfo`). На macOS - `sysctl -n machdep.cpu.brand_string`, в Windows - `wmic cpu get name`. Выпишите модель, число ядер и потоков, базовую частоту, размеры L1d, L2, L3.
2. Спросите то же у Python:

```python
import os, platform
print(platform.machine())    # x86_64, aarch64, ...
print(os.cpu_count())        # сколько логических ядер
```

`platform.processor()` на Linux часто возвращает пустую строку, поэтому берем `machine()`.

3. Объясните: зачем ядру кэш, почему ядер несколько, что такое такт.
4. Оцените пик одного ядра в GFLOPS по формуле $f \cdot 2 \cdot N_{FMA} \cdot W$, где $W$ - ширина вектора (4 для SSE, 8 для AVX2, 16 для AVX-512). Посмотрите в `lscpu`, есть ли у машины `avx2` или `avx512f`.

## Часть 2. Python: реализуйте умножение сами

В файле `student.py` напишите только функцию умножения матриц:

```python
def matmul(a, b):
    """Вернуть c = a * b для квадратных матриц."""
    raise NotImplementedError("реализуйте умножение матриц")
```

Обвязка `bench.py` уже написана: она генерирует матрицы, делает пять запусков, считает медиану и пишет CSV. Ее менять не нужно.

```python
import csv
import random
import statistics
import time

from student import matmul

n = 256
runs = 5
rows = []

for run in range(1, runs + 1):
    a = [[random.random() for _ in range(n)] for _ in range(n)]
    b = [[random.random() for _ in range(n)] for _ in range(n)]

    t0 = time.perf_counter()
    c = matmul(a, b)
    t = time.perf_counter() - t0

    rows.append((run, t, 2 * n**3 / t / 1e9))
    print(f"run {run}: {t:.3f} s, {rows[-1][2]:.4f} GFLOPS, c[0][0]={c[0][0]:.3f}")

with open("python-matmul.csv", "w", newline="") as f:
    w = csv.writer(f)
    w.writerow(["run", "seconds", "gflops"])
    w.writerows(rows)

med = statistics.median(r[1] for r in rows)
print(f"median: {med:.3f} s, {2 * n**3 / med / 1e9:.4f} GFLOPS")
print(f"оценка для n = 1024: {med * 64:.0f} s")
```

Запуск и график:

```bash
python3 bench.py
python3 plot_python.py
```

`plot_python.py` - готовая обвязка для графика:

```python
import csv

import matplotlib.pyplot as plt

rows = list(csv.DictReader(open("python-matmul.csv")))
xs = [int(r["run"]) for r in rows]
ys = [float(r["seconds"]) for r in rows]

plt.plot(xs, ys, "o-")
plt.xlabel("запуск")
plt.ylabel("время, с")
plt.title("Python: умножение матриц 256x256")
plt.grid(True)
plt.savefig("python-matmul.png", dpi=150)
```

Ответьте: почему оценка для $n = 1024$ получается умножением на 64? Сколько операций делает умножение матриц $1024 \times 1024$?

## Часть 3. C: все шесть порядков циклов

Три вложенных цикла можно переставить шестью способами: `ijk`, `ikj`, `jik`, `jki`, `kij`, `kji`. Число операций у всех одно, но память они обходят по-разному.

### Проверка результата

В CSV есть столбец `control`. Это значение одного элемента результата, `c[7][7]`, а не время и не оценка производительности. Оно нужно, чтобы поймать ошибку в циклах.

Все шесть порядков считают один и тот же элемент `c[7][7]`: для фиксированных `i = 7` и `j = 7` сумма по `k` идет в одном и том же порядке. Поэтому у правильных реализаций `control` должен совпасть у всех шести порядков бит в бит. Если там ноль, цикл не записывает результат; если значения разные, перепутаны индексы или порядок накопления.

Индекс `7` выбран просто как удобный: он существует при всех используемых размерах матрицы (от 128). Сумму всей матрицы для проверки не берем: разные порядки циклов могут сложить элементы в разном порядке и получить разные последние биты, хотя матрица посчитана правильно.

Ниже - только обвязка: таймер, генерация матриц, цикл по размерам и печать CSV. Шесть функций `mul_*` пустые; циклы в них пишете вы.

```c
#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define MAXN 768
#define RUNS 5

static int n;
static double a[MAXN][MAXN], b[MAXN][MAXN], c[MAXN][MAXN];

static double now(void) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec + ts.tv_nsec * 1e-9;
}

static void init(void) {
    for (int i = 0; i < n; i++)
        for (int j = 0; j < n; j++) {
            a[i][j] = (double)rand() / RAND_MAX;
            b[i][j] = (double)rand() / RAND_MAX;
        }
}

/* TODO: заполните шесть функций циклами в нужном порядке. */
static void mul_ijk(void) { }
static void mul_ikj(void) { }
static void mul_jik(void) { }
static void mul_jki(void) { }
static void mul_kij(void) { }
static void mul_kji(void) { }

typedef void (*mul_fn)(void);

int main(void) {
    const int sizes[] = {128, 256, 384, 512};
    const int nsizes = (int)(sizeof sizes / sizeof sizes[0]);
    struct { const char *name; mul_fn fn; } methods[] = {
        {"ijk", mul_ijk}, {"ikj", mul_ikj}, {"jik", mul_jik},
        {"jki", mul_jki}, {"kij", mul_kij}, {"kji", mul_kji},
    };

    printf("n,method,run,seconds,gflops,control\n");
    for (int s = 0; s < nsizes; s++) {
        n = sizes[s];
        init();
        for (int m = 0; m < 6; m++) {
            for (int r = 1; r <= RUNS; r++) {
                for (int i = 0; i < n; i++)
                    for (int j = 0; j < n; j++)
                        c[i][j] = 0.0;

                double t0 = now();
                methods[m].fn();
                double t = now() - t0;

                printf("%d,%s,%d,%.6f,%.3f,%.9f\n",
                       n, methods[m].name, r, t,
                       2.0 * n * n * n / t / 1e9, c[7][7]);
            }
        }
    }
    return 0;
}
```

Сборка и запуск:

```bash
gcc -O2 matmul.c -o matmul
./matmul > matmul.csv
```

Для сравнения флагов соберите еще `-O0` и `-O3 -march=native` в отдельные бинарники и сохраните отдельные CSV.

График зависимости времени от размера для всех шести порядков:

```python
import csv
import statistics

import matplotlib.pyplot as plt

data = {}
with open("matmul.csv") as f:
    for row in csv.DictReader(f):
        key = (row["method"], int(row["n"]))
        data.setdefault(key, []).append(float(row["seconds"]))

methods = ["ijk", "ikj", "jik", "jki", "kij", "kji"]
colors  = ["#0072B2", "#E69F00", "#009E73", "#D55E00", "#CC79A7", "#56B4E9"]
markers = ["o", "s", "^", "D", "v", "P"]

for method, color, marker in zip(methods, colors, markers):
    ns = sorted(n for (m, n) in data if m == method)
    med = [statistics.median(data[(method, n)]) for n in ns]
    plt.plot(ns, med, marker=marker, color=color, label=method)

plt.xscale("log")
plt.yscale("log")
plt.xlabel("размер матрицы n")
plt.ylabel("медиана времени, с")
plt.legend()
plt.grid(True, which="both")
plt.tight_layout()
plt.savefig("matmul-orders.png", dpi=150)
```

На логарифмическом графике кубическая зависимость выглядит прямой с наклоном около 3. Если наклон другой, объясните почему.

Для ассемблера используйте Godbolt: `x86-64 gcc`, `-O2`, `-masm=intel`. Сравните внутренние циклы `ijk` и `ikj`: какие обращения к `b` идут подряд, а какие с шагом в строку.

## Часть 4. Сопоставьте все шесть

Заполните таблицу: строки - порядки, столбцы - размеры, в клетках - медиана времени. Ответьте, какие порядки быстрее на маленьких матрицах, какие на больших и как меняется разрыв. Сравните с Python из части 2.

## Часть 5. Штрассен: меньше умножений

В модуле 1 разобраны формулы Штрассена: семь умножений вместо восьми для блоков $2 \times 2$ и сложность $O(n^{\log_2 7})$. Здесь нужно реализовать его самостоятельно.

Готовая обвязка: генерация матриц, эталонное умножение, таймер, пять запусков и проверка максимального расхождения. Функция `strassen` пустая - ее пишете вы.

```c
#define _POSIX_C_SOURCE 200809L
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define MAXN 1024
#define RUNS 5

static int n;
static double a[MAXN][MAXN], b[MAXN][MAXN];
static double c[MAXN][MAXN], ref[MAXN][MAXN];

static double now(void) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec + ts.tv_nsec * 1e-9;
}

static void init(void) {
    for (int i = 0; i < n; i++)
        for (int j = 0; j < n; j++) {
            a[i][j] = (double)rand() / RAND_MAX;
            b[i][j] = (double)rand() / RAND_MAX;
        }
}

/* Эталон: обычное умножение, нужно для проверки и как базовый случай. */
static void naive(double out[MAXN][MAXN]) {
    for (int i = 0; i < n; i++)
        for (int j = 0; j < n; j++) {
            double s = 0;
            for (int k = 0; k < n; k++)
                s += a[i][k] * b[k][j];
            out[i][j] = s;
        }
}

/* TODO: реализуйте алгоритм Штрассена; результат положите в c. */
static void strassen(void) {
}

static double maxdiff(void) {
    double d = 0;
    for (int i = 0; i < n; i++)
        for (int j = 0; j < n; j++) {
            double x = fabs(c[i][j] - ref[i][j]);
            if (x > d) d = x;
        }
    return d;
}

int main(void) {
    const int sizes[] = {128, 256, 512, 1024};
    const int nsizes = (int)(sizeof sizes / sizeof sizes[0]);

    printf("n,method,run,seconds,gflops,maxdiff\n");
    for (int s = 0; s < nsizes; s++) {
        n = sizes[s];
        init();
        naive(ref);

        for (int m = 0; m < 2; m++) {
            const char *name = m ? "strassen" : "naive";
            for (int r = 1; r <= RUNS; r++) {
                for (int i = 0; i < n; i++)
                    for (int j = 0; j < n; j++)
                        c[i][j] = 0.0;

                double t0 = now();
                if (m) strassen();
                else   naive(c);
                double t = now() - t0;

                printf("%d,%s,%d,%.6f,%.3f,%.3e\n",
                       n, name, r, t, 2.0 * n * n * n / t / 1e9, maxdiff());
            }
        }
    }
    return 0;
}

```

Сборка и запуск:

```bash
gcc -O2 strassen.c -o strassen -lm
./strassen > strassen.csv
```

`naive` - эталон для проверки и возможный базовый случай, а не решение Штрассена: саму рекурсию и семь произведений пишете вы.

Матрицы квадратные, размер - степень двойки. Если размер не степень двойки, дополните матрицу нулями. Порог, начиная с которого вызывается обычное умножение, выберите сами; `naive` можно использовать как эталон и как базовый случай, но для подматриц придется написать свою функцию или работать с указателями и шагом строки.

График:

```python
import csv
import statistics

import matplotlib.pyplot as plt

data = {}
with open("strassen.csv") as f:
    for row in csv.DictReader(f):
        key = (row["method"], int(row["n"]))
        data.setdefault(key, []).append(float(row["seconds"]))

for method in ["naive", "strassen"]:
    ns = sorted(n for (m, n) in data if m == method)
    med = [statistics.median(data[(method, n)]) for n in ns]
    plt.plot(ns, med, "o-", label=method)

plt.xscale("log", base=2)
plt.yscale("log")
plt.xlabel("размер матрицы n")
plt.ylabel("медиана времени, с")
plt.legend()
plt.grid(True, which="both")
plt.savefig("strassen.png", dpi=150)
```

Сравните не только время, но и `maxdiff`: у Штрассена другой порядок сложений, поэтому с эталоном он совпадает не бит в бит, а с точностью до нескольких последних битов. Если `maxdiff` большой, ошибка в формулах или в разбиении на блоки.

## Критерии готовности

- Четыре размера, шесть порядков, пять запусков каждого - все в CSV.
- Построен логарифмический график `matmul-orders.png` с шестью линиями.
- `control` (`c[7][7]`) совпадает у всех порядков при одном размере.
- Объяснено, какие обращения к `b` последовательные, а какие с шагом.
- Для `ijk` и `ikj` найден внутренний цикл в ассемблере.
- Штрассен реализован, `maxdiff` меньше $10^{-6}$, график `strassen.png` построен.

## Вопросы

1. Какой порядок самый быстрый на $n = 128$? А на $n = 512$? Изменился ли рейтинг?
2. Почему `ikj` и `kij` быстрее `ijk` и `jik`? Что происходит с `b[k][j]` в каждом случае?
3. Как время зависит от $n$? Оцените наклон графика в логарифмических координатах и сравните с $O(n^3)$.
4. Почему Python медленнее C? Видна ли на Python та же разница между порядками циклов?
5. Что проверяет `control` и почему он должен совпадать?
6. Почему Штрассен делает меньше умножений, но на маленьких матрицах может проигрывать обычному умножению?
7. На каком размере Штрассен обогнал `naive` на вашей машине? Если не обогнал, какие накладные расходы это объясняют?
8. Сколько GFLOPS дает лучший порядок на вашей машине? Сравните с пиком из части 1.

## Что попробовать дальше

Проверьте, меняются ли выводы при `-O3 -march=native`, на другом процессоре и при другом seed. Для Штрассена подберите порог перехода к обычному умножению: оптимум может отличаться от 64.
