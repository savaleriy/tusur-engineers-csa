#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#define N 10000000

static float a[N];

static float kahan(const float *p, size_t n) {
    float s = 0.0f, c = 0.0f;     /* c - накопленная потеря */
    for (size_t i = 0; i < n; i++) {
        float y = p[i] - c;       /* вернуть то, что потеряли в прошлый раз */
        float t = s + y;          /* большое + маленькое: младшие биты y теряются */
        c = (t - s) - y;          /* (t - s) - то, что реально прибавилось; минус y - потеря */
        s = t;
    }
    return s;
}

static float pairwise(size_t lo, size_t hi) {
    if (hi - lo <= 64) {
        float s = 0.0f;
        for (size_t i = lo; i < hi; i++) s += a[i];
        return s;
    }
    size_t mid = lo + (hi - lo) / 2;
    return pairwise(lo, mid) + pairwise(mid, hi);
}

int main(void) {
    for (size_t i = 0; i < N; i++) a[i] = 1.0f / (float)(i + 1);

    float fwd = 0.0f;
    for (size_t i = 0; i < N; i++) fwd += a[i];

    float bwd = 0.0f;
    for (size_t i = N; i-- > 0;) bwd += a[i];

    double exact = 0.0;
    for (size_t i = 0; i < N; i++) exact += 1.0 / (double)(i + 1);

    printf("exact    = %.9f\n", exact);
    printf("forward  = %.9f\n", (double)fwd);
    printf("backward = %.9f\n", (double)bwd);
    printf("pairwise = %.9f\n", (double)pairwise(0, N));
    printf("kahan    = %.9f\n", (double)kahan(a, N));
    return 0;
}
