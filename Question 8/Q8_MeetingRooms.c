#include <stdio.h>
#include <stdlib.h>
#include <time.h>

typedef struct {
    int s;
    int e;
} Interval;

static double now_ms(void) {
    return 1000.0 * (double)clock() / (double)CLOCKS_PER_SEC;
}

static int cmp_int(const void *a, const void *b) {
    int x = *(const int *)a;
    int y = *(const int *)b;
    return (x > y) - (x < y);
}

int min_rooms(const Interval *iv, int n) {
    int *starts = (int *)malloc(n * sizeof(int));
    int *ends = (int *)malloc(n * sizeof(int));
    int i, j = 0, rooms = 0;
    for (i = 0; i < n; i++) {
        starts[i] = iv[i].s;
        ends[i] = iv[i].e;
    }
    qsort(starts, n, sizeof(int), cmp_int);
    qsort(ends, n, sizeof(int), cmp_int);
    for (i = 0; i < n; i++) {
        if (starts[i] < ends[j]) rooms++;
        else j++;
    }
    free(starts);
    free(ends);
    return rooms;
}

static int brute_rooms(const Interval *iv, int n) {
    int i, j, best = 0;
    for (i = 0; i < n; i++) {
        int c = 0;
        for (j = 0; j < n; j++) {
            if (iv[j].s <= iv[i].s && iv[i].s < iv[j].e) c++;
        }
        if (c > best) best = c;
    }
    return best;
}

static void run_sample(const char *label, const Interval *iv, int n) {
    int i;
    printf("%s\n", label);
    printf("  intervals:");
    for (i = 0; i < n; i++) printf(" [%d,%d]", iv[i].s, iv[i].e);
    printf("\n  minimum rooms = %d\n\n", min_rooms(iv, n));
}

static void benchmark(void) {
    int sizes[] = {10000, 20000, 40000, 80000, 160000, 320000, 640000};
    int k, i;
    printf("n,time_ms\n");
    for (k = 0; k < 7; k++) {
        int n = sizes[k];
        Interval *iv = (Interval *)malloc(n * sizeof(Interval));
        double start, avg;
        int reps = 0;
        for (i = 0; i < n; i++) {
            iv[i].s = rand() % 1000000;
            iv[i].e = iv[i].s + 1 + rand() % 1000;
        }
        start = now_ms();
        do {
            min_rooms(iv, n);
            reps++;
        } while (now_ms() - start < 30.0);
        avg = (now_ms() - start) / reps;
        printf("%d,%.4f\n", n, avg);
        free(iv);
    }
}

int main(void) {
    Interval a[3] = {{0, 30}, {5, 10}, {15, 20}};
    Interval b[2] = {{7, 10}, {2, 4}};
    Interval c[3] = {{1, 5}, {5, 8}, {8, 10}};
    Interval d[5] = {{1, 10}, {2, 7}, {3, 19}, {8, 12}, {10, 20}};
    int trial, ok = 0, total_trials = 2000;

    srand(12345);
    printf("Q8: Minimum Number of Meeting Rooms\n");
    printf("Input: array of intervals [start, end), separate sorted arrays starts[] and ends[]\n\n");

    run_sample("Sample 1", a, 3);
    run_sample("Sample 2", b, 2);
    run_sample("Sample 3 (back to back meetings reuse the room)", c, 3);
    run_sample("Sample 4", d, 5);

    for (trial = 0; trial < total_trials; trial++) {
        Interval iv[12];
        int n = 1 + rand() % 12, i;
        for (i = 0; i < n; i++) {
            iv[i].s = rand() % 30;
            iv[i].e = iv[i].s + 1 + rand() % 10;
        }
        if (min_rooms(iv, n) == brute_rooms(iv, n)) ok++;
    }
    printf("Validation against O(n^2) maximum overlap count: %d/%d random tests matched\n\n", ok, total_trials);

    printf("Time complexity analysis\n");
    printf("  Copying start and end times            : O(n)\n");
    printf("  Sorting starts and sorting ends        : 2 * O(n log n)\n");
    printf("  Two pointer sweep (each pointer moves at most n steps) : O(n)\n");
    printf("  Total T(n) = O(n log n), space = O(n)\n");
    printf("  Correctness: a meeting needs a new room only if it starts before the earliest unfinished meeting ends, which is exactly the maximum number of overlapping meetings\n\n");

    printf("Measured running time\n");
    benchmark();
    return 0;
}
