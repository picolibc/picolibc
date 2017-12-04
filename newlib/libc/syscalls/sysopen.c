/* connector for open */

#include <reent.h>
#include <fcntl.h>

#ifdef _HAVE_STDC

/* The prototype in <fcntl.h> uses ..., so we must correspond.  */

#include <stdarg.h>

int
open (const char *file,
        int flags, ...)
{
  va_list ap;
  int ret;

  va_start (ap, flags);
  ret = _open_r (_REENT, file, flags, va_arg (ap, int));
  va_end (ap);
  return ret;
}

#else /* ! _HAVE_STDC */

int 
open (file, flags, mode)
     const char *file;
     int flags;
     int mode;
{
  return _open_r (_REENT, file, flags, mode);
}

#endif /* ! _HAVE_STDC */
