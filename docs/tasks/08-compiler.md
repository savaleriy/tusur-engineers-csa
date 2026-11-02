# Задание 8. Компилятор и неопределенное поведение

!!! warning "WIP"
    Задание в работе: условия и код могут меняться.

> Модуль 8: [Компилятор: флаги, UB и контракты](../modules/08-compiler.md). Нужно: `gcc` или `clang`, Godbolt, Python с `matplotlib`.

## Цель

Понять, что компилятор оптимизирует только то, что может доказать, и что неопределенное поведение - это не *мелкая придирка*, а разрешение выбросить ваш код целиком.

## Что сделать

### 1. Флаги

Полная программа для замеров:

```c
#include <stdint.h>
#include <stdio.h>
#include <time.h>

#define N 1000000

static int32_t a[N], b[N], c[N];
static volatile int32_t sink;

static double now(void) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec + ts.tv_nsec * 1e-9;
}

__attribute__((noinline))
static void add_arrays(int32_t *x, const int32_t *y, const int32_t *z, size_t n) {
    for (size_t i = 0; i < n; i++)
        x[i] = y[i] + z[i];
}

int main(void) {
    for (size_t i = 0; i < N; i++) {
        b[i] = (int32_t)i;
        c[i] = (int32_t)(2 * i);
    }
    double t[5];
    for (int r = 0; r < 5; r++) {
        double t0 = now();
        add_arrays(a, b, c, N);
        t[r] = now() - t0;
    }
    for (int i = 0; i < 5; i++)
        for (int j = i + 1; j < 5; j++)
            if (t[j] < t[i]) { double z = t[i]; t[i] = t[j]; t[j] = z; }
    sink = a[N - 1];
    printf("%.6f\n", t[2] * 1e9 / N);
    return 0;
}

```

Соберите ее с разными флагами и измерьте время:

```bash
gcc -O0 optbench.c -o opt_O0
gcc -O1 optbench.c -o opt_O1
gcc -O2 optbench.c -o opt_O2
gcc -O3 optbench.c -o opt_O3
gcc -O3 -march=native optbench.c -o opt_native

for b in O0 O1 O2 O3 native; do echo -n "$b "; ./opt_$b; done
size opt_O0 opt_O1 opt_O2 opt_O3 opt_native
```

График времени:

```python
import subprocess
import matplotlib.pyplot as plt

labels = ["-O0", "-O1", "-O2", "-O3", "-O3 native"]
bins   = ["opt_O0", "opt_O1", "opt_O2", "opt_O3", "opt_native"]
times  = [float(subprocess.check_output([f"./{b}"], text=True)) for b in bins]

plt.bar(labels, times)
plt.ylabel("нс на элемент")
plt.xticks(rotation=30)
plt.savefig("opt-levels.png", dpi=150)
```

Для каждой сборки запишите время, размер `.text` и наличие векторных инструкций (`ymm`/`zmm` или `v` в RISC-V).

Объясните, за счет чего меняется время и какие флаги дают реальный выигрыш именно здесь.

### 2. Неопределенное поведение

Для каждого фрагмента соберите программу с `-O2` и объясните, что произошло. Где компилятор *съел* проверку - докажите по ассемблеру.

```c
#include <stddef.h>
#include <stdio.h>

int f(int x) { return x + 1 > x; }          /* знаковое переполнение */

int g(int x) { return x << 8; }             /* сдвиг отрицательного */

size_t h(size_t x) { return x << 64; }      /* сдвиг на ширину типа */

int main(void) {
    printf("f: %d\n", f(2147483647));
    printf("g: %d\n", g(-1));
    printf("h: %zu\n", h(1));
    return 0;
}
```

Соберите с `-O2` и с санитайзером, запустите оба варианта:

```bash
gcc -O2 ub.c -o ub
gcc -O2 -fsanitize=undefined -fno-sanitize-recover=all ub.c -o ub_san
./ub
./ub_san
```

Санитайзер печатает файл, строку и вид UB, а затем останавливает программу (для этого нужен `-fno-sanitize-recover=all`). Без санитайзера та же программа может напечатать что угодно. Обратите внимание: `f` компилятор мог свернуть в константу еще до выполнения, и тогда санитайзер ее не увидит.

### 3. Алиасинг и `restrict`

Соберите цикл:

```c
void add_arrays(int *a, const int *b, const int *c, int n) {
    for (int i = 0; i < n; i++)
        a[i] = b[i] + c[i];
}
```

с `-O2` и с `-O2 -fno-strict-aliasing`, затем тот же цикл с `restrict` у всех указателей. Найдите в ассемблере признаки "versioned loop" или векторизации. Объясните, чего боится компилятор.

### 4. Inline asm

Напишите функцию, которая складывает два числа через `asm`:

```c
long add_asm(long a, long b) {
    long r;
    __asm__ ("add %0, %1, %2" : "=r"(r) : "r"(a), "r"(b));
    return r;
}
```

Соберите и проверьте результат. Затем прочитайте счетчик тактов (`rdtsc` на x86 или `rdcycle` на RISC-V) и объясните, почему его показания - не всегда такты.

## Вопросы

1. Почему `x + 1 > x` компилятор заменяет на константу 1? В каком стандарте это разрешено?
2. Чем `restrict` помогает компилятору и в каком случае он опасен?
3. Почему `-ffast-math` может сломать программу, которая считает деньги или физику?
4. Когда inline asm действительно нужен, а когда это вредит?
