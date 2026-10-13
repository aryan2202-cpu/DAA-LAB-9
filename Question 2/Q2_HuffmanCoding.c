#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

typedef struct {
    long long w;
    int l;
    int r;
} Node;

typedef struct {
    int len;
    int idx;
} Sym;

static Node *tree;
static int *heap;
static int hsz;

static double now_ms(void) {
    return 1000.0 * (double)clock() / (double)CLOCKS_PER_SEC;
}

static int less(int a, int b) {
    if (tree[a].w != tree[b].w) return tree[a].w < tree[b].w;
    return a < b;
}

static void sift_up(int i) {
    int x = heap[i];
    while (i > 0) {
        int p = (i - 1) / 2;
        if (!less(x, heap[p])) break;
        heap[i] = heap[p];
        i = p;
    }
    heap[i] = x;
}

static void sift_down(int i) {
    int x = heap[i];
    for (;;) {
        int c = 2 * i + 1;
        if (c >= hsz) break;
        if (c + 1 < hsz && less(heap[c + 1], heap[c])) c++;
        if (!less(heap[c], x)) break;
        heap[i] = heap[c];
        i = c;
    }
    heap[i] = x;
}

static int heap_pop(void) {
    int top = heap[0];
    hsz--;
    if (hsz > 0) {
        heap[0] = heap[hsz];
        sift_down(0);
    }
    return top;
}

static void heap_push(int x) {
    heap[hsz] = x;
    sift_up(hsz);
    hsz++;
}

void huffman_lengths(const long long *freq, int n, int *len) {
    int i, next;
    int *depth;
    if (n == 1) {
        len[0] = 1;
        return;
    }
    tree = (Node *)malloc((2 * n - 1) * sizeof(Node));
    heap = (int *)malloc(n * sizeof(int));
    depth = (int *)malloc((2 * n - 1) * sizeof(int));
    for (i = 0; i < n; i++) {
        tree[i].w = freq[i];
        tree[i].l = -1;
        tree[i].r = -1;
        heap[i] = i;
    }
    hsz = n;
    for (i = n / 2 - 1; i >= 0; i--) sift_down(i);
    next = n;
    while (hsz > 1) {
        int a = heap_pop();
        int b = heap_pop();
        tree[next].w = tree[a].w + tree[b].w;
        tree[next].l = a;
        tree[next].r = b;
        heap_push(next);
        next++;
    }
    depth[next - 1] = 0;
    for (i = next - 1; i >= n; i--) {
        depth[tree[i].l] = depth[i] + 1;
        depth[tree[i].r] = depth[i] + 1;
    }
    for (i = 0; i < n; i++) len[i] = depth[i];
    free(tree);
    free(heap);
    free(depth);
}

static int cmp_sym(const void *a, const void *b) {
    const Sym *x = (const Sym *)a;
    const Sym *y = (const Sym *)b;
    if (x->len != y->len) return x->len - y->len;
    return x->idx - y->idx;
}

static int cmp_ll(const void *a, const void *b) {
    long long x = *(const long long *)a;
    long long y = *(const long long *)b;
    return (x > y) - (x < y);
}

void canonical_codes(const int *len, int n, unsigned long long *code, Sym *order) {
    int i;
    for (i = 0; i < n; i++) {
        order[i].len = len[i];
        order[i].idx = i;
    }
    qsort(order, n, sizeof(Sym), cmp_sym);
    code[order[0].idx] = 0;
    for (i = 1; i < n; i++) {
        unsigned long long c = code[order[i - 1].idx] + 1;
        c <<= (order[i].len - order[i - 1].len);
        code[order[i].idx] = c;
    }
}

static void print_bits(unsigned long long c, int len) {
    int i;
    for (i = len - 1; i >= 0; i--) putchar(((c >> i) & 1ULL) ? '1' : '0');
}

static long long two_queue_cost(const long long *freq, int n) {
    long long *a = (long long *)malloc(n * sizeof(long long));
    long long *b = (long long *)malloc(n * sizeof(long long));
    int ia = 0, ib = 0, nb = 0, i;
    long long cost = 0;
    for (i = 0; i < n; i++) a[i] = freq[i];
    qsort(a, n, sizeof(long long), cmp_ll);
    for (i = 0; i < n - 1; i++) {
        long long x, y;
        if (ib >= nb || (ia < n && a[ia] <= b[ib])) x = a[ia++]; else x = b[ib++];
        if (ib >= nb || (ia < n && a[ia] <= b[ib])) y = a[ia++]; else y = b[ib++];
        b[nb++] = x + y;
        cost += x + y;
    }
    free(a);
    free(b);
    return cost;
}

