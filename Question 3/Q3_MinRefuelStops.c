#include <stdio.h>
#include <stdlib.h>
#include <time.h>

typedef struct {
    long long d;
    long long f;
} Station;

static long long *hp;
static int hsz;

static double now_ms(void) {
    return 1000.0 * (double)clock() / (double)CLOCKS_PER_SEC;
}

static int cmp_station(const void *a, const void *b) {
    const Station *x = (const Station *)a;
    const Station *y = (const Station *)b;
    return (x->d > y->d) - (x->d < y->d);
}

static void heap_push(long long v) {
    int i = hsz++;
    while (i > 0) {
        int p = (i - 1) / 2;
        if (hp[p] >= v) break;
        hp[i] = hp[p];
        i = p;
    }
    hp[i] = v;
}

static long long heap_pop(void) {
    long long top = hp[0];
    long long x = hp[--hsz];
    int i = 0;
    if (hsz > 0) {
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
    return top;
}

int min_stops(Station *s, int n, long long D, long long F, long long *taken) {
    long long reach = F;
    int i = 0, stops = 0;
    qsort(s, n, sizeof(Station), cmp_station);
    hp = (long long *)malloc((n + 1) * sizeof(long long));
    hsz = 0;
    while (reach < D) {
        while (i < n && s[i].d <= reach) heap_push(s[i++].f);
        if (hsz == 0) {
            free(hp);
            return -1;
        }
        long long x = heap_pop();
        reach += x;
        if (taken) taken[stops] = x;
        stops++;
    }
    free(hp);
    return stops;
}

int dp_stops(Station *s, int n, long long D, long long F) {
    long long *dp = (long long *)malloc((n + 1) * sizeof(long long));
    int i, k, ans = -1;
    qsort(s, n, sizeof(Station), cmp_station);
    for (k = 0; k <= n; k++) dp[k] = -1;
    dp[0] = F;
    for (i = 0; i < n; i++) {
        for (k = i; k >= 0; k--) {
            if (dp[k] >= s[i].d && dp[k] + s[i].f > dp[k + 1]) dp[k + 1] = dp[k] + s[i].f;
        }
    }
    for (k = 0; k <= n; k++) {
        if (dp[k] >= D) {
            ans = k;
            break;
        }
    }
    free(dp);
    return ans;
}

static void run_sample(const char *label, const Station *src, int n, long long D, long long F) {
    Station *s = (Station *)malloc((n + 1) * sizeof(Station));
    long long *taken = (long long *)malloc((n + 1) * sizeof(long long));
    int i, r;
    for (i = 0; i < n; i++) s[i] = src[i];
    r = min_stops(s, n, D, F, taken);
    printf("%s\n", label);
    printf("  target D = %lld, initial fuel F = %lld, stations (distance, fuel):", D, F);
    for (i = 0; i < n; i++) printf(" (%lld,%lld)", src[i].d, src[i].f);
    printf("\n");
    if (r < 0) {
        printf("  result = -1 (target cannot be reached)\n\n");
    } else {
        printf("  minimum refuelling stops = %d", r);
        if (r > 0) {
            printf(", fuel taken in order:");
            for (i = 0; i < r; i++) printf(" %lld", taken[i]);
        }
        printf("\n\n");
    }
    free(s);
    free(taken);
}

static void benchmark(void) {
    int sizes[] = {10000, 20000, 40000, 80000, 160000, 320000, 640000};
    int k, i;
    printf("n,time_ms\n");
    for (k = 0; k < 7; k++) {
        int n = sizes[k];
        Station *base = (Station *)malloc(n * sizeof(Station));
        Station *work = (Station *)malloc(n * sizeof(Station));
        double start, avg;
        int reps = 0;
        for (i = 0; i < n; i++) {
            base[i].d = (long long)(n - i) * 10;
            base[i].f = 10 + rand() % 51;
        }
        start = now_ms();
        do {
            for (i = 0; i < n; i++) work[i] = base[i];
            min_stops(work, n, (long long)(n + 1) * 10, 20, NULL);
            reps++;
        } while (now_ms() - start < 30.0);
        avg = (now_ms() - start) / reps;
        printf("%d,%.4f\n", n, avg);
        free(base);
        free(work);
    }
}

int main(void) {
    Station a[4] = {{10, 60}, {20, 30}, {30, 30}, {60, 40}};
    Station b[1] = {{10, 100}};
    int trial, ok = 0, total_trials = 1000;

    srand(12345);
    printf("Q3: Minimum Number of Refuelling Stops\n");
    printf("Input: array of stations (distance d, fuel f) sorted by distance, target D, initial fuel F, max-heap of passed fuel amounts\n\n");

    run_sample("Sample 1", a, 4, 100, 10);
    run_sample("Sample 2 (unreachable)", b, 1, 100, 1);
    run_sample("Sample 3 (no stop needed)", b, 1, 60, 60);

    for (trial = 0; trial < total_trials; trial++) {
        Station s1[8], s2[8];
        int n = rand() % 9, i;
        long long D = 20 + rand() % 60;
        long long F = 1 + rand() % 15;
        for (i = 0; i < n; i++) {
            s1[i].d = 1 + rand() % (D - 1);
            s1[i].f = 1 + rand() % 20;
            s2[i] = s1[i];
        }
        if (min_stops(s1, n, D, F, NULL) == dp_stops(s2, n, D, F)) ok++;
    }
    printf("Validation against O(n^2) dynamic programming: %d/%d random tests matched\n\n", ok, total_trials);

    printf("Time complexity analysis\n");
    printf("  Sorting stations by distance          : O(n log n)\n");
    printf("  Each station is pushed into the heap at most once  : n * O(log n)\n");
    printf("  Each refuelling stop pops the heap once, at most n pops : n * O(log n)\n");
    printf("  Total T(n) = O(n log n), space = O(n)\n");
    printf("  Correctness: when fuel runs out, the best choice is to have stopped at the largest station already passed (greedy choice)\n\n");

    printf("Measured running time\n");
    benchmark();
    return 0;
}
