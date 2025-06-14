#include "math2.h"

int main(void)
{
    float64_t   x;

    for (x = float64(0x1p-4); x <= 1755; x += float64(0x1p-4)) {
        float64_t       l = gamma(x);
        float64_t       m = gamma_64(x);

        printf("%a -> %a,%a\n", x, l, m);
    }
}
