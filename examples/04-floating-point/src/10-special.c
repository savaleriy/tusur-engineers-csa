#include <float.h>
#include <math.h>
#include <stdio.h>

int main(void) {
    volatile double zero = 0.0;
    printf("1 / +0.0 = %g\n", 1.0 / zero);
    printf("1 / -0.0 = %g\n", 1.0 / -zero);
    printf("0.0 / 0.0 = %g\n", zero / zero);
    printf("fmax(NAN, 1.0) = %g\n", fmax(NAN, 1.0));
    printf("FLT_MIN = %g\n", FLT_MIN);
    printf("FLT_TRUE_MIN = %g\n", FLT_TRUE_MIN);
    printf("2^24 + 1 == 2^24: %d\n", 16777216.0f + 1.0f == 16777216.0f);
    return 0;
}
