/*
 * heat.c - остывание квадратной пластины из композитного материала.
 *
 * Модель. Пластина N x N, каждая ячейка - металл или полимер. Материал
 * задается генератором xorshift32 с фиксированным seed 12345. Границы
 * пластины поддерживаются при нулевой температуре, внутренние ячейки
 * обновляются явной схемой для уравнения теплопроводности:
 *
 *     T'[i][j] = T[i][j]
 *         + cn * (T[i-1][j] - T[i][j]) + cs * (T[i+1][j] - T[i][j])
 *         + cw * (T[i][j-1] - T[i][j]) + ce * (T[i][j+1] - T[i][j])
 *
 * где c = 0.5 * (alpha[i][j] + alpha[сосед]) - коэффициент на границе
 * двух ячеек, alpha = 0.24 для металла и 0.0024 для полимера. Сумма
 * коэффициентов не превосходит 0.96, поэтому схема устойчива и не
 * выходит за пределы начального диапазона температур.
 *
 * Начальная температура: линейный градиент от 100 у левой границы
 * к 0 у правой. Каждые 10 шагов считаются полная теплота, максимум и
 * гистограмма температур по внутренним ячейкам.
 *
 * Сборка и запуск:
 *
 *     gcc -O2 -g heat.c -o heat -lm
 *     ./heat --n 512  --steps 100
 *     ./heat --n 2048 --steps 200 --out my.txt
 */

#include <errno.h>
#include <limits.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define STATS_EVERY   10
#define HIST_BINS     20
#define HIST_WIDTH    5.0f

#define ALPHA_METAL   0.24f
#define ALPHA_POLYMER 0.0024f
#define SEED          12345u

#define N_MIN         8
#define N_MAX         4096
#define STEPS_MIN     10
#define STEPS_MAX     100000

struct stats {
    float total;
    float tmax;
    int hist[HIST_BINS];
};

static uint32_t rng_next(uint32_t *state) {
    uint32_t x = *state;
    x ^= x << 13;
    x ^= x >> 17;
    x ^= x << 5;
    *state = x;
    return x;
}

static void init_materials(int *metal, size_t cells) {
    uint32_t state = SEED;
    for (size_t k = 0; k < cells; k++)
        metal[k] = (int)(rng_next(&state) & 1u);
}

static void init_temperature(float *t, int n) {
    for (int i = 1; i < n - 1; i++) {
        for (int j = 1; j < n - 1; j++) {
            t[(size_t)i * (size_t)n + (size_t)j] =
                100.0f * (float)(n - 1 - j) / (float)(n - 1);
        }
    }
}

/* Не встраиваем: иначе perf покажет все время в main. */
__attribute__((noinline))
static void step(const float *cur, float *next, const int *metal, int n) {
    for (int j = 1; j < n - 1; j++) {
        for (int i = 1; i < n - 1; i++) {
            size_t k = (size_t)i * (size_t)n + (size_t)j;

            float am = metal[k] ? ALPHA_METAL : ALPHA_POLYMER;
            float an = metal[k - 1] ? ALPHA_METAL : ALPHA_POLYMER;
            float as = metal[k + 1] ? ALPHA_METAL : ALPHA_POLYMER;
            float aw = metal[k - (size_t)n] ? ALPHA_METAL : ALPHA_POLYMER;
            float ae = metal[k + (size_t)n] ? ALPHA_METAL : ALPHA_POLYMER;

            float cn = 0.5f * (am + an);
            float cs = 0.5f * (am + as);
            float cw = 0.5f * (am + aw);
            float ce = 0.5f * (am + ae);
            float v = cur[k];

            next[k] = v
                + cn * (cur[k - 1] - v)
                + cs * (cur[k + 1] - v)
                + cw * (cur[k - (size_t)n] - v)
                + ce * (cur[k + (size_t)n] - v);
        }
    }
}

/* Не встраиваем: хотим видеть отдельную долю времени в perf. */
__attribute__((noinline))
static float total_heat(const float *t, int n) {
    float total = 0.0f;

    for (int j = 1; j < n - 1; j++) {
        for (int i = 1; i < n - 1; i++)
            total += t[(size_t)i * (size_t)n + (size_t)j];
    }
    return total;
}

/* Не встраиваем: хотим видеть отдельную долю времени в perf. */
__attribute__((noinline))
static float max_temp(const float *t, int n) {
    float tmax = 0.0f;

    for (int j = 1; j < n - 1; j++) {
        for (int i = 1; i < n - 1; i++) {
            float v = t[(size_t)i * (size_t)n + (size_t)j];
            if (v > tmax)
                tmax = v;
        }
    }
    return tmax;
}

