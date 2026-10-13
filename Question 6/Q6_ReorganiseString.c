#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

static double now_ms(void) {
    return 1000.0 * (double)clock() / (double)CLOCKS_PER_SEC;
}

int reorganise(const char *s, int n, int K, char *out) {
    int cnt[26] = {0};
    int nextpos[26] = {0};
    int i, pos;
    for (i = 0; i < n; i++) cnt[s[i] - 'a']++;
    if (K <= 1) {
        memcpy(out, s, n);
        out[n] = '\0';
        return 1;
    }
    for (pos = 0; pos < n; pos++) {
        int best = -1;
        for (i = 0; i < 26; i++) {
            if (cnt[i] > 0 && nextpos[i] <= pos && (best < 0 || cnt[i] > cnt[best])) best = i;
        }
        if (best < 0) {
            out[0] = '\0';
            return 0;
        }
        out[pos] = (char)('a' + best);
        cnt[best]--;
        nextpos[best] = pos + K;
    }
    out[n] = '\0';
    return 1;
}

static int cnt_b[26];
static int last_b[26];

static int dfs(int pos, int n, int K) {
    int c;
    if (pos == n) return 1;
    for (c = 0; c < 26; c++) {
        if (cnt_b[c] > 0 && (last_b[c] < 0 || pos - last_b[c] >= K)) {
            int saved = last_b[c];
            cnt_b[c]--;
            last_b[c] = pos;
            if (dfs(pos + 1, n, K)) {
                cnt_b[c]++;
                last_b[c] = saved;
                return 1;
            }
            cnt_b[c]++;
            last_b[c] = saved;
        }
    }
    return 0;
}

static int brute_possible(const char *s, int n, int K) {
    int i;
    for (i = 0; i < 26; i++) {
        cnt_b[i] = 0;
        last_b[i] = -1;
    }
    for (i = 0; i < n; i++) cnt_b[s[i] - 'a']++;
    return dfs(0, n, K);
}

static int is_valid(const char *s, const char *t, int n, int K) {
    int c1[26] = {0}, c2[26] = {0}, last[26], i;
    for (i = 0; i < 26; i++) last[i] = -1;
    for (i = 0; i < n; i++) {
        c1[s[i] - 'a']++;
        c2[t[i] - 'a']++;
    }
    for (i = 0; i < 26; i++) if (c1[i] != c2[i]) return 0;
    for (i = 0; i < n; i++) {
        int c = t[i] - 'a';
        if (last[c] >= 0 && i - last[c] < K) return 0;
        last[c] = i;
    }
    return 1;
}

static void run_sample(const char *label, const char *s, int K) {
    int n = (int)strlen(s);
    char *out = (char *)malloc(n + 1);
    int ok = reorganise(s, n, K, out);
    printf("%s\n", label);
    printf("  S = \"%s\", K = %d\n", s, K);
    if (ok) printf("  result = \"%s\"\n\n", out);
    else printf("  result = \"\" (impossible)\n\n");
    free(out);
}

static void benchmark(void) {
    int sizes[] = {100000, 200000, 400000, 800000, 1600000, 3200000};
    int k, i;
    printf("n,time_ms\n");
    for (k = 0; k < 6; k++) {
        int n = sizes[k];
        char *s = (char *)malloc(n + 1);
        char *out = (char *)malloc(n + 1);
        double start, avg;
        int reps = 0;
        for (i = 0; i < n; i++) s[i] = (char)('a' + rand() % 26);
        s[n] = '\0';
        start = now_ms();
        do {
            reorganise(s, n, 10, out);
            reps++;
        } while (now_ms() - start < 30.0);
        avg = (now_ms() - start) / reps;
        printf("%d,%.4f\n", n, avg);
        free(s);
        free(out);
    }
}

int main(void) {
    int trial, ok = 0, total_trials = 3000;

    srand(12345);
    printf("Q6: Reorganise String with K-Distance Apart\n");
    printf("Input: string S (lowercase letters), integer K, frequency array cnt[26], array nextpos[26]\n\n");

    run_sample("Sample 1", "aabbcc", 3);
    run_sample("Sample 2", "aaabc", 3);
    run_sample("Sample 3", "aaadbbcc", 2);
    run_sample("Sample 4", "aaabbbccc", 4);

    for (trial = 0; trial < total_trials; trial++) {
        char s[10], out[10];
        int n = 1 + rand() % 8, K = 1 + rand() % 5, i, g, b;
        for (i = 0; i < n; i++) s[i] = (char)('a' + rand() % 3);
        s[n] = '\0';
        g = reorganise(s, n, K, out);
        b = brute_possible(s, n, K);
        if (g == b && (!g || is_valid(s, out, n, K))) ok++;
    }
    printf("Validation (feasibility matches exhaustive search, output obeys distance K): %d/%d random tests matched\n\n", ok, total_trials);

    printf("Time complexity analysis\n");
    printf("  Counting character frequencies         : O(n)\n");
    printf("  Each of the n positions scans the 26 letters to pick the available letter with the highest remaining count : n * O(26)\n");
    printf("  Total T(n) = O(26 n) = O(n), space = O(n) for the output (O(1) extra for the 26 counters)\n");
    printf("  With a max-heap over the sigma distinct letters the same method runs in O(n log sigma)\n");
    printf("  Correctness: always placing the most frequent letter that is allowed keeps the heaviest letters spread out, so if this fails no valid arrangement exists\n\n");

    printf("Measured running time\n");
    benchmark();
    return 0;
}
