/* connector for unlink */

#include <reent.h>
#include <unistd.h>

int
_DEFUN (unlink, (file),
        _CONST char *file)
{
  return _unlink_r (_REENT, file);
}
