/* bsd_mutex.cc

   Copyright 2003, 2004, 2005, 2007 Red Hat Inc.

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */
#ifdef __OUTSIDE_CYGWIN__
#include "woutsup.h"
#include <errno.h>
#define _KERNEL 1
#define __BSD_VISIBLE 1
#include <sys/smallprint.h>
#include <limits.h>
#include <stdlib.h>
#include <sys/msg.h>
#include <sys/sem.h>

#include "process.h"
#include "cygserver_ipc.h"

/* A BSD kernel global mutex. */
struct mtx Giant;

void
mtx_init (mtx *m, const char *name, const void *, int)
{
  m->name = name;
  m->owner = 0;
  m->cnt = 0;
  /* Can't use Windows Mutexes here since Windows Mutexes are only
     unlockable by the lock owner. */
  m->h = CreateSemaphore (NULL, 1, 1, NULL);
  if (!m->h)
    panic ("couldn't allocate %s mutex, %lu\n", name, GetLastError ());
}

void
_mtx_lock (mtx *m, DWORD winpid, const char *file, int line)
{
  _log (file, line, LOG_DEBUG, "Try locking mutex %s (%u) (hold: %u)",
	m->name, winpid, m->owner);
  if (WaitForSingleObject (m->h, INFINITE) != WAIT_OBJECT_0)
    _panic (file, line, "wait for %s in %d failed, %lu", m->name, winpid,
	    GetLastError ());
  m->owner = winpid;
  _log (file, line, LOG_DEBUG, "Locked      mutex %s/%u (%u)",
	m->name, ++m->cnt, winpid);
}

int
mtx_owned (mtx *m, DWORD winpid)
{
  return m->owner == winpid;
}

void
_mtx_assert (mtx *m, int what, DWORD winpid, const char *file, int line)
{
  switch (what)
    {
      case MA_OWNED:
        if (!mtx_owned (m, winpid))
	  _panic (file, line, "Mutex %s not owned", m->name);
	break;
      case MA_NOTOWNED:
        if (mtx_owned (m, winpid))
	  _panic (file, line, "Mutex %s is owned", m->name);
        break;
      default:
        break;
    }
}

void
_mtx_unlock (mtx *m, const char *file, int line)
{
  DWORD owner = m->owner;
  unsigned long cnt = m->cnt;
  m->owner = 0;
  /* Cautiously check if mtx_destroy has been called (shutdown).
     In that case, m->h is NULL. */
  if (m->h && !ReleaseSemaphore (m->h, 1, NULL))
    {
      /* Check if the semaphore was already on it's max value. */
      if (GetLastError () != ERROR_TOO_MANY_POSTS)
	_panic (file, line, "release of mutex %s failed, %lu", m->name,
		GetLastError ());
    }
  _log (file, line, LOG_DEBUG, "Unlocked    mutex %s/%u (owner: %u)",
  	m->name, cnt, owner);
}

void
mtx_destroy (mtx *m)
{
  HANDLE tmp = m->h;
  m->h = NULL;
  if (tmp)
    CloseHandle (tmp);
}

/*
 * Helper functions for msleep/wakeup.
 */

static int
win_priority (int priority)
{
  int p = (int)((priority) & PRIO_MASK) - PZERO;
  /* Generating a valid priority value is a bit tricky.  The only valid
     values on NT4 are -15, -2, -1, 0, 1, 2, 15. */
  switch (p)
    {
      case -15: case -14: case -13: case -12: case -11:
        return THREAD_PRIORITY_IDLE;
      case -10: case -9: case -8: case -7: case -6:
        return THREAD_PRIORITY_LOWEST;
      case -5: case -4: case -3: case -2: case -1:
        return THREAD_PRIORITY_BELOW_NORMAL;
      case 0:
        return THREAD_PRIORITY_NORMAL;
      case 1: case 2: case 3: case 4: case 5:
        return THREAD_PRIORITY_ABOVE_NORMAL;
      case 6: case 7: case 8: case 9: case 10:
      	return THREAD_PRIORITY_HIGHEST;
      case 11: case 12: case 13: case 14: case 15:
        return THREAD_PRIORITY_TIME_CRITICAL;
    }
  return THREAD_PRIORITY_NORMAL;
}

