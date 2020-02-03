/* Copyright (c) 2002 Jeff Johnston <jjohnstn@redhat.com> */
/* default reentrant pointer when multithread enabled */

#ifdef GETREENT_PROVIDED

int _dummy_getreent;

#else

#include <_ansi.h>
#include <reent.h>

#ifdef __getreent
#undef __getreent
#endif

struct _reent *
__getreent (void)
{
  return _impure_ptr;
}

#endif
