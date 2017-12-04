#include <stdlib.h>
#include <_ansi.h>

float
_DEFUN (atoff, (s),
	const char *s)
{
  return strtof (s, NULL);
}
