#include <pmmintrin.h>
#include <stdio.h>
#include <time.h>
#include <xmmintrin.h>

#define N 2000000
#define K 16

static volatile float sink;

static float work(float base) {
    float x[K];
    for (int k = 0; k < K; k++) x[k] = base;
    for (int i = 0; i < N; i++)
        for (int k = 0; k < K; k++)
            x[k] = 0.999f * x[k] + 0.001f * base;
    float s = 0.0f;
    for (int k = 0; k < K; k++) s += x[k];
    sink = s;
    return s;
}

static double now(void) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec + ts.tv_nsec * 1e-9;
}

static void bench(const char *name, float base) {
    double t[5];
    for (int r = 0; r < 5; r++) {
        double t0 = now();
        work(base);
        t[r] = now() - t0;
    }
    for (int i = 0; i < 5; i++)
        for (int j = i + 1; j < 5; j++)
            if (t[j] < t[i]) { double tmp = t[i]; t[i] = t[j]; t[j] = tmp; }
    printf("%-26s %.4f s  (", name, t[2]);
    for (int i = 0; i < 5; i++) printf(" %.4f", t[i]);
    printf(" )\n");
}

int main(void) {
    bench("x ~ 1", 1.0f);
    bench("x ~ 1e-39", 1e-39f);
    _MM_SET_FLUSH_ZERO_MODE(_MM_FLUSH_ZERO_ON);
    _MM_SET_DENORMALS_ZERO_MODE(_MM_DENORMALS_ZERO_ON);
    bench("x ~ 1, FTZ/DAZ", 1.0f);
    bench("x ~ 1e-39, FTZ/DAZ", 1e-39f);
    return 0;
}
