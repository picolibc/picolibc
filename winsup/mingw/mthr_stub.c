/*
 * mthr_stub.c
 *
 * Implement Mingw thread-support stubs for single-threaded C++ apps.
 *
 * This file is used by if gcc is built with --enable-threads=win32 and
 * iff gcc does *NOT* use -mthreads option. 
 *
 * The -mthreads implementation is in mthr.c.
 *
 * Created by Mumit Khan  <khan@nanotech.wisc.edu>
 *
 */

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#undef WIN32_LEAN_AND_MEAN
#include <assert.h>

/*
 * __mingwthr_register_key_dtor (DWORD key, void (*dtor) (void *))
 *
 * Public interface called by C++ exception handling mechanism in
 * libgcc (cf: __gthread_key_create).
 *
 * THIS SHOULD NEVER BE CALLED!
 *
 */

int
__mingwthr_key_dtor (DWORD key, void (*dtor) (void *))
{
  assert (0);
  /* NOTREACHED */
  return 0;
}


int
__mingwthr_remove_key_dtor (DWORD key )
{
  assert (0);
  /* NOTREACHED */
  return 0;
}
