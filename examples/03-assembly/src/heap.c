#include <stdlib.h>

long f(int n) {
    int a[100];                              /* стек: умрет при выходе */
    a[0] = n;
    int *p = malloc((size_t)n * sizeof *p);  /* куча: живет до free */
    if (p == NULL)
        return a[0];
    p[0] = a[0] + 1;
    long r = p[0];
    free(p);
    return r;
}
