#include <stdint.h>
#include <stdio.h>
#include <string.h>

static void print_float(float f) {
    uint32_t u;
    memcpy(&u, &f, sizeof u);          /* законный способ посмотреть на биты */
    uint32_t s = u >> 31;
    uint32_t e = (u >> 23) & 0xFFu;
    uint32_t m = u & 0x7FFFFFu;
    printf("%-14g 0x%08X  s=%u e=%3u (2^%4d) m=0x%06X\n",
           (double)f, u, s, e, (int)e - 127, m);
}

int main(void) {
    print_float(1.0f);
    print_float(-6.25f);
    print_float(0.1f);
    print_float(1e-40f);
    print_float(1.0f / 0.0f);
    print_float(0.0f / 0.0f);
    return 0;
}
