/* common.cc: common crt0 function for cygwin crt0's.

   Copyright 2000 Cygnus Solutions.

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

#include "winsup.h"
#include "crt0.h"
#include <reent.h>
#include <stdlib.h>

extern "C"
{
char **environ;
void cygwin_crt0 (MainFunc);
int cygwin_attach_dll (HMODULE, MainFunc);
int cygwin_attach_noncygwin_dll (HMODULE, MainFunc);
int main (int, char **, char **);
struct _reent *_impure_ptr;
int _fmode;

/* Set up pointers to various pieces so the dll can then use them,
   and then jump to the dll.  */

int __stdcall
_cygwin_crt0_common (MainFunc f, per_process *u)
{
  /* This is used to record what the initial sp was.  The value is needed
     when copying the parent's stack to the child during a fork.  */
  DWORD newu;
  int uwasnull;

  if (u != NULL)
    uwasnull = 0;	/* Caller allocated space for per_process structure */
  else if ((newu = cygwin_internal (CW_USER_DATA)) == (DWORD) -1)
    return 0;
  else
    {
      u = (per_process *) newu;	/* Using DLL built-in per_process */
      uwasnull = 1;	/* Remember for later */
    }

  /* The version numbers are the main source of compatibility checking.
     As a backup to them, we use the size of the per_process struct.  */
  u->magic_biscuit = sizeof (per_process);

  /* cygwin.dll version number in effect at the time the app was created.  */
  u->dll_major = CYGWIN_VERSION_DLL_MAJOR;
  u->dll_minor = CYGWIN_VERSION_DLL_MINOR;
  u->api_major = CYGWIN_VERSION_API_MAJOR;
  u->api_minor = CYGWIN_VERSION_API_MINOR;

  u->ctors = &__CTOR_LIST__;
  u->dtors = &__DTOR_LIST__;
  u->envptr = &environ;
  if (uwasnull)
    _impure_ptr = u->impure_ptr;	/* Use field initialized in newer DLLs. */
  else
    u->impure_ptr_ptr = &_impure_ptr;	/* Older DLLs need this. */
      
  u->forkee = 0;			/* This should only be set in dcrt0.cc
					   when the process is actually forked */
  u->main = f;

  /* These functions are executed prior to main.  They are just stubs unless the
     user overrides them. */
  u->premain[0] = cygwin_premain0;
  u->premain[1] = cygwin_premain1;
  u->premain[2] = cygwin_premain2;
  u->premain[3] = cygwin_premain3;
  u->fmode_ptr = &_fmode;
  u->initial_sp = (char *) __builtin_frame_address (1);

  /* Remember whatever the user linked his application with - or
     point to entries in the dll.  */
  u->malloc = &malloc;
  u->free = &free;
  u->realloc = &realloc;
  u->calloc = &calloc;

  /* Setup the module handle so fork can get the path name. */
  u->hmodule = GetModuleHandle (0);

  /* variables for fork */
  u->data_start = &_data_start__;
  u->data_end = &_data_end__;
  u->bss_start = &_bss_start__;
  u->bss_end = &_bss_end__;
  return 1;
}
} /* "C" */
