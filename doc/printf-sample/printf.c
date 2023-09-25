#include <stdio.h>

int main(void) {
        printf(" 2⁶¹ = %lld π ≃ %.17g\n", 1ll << 61, printf_float(3.141592653589793));
        return 0;
}