/*
 * Sets the thread priority, returns the old priority.
 */
static int
set_priority (int priority)
{
  int old_prio = GetThreadPriority (GetCurrentThread ());
  if (!SetThreadPriority (GetCurrentThread (), win_priority (priority)))
    log (LOG_WARNING,
    	  "Warning: Setting thread priority to %d failed with error %lu\n",
	  win_priority (priority), GetLastError ());
  return old_prio;
}

/*
 * Original description from BSD code:
 *
 * General sleep call.  Suspends the current process until a wakeup is
 * performed on the specified identifier.  The process will then be made
 * runnable with the specified priority.  Sleeps at most timo/hz seconds
 * (0 means no timeout).  If pri includes PCATCH flag, signals are checked
 * before and after sleeping, else signals are not checked.  Returns 0 if
 * awakened, EWOULDBLOCK if the timeout expires.  If PCATCH is set and a
 * signal needs to be delivered, ERESTART is returned if the current system
 * call should be restarted if possible, and EINTR is returned if the system
 * call should be interrupted by the signal (return EINTR).
 *
 * The mutex argument is exited before the caller is suspended, and
 * entered before msleep returns.  If priority includes the PDROP
 * flag the mutex is not entered before returning.
 */
static HANDLE msleep_glob_evt;

class msleep_sync_array
{
  enum msleep_action {
    MSLEEP_ENTER = 0,
    MSLEEP_LEAVE,
    MSLEEP_WAKEUP
  };

  CRITICAL_SECTION cs;
  long cnt;
  long max_cnt;
  struct msleep_record {
    void *ident;
    HANDLE wakeup_evt;
    LONG threads;
  } *a;

  int find_ident (void *ident, msleep_action action)
  {
    int i;
    for (i = 0; i < cnt; ++i)
      if (a[i].ident == ident)
	return i;
    if (i >= max_cnt)
      panic ("ident %x not found and run out of slots.", ident);
    if (i >= cnt && action == MSLEEP_LEAVE)
      panic ("ident %x not found (%d).", ident, action);
    return i;
  }

  HANDLE first_entry (int i, void *ident)
  {
    debug ("New ident %x, index %d", ident, i);
    a[i].ident = ident;
    a[i].wakeup_evt = CreateEvent (NULL, TRUE, FALSE, NULL);
    if (!a[i].wakeup_evt)
      panic ("CreateEvent failed: %lu", GetLastError ());
    debug ("i = %d, CreateEvent: %x", i, a[i].wakeup_evt);
    a[i].threads = 1;
    ++cnt;
    return a[i].wakeup_evt;
  }

  HANDLE next_entry (int i)
  {
    if (a[i].ident && WaitForSingleObject (a[i].wakeup_evt, 0) != WAIT_OBJECT_0)
      {
        ++a[i].threads;
	return a[i].wakeup_evt;
      }
    return NULL;
  }

public:

  msleep_sync_array (int count) : cnt (0), max_cnt (count)
  {
    InitializeCriticalSection (&cs);
    if (!(a = new msleep_record[count]))
      panic ("Allocating msleep records failed: %d", errno);
  }

  HANDLE enter (void *ident)
  {
    HANDLE evt = NULL;
    while (!evt)
      {
        EnterCriticalSection (&cs);
	int i = find_ident (ident, MSLEEP_ENTER);
	if (i >= cnt)
	  evt = first_entry (i, ident);
	else if (!(evt = next_entry (i)))
	  {
	    /* wakeup has been called, so sleep to wait until all
	       formerly waiting threads have left and retry. */
	    LeaveCriticalSection (&cs);
	    Sleep (1L);
	  }
      }
    LeaveCriticalSection (&cs);
    return evt;
  }

  void leave (void *ident)
  {
    EnterCriticalSection (&cs);
    int i = find_ident (ident, MSLEEP_LEAVE);
    if (--a[i].threads == 0)
      {
	debug ("i = %d, CloseEvent: %x", i, a[i].wakeup_evt);
	CloseHandle (a[i].wakeup_evt);
	a[i].ident = NULL;
	--cnt;
	if (i < cnt)
	  a[i] = a[cnt];
      }
    LeaveCriticalSection (&cs);
  }

