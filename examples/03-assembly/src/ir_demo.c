/* Демо для дампов IR: сравните .gimple и .optimized. */
long sum(const long *a, long n) {
    long s = 0;
    for (long i = 0; i < n; i++)
        s += a[i];
    return s;
}

long fact(long n) {
    return n <= 1 ? 1 : n * fact(n - 1);
}
