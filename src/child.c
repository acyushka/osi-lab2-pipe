#include <stdio.h>
#include <stdlib.h>

int main(void) {
    char line[4096];

    while (fgets(line, sizeof(line), stdin)) {
        const char *p = line;
        float acc;
        int off = 0;

        if (sscanf(p, "%f%n", &acc, &off) != 1) {
            continue;
        }
        p += off;

        int div_by_zero = 0;
        for (;;) {
            float b;
            int n = 0;
            if (sscanf(p, "%f%n", &b, &n) != 1) break;
            p += n;

            if (b == 0.0f) { div_by_zero = 1; break; }
            acc /= b;
        }

        if (div_by_zero) {
            fprintf(stderr, "division by zero\n");
            return 1;
        }

        if (printf("%g\n", acc) < 0) {
            perror("printf");
            return 2;
        }
        if (fflush(stdout) != 0) {
            perror("fflush");
            return 2;
        }
    }

    if (ferror(stdin)) {
        perror("fgets");
        return 2;
    }
    return 0;
}
