/*
 * gmtime_r.c
 */

#include <time.h>
#include "local.h"

struct tm *
_DEFUN (gmtime_r, (tim_p, res),
	_CONST time_t *__restrict tim_p _AND
	struct tm *__restrict res)
{
  return (_mktm_r (tim_p, res, 1));
}
