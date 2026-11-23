#include <math.h>
#include <stdio.h>

static float r(float a, float b, float c) { return a * b - c; }

int main(void) {
    volatile float vx = 1.0f / 3.0f;   /* volatile: иначе GCC посчитает все заранее */
    float x = vx;
    printf("%a\n", (double)r(x, 3.0f, 1.0f));
    return 0;
}
