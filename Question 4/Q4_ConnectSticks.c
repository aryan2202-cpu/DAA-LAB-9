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
        if (c + 1 < hsz && hp[c + 1] < hp[c]) c++;
        if (hp[c] >= x) break;
        hp[i] = hp[c];
        i = c;
    }
    hp[i] = x;
}

static void heap_push(long long v) {
    int i = hsz++;
    while (i > 0) {
        int p = (i - 1) / 2;
        if (hp[p] <= v) break;
        hp[i] = hp[p];
        i = p;
    }
    hp[i] = v;
}

static long long heap_pop(void) {
    long long top = hp[0];
    hsz--;
    if (hsz > 0) {
        hp[0] = hp[hsz];
        sift_down(0);
    }
    return top;
}

long long connect_sticks(const long long *len, int n, long long *steps) {
    long long cost = 0;
    int i, k = 0;
    hp = (long long *)malloc((n + 1) * sizeof(long long));
    hsz = n;
    for (i = 0; i < n; i++) hp[i] = len[i];
    for (i = n / 2 - 1; i >= 0; i--) sift_down(i);
    while (hsz > 1) {
        long long x = heap_pop();
        long long y = heap_pop();
        cost += x + y;
        if (steps) {
            steps[2 * k] = x;
            steps[2 * k + 1] = y;
        }
        k++;
        heap_push(x + y);
    }
    free(hp);
    return cost;
}

static long long brute(long long *a, int m) {
    long long best = -1;
    int i, j, k;
    long long b[8];
    if (m == 1) return 0;
    for (i = 0; i < m; i++) {
        for (j = i + 1; j < m; j++) {
            long long c;
            int t = 0;
            for (k = 0; k < m; k++) if (k != i && k != j) b[t++] = a[k];
            b[t++] = a[i] + a[j];
            c = a[i] + a[j] + brute(b, m - 1);
            if (best < 0 || c < best) best = c;
        }
    }
    return best;
}

static void run_sample(const char *label, const long long *len, int n) {
    long long *steps = (long long *)malloc(2 * n * sizeof(long long));
    long long cost = connect_sticks(len, n, steps);
    int i;
    printf("%s\n", label);
    printf("  sticks:");
    for (i = 0; i < n; i++) printf(" %lld", len[i]);
    printf("\n");
    for (i = 0; i < n - 1; i++) {
        printf("  step %d: connect %lld and %lld, cost %lld\n", i + 1, steps[2 * i], steps[2 * i + 1], steps[2 * i] + steps[2 * i + 1]);
    }
    printf("  minimum total cost = %lld\n\n", cost);
    free(steps);
}

static void benchmark(void) {
    int sizes[] = {10000, 20000, 40000, 80000, 160000, 320000, 640000};
    int s, i;
    printf("n,time_ms\n");
    for (s = 0; s < 7; s++) {
        int n = sizes[s];
        long long *len = (long long *)malloc(n * sizeof(long long));
        double start, avg;
        int reps = 0;
        for (i = 0; i < n; i++) len[i] = 1 + rand() % 1000;
        start = now_ms();
        do {
            connect_sticks(len, n, NULL);
            reps++;
        } while (now_ms() - start < 30.0);
        avg = (now_ms() - start) / reps;
        printf("%d,%.4f\n", n, avg);
        free(len);
    }
}

int main(void) {
    long long a[3] = {2, 4, 3};
    long long b[4] = {1, 8, 3, 5};
    long long c[5] = {5, 5, 5, 5, 5};
    int trial, ok = 0, total_trials = 1000;

    srand(12345);
    printf("Q4: Minimum Cost to Connect Sticks\n");
    printf("Input: array L[] of stick lengths, min-heap\n\n");

    run_sample("Sample 1", a, 3);
    run_sample("Sample 2", b, 4);
    run_sample("Sample 3 (equal sticks)", c, 5);

    for (trial = 0; trial < total_trials; trial++) {
        long long v[7];
        int n = 1 + rand() % 7, i;
        for (i = 0; i < n; i++) v[i] = 1 + rand() % 30;
        if (connect_sticks(v, n, NULL) == brute(v, n)) ok++;
    }
    printf("Validation against exhaustive search over all merge orders: %d/%d random tests matched\n\n", ok, total_trials);

    printf("Time complexity analysis\n");
    printf("  Building the min-heap (heapify)       : O(n)\n");
    printf("  n-1 rounds, each = 2 extract-min + 1 insert = 3 * O(log n) : O(n log n)\n");
    printf("  Total T(n) = O(n) + O(n log n) = O(n log n), space = O(n)\n");
    printf("  Correctness: the two shortest sticks are paid for in every later merge of their group, so they should sit deepest in the merge tree (same argument as Huffman)\n\n");

    printf("Measured running time\n");
    benchmark();
    return 0;
}
