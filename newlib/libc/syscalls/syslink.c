/* connector for link */

#include <reent.h>
#include <unistd.h>

int
_DEFUN (link, (old, new),
     const char *old,
     const char *new)
{
  return _link_r (_REENT, old, new);
}
