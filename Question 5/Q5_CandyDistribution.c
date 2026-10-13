#include <stdio.h>
#include <stdlib.h>
#include <time.h>

static double now_ms(void) {
    return 1000.0 * (double)clock() / (double)CLOCKS_PER_SEC;
}

long long candies(const int *rating, int n, int *candy) {
    long long total = 0;
    int i;
    for (i = 0; i < n; i++) candy[i] = 1;
    for (i = 1; i < n; i++) {
        if (rating[i] > rating[i - 1]) candy[i] = candy[i - 1] + 1;
    }
    for (i = n - 2; i >= 0; i--) {
        if (rating[i] > rating[i + 1] && candy[i] <= candy[i + 1]) candy[i] = candy[i + 1] + 1;
    }
    for (i = 0; i < n; i++) total += candy[i];
    return total;
}

static long long relax_check(const int *rating, int n) {
    int c[16];
    int changed = 1, i;
    long long total = 0;
    for (i = 0; i < n; i++) c[i] = 1;
    while (changed) {
        changed = 0;
        for (i = 0; i < n; i++) {
            if (i > 0 && rating[i] > rating[i - 1] && c[i] <= c[i - 1]) {
                c[i] = c[i - 1] + 1;
                changed = 1;
            }
            if (i < n - 1 && rating[i] > rating[i + 1] && c[i] <= c[i + 1]) {
                c[i] = c[i + 1] + 1;
                changed = 1;
            }
        }
    }
    for (i = 0; i < n; i++) total += c[i];
    return total;
}

static void run_sample(const char *label, const int *rating, int n) {
    int *candy = (int *)malloc(n * sizeof(int));
    long long total = candies(rating, n, candy);
    int i;
    printf("%s\n", label);
    printf("  ratings:");
    for (i = 0; i < n; i++) printf(" %d", rating[i]);
    printf("\n  candies:");
    for (i = 0; i < n; i++) printf(" %d", candy[i]);
    printf("\n  minimum total candies = %lld\n\n", total);
    free(candy);
}

static void benchmark(void) {
    int sizes[] = {200000, 400000, 800000, 1600000, 3200000, 6400000};
    int s, i;
    printf("n,time_ms\n");
    for (s = 0; s < 6; s++) {
        int n = sizes[s];
        int *rating = (int *)malloc(n * sizeof(int));
        int *candy = (int *)malloc(n * sizeof(int));
        double start, avg;
        int reps = 0;
        for (i = 0; i < n; i++) rating[i] = rand() % 1000;
        start = now_ms();
        do {
            candies(rating, n, candy);
            reps++;
        } while (now_ms() - start < 30.0);
        avg = (now_ms() - start) / reps;
        printf("%d,%.4f\n", n, avg);
        free(rating);
        free(candy);
    }
}

int main(void) {
    int a[3] = {1, 0, 2};
    int b[3] = {1, 2, 2};
    int c[5] = {1, 3, 4, 5, 2};
    int d[7] = {1, 2, 87, 87, 87, 2, 1};
    int trial, ok = 0, total_trials = 2000;

    srand(12345);
    printf("Q5: Candy Distribution (bi-directional slope greedy)\n");
    printf("Input: array rating[] of size n, array candy[] of size n\n\n");

    run_sample("Sample 1", a, 3);
    run_sample("Sample 2", b, 3);
    run_sample("Sample 3 (strictly rising then drop)", c, 5);
    run_sample("Sample 4 (plateau in the middle)", d, 7);

    for (trial = 0; trial < total_trials; trial++) {
        int r[16], cd[16];
        int n = 1 + rand() % 12, i;
        for (i = 0; i < n; i++) r[i] = rand() % 6;
        if (candies(r, n, cd) == relax_check(r, n)) ok++;
    }
    printf("Validation against repeated relaxation until no rule is violated: %d/%d random tests matched\n\n", ok, total_trials);

    printf("Time complexity analysis\n");
    printf("  Initialising every child with 1 candy     : O(n)\n");
    printf("  Left to right pass (compare with left neighbour)  : O(n)\n");
    printf("  Right to left pass (compare with right neighbour) : O(n)\n");
    printf("  Summing the candies                       : O(n)\n");
    printf("  Total T(n) = 4 * O(n) = O(n), space = O(n)\n");
    printf("  Correctness: each pass enforces one neighbour rule using the smallest value possible, and the right pass never breaks the left rule because it only increases a value\n\n");

    printf("Measured running time\n");
    benchmark();
    return 0;
}
