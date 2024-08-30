AC_CACHE_CHECK([for XCHAL_HAVE_FP_SQRT], newlib_cv_xchal_have_fp_sqrt, [dnl
    AC_PREPROC_IFELSE([AC_LANG_PROGRAM(
[[#define _LIBM
// targ-include does not exist yet, use relative path
#include "../machine/xtensa/include/xtensa/config/core-isa.h"
#if (!XCHAL_HAVE_FP_SQRT)
# error "Have not XCHAL_HAVE_FP_SQRT"
#endif
]])], [newlib_cv_xchal_have_fp_sqrt="yes"], [newlib_cv_xchal_have_fp_sqrt="no"])])

AM_CONDITIONAL(XTENSA_XCHAL_HAVE_FP_SQRT, test "$newlib_cv_xchal_have_fp_sqrt" = "yes")
