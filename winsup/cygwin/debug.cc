/* debug.cc

   Copyright 1998, 1999, 2000 Cygnus Solutions.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

#define NO_DEBUG_DEFINES
#include "winsup.h"
#include "exceptions.h"
#include "perthread.h"
#include "pinfo.h"

static muto NO_COPY *threadname_lock = NULL;
#define lock_threadname() \
  do {if (threadname_lock) threadname_lock->acquire (INFINITE); } while (0)

#define unlock_threadname() \
  do {if (threadname_lock) threadname_lock->release (); } while (0)

typedef struct
  {
    DWORD id;
    const char *name;
  } thread_info;

static NO_COPY thread_info threads[32] = {{0, NULL}};	// increase as necessary
#define NTHREADS (sizeof(threads) / sizeof(threads[0]))

void
threadname_init ()
{
  threadname_lock = new_muto (FALSE, "threadname_lock");
}

void __stdcall
regthread (const char *name, DWORD tid)
{
  lock_threadname ();
  for (DWORD i = 0; i < NTHREADS; i++)
    if (threads[i].name == NULL || strcmp (threads[i].name, name) == 0 ||
	threads[i].id == tid)
      {
	threads[i].name = name;
	threads[i].id = tid;
	break;
      }
  unlock_threadname ();
}

int __stdcall
iscygthread()
{
  DWORD tid = GetCurrentThreadId ();
  if (tid != mainthread.id)
    for (DWORD i = 0; i < NTHREADS && threads[i].name != NULL; i++)
      if (threads[i].id == tid)
	return 1;
  return 0;
}

struct thread_start
  {
    LONG notavail;
    LPTHREAD_START_ROUTINE func;
    VOID *arg;
  };

/* A place to store arguments to thread_stub since they can't be
  stored on the stack.  An available element is !notavail. */
thread_start NO_COPY start_buf[NTHREADS] = {{0, NULL,NULL}};

/* Initial stub called by makethread. Performs initial per-thread
   initialization.  */
static DWORD WINAPI
thread_stub (VOID *arg)
{
  LPTHREAD_START_ROUTINE threadfunc = ((thread_start *) arg)->func;
  VOID *threadarg = ((thread_start *) arg)->arg;

  exception_list except_entry;

  /* Give up our slot in the start_buf array */
  InterlockedExchange (&((thread_start *) arg)->notavail, 0);

  /* Initialize this thread's ability to respond to things like
     SIGSEGV or SIGFPE. */
  init_exceptions (&except_entry);

  set_reent (user_data->impure_ptr);
  ExitThread (threadfunc (threadarg));
}

/* Wrapper for CreateThread.  Registers the thread name/id and ensures that
   cygwin threads are properly initialized. */
HANDLE __stdcall
makethread (LPTHREAD_START_ROUTINE start, LPVOID param, DWORD flags,
	    const char *name)
{
  DWORD tid;
  HANDLE h;
  SECURITY_ATTRIBUTES *sa;
  thread_start *info;	/* Various information needed by the newly created thread */

  for (;;)
    {
      /* Search the start_buf array for an empty slot to use */
      for (info = start_buf; info < start_buf + NTHREADS; info++)
	if (!InterlockedExchange (&info->notavail, 1))
	  goto out;

      /* Should never hit here, but be defensive anyway. */
      Sleep (0);
    }

out:
  info->func = start;	/* Real function to start */
  info->arg = param;	/* The single parameter to the thread */

  if (*name != '+')
    sa = &sec_none_nih;	/* The handle should not be inherited by subprocesses. */
  else
    {
      name++;
      sa = &sec_none;	/* The handle should be inherited by subprocesses. */
    }

  if ((h = CreateThread (sa, 0, thread_stub, (VOID *) info, flags, &tid)))
    regthread (name, tid);	/* Register this name/thread id for debugging output. */

  return h;
}

/* Return the symbolic name of the current thread for debugging.
 */
