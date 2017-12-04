/* connector for link */

#include <reent.h>
#include <unistd.h>

int
link (const char *old,
     const char *new)
{
  return _link_r (_REENT, old, new);
}
