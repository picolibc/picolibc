#include <stddef.h>
static int overflows (size_t a, size_t b) { size_t x; return __builtin_add_overflow(a, b, &x); }
volatile size_t aa = 42;
int main (void) { return overflows(aa, aa); }
