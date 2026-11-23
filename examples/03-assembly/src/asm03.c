long sum(const long *a, long n) {
    long s = 0;
    for (long i = 0; i < n; i++) s += a[i];
    return s;
}

long f(long n) {
    long s = 0;
    for (long i = 0; i < n; i++) s += i * i;
    return s;
}

int max2(int a, int b) { return a > b ? a : b; }

int max3(int a, int b, int c) {
    int m = a > b ? a : b;
    return m > c ? m : c;
}

extern long g(long);

long frame(long x, long y) {
    long a = x + 1;
    long b = y * 2;
    return g(a * b) + x + y;
}

long fact(long n) { return n <= 1 ? 1 : n * fact(n - 1); }

long tail(long x) { return g(x + 1); }

long apply(long (*fn)(long), const long *a, long n) {
    long s = 0;
    for (long i = 0; i < n; i++) s += fn(a[i]);
    return s;
}

long seven(long a, long b, long c, long d, long e, long f6, long g7) {
    return a + b + c + d + e + f6 + g7;
}

long leaf(long x) { return x * 3 + 1; }
