#include <float.h>

/* Check if long double is as wide as double. */
#if (!defined(__STRICT_ANSI__) || __STDC_VERSION__ > 199901L || \
  defined(__cplusplus)) && defined(LDBL_MANT_DIG) && \
    (DBL_MANT_DIG == LDBL_MANT_DIG && LDBL_MIN_EXP == DBL_MIN_EXP && \
    LDBL_MAX_EXP == DBL_MAX_EXP)
 #define _LDBL_EQ_DBL
#endif
