#include <stdlib.h>
#include <stddef.h>
#include <stdio.h>
#include <math.h>

int main(int argc, char **argv) {
    int n = strtol(argv[1], NULL, 0);
    double x = strtod(argv[2], NULL);
    double j, y;

    switch (n) {
    case 0:
        j = j0(x);
        y = y0(x);
        break;
    case 1:
        j = j1(x);
        y = y1(x);
        break;
    default:
        j = jn(n, x);
        y = yn(n, x);
        break;
    }
    printf("jn(%d, %.17g %a) = %.17g %a\n", n, x, x, j, j);
    printf("yn(%d, %.17g %a) = %.17g %a\n", n, x, x, y, y);
    return 0;
}
