/* connector for sbrk */

#include <reent.h>
#include <unistd.h>

extern void *_sbrk_r (struct _reent *, ptrdiff_t);
extern void *_sbrk (ptrdiff_t);

void *
sbrk (ptrdiff_t incr)
{
  return _sbrk_r (_REENT, incr);
}
