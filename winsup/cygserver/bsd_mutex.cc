/* bsd_mutex.cc

   Copyright 2003, 2004 Red Hat Inc.

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

#include "process.h"
#include "cygserver_ipc.h"

/* A BSD kernel global mutex. */
struct mtx Giant;

void
mtx_init (mtx *m, const char *name, const void *, int)
{
  m->name = name;
  m->owner = 0;
  /* Can't use Windows Mutexes here since Windows Mutexes are only
     unlockable by the lock owner. */
  m->h = CreateSemaphore (NULL, 1, 1, NULL);
  if (!m->h)
    panic ("couldn't allocate %s mutex, %E\n", name);
}

void
_mtx_lock (mtx *m, DWORD winpid, const char *file, int line)
{
  _log (file, line, LOG_DEBUG, "Try locking mutex %s", m->name);
  if (WaitForSingleObject (m->h, INFINITE) != WAIT_OBJECT_0)
    _panic (file, line, "wait for %s in %d failed, %E", m->name, winpid);
  m->owner = winpid;
  _log (file, line, LOG_DEBUG, "Locked      mutex %s", m->name);
}

int
mtx_owned (mtx *m)
{
  return m->owner > 0;
}

void
_mtx_assert (mtx *m, int what, const char *file, int line)
{
  switch (what)
    {
      case MA_OWNED:
        if (!mtx_owned (m))
	  _panic (file, line, "Mutex %s not owned", m->name);
	break;
      case MA_NOTOWNED:
        if (mtx_owned (m))
	  _panic (file, line, "Mutex %s is owned", m->name);
        break;
      default:
        break;
    }
}