const char * __stdcall
threadname (DWORD tid, int lockit)
{
  const char *res = NULL;
  if (!tid)
    tid = GetCurrentThreadId ();

  if (lockit)
    lock_threadname ();
  for (DWORD i = 0; i < NTHREADS && threads[i].name != NULL; i++)
    if (threads[i].id == tid)
      {
	res = threads[i].name;
	break;
      }
  if (lockit)
    unlock_threadname ();

  if (!res)
    {
      static char buf[30] NO_COPY = {0};
      __small_sprintf (buf, "unknown (%p)", tid);
      res = buf;
    }

  return res;
}

#ifdef DEBUGGING
/* Here lies extra debugging routines which help track down internal
   Cygwin problems when compiled with -DDEBUGGING . */
#include <stdlib.h>

typedef struct _h
  {
    BOOL allocated;
    HANDLE h;
    const char *name;
    const char *func;
    int ln;
    struct _h *next;
  } handle_list;

static NO_COPY handle_list starth = {0, NULL, NULL, NULL, 0, NULL};
static NO_COPY handle_list *endh = NULL;

static handle_list NO_COPY freeh[1000] = {{0, NULL, NULL, NULL, 0, NULL}};
#define NFREEH (sizeof (freeh) / sizeof (freeh[0]))

static muto NO_COPY *debug_lock = NULL;

#define lock_debug() \
  do {if (debug_lock) debug_lock->acquire (INFINITE); } while (0)

#define unlock_debug() \
  do {if (debug_lock) debug_lock->release (); } while (0)

void
debug_init ()
{
  debug_lock = new_muto (FALSE, "debug_lock");
}

/* Find a registered handle in the linked list of handles. */
static handle_list * __stdcall
find_handle (HANDLE h)
{
  handle_list *hl;
  for (hl = &starth; hl->next != NULL; hl = hl->next)
    if (hl->next->h == h)
      goto out;
  endh = hl;
  hl = NULL;

out:
  return hl;
}

/* Create a new handle record */
static handle_list * __stdcall
newh ()
{
  handle_list *hl;
  lock_debug ();
  for (hl = freeh; hl < freeh + NFREEH; hl++)
    if (hl->name == NULL)
      goto out;

  /* All used up??? */
  if ((hl = (handle_list *)malloc (sizeof *hl)) != NULL)
    {
      memset (hl, 0, sizeof (*hl));
      hl->allocated = TRUE;
    }

out:
  unlock_debug ();
  return hl;
}

/* Add a handle to the linked list of known handles. */
void __stdcall
add_handle (const char *func, int ln, HANDLE h, const char *name)
{
  handle_list *hl;
  lock_debug ();

  if (find_handle (h))
    goto out;		/* Already did this once */

  if ((hl = newh()) == NULL)
    {
      unlock_debug ();
      system_printf ("couldn't allocate memory for %s(%d): %s(%p)",
		     func, ln, name, h);
      return;
    }
  hl->h = h;
  hl->name = name;
  hl->func = func;
  hl->ln = ln;
  hl->next = NULL;
  endh->next = hl;
  endh = hl;

out:
  unlock_debug ();
}

/* Close a known handle.  Complain if !force and closing a known handle or
   if the name of the handle being closed does not match the registered name. */
BOOL __stdcall
close_handle (const char *func, int ln, HANDLE h, const char *name, BOOL force)
{
  BOOL ret;
  handle_list *hl;
  lock_debug ();

  if ((hl = find_handle (h)) && !force)
    {
      hl = hl->next;
      unlock_debug ();	// race here
      system_printf ("attempt to close protected handle %s:%d(%s<%p>)",
		     hl->func, hl->ln, hl->name, hl->h);
      system_printf (" by %s:%d(%s<%p>)", func, ln, name, h);
      return FALSE;
    }

  handle_list *hln;
  if (hl && (hln = hl->next) && strcmp (name, hln->name))
    {
      system_printf ("closing protected handle %s:%d(%s<%p>)",
		     hln->func, hln->ln, hln->name, hln->h);
      system_printf (" by %s:%d(%s<%p>)", func, ln, name, h);
    }
  ret = CloseHandle (h);
  if (hl)
    {
      handle_list *hnuke = hl->next;
      hl->next = hl->next->next;
      if (hnuke->allocated)
	free (hnuke);
      else
	memset (hnuke, 0, sizeof (*hnuke));
    }

  unlock_debug ();
  return ret;
}
#endif /*DEBUGGING*/
