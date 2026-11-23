#include <float.h>
#include <stdio.h>

int main(void) {
    printf("sizeof(long double) = %zu\n", sizeof(long double));
    printf("LDBL_MANT_DIG = %d\n", LDBL_MANT_DIG);
    printf("LDBL_MAX_EXP = %d\n", LDBL_MAX_EXP);
    printf("LDBL_MIN_EXP = %d\n", LDBL_MIN_EXP);
    return 0;
}
