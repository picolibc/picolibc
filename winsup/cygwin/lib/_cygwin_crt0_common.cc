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

extern __declspec(dllimport) per_process __cygwin_user_data;

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

void
_cygwin_crt0_common (MainFunc f)
{
  /* This is used to record what the initial sp was.  The value is needed
     when copying the parent's stack to the child during a fork.  */
  int onstack;

  /* The version numbers are the main source of compatibility checking.
     As a backup to them, we use the size of the per_process struct.  */
  __cygwin_user_data.magic_biscuit = sizeof (per_process);

  /* cygwin.dll version number in effect at the time the app was created.  */
  __cygwin_user_data.dll_major = CYGWIN_VERSION_DLL_MAJOR;
  __cygwin_user_data.dll_minor = CYGWIN_VERSION_DLL_MINOR;
  __cygwin_user_data.api_major = CYGWIN_VERSION_API_MAJOR;
  __cygwin_user_data.api_minor = CYGWIN_VERSION_API_MINOR;

  __cygwin_user_data.ctors = &__CTOR_LIST__;
  __cygwin_user_data.dtors = &__DTOR_LIST__;
  __cygwin_user_data.envptr = &environ;
  _impure_ptr = __cygwin_user_data.impure_ptr;
  __cygwin_user_data.main = f;
  __cygwin_user_data.premain[0] = cygwin_premain0;
  __cygwin_user_data.premain[1] = cygwin_premain1;
  __cygwin_user_data.premain[2] = cygwin_premain2;
  __cygwin_user_data.premain[3] = cygwin_premain3;
  __cygwin_user_data.fmode_ptr = &_fmode;
  __cygwin_user_data.initial_sp = (char *) &onstack;

  /* Remember whatever the user linked his application with - or
     point to entries in the dll.  */
  __cygwin_user_data.malloc = &malloc;
  __cygwin_user_data.free = &free;
  __cygwin_user_data.realloc = &realloc;
  __cygwin_user_data.calloc = &calloc;

  /* Setup the module handle so fork can get the path name. */
  __cygwin_user_data.hmodule = GetModuleHandle (0);

  /* variables for fork */
  __cygwin_user_data.data_start = &_data_start__;
  __cygwin_user_data.data_end = &_data_end__;
  __cygwin_user_data.bss_start = &_bss_start__;
  __cygwin_user_data.bss_end = &_bss_end__;
}
} /* "C" */
