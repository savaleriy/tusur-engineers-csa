# Задание 10. Линия, ассоциативность, TLB

!!! warning "WIP"
    Задание в работе: условия и код могут меняться.

> Модуль 10: [Виртуальная память и предвыборка](../modules/10-virtual-memory.md). Нужно: Linux с `perf`, `gcc`, Python с `matplotlib`.

## Цель

Измерить то, что обычно берут из документации: размер кэш-линии, ассоциативность, число одновременных промахов и цену трансляции адресов.

## Что сделать

### 1. Размер кэш-линии

Обходите массив с шагом $s = 1, 2, 4, \dots, 4096$ байт и меряйте время на обращение. Пока шаг меньше линии, каждое обращение попадает в уже загруженную линию и стоит дешево; как только шаг превышает линию, каждое обращение - новый промах. Где кривая перестает расти?

Полная программа:

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

int main(int argc, char **argv) {
    int stride = argc > 1 ? atoi(argv[1]) : 64;
    uint8_t *a = malloc(BYTES);
    if (!a) return 1;
    for (size_t i = 0; i < BYTES; i++) a[i] = (uint8_t)i;

    volatile uint64_t sink = 0;
    double best = 1e30;
    for (int r = 0; r < 3; r++) {
        uint64_t s = 0;
        double t0 = now();
        for (size_t i = 0; i < BYTES; i += (size_t)stride) s += a[i];
        double t = now() - t0;
        if (t < best) best = t;
        sink ^= s;
    }
    double accesses = (double)BYTES / (double)stride;
    printf("%d %.3f %.2f\n", stride, best * 1e9 / accesses,
           (double)BYTES / best / 1e9);
    free(a);
    return 0;
}

```

Сборка и замеры:

```bash
gcc -O3 -march=native stride.c -o stride
for s in 4 8 16 32 64 128 256 512 1024 2048 4096; do ./stride $s; done > stride.csv
```

График:

```python
import matplotlib.pyplot as plt

rows = [line.split() for line in open("stride.csv")]
ss = [int(r[0]) for r in rows]
ns = [float(r[1]) for r in rows]

plt.plot(ss, ns, "o-")
plt.xscale("log", base=2)
plt.xlabel("шаг, байт")
plt.ylabel("нс на обращение")
plt.grid(True)
plt.savefig("stride.png", dpi=150)
```

### 2. Ассоциативность L1

Возьмите $k$ адресов с шагом 4096 байт (они попадают в один набор) и обходите их по кругу. При каком $k$ время резко растет? Это и есть число путей в наборе.

Проверьте, что при шаге 4096 байт конфликт действительно по набору, а не по странице: сравните с шагом 4096 + 64.

### 3. MLP: сколько промахов держит ядро

Запустите $m = 1, 2, 4, 8, 16, 32$ независимых цепочек pointer chasing одновременно на массиве в 1 ГБ. Пропускная способность должна расти, пока ядро не упрется в число одновременных промахов. Оцените это число.

### 4. TLB и огромные страницы

Повторите pointer chasing на 1 ГБ с обычными страницами 4 КБ и с `madvise(MADV_HUGEPAGE)`. Сравните время. Проверьте, что страницы действительно стали большими:

```bash
cat /sys/kernel/mm/transparent_hugepage/enabled
grep AnonHugePages /proc/meminfo
```

## Вопросы

1. Почему промах по конфликту возможен в кэше, который заполнен меньше чем наполовину?
2. Как связаны размер линии, шаг обхода и число промахов?
3. Зачем нужен TLB, если есть кэш данных?
4. Почему огромные страницы ускоряют обход большого массива и когда они вредят?
