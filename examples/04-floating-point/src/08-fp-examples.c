#include <math.h>
#include <stdio.h>

float f2c(float fahr) {
    return (5.0f / 9.0f) * (fahr - 32.0f);
}

void sqrt_demo(int src, float *dst, int *idst) {
    float x = (float)src;
    if (x >= 0.0f)
        x = __builtin_sqrtf(x);
    *dst = x;
    *idst = (int)x;
}

double e_const(double eps) {
    double e = 1.0, term = 1.0;
    for (int n = 1; term >= eps; n++) {
        term /= n;
        e += term;
    }
    return e;
}

long double add_ld(long double a, long double b) {
    return a + b;
}

int main(void) {
    printf("f2c(32)  = %.9g\n", f2c(32.0f));
    printf("f2c(212) = %.9g\n", f2c(212.0f));
    printf("f2c(-40) = %.9g\n", f2c(-40.0f));

    int vals[] = {100, 2, -1};
    for (int i = 0; i < 3; i++) {
        float dst;
        int idst;
        sqrt_demo(vals[i], &dst, &idst);
        printf("sqrt_demo(%d): dst = %.9g, idst = %d\n", vals[i], dst, idst);
    }

    double eps[] = {1e-6, 1e-12, 1e-16};
    for (int i = 0; i < 3; i++)
        printf("e_const(%.0e) = %.17g (M_E = %.17g)\n",
               eps[i], e_const(eps[i]), M_E);

    printf("add_ld(1, 2) = %.21Lg\n", add_ld(1.0L, 2.0L));
    return 0;
}
