/* libccrt0.cc: crt0 for libc [newlib calls this one]

   Copyright 1996, 1998, 1999, 2000 Cygnus Solutions.

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

#undef MALLOC_DEBUG

#include "winsup.h"
#include "sys/cygwin.h"
#include <reent.h>
#include <stdlib.h>

typedef int (*MainFunc) (int argc, char *argc[], char **env);

extern "C"
{
  char **environ;
  void cygwin_crt0 (MainFunc);
  int cygwin_attach_dll (HMODULE, MainFunc);
  int cygwin_attach_noncygwin_dll (HMODULE, MainFunc);
  int main (int, char **, char **);
  struct _reent *_impure_ptr;
  int _fmode;
};

static per_process this_proc;

/* Set up pointers to various pieces so the dll can then use them,
   and then jump to the dll.  */

static void
cygwin_crt0_common (MainFunc f)
{
  /* This is used to record what the initial sp was.  The value is needed
     when copying the parent's stack to the child during a fork.  */
  int onstack;

  /* The version numbers are the main source of compatibility checking.
     As a backup to them, we use the size of the per_process struct.  */
  this_proc.magic_biscuit = sizeof (per_process);

  /* cygwin.dll version number in effect at the time the app was created.  */
  this_proc.dll_major = CYGWIN_VERSION_DLL_MAJOR;
  this_proc.dll_minor = CYGWIN_VERSION_DLL_MINOR;
  this_proc.api_major = CYGWIN_VERSION_API_MAJOR;
  this_proc.api_minor = CYGWIN_VERSION_API_MINOR;

  this_proc.ctors = &__CTOR_LIST__;
  this_proc.dtors = &__DTOR_LIST__;
  this_proc.envptr = &environ;
  this_proc.impure_ptr_ptr = &_impure_ptr;
  this_proc.main = f;
  this_proc.premain[0] = cygwin_premain0;
  this_proc.premain[1] = cygwin_premain1;
  this_proc.premain[2] = cygwin_premain2;
  this_proc.premain[3] = cygwin_premain3;
  this_proc.fmode_ptr = &_fmode;
  this_proc.initial_sp = (char *) &onstack;

  /* Remember whatever the user linked his application with - or
     point to entries in the dll.  */
  this_proc.malloc = &malloc;
  this_proc.free = &free;
  this_proc.realloc = &realloc;
  this_proc.calloc = &calloc;

  /* Setup the module handle so fork can get the path name. */
  this_proc.hmodule = GetModuleHandle (0);

  /* variables for fork */
  this_proc.data_start = &_data_start__;
  this_proc.data_end = &_data_end__;
  this_proc.bss_start = &_bss_start__;
  this_proc.bss_end = &_bss_end__;
}

/* for main module */
void
cygwin_crt0 (MainFunc f)
{
  cygwin_crt0_common (f);

 /* Jump into the dll. */
  dll_crt0 (&this_proc);
}

/* for a loaded dll */
int
cygwin_attach_dll (HMODULE h, MainFunc f)
{
  cygwin_crt0_common (f);

  /* jump into the dll. */
  return dll_dllcrt0 (h, &this_proc);
}
