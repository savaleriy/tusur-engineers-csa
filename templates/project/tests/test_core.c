#include <stdio.h>

#include "core.h"

int main(void) {
    int a[] = {1, 2, 3};
    if (sum(a, 3) != 6) {
        fprintf(stderr, "FAIL: sum(1,2,3) != 6\n");
        return 1;
    }
    if (sum(a, 0) != 0) {
        fprintf(stderr, "FAIL: sum(empty) != 0\n");
        return 1;
    }
    puts("OK");
    return 0;
}