void
_mtx_unlock (mtx *m, const char *file, int line)
{
  m->owner = 0;
  /* Cautiously check if mtx_destroy has been called (shutdown).
     In that case, m->h is NULL. */
  if (m->h && !ReleaseSemaphore (m->h, 1, NULL))
    {
      /* Check if the semaphore was already on it's max value.  In this case,
         ReleaseSemaphore returns FALSE with an error code which *sic* depends
	 on the OS. */
      if (  (!wincap.is_winnt () && GetLastError () != ERROR_INVALID_PARAMETER)
          || (wincap.is_winnt () && GetLastError () != ERROR_TOO_MANY_POSTS))
	_panic (file, line, "release of mutex %s failed, %E", m->name);
    }
  _log (file, line, LOG_DEBUG, "Unlocked    mutex %s", m->name);
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

/* Values for which */
#define MSLEEP_MUTEX	0
#define MSLEEP_SEM	1
#define MSLEEP_EVENT	2

static char *
msleep_event_name (void *ident, char *name, int which)
{
  if (wincap.has_terminal_services ())
    __small_sprintf (name, "Global\\cygserver.msleep.evt.%1d.%08x",
		     which, ident);
  else
    __small_sprintf (name, "cygserver.msleep.evt.%1d.%08x", which, ident);
  return name;
}

static int
win_priority (int priority)
{
  int p = (int)((priority) & PRIO_MASK) - PZERO;
  /* Generating a valid priority value is a bit tricky.  The only valid
     values on 9x and NT4 are -15, -2, -1, 0, 1, 2, 15. */
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

void
msleep_init (void)
{
  msleep_glob_evt = CreateEvent (NULL, TRUE, FALSE, NULL);
  if (!msleep_glob_evt)
    panic ("CreateEvent in msleep_init failed: %E");
}

int
_msleep (void *ident, struct mtx *mtx, int priority,
	const char *wmesg, int timo, struct thread *td)
{
  int ret = -1;
  char name[64];

  /* The mutex is used to indicate an ident specific critical section.
     The critical section is needed to synchronize access to the
     semaphore and eventually the event object.  The whole idea is
     that a wakeup is *guaranteed* to wakeup *all* threads.  If that's
     not synchronized, sleeping threads could return into the msleep
     function before all other threads have called CloseHandle(evt).
     That's bad, since the event still exists and is signalled! */
  HANDLE mutex = CreateMutex (NULL, FALSE,
			      msleep_event_name (ident, name, MSLEEP_MUTEX));
  if (!mutex)
    panic ("CreateMutex in msleep (%s) failed: %E", wmesg);
  WaitForSingleObject (mutex, INFINITE);

  /* Ok, we're in the critical section now.  We create an ident specific
     semaphore, which is used to synchronize the waiting threads. */
  HANDLE sem = CreateSemaphore (NULL, 0, LONG_MAX,
				msleep_event_name (ident, name, MSLEEP_SEM));
  if (!sem)
    panic ("CreateSemaphore in msleep (%s) failed: %E", wmesg);

  /* This thread is one more thread sleeping.  The semaphore value is
     so used as a counter of sleeping threads.  That info is needed by
     the wakeup function. */
  ReleaseSemaphore (sem, 1, NULL);

  /* Leave critical section. */
  ReleaseMutex (mutex);

  HANDLE evt = CreateEvent (NULL, TRUE, FALSE,
			    msleep_event_name (ident, name, MSLEEP_EVENT));
  if (!evt)
    panic ("CreateEvent in msleep (%s) failed: %E", wmesg);
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
        break;
      case WAIT_OBJECT_0 + 1:	/* Shutdown event (triggered by wakeup_all). */
        priority |= PDROP;
	/*FALLTHRU*/
      case WAIT_OBJECT_0 + 2:	/* The dependent process has exited. */
	ret = EIDRM;
        break;
      case WAIT_OBJECT_0 + 3:	/* Signal for calling process arrived. */
        ret = EINTR;
	break;
      case WAIT_TIMEOUT:
        ret = EWOULDBLOCK;
        break;
      default:
	panic ("wait in msleep (%s) failed, %E", wmesg);
	break;
    }

  CloseHandle (evt);
  /* wakeup has reset the semaphore to 0.  Now indicate that this thread
     has called CloseHandle (evt) and enter the critical section.  The
     critical section is still hold by wakeup, until all formerly sleeping
     threads have indicated that the event has been dismissed.  That's
     the signal for wakeup that it's the only thread still holding a
     handle to the event object.  wakeup will then close the last handle
     and leave the critical section. */
  ReleaseSemaphore (sem, 1, NULL);
  WaitForSingleObject (mutex, INFINITE);
  CloseHandle (sem);
  ReleaseMutex (mutex);
  CloseHandle (mutex);

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
  char name[64];
  LONG threads;

  HANDLE evt = OpenEvent (EVENT_MODIFY_STATE, FALSE,
  			  msleep_event_name (ident, name, MSLEEP_EVENT));
  if (!evt) /* No thread is waiting. */
    {
      /* Another round of different error codes returned by 9x and NT
         systems. Oh boy... */
      if (  (!wincap.is_winnt () && GetLastError () != ERROR_INVALID_NAME)
	  || (wincap.is_winnt () && GetLastError () != ERROR_FILE_NOT_FOUND))
	panic ("OpenEvent (%s) in wakeup failed: %E", name);
      return 0;
    }

  /* The mutex is used to indicate an ident specific critical section.
     The critical section is needed to synchronize access to the
     semaphore and eventually the event object.  The whole idea is
     that a wakeup is *guaranteed* to wakeup *all* threads.  If that's
     not synchronized, sleeping threads could return into the msleep
     function before all other threads have called CloseHandle(evt).
     That's bad, since the event still exists and is signalled! */
  HANDLE mutex = OpenMutex (MUTEX_ALL_ACCESS, FALSE,
			    msleep_event_name (ident, name, MSLEEP_MUTEX));
  if (!mutex)
    panic ("OpenMutex (%s) in wakeup failed: %E", name);
  WaitForSingleObject (mutex, INFINITE);
  /* Ok, we're in the critical section now.  We create an ident specific
     semaphore, which is used to synchronize the waiting threads. */
  HANDLE sem = OpenSemaphore (SEMAPHORE_ALL_ACCESS, FALSE,
			      msleep_event_name (ident, name, MSLEEP_SEM));
  if (!sem)
    panic ("OpenSemaphore (%s) in wakeup failed: %E", name);
  ReleaseSemaphore (sem, 1, &threads);
  /* `threads' is the number of waiting threads.  Now reset the semaphore
     to 0 and wait for this number of threads to indicate that they have
     called CloseHandle (evt).  Then it's save to do the same here in
     wakeup, which then means that the event object is destroyed and
     can get safely recycled. */
  for (int i = threads + 1; i > 0; --i)
    WaitForSingleObject (sem, INFINITE);

  if (!SetEvent (evt))
    panic ("SetEvent (%s) in wakeup failed, %E", name);

  /* Now wait for all threads which were waiting for this wakeup. */
  while (threads-- > 0)
    WaitForSingleObject (sem, INFINITE);

  /* Now our handle is the last handle to this event object. */
  CloseHandle (evt);
  /* But paranoia rulez, so we check here again. */
  evt = OpenEvent (EVENT_MODIFY_STATE, FALSE,
		   msleep_event_name (ident, name, MSLEEP_EVENT));
  if (evt)
    panic ("Event %s has not been destroyed.  Obviously I can't count :-(",
	   name);

  CloseHandle (sem);

  /* Leave critical section (all of wakeup is critical). */
  ReleaseMutex (mutex);
  CloseHandle (mutex);

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
