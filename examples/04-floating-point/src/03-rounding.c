#include <fenv.h>
#include <stdio.h>

static float count_up(void) {
    float x = 0.0f;
    for (int i = 0; i < (1 << 25); i++) x++;
    return x;
}

int main(void) {
    printf("default: %.0f\n", (double)count_up());
    fesetround(FE_UPWARD);
    printf("upward:  %.0f\n", (double)count_up());
    return 0;
}
