/*
 * gmtime_r.c
 */

#include <time.h>

#define _GMT_OFFSET 0

struct tm *
_DEFUN (gmtime_r, (tim_p, res),
	_CONST time_t * tim_p _AND
	struct tm *res)
{
  time_t tim = *tim_p + _GMT_OFFSET;

  return (localtime_r (&tim, res));
}