static void run_sample(const char *label, const char *syms, const long long *freq, int n) {
    int *len = (int *)malloc(n * sizeof(int));
    unsigned long long *code = (unsigned long long *)malloc(n * sizeof(unsigned long long));
    Sym *order = (Sym *)malloc(n * sizeof(Sym));
    long long total = 0, bits = 0;
    int i;
    huffman_lengths(freq, n, len);
    canonical_codes(len, n, code, order);
    for (i = 0; i < n; i++) {
        total += freq[i];
        bits += freq[i] * len[i];
    }
    printf("%s\n", label);
    printf("  symbol  freq  length  canonical code\n");
    for (i = 0; i < n; i++) {
        int k = order[i].idx;
        printf("  %6c  %4lld  %6d  ", syms[k], freq[k], len[k]);
        print_bits(code[k], len[k]);
        printf("\n");
    }
    printf("  total bits = %lld, expected length = %.4f bits/symbol\n\n", bits, (double)bits / (double)total);
    free(len);
    free(code);
    free(order);
}

static int valid_code(const int *len, const unsigned long long *code, int n) {
    int i, j, maxl = 0;
    long long kraft = 0;
    for (i = 0; i < n; i++) if (len[i] > maxl) maxl = len[i];
    for (i = 0; i < n; i++) kraft += 1LL << (maxl - len[i]);
    if (kraft != (1LL << maxl)) return 0;
    for (i = 0; i < n; i++) {
        for (j = 0; j < n; j++) {
            if (i != j && len[i] <= len[j] && (code[j] >> (len[j] - len[i])) == code[i]) return 0;
        }
    }
    return 1;
}

static void benchmark(void) {
    int sizes[] = {10000, 20000, 40000, 80000, 160000, 320000, 640000};
    int s, i;
    printf("n,time_ms\n");
    for (s = 0; s < 7; s++) {
        int n = sizes[s];
        long long *freq = (long long *)malloc(n * sizeof(long long));
        int *len = (int *)malloc(n * sizeof(int));
        double start, avg;
        int reps = 0;
        for (i = 0; i < n; i++) freq[i] = 1 + rand() % 1000000;
        start = now_ms();
        do {
            huffman_lengths(freq, n, len);
            reps++;
        } while (now_ms() - start < 30.0);
        avg = (now_ms() - start) / reps;
        printf("%d,%.4f\n", n, avg);
        free(freq);
        free(len);
    }
}

int main(void) {
    long long f1[6] = {45, 13, 12, 16, 9, 5};
    long long f2[4] = {5, 5, 5, 5};
    long long f3[5] = {1, 1, 2, 3, 5};
    int trial, ok = 0, total_trials = 500;

    srand(12345);
    printf("Q2: Huffman Coding with canonical codebook\n");
    printf("Input: arrays sym[] and freq[] of size n, min-heap of tree nodes\n\n");

    run_sample("Sample 1", "abcdef", f1, 6);
    run_sample("Sample 2 (equal frequencies)", "wxyz", f2, 4);
    run_sample("Sample 3 (Fibonacci-like frequencies)", "pqrst", f3, 5);

    for (trial = 0; trial < total_trials; trial++) {
        long long freq[12];
        int len[12];
        unsigned long long code[12];
        Sym order[12];
        long long bits = 0;
        int n = 2 + rand() % 11, i;
        for (i = 0; i < n; i++) freq[i] = 1 + rand() % 50;
        huffman_lengths(freq, n, len);
        canonical_codes(len, n, code, order);
        for (i = 0; i < n; i++) bits += freq[i] * len[i];
        if (bits == two_queue_cost(freq, n) && valid_code(len, code, n)) ok++;
    }
    printf("Validation (cost equals independent two-queue Huffman, Kraft sum = 1, prefix-free): %d/%d random tests passed\n\n", ok, total_trials);

    printf("Time complexity analysis\n");
    printf("  Building the min-heap of n leaves   : O(n)\n");
    printf("  n-1 merges, each = 2 extract-min + 1 insert = 3 * O(log n) : O(n log n)\n");
    printf("  Computing code lengths from the tree: O(n)\n");
    printf("  Sorting symbols by (length, symbol) for the canonical codebook: O(n log n)\n");
    printf("  Total T(n) = O(n log n), space = O(n)\n");
    printf("  Correctness: the two least frequent symbols can always be siblings at the deepest level (greedy choice property)\n\n");

    printf("Measured running time (code lengths via heap)\n");
    benchmark();
    return 0;
}
