/* bsd_mutex.cc

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
#include <sys/shm.h>
#include <sys/msg.h>
#include <sys/sem.h>

#include "bsd_helper.h"
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
    panic ("couldn't allocate %s mutex, %u\n", name, GetLastError ());
}

void
_mtx_lock (mtx *m, DWORD winpid, const char *file, int line)
{
  _log (file, line, LOG_DEBUG, "Try locking mutex %s (%u) (hold: %u)",
	m->name, winpid, m->owner);
  if (WaitForSingleObject (m->h, INFINITE) != WAIT_OBJECT_0)
    _panic (file, line, "wait for %s in %d failed, %u", m->name, winpid,
	    GetLastError ());
  m->owner = winpid;
  _log (file, line, LOG_DEBUG, "Locked      mutex %s/%u (owner: %u)",
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
	_panic (file, line, "release of mutex %s failed, %u", m->name,
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
    	  "Warning: Setting thread priority to %d failed with error %u\n",
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
  PHANDLE wakeup_evt;

public:

  msleep_sync_array (int count)
  {
    InitializeCriticalSection (&cs);
    wakeup_evt = (PHANDLE) calloc (count, sizeof (HANDLE));
    if (!wakeup_evt)
      panic ("Allocating msleep records failed: %d", errno);
  }

  ~msleep_sync_array () { free (wakeup_evt); }

  HANDLE enter (int idx)
  {
    if (!wakeup_evt[idx])
      {
	EnterCriticalSection (&cs);
	if (!wakeup_evt[idx])
	  {
	    wakeup_evt[idx] = CreateSemaphore (NULL, 0, 1024, NULL);
	    if (!wakeup_evt[idx])
	      panic ("CreateSemaphore failed: %u", GetLastError ());
	  }
	LeaveCriticalSection (&cs);
      }
    return wakeup_evt[idx];
  }

  void leave (int idx)
  {
    /* Placeholder */
  }

  void wakeup (int idx)
  {
    ReleaseSemaphore (wakeup_evt[idx], 1, NULL);
  }
};

static msleep_sync_array *msleep_sync;

extern struct msginfo msginfo;
extern struct seminfo seminfo;
extern struct shminfo shminfo;
int32_t mni[3];
int32_t off[3];

void
msleep_init (void)
{
  msleep_glob_evt = CreateEvent (NULL, TRUE, FALSE, NULL);
  if (!msleep_glob_evt)
    panic ("CreateEvent in msleep_init failed: %u", GetLastError ());
  mni[SHM] = support_sharedmem ? shminfo.shmmni : 0;
  mni[MSQ] = support_msgqueues ? msginfo.msgmni : 0;
  mni[SEM] = support_semaphores ? seminfo.semmni : 0;
  TUNABLE_INT_FETCH ("kern.ipc.shmmni", &mni[SHM]);
  TUNABLE_INT_FETCH ("kern.ipc.msgmni", &mni[MSQ]);
  TUNABLE_INT_FETCH ("kern.ipc.semmni", &mni[SEM]);
  debug ("Allocating shmmni (%d) + msgmni (%d) + semmni (%d) msleep records",
	 mni[SHM], mni[MSQ], mni[SEM]);
  msleep_sync = new msleep_sync_array (mni[SHM] + mni[MSQ] + mni[SEM]);
  if (!msleep_sync)
    panic ("Allocating msleep records in msleep_init failed: %d", errno);
  /* Convert mni values to offsets. */
  off[SHM] = 0;
  off[MSQ] = mni[SHM];
  off[SEM] = mni[SHM] + mni[MSQ];
}

int
_sleep (ipc_type type, int ident, struct mtx *mtx, int priority,
	const char *wmesg, int timo, struct thread *td)
{
  int ret = -1;

  HANDLE evt = msleep_sync->enter (off[type] + ident);

  if (mtx)
    mtx_unlock (mtx);

  int old_priority = set_priority (priority);

  HANDLE obj[4] =
    {
      evt,
      msleep_glob_evt,
      td->client->handle (),
      td->ipcblk->signal_arrived
    };
  /* PCATCH handling.  If PCATCH is given and signal_arrived is a valid
     handle, then it's used in the WaitFor call and EINTR is returned. */
  int obj_cnt = 3;
  if ((priority & PCATCH) && obj[3])
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
	  panic ("wait in msleep (%s) failed, %u", wmesg, GetLastError ());
	debug ("wait in msleep (%s) failed for %d, %u", wmesg,
	       td->td_proc->winpid, GetLastError ());
	ret = EIDRM;
	break;
    }

  set_priority (old_priority);

  if (mtx && !(priority & PDROP))
    mtx_lock (mtx);

  msleep_sync->leave (off[type] + ident);

  return ret;
}

/*
 * Make all threads sleeping on the specified identifier runnable.
 */
int
_wakeup (ipc_type type, int ident)
{
  msleep_sync->wakeup (off[type] + ident);
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
