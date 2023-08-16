#include <xtensa/config/core-isa.h>

#if !XCHAL_HAVE_FP_SQRT
#error "__ieee754_sqrtf from common libm must be used"
#else
/* Built-in GCC __ieee754_sqrtf must be used */
#endif
