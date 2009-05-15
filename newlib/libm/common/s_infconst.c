/* Infinity as a constant value.   This is used for HUGE_VAL.
 * Added by Cygnus Support.
 */

#include <float.h>
#include <math.h>

/* These should never actually be used any longer, as their use in math.h was
 * removed, but they are kept here in case a user was pointing to them.
 * FIXME:  deprecate these identifiers and then delete them.  */
 
/* Float version of infinity.  */
const union __fmath __infinityf[1] = { { FLT_MAX+FLT_MAX } };

/* Double version of infinity.  */
const union __dmath __infinity[1] = { { DBL_MAX+DBL_MAX } };

/* Long double version of infinity.  */
#if defined(_HAVE_LONG_DOUBLE)
const union __ldmath __infinityld[1] = { { LDBL_MAX+LDBL_MAX } };
#endif
