#include <machine/syscall.h>
#include <sys/stat.h>
#include "internal_syscall.h"

extern int _fstat(int file, struct stat *st);

/* Query whether output stream is a terminal. For consistency with the
   other minimal implementations, which only support output to stdout,
   this minimal implementation is suggested by the newlib docs.  */

int
_isatty(int file)
{
  struct stat s;
  int ret = _fstat (file, &s);
  return ret == -1 ? 0 : !!(s.st_mode & S_IFCHR);
}
