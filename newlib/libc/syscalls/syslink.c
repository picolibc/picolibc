/* connector for link */

#include <reent.h>

int
_DEFUN (link, (old, new),
     char *old _AND
     char *new)
{
  return _link_r (_REENT, old, new);
}