/* Не встраиваем: хотим видеть отдельную долю времени в perf. */
__attribute__((noinline))
static void temperature_hist(const float *t, int n, int *hist) {
    memset(hist, 0, HIST_BINS * sizeof *hist);

    for (int j = 1; j < n - 1; j++) {
        for (int i = 1; i < n - 1; i++) {
            float v = t[(size_t)i * (size_t)n + (size_t)j];
            int b;

            if (v < 1.0f * HIST_WIDTH)       b = 0;
            else if (v < 2.0f * HIST_WIDTH)  b = 1;
            else if (v < 3.0f * HIST_WIDTH)  b = 2;
            else if (v < 4.0f * HIST_WIDTH)  b = 3;
            else if (v < 5.0f * HIST_WIDTH)  b = 4;
            else if (v < 6.0f * HIST_WIDTH)  b = 5;
            else if (v < 7.0f * HIST_WIDTH)  b = 6;
            else if (v < 8.0f * HIST_WIDTH)  b = 7;
            else if (v < 9.0f * HIST_WIDTH)  b = 8;
            else if (v < 10.0f * HIST_WIDTH) b = 9;
            else if (v < 11.0f * HIST_WIDTH) b = 10;
            else if (v < 12.0f * HIST_WIDTH) b = 11;
            else if (v < 13.0f * HIST_WIDTH) b = 12;
            else if (v < 14.0f * HIST_WIDTH) b = 13;
            else if (v < 15.0f * HIST_WIDTH) b = 14;
            else if (v < 16.0f * HIST_WIDTH) b = 15;
            else if (v < 17.0f * HIST_WIDTH) b = 16;
            else if (v < 18.0f * HIST_WIDTH) b = 17;
            else if (v < 19.0f * HIST_WIDTH) b = 18;
            else                             b = 19;

            hist[b]++;
        }
    }
}

static void print_stats(FILE *f, int step_no, const struct stats *s) {
    fprintf(f, "%d %.6f %.6f", step_no, (double)s->total, (double)s->tmax);
    for (int b = 0; b < HIST_BINS; b++)
        fprintf(f, " %d", s->hist[b]);
    fputc('\n', f);
}

static void usage(FILE *f, const char *prog) {
    fprintf(f, "usage: %s [--n N] [--steps S] [--out FILE]\n", prog);
    fprintf(f, "  --n N       размер сетки, %d..%d (по умолчанию 512)\n",
            N_MIN, N_MAX);
    fprintf(f, "  --steps S   число шагов, %d..%d (по умолчанию 100)\n",
            STEPS_MIN, STEPS_MAX);
    fprintf(f, "  --out FILE  записать результат в файл\n");
    fprintf(f, "  -h, --help  показать эту справку\n");
}

static int parse_int(const char *text, int *value) {
    char *end = NULL;
    errno = 0;
    long v = strtol(text, &end, 10);

    if (errno != 0 || end == text || *end != '\0' || v < INT_MIN || v > INT_MAX)
        return -1;
    *value = (int)v;
    return 0;
}

/* 0 - успех, 1 - показана справка, -1 - ошибка. */
static int parse_args(int argc, char **argv, int *n, int *steps,
                      const char **out_path) {
    for (int k = 1; k < argc; k++) {
        if (!strcmp(argv[k], "--n")) {
            if (++k == argc || parse_int(argv[k], n) != 0)
                return -1;
        } else if (!strcmp(argv[k], "--steps")) {
            if (++k == argc || parse_int(argv[k], steps) != 0)
                return -1;
        } else if (!strcmp(argv[k], "--out")) {
            if (++k == argc)
                return -1;
            *out_path = argv[k];
        } else if (!strcmp(argv[k], "-h") || !strcmp(argv[k], "--help")) {
            usage(stdout, argv[0]);
            return 1;
        } else {
            return -1;
        }
    }

    if (*n < N_MIN || *n > N_MAX || *steps < STEPS_MIN || *steps > STEPS_MAX)
        return -1;
    return 0;
}

int main(int argc, char **argv) {
    int n = 512;
    int steps = 100;
    const char *out_path = NULL;

    int rc = parse_args(argc, argv, &n, &steps, &out_path);
    if (rc == 1)
        return 0;
    if (rc != 0) {
        usage(stderr, argv[0]);
        return 2;
    }

    size_t cells = (size_t)n * (size_t)n;
    float *cur = calloc(cells, sizeof *cur);
    float *next = calloc(cells, sizeof *next);
    int *metal = malloc(cells * sizeof *metal);
    if (!cur || !next || !metal) {
        fprintf(stderr, "не хватило памяти на сетку %d x %d\n", n, n);
        free(cur);
        free(next);
        free(metal);
        return 1;
    }

    init_materials(metal, cells);
    init_temperature(cur, n);

    FILE *out = stdout;
    if (out_path != NULL) {
        out = fopen(out_path, "w");
        if (!out) {
            fprintf(stderr, "не удалось открыть %s: %s\n", out_path,
                    strerror(errno));
            free(cur);
            free(next);
            free(metal);
            return 1;
        }
    }

    fprintf(out, "n=%d steps=%d seed=%u\n", n, steps, SEED);
    fprintf(out, "# step total_heat max_temp hist0..hist19\n");

    struct stats st;
    for (int s = 1; s <= steps; s++) {
        step(cur, next, metal, n);

        for (int i = 0; i < n; i++) {
            for (int j = 0; j < n; j++)
                cur[(size_t)i * (size_t)n + (size_t)j] =
                    next[(size_t)i * (size_t)n + (size_t)j];
        }

        if (s % STATS_EVERY == 0) {
            st.total = total_heat(cur, n);
            st.tmax = max_temp(cur, n);
            temperature_hist(cur, n, st.hist);
            print_stats(out, s, &st);
        }
    }

    if (out != stdout)
        fclose(out);

    free(cur);
    free(next);
    free(metal);
    return 0;
}
