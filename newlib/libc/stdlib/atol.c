/*
 * Andy Wilson, 2-Oct-89.
 */

#include <stdlib.h>
#include <_ansi.h>

long
_DEFUN (atol, (s), _CONST char *s)
{
  return strtol (s, NULL, 10);
}
