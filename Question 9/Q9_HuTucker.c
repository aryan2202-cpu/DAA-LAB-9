#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

typedef struct {
    long long w;
    int term;
    int id;
} Cell;

static double now_ms(void) {
    return 1000.0 * (double)clock() / (double)CLOCKS_PER_SEC;
}

void hu_tucker(const long long *w, int n, int *depth) {
    int total = 2 * n - 1;
    int *lc, *rc, *d;
    Cell *cur;
    int m = n, next = n, i;
    if (n == 1) {
        depth[0] = 0;
        return;
    }
    lc = (int *)malloc(total * sizeof(int));
    rc = (int *)malloc(total * sizeof(int));
    d = (int *)malloc(total * sizeof(int));
    cur = (Cell *)malloc(n * sizeof(Cell));
    for (i = 0; i < n; i++) {
        cur[i].w = w[i];
        cur[i].term = 1;
        cur[i].id = i;
    }
    while (m > 1) {
        long long best = -1, m1 = 0, m2 = 0;
        int p1 = -1, p2 = -1, bl = -1, br = -1, keep, drop;
        Cell merged;
        for (i = 0; i < m; i++) {
            if (p1 < 0 || cur[i].w < m1) {
                m2 = m1;
                p2 = p1;
                m1 = cur[i].w;
                p1 = i;
            } else if (p2 < 0 || cur[i].w < m2) {
                m2 = cur[i].w;
                p2 = i;
            }
            if (cur[i].term || i == m - 1) {
                if (p2 >= 0 && (best < 0 || m1 + m2 < best)) {
                    best = m1 + m2;
                    bl = p1 < p2 ? p1 : p2;
                    br = p1 < p2 ? p2 : p1;
                }
                m1 = cur[i].w;
                p1 = i;
                m2 = 0;
                p2 = -1;
            }
        }
        lc[next] = cur[bl].id;
        rc[next] = cur[br].id;
        merged.w = best;
        merged.term = 0;
        merged.id = next++;
        keep = bl;
        drop = br;
        cur[keep] = merged;
        memmove(&cur[drop], &cur[drop + 1], (m - drop - 1) * sizeof(Cell));
        m--;
    }
    d[next - 1] = 0;
    for (i = next - 1; i >= n; i--) {
        d[lc[i]] = d[i] + 1;
        d[rc[i]] = d[i] + 1;
    }
    for (i = 0; i < n; i++) depth[i] = d[i];
    free(lc);
    free(rc);
    free(d);
    free(cur);
}

void assign_codes(const int *depth, int n, unsigned long long *code) {
    int i;
    code[0] = 0;
    for (i = 1; i < n; i++) {
        unsigned long long c = code[i - 1] + 1;
        if (depth[i] >= depth[i - 1]) c <<= (depth[i] - depth[i - 1]);
        else c >>= (depth[i - 1] - depth[i]);
        code[i] = c;
    }
}

static long long tree_cost(const long long *w, const int *depth, int n) {
    long long c = 0;
    int i;
    for (i = 0; i < n; i++) c += w[i] * depth[i];
    return c;
}

static long long dp_cost(const long long *w, int n) {
    long long pre[16], cost[16][16];
    int i, j, k, len;
    pre[0] = 0;
    for (i = 0; i < n; i++) pre[i + 1] = pre[i] + w[i];
    for (i = 0; i < n; i++) cost[i][i] = 0;
    for (len = 2; len <= n; len++) {
        for (i = 0; i + len - 1 < n; i++) {
            long long best = -1;
            j = i + len - 1;
            for (k = i; k < j; k++) {
                long long c = cost[i][k] + cost[k + 1][j];
                if (best < 0 || c < best) best = c;
            }
            cost[i][j] = best + pre[j + 1] - pre[i];
        }
    }
    return cost[0][n - 1];
}

static int valid_alphabetic(const int *depth, const unsigned long long *code, int n) {
    int i, j, maxd = 0;
    long long kraft = 0;
    for (i = 0; i < n; i++) if (depth[i] > maxd) maxd = depth[i];
    for (i = 0; i < n; i++) kraft += 1LL << (maxd - depth[i]);
    if (kraft != (1LL << maxd)) return 0;
    for (i = 0; i + 1 < n; i++) {
        unsigned long long a = code[i] << (maxd - depth[i]);
        unsigned long long b = code[i + 1] << (maxd - depth[i + 1]);
        if (a >= b) return 0;
    }
    for (i = 0; i < n; i++) {
        for (j = 0; j < n; j++) {
            if (i != j && depth[i] <= depth[j] && (code[j] >> (depth[j] - depth[i])) == code[i]) return 0;
        }
    }
    return 1;
}

