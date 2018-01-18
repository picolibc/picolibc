/* sincos -- currently no more efficient than two separate calls to
   sin and cos. */
#include "fdlibm.h"
#include <errno.h>

	void sincosf(float x, float *sinx, float *cosx)
{
  *sinx = sinf (x);
  *cosx = cosf (x);
}

#ifdef _DOUBLE_IS_32BITS

	void sincos(double x, double *sinx, double *cosx)
{
  *sinx = sinf((float) x);
  *cosx = cosf((float) x);
}
#endif /* defined(_DOUBLE_IS_32BITS) */
