#if defined(_HAVE_BUILTIN_MUL_OVERFLOW) && !defined(__MSP430__)
// gcc should use the correct one here
#define mul_overflow __builtin_mul_overflow
#else
/**
 * Since __builtin_mul_overflow doesn't seem to exist,
 * use a (slower) software implementation instead.
 * This is only implemented for size_t, so in case mallocr.c's INTERNAL_SIZE_T
 *  is non default, mallocr.c throws an #error.
 */
static int mul_overflow_size_t(size_t a, size_t b, size_t *res)
{
    // always fill the result (gcc doesn't define what happens here)
    if (res)
        *res = a * b;

    // multiply with 0 can not overflow (and avoid div-by-zero):
    if (a == 0 || b == 0)
        return 0;

#ifdef __MSP430__
    volatile uint32_t ia = (uint32_t) a;
    volatile uint32_t ib = (uint32_t) b;

    // check if overflow would happen:
    if ( (ia > SIZE_MAX / ib) || (ib > SIZE_MAX / ia)) {
        return 1;
    }
#else
    // check if overflow would happen:
    if ( (a > SIZE_MAX / b) || (b > SIZE_MAX / a)) {
        return 1;
    }
#endif

    return 0;
}
#define mul_overflow mul_overflow_size_t
#endif
