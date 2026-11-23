#include <stdio.h>

int main(void) {
    printf("%.17g\n", 0.1 + 0.2);
    printf("%d\n", 0.1 + 0.2 == 0.3);
    printf("%d\n", 0.1f + 0.2f == 0.3f);
    printf("%d\n", 0.3f - 0.2f == 0.1f);
    return 0;
}
