/* connector for link */

#include <reent.h>
#include <unistd.h>

int
_DEFUN (link, (old, new),
     _CONST char *old _AND
     _CONST char *new)
{
  return _link_r (_REENT, old, new);
}
