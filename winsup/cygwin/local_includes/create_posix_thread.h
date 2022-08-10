/* create_posix_thread.h

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

#pragma once

extern "C"
{

  PVOID create_new_main_thread_stack (PVOID &allocationbase);
  DWORD pthread_wrapper (PVOID arg);
  HANDLE create_posix_thread (LPTHREAD_START_ROUTINE thread_func,
			      PVOID thread_arg, PVOID stackaddr,
			      ULONG stacksize, ULONG guardsize,
			      DWORD creation_flags, LPDWORD thread_id);

}
