/* connector for unlink */

#include <reent.h>

int
_DEFUN (unlink, (file),
        char *file)
{
  return _unlink_r (_REENT, file);
}
