/* cygtls.h

   Copyright 2003 Red Hat, Inc.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

#include "winsup.h"
#include "thread.h"
#include "cygtls.h"
#include "assert.h"
#include <syslog.h>

static _threadinfo NO_COPY dummy_thread;
_threadinfo NO_COPY *_last_thread = &dummy_thread;

CRITICAL_SECTION NO_COPY _threadinfo::protect_linked_list;

void
_threadinfo::set_state (bool is_exception)
{
  initialized = CYGTLS_INITIALIZED + is_exception;
}

void
_threadinfo::reset_exception ()
{
  if (initialized == CYGTLS_EXCEPTION)
    {
#ifdef DEBUGGING
      debug_printf ("resetting stack after an exception stack %p, stackptr %p", stack, stackptr);
#endif
      set_state (false);
      stackptr--;
    }
}

/* Two calls to get the stack right... */
void
_threadinfo::call (DWORD (*func) (void *, void *), void *arg)
{
  char buf[CYGTLS_PADSIZE];
  call2 (func, arg, buf);
}

void
_threadinfo::call2 (DWORD (*func) (void *, void *), void *arg, void *buf)
{
  _my_tls.init_thread (buf);
  ExitThread (func (arg, buf));
}

void
_threadinfo::init ()
{
  InitializeCriticalSection (&protect_linked_list);
}

void
_threadinfo::init_thread (void *x)
{
  if (x)
    {
      memset (this, 0, sizeof (*this));
      stackptr = stack;
      if (_GLOBAL_REENT)
	{
	  local_clib._stdin = _GLOBAL_REENT->_stdin;
	  local_clib._stdout = _GLOBAL_REENT->_stdout;
	  local_clib._stderr = _GLOBAL_REENT->_stderr;
	  local_clib.__sdidinit = _GLOBAL_REENT->__sdidinit;
	  local_clib.__cleanup = _GLOBAL_REENT->__cleanup;
	}
      local_clib._current_locale = "C";
      locals.process_logmask = LOG_UPTO (LOG_DEBUG);
    }

  EnterCriticalSection (&protect_linked_list);
  prev = _last_thread;
  _last_thread->next = this;
  _last_thread = this;
  LeaveCriticalSection (&protect_linked_list);

  set_state (false);
  errno_addr = &(local_clib._errno);
}

void
_threadinfo::remove ()
{
  EnterCriticalSection (&protect_linked_list);
  if (prev)
    {
      prev->next = next;
      if (next)
	next->prev = prev;
      if (this == _last_thread)
	_last_thread = prev;
      prev = next = NULL;
    }
  LeaveCriticalSection (&protect_linked_list);
}

void
_threadinfo::push (__stack_t addr, bool exception)
{
  *stackptr++ = (__stack_t) addr;
  set_state (exception);
}

__stack_t
_threadinfo::pop ()
{
#ifdef DEBUGGING
  assert (stackptr > stack);
#endif
  __stack_t res = *--stackptr;
#ifdef DEBUGGING
  *stackptr = 0;
  debug_printf ("popped %p, stack %p, stackptr %p", res, stack, stackptr);
#endif
  return res;
}