  void wakeup (void *ident)
  {
    EnterCriticalSection (&cs);
    int i = find_ident (ident, MSLEEP_WAKEUP);
    if (i < cnt && a[i].ident)
      SetEvent (a[i].wakeup_evt);
    LeaveCriticalSection (&cs);
  }
};

static msleep_sync_array *msleep_sync;

void
msleep_init (void)
{
  extern struct msginfo msginfo;
  extern struct seminfo seminfo;

  msleep_glob_evt = CreateEvent (NULL, TRUE, FALSE, NULL);
  if (!msleep_glob_evt)
    panic ("CreateEvent in msleep_init failed: %lu", GetLastError ());
  long msgmni = support_msgqueues ? msginfo.msgmni : 0;
  long semmni = support_semaphores ? seminfo.semmni : 0;
  TUNABLE_INT_FETCH ("kern.ipc.msgmni", &msgmni);
  TUNABLE_INT_FETCH ("kern.ipc.semmni", &semmni);
  debug ("Try allocating msgmni (%d) + semmni (%d) msleep records",
  	 msgmni, semmni);
  msleep_sync = new msleep_sync_array (msgmni + semmni);
  if (!msleep_sync)
    panic ("Allocating msleep records in msleep_init failed: %d", errno);
}

int
_msleep (void *ident, struct mtx *mtx, int priority,
	const char *wmesg, int timo, struct thread *td)
{
  int ret = -1;

  HANDLE evt = msleep_sync->enter (ident);

  if (mtx)
    mtx_unlock (mtx);
  int old_priority = set_priority (priority);
  HANDLE obj[4] =
    {
      evt,
      msleep_glob_evt,
      td->client->handle (),
      td->client->signal_arrived ()
    };
  /* PCATCH handling.  If PCATCH is given and signal_arrived is a valid
     handle, then it's used in the WaitFor call and EINTR is returned. */
  int obj_cnt = 3;
  if ((priority & PCATCH)
      && td->client->signal_arrived () != INVALID_HANDLE_VALUE)
    obj_cnt = 4;
  switch (WaitForMultipleObjects (obj_cnt, obj, FALSE, timo ?: INFINITE))
    {
      case WAIT_OBJECT_0:	/* wakeup() has been called. */
	ret = 0;
	debug ("msleep wakeup called for %d", td->td_proc->winpid);
        break;
      case WAIT_OBJECT_0 + 1:	/* Shutdown event (triggered by wakeup_all). */
        priority |= PDROP;
	/*FALLTHRU*/
      case WAIT_OBJECT_0 + 2:	/* The dependent process has exited. */
	debug ("msleep process exit or shutdown for %d", td->td_proc->winpid);
	ret = EIDRM;
        break;
      case WAIT_OBJECT_0 + 3:	/* Signal for calling process arrived. */
	debug ("msleep process got signal for %d", td->td_proc->winpid);
        ret = EINTR;
	break;
      case WAIT_TIMEOUT:
        ret = EWOULDBLOCK;
        break;
      default:
	/* There's a chance that a process has been terminated before
	   WaitForMultipleObjects has been called.  In this case the handles
	   might be invalid.  The error code returned is ERROR_INVALID_HANDLE.
	   Since we can trust the values of these handles otherwise, we
	   treat an ERROR_INVALID_HANDLE as a normal process termination and
	   hope for the best. */
	if (GetLastError () != ERROR_INVALID_HANDLE)
	  panic ("wait in msleep (%s) failed, %lu", wmesg, GetLastError ());
	debug ("wait in msleep (%s) failed for %d, %lu", wmesg,
	       td->td_proc->winpid, GetLastError ());
	ret = EIDRM;
	break;
    }

  msleep_sync->leave (ident);

  set_priority (old_priority);

  if (mtx && !(priority & PDROP))
    mtx_lock (mtx);
  return ret;
}

/*
 * Make all threads sleeping on the specified identifier runnable.
 */
int
wakeup (void *ident)
{
  msleep_sync->wakeup (ident);
  return 0;
}

/*
 * Wakeup all sleeping threads.  Only called in the context of cygserver
 * shutdown.
 */
void
wakeup_all (void)
{
  SetEvent (msleep_glob_evt);
}
#endif /* __OUTSIDE_CYGWIN__ */
