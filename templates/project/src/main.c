#include <stdio.h>

#include "core.h"

int main(void) {
    int a[] = {1, 2, 3, 4, 5};
    printf("sum = %ld\n", sum(a, sizeof a / sizeof a[0]));
    return 0;
}
