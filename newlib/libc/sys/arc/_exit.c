#include <reent.h>

void
_exit (int code)
{
  _exit_r (_REENT, code);
}
