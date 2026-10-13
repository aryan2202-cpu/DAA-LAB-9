#include <stdio.h>
#include <stdlib.h>
#include <time.h>

static long long *hp;
static int hsz;

static double now_ms(void) {
    return 1000.0 * (double)clock() / (double)CLOCKS_PER_SEC;
}

static void sift_down(int i) {
    long long x = hp[i];
    for (;;) {
        int c = 2 * i + 1;
        if (c >= hsz) break;
        if (c + 1 < hsz && hp[c + 1] > hp[c]) c++;
        if (hp[c] <= x) break;
        hp[i] = hp[c];
        i = c;
    }
    hp[i] = x;
}

long long min_deviation(const long long *a, int n) {
    long long mn = 0, best = 0;
    int i;
    hp = (long long *)malloc(n * sizeof(long long));
    hsz = n;
    for (i = 0; i < n; i++) {
        hp[i] = (a[i] % 2) ? a[i] * 2 : a[i];
        if (i == 0 || hp[i] < mn) mn = hp[i];
    }
    for (i = n / 2 - 1; i >= 0; i--) sift_down(i);
    best = hp[0] - mn;
    for (;;) {
        long long top = hp[0];
        if (top - mn < best) best = top - mn;
        if (top % 2) break;
        hp[0] = top / 2;
        if (hp[0] < mn) mn = hp[0];
        sift_down(0);
    }
    free(hp);
    return best;
}

static long long brute_best;

static void dfs(const long long *a, int n, int i, long long mn, long long mx) {
    long long b, v;
    int steps, k;
    if (i == n) {
        if (brute_best < 0 || mx - mn < brute_best) brute_best = mx - mn;
        return;
    }
    b = a[i];
    steps = 0;
    while (b % 2 == 0) {
        b /= 2;
        steps++;
    }
    if (steps < 1) steps = 1;
    v = b;
    for (k = 0; k <= steps; k++) {
        long long nmn = (i == 0 || v < mn) ? v : mn;
        long long nmx = (i == 0 || v > mx) ? v : mx;
        dfs(a, n, i + 1, nmn, nmx);
        v *= 2;
    }
}

static long long brute(const long long *a, int n) {
    brute_best = -1;
    dfs(a, n, 0, 0, 0);
    return brute_best;
}

static void run_sample(const char *label, const long long *a, int n) {
    int i;
    printf("%s\n", label);
    printf("  A =");
    for (i = 0; i < n; i++) printf(" %lld", a[i]);
    printf("\n  minimum deviation = %lld\n\n", min_deviation(a, n));
}

static void benchmark(void) {
    int sizes[] = {10000, 20000, 40000, 80000, 160000, 320000};
    int k, i;
    printf("n,time_ms\n");
    for (k = 0; k < 6; k++) {
        int n = sizes[k];
        long long *a = (long long *)malloc(n * sizeof(long long));
        double start, avg;
        int reps = 0;
        for (i = 0; i < n; i++) a[i] = 1 + rand() % 1000000;
        start = now_ms();
        do {
            min_deviation(a, n);
            reps++;
        } while (now_ms() - start < 30.0);
        avg = (now_ms() - start) / reps;
        printf("%d,%.4f\n", n, avg);
        free(a);
    }
}

int main(void) {
    long long a[4] = {1, 2, 3, 4};
    long long b[5] = {4, 1, 5, 20, 3};
    long long c[3] = {2, 10, 8};
    int trial, ok = 0, total_trials = 2000;

    srand(12345);
    printf("Q7: Minimise Deviation in Array (two-way greedy with max-heap)\n");
    printf("Input: array A[] of positive integers, max-heap, running minimum\n\n");

    run_sample("Sample 1", a, 4);
    run_sample("Sample 2", b, 5);
    run_sample("Sample 3", c, 3);

    for (trial = 0; trial < total_trials; trial++) {
        long long v[5];
        int n = 1 + rand() % 5, i;
        for (i = 0; i < n; i++) v[i] = 1 + rand() % 40;
        if (min_deviation(v, n) == brute(v, n)) ok++;
    }
    printf("Validation against exhaustive search over every reachable value of every element: %d/%d random tests matched\n\n", ok, total_trials);

    printf("Time complexity analysis\n");
    printf("  Doubling every odd element makes all values even, so only the divide operation remains : O(n)\n");
    printf("  Building the max-heap                  : O(n)\n");
    printf("  An element x can be halved at most log2(x) <= log2(M) times, so there are at most n log M heap updates, each O(log n)\n");
    printf("  Total T(n) = O(n log n log M), where M is the largest value, space = O(n)\n");
    printf("  Correctness: only the maximum can be reduced to shrink the deviation, and the loop stops when the maximum is odd because it can no longer be halved\n\n");

    printf("Measured running time\n");
    benchmark();
    return 0;
}
