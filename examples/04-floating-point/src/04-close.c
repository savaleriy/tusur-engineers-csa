#include <float.h>
#include <math.h>
#include <stdio.h>

static int close(double a, double b) {
    double scale = fmax(fabs(a), fabs(b));
    double rel = 8 * DBL_EPSILON * scale;
    double abs_tol = 1e-12;
    return fabs(a - b) <= fmax(rel, abs_tol);
}

int main(void) {
    printf("0.1+0.2 == 0.3: %d\n", 0.1 + 0.2 == 0.3);
    printf("close:          %d\n", close(0.1 + 0.2, 0.3));
    printf("close(1e8+1, 1e8): %d\n", close(1e8 + 1, 1e8));
    printf("close(NAN, NAN):   %d\n", close(NAN, NAN));
    return 0;
}
