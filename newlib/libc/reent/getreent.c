/* default reentrant pointer when multithread enabled */
#include <_ansi.h>
#include <reent.h>

#ifdef __getreent
#undef __getreent
#endif

struct _reent *
_DEFUN_VOID(__getreent)
{
#ifdef __CYGWIN__
  /* Utilize Cygwin's inline definition from include/cygwin/config.h
     (note the extra underscore) */
  return __inline_getreent ();
#else
  return _impure_ptr;
#endif
}
