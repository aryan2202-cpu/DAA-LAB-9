#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>

typedef struct {
    double v;
    double w;
    double lam;
    double eff;
    int id;
} Item;

static double now_ms(void) {
    return 1000.0 * (double)clock() / (double)CLOCKS_PER_SEC;
}

static int cmp_eff_desc(const void *a, const void *b) {
    const Item *x = (const Item *)a;
    const Item *y = (const Item *)b;
    if (x->eff < y->eff) return 1;
    if (x->eff > y->eff) return -1;
    return x->id - y->id;
}

double solve(Item *it, int n, double W, double t, double *take) {
    int i;
    double rem = W;
    double total = 0.0;
    for (i = 0; i < n; i++) {
        it[i].eff = it[i].v / it[i].w - it[i].lam * t;
        if (take) take[i] = 0.0;
    }
    qsort(it, n, sizeof(Item), cmp_eff_desc);
    for (i = 0; i < n && rem > 0; i++) {
        double x;
        if (it[i].eff <= 0) break;
        x = it[i].w < rem ? it[i].w : rem;
        if (take) take[i] = x;
        total += x * it[i].eff;
        rem -= x;
    }
    return total;
}

static double brute_best;

static void dfs(const Item *it, int n, int *used, int depth, double rem, double val, double t) {
    int j;
    if (depth == n) {
        if (val > brute_best) brute_best = val;
        return;
    }
    for (j = 0; j < n; j++) {
        if (!used[j]) {
            double eff = it[j].v / it[j].w - it[j].lam * t;
            double x = it[j].w < rem ? it[j].w : rem;
            used[j] = 1;
            dfs(it, n, used, depth + 1, rem - x, val + (eff > 0 ? eff * x : 0.0), t);
            used[j] = 0;
        }
    }
}

static double brute(const Item *it, int n, double W, double t) {
    int used[8] = {0};
    brute_best = 0.0;
    dfs(it, n, used, 0, W, 0.0, t);
    return brute_best;
}

static void run_sample(const char *label, const Item *src, int n, double W, double t) {
    Item *it = (Item *)malloc(n * sizeof(Item));
    double *take = (double *)malloc(n * sizeof(double));
    double total;
    int i;
    for (i = 0; i < n; i++) it[i] = src[i];
    total = solve(it, n, W, t, take);
    printf("%s (W = %.0f, t = %.1f)\n", label, W, t);
    printf("  order  item  eff.density  taken/weight\n");
    for (i = 0; i < n; i++) {
        printf("  %5d  %4d  %11.2f  %5.1f/%.1f\n", i + 1, it[i].id + 1, it[i].eff, take[i], it[i].w);
    }
    printf("  maximum total value = %.2f\n\n", total);
    free(it);
    free(take);
}

static void benchmark(void) {
    int sizes[] = {10000, 20000, 40000, 80000, 160000, 320000, 640000};
    int s, i;
    printf("n,time_ms\n");
    for (s = 0; s < 7; s++) {
        int n = sizes[s];
        Item *base = (Item *)malloc(n * sizeof(Item));
        Item *work = (Item *)malloc(n * sizeof(Item));
        double start, avg;
        int reps = 0;
        for (i = 0; i < n; i++) {
            base[i].w = 1 + rand() % 50;
            base[i].v = base[i].w * (1 + rand() % 20);
            base[i].lam = (rand() % 100) / 50.0;
            base[i].id = i;
        }
        start = now_ms();
        do {
            for (i = 0; i < n; i++) work[i] = base[i];
            solve(work, n, (double)n * 5.0, 3.0, NULL);
            reps++;
        } while (now_ms() - start < 30.0);
        avg = (now_ms() - start) / reps;
        printf("%d,%.4f\n", n, avg);
        free(base);
        free(work);
    }
}

int main(void) {
    Item items[4] = {
        {60, 10, 0.5, 0, 0},
        {100, 20, 0.2, 0, 1},
        {120, 30, 1.0, 0, 2},
        {90, 15, 0.1, 0, 3}
    };
    int trial, ok = 0, total_trials = 500;

    srand(12345);
    printf("Q1: Fractional Knapsack with Deterioration Rate\n");
    printf("Input: arrays v[], w[], lambda[] (struct Item), capacity W, consumption time t\n");
    printf("Model: an item consumed at time t has density v/w - lambda*t, items with density <= 0 are never taken\n\n");

    run_sample("Sample 1", items, 4, 50, 2.0);
    run_sample("Sample 2 (same items, later consumption time)", items, 4, 50, 4.0);

    for (trial = 0; trial < total_trials; trial++) {
        Item it[6];
        Item cp[6];
        int n = 1 + rand() % 6, i;
        double W = 1 + rand() % 60;
        double t = (rand() % 50) / 10.0;
        double a, b;
        for (i = 0; i < n; i++) {
            it[i].w = 1 + rand() % 20;
            it[i].v = 1 + rand() % 60;
            it[i].lam = (rand() % 20) / 10.0;
            it[i].id = i;
            cp[i] = it[i];
        }
        a = solve(cp, n, W, t, NULL);
        b = brute(it, n, W, t);
        if (fabs(a - b) < 1e-6) ok++;
    }
    printf("Validation against brute force over all item orders: %d/%d random tests matched\n\n", ok, total_trials);

    printf("Time complexity analysis\n");
    printf("  Computing effective densities       : O(n)\n");
    printf("  Sorting by effective density        : O(n log n)\n");
    printf("  Single greedy pass filling capacity : O(n)\n");
    printf("  Total T(n) = O(n) + O(n log n) + O(n) = O(n log n), space = O(n)\n");
    printf("  Correctness: swapping a taken item with a lower density item can only reduce the value (exchange argument)\n\n");

    printf("Measured running time\n");
    benchmark();
    return 0;
}