static void print_bits(unsigned long long c, int len) {
    int i;
    if (len == 0) {
        putchar('-');
        return;
    }
    for (i = len - 1; i >= 0; i--) putchar(((c >> i) & 1ULL) ? '1' : '0');
}

static void run_sample(const char *label, const long long *w, int n) {
    int *depth = (int *)malloc(n * sizeof(int));
    unsigned long long *code = (unsigned long long *)malloc(n * sizeof(unsigned long long));
    int i;
    hu_tucker(w, n, depth);
    assign_codes(depth, n, code);
    printf("%s\n", label);
    printf("  leaf  weight  depth  code\n");
    for (i = 0; i < n; i++) {
        printf("  %4d  %6lld  %5d  ", i + 1, w[i], depth[i]);
        print_bits(code[i], depth[i]);
        printf("\n");
    }
    printf("  minimum cost sum(w_i * depth_i) = %lld", tree_cost(w, depth, n));
    if (n <= 15) printf("  (dynamic programming optimum = %lld)", dp_cost(w, n));
    printf("\n\n");
    free(depth);
    free(code);
}

static void benchmark(void) {
    int sizes[] = {250, 500, 1000, 2000, 4000, 8000};
    int k, i;
    printf("n,time_ms\n");
    for (k = 0; k < 6; k++) {
        int n = sizes[k];
        long long *w = (long long *)malloc(n * sizeof(long long));
        int *depth = (int *)malloc(n * sizeof(int));
        double start, avg;
        int reps = 0;
        for (i = 0; i < n; i++) w[i] = 1 + rand() % 1000;
        start = now_ms();
        do {
            hu_tucker(w, n, depth);
            reps++;
        } while (now_ms() - start < 30.0);
        avg = (now_ms() - start) / reps;
        printf("%d,%.4f\n", n, avg);
        free(w);
        free(depth);
    }
}

int main(void) {
    long long a[8] = {3, 1, 4, 1, 5, 9, 2, 6};
    long long b[6] = {10, 1, 1, 10, 1, 1};
    long long c[5] = {1, 1, 1, 1, 1};
    int trial, ok = 0, total_trials = 3000;

    srand(12345);
    printf("Q9: Hu-Tucker Greedy Simulation (optimal alphabetic binary tree)\n");
    printf("Input: ordered array w[] of weights, working list of cells (weight, terminal flag), merge tree arrays\n\n");

    run_sample("Sample 1", a, 8);
    run_sample("Sample 2", b, 6);
    run_sample("Sample 3 (equal weights)", c, 5);

    for (trial = 0; trial < total_trials; trial++) {
        long long w[14];
        int depth[14];
        unsigned long long code[14];
        int n = 2 + rand() % 12, i;
        for (i = 0; i < n; i++) w[i] = 1 + rand() % 9;
        hu_tucker(w, n, depth);
        assign_codes(depth, n, code);
        if (tree_cost(w, depth, n) == dp_cost(w, n) && valid_alphabetic(depth, code, n)) ok++;
    }
    printf("Validation (cost equals O(n^3) interval DP optimum, Kraft sum = 1, codes ordered and prefix-free): %d/%d random tests passed\n\n", ok, total_trials);

    printf("Time complexity analysis\n");
    printf("  Phase 1 (combination): n-1 rounds, each round scans the working list once to find the cheapest compatible pair (two smallest weights between consecutive terminal leaves) : O(n) per round\n");
    printf("  Phase 2 (level assignment): one top-down pass over the 2n-1 nodes of the merge tree : O(n)\n");
    printf("  Phase 3 (reconstruction): one pass assigning alphabetic codes from the leaf depths : O(n)\n");
    printf("  Total T(n) = (n-1) * O(n) + O(n) + O(n) = O(n^2) for this simulation, space = O(n)\n");
    printf("  Replacing the scan with a priority queue structure gives the classical O(n log n) Hu-Tucker bound\n");
    printf("  Correctness: Hu and Tucker proved that the leaf depths of the combination tree are the depths of an optimal alphabetic tree\n\n");

    printf("Measured running time\n");
    benchmark();
    return 0;
}
