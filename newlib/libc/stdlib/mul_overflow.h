#ifdef _HAVE_BUILTIN_MUL_OVERFLOW
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
        return 1;

    // check if overflow would happen:
    if ( (a > SIZE_MAX / b) || (b > SIZE_MAX / a))
        return 1;

    return 0;
}
#define mul_overflow mul_overflow_size_t
#endif
