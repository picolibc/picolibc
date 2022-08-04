/* aio.cc: Posix asynchronous i/o functions.

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

#include "winsup.h"
#include "path.h"
#include "fhandler.h"
#include "dtable.h"
#include "cygheap.h"
#include "sigproc.h"
#include <aio.h>
#include <fcntl.h>
#include <semaphore.h>
#include <unistd.h>

#ifdef __cplusplus
extern "C" {
#endif

/* 'aioinitialized' is a thread-safe status of AIO feature initialization:
 * 0 means uninitialized, >0 means initializing, <0 means initialized
 */
static NO_COPY volatile LONG    aioinitialized = 0;

/* This implementation supports two flavors of asynchronous operation:
 * "inline" and "queued".  Inline AIOs are used when:
 *     (1) fd refers to a local non-locked disk file opened in binary mode,
 *     (2) no more than AIO_MAX inline AIOs will be in progress at same time.
 * In all other cases queued AIOs will be used.
 *
 * An inline AIO is performed by the calling app's thread as a pread|pwrite on
 * a shadow fd that permits Windows asynchronous i/o, with event notification
 * on completion.  Event arrival causes AIO context for the fd to be updated.
 *
 * A queued AIO is performed in a similar manner, but by an AIO worker thread
 * rather than the calling app's thread.  The queued flavor can also operate
 * on sockets, pipes, non-binary files, mandatory-locked files, and files
 * that don't support pread|pwrite.  Generally all these cases are handled as
 * synchronous read|write operations, but still don't delay the app because
 * they're taken care of by AIO worker threads.
 */

/* These variables support inline AIO operations */
static NO_COPY HANDLE            evt_handles[AIO_MAX];
static NO_COPY struct aiocb     *evt_aiocbs[AIO_MAX];
static NO_COPY CRITICAL_SECTION  evt_locks[AIO_MAX]; /* per-slot locks */
static NO_COPY CRITICAL_SECTION  slotcrit; /* lock for slot variables in toto */

/* These variables support queued AIO operations */
static NO_COPY sem_t             worksem;   /* tells whether AIOs are queued */
static NO_COPY CRITICAL_SECTION  workcrit;        /* lock for AIO work queue */
TAILQ_HEAD(queue, aiocb) worklist = TAILQ_HEAD_INITIALIZER(worklist);

static int
aiochkslot (struct aiocb *aio)
{
  EnterCriticalSection (&slotcrit);

  /* Sanity check.. make sure this AIO is not already busy */
  for (int slot = 0; slot < AIO_MAX; ++slot)
    if (evt_aiocbs[slot] == aio)
      {
        debug_printf ("aio %p is already busy in slot %d", aio, slot);
        LeaveCriticalSection (&slotcrit);
        return slot;
      }

  LeaveCriticalSection (&slotcrit);
  return -1;
}

static int
aiogetslot (struct aiocb *aio)
{
  EnterCriticalSection (&slotcrit);

  /* Find free slot for this inline AIO; if none available AIO will be queued */
  for (int slot = 0; slot < AIO_MAX; ++slot)
    if (evt_aiocbs[slot] == NULL)
      {
        /* If aio is NULL this is just an availability check.. no change made */
        if (aio)
          evt_aiocbs[slot] = aio;
        LeaveCriticalSection (&slotcrit);
        return slot;
      }

  LeaveCriticalSection (&slotcrit);
  return -1;
}

static int
aiorelslot (struct aiocb *aio)
{
  EnterCriticalSection (&slotcrit);

  /* Find slot associated with this inline AIO and free it */
  for (int slot = 0; slot < AIO_MAX; ++slot)
    if (evt_aiocbs[slot] == aio)
      {
        evt_aiocbs[slot] = NULL;
        LeaveCriticalSection (&slotcrit);
        return slot;
      }

  LeaveCriticalSection (&slotcrit);
  return -1;
}

static void
aionotify_on_pthread (struct sigevent *evp)
{
  pthread_attr_t *attr;
  pthread_attr_t  default_attr;
  int             rc;
  pthread_t       vaquita; /* == "little porpoise", endangered, see below */

  if (evp->sigev_notify_attributes)
    attr = evp->sigev_notify_attributes;
  else
    {
      pthread_attr_init (attr = &default_attr);
      pthread_attr_setdetachstate (attr, PTHREAD_CREATE_DETACHED);
    }

  /* A "vaquita" thread is a temporary pthread created to deliver a signal to
   * the application.  We don't wait around for the thread to return from the
   * app.  There's some symbolism here of sending a little creature off to tell
   * the app something important.  If all the vaquitas end up wiped out in the
   * wild, a distinct near-term possibility, at least this code remembers them.
   */
  rc = pthread_create (&vaquita, attr,
                       (void * (*) (void *)) evp->sigev_notify_function,
                       evp->sigev_value.sival_ptr);

  /* The following error is not expected. If seen often, develop a recovery. */
  if (rc)
    debug_printf ("aio vaquita thread creation failed, %E");

  /* Should we wait for the signal delivery thread to finish?  We can't: Who
   * knows what mischief the app coder may have in their handler?  Worst case
   * is they accidentally used non-signal-safe functions in their handler.  We
   * return hoping for the best and finish cleaning up our end of notification.
   */
  return;
}

static void
aionotify (struct aiocb *aio)
{
  siginfo_t si = {0};
  si.si_code = SI_ASYNCIO;

  /* If signal notification wanted, send AIO-complete signal */
  switch (aio->aio_sigevent.sigev_notify) {
  case SIGEV_NONE:
    break;

  case SIGEV_SIGNAL:
    si.si_signo = aio->aio_sigevent.sigev_signo;
    si.si_value = aio->aio_sigevent.sigev_value;
    if (si.si_signo)
      sig_send (myself, si);
    break;

  case SIGEV_THREAD:
    aionotify_on_pthread (&aio->aio_sigevent);
    break;
  }

  /* If this op is on LIO list and is last op, send LIO-complete signal */
  if (aio->aio_liocb)
    {
      if (1 == InterlockedExchangeAdd (&aio->aio_liocb->lio_count, -1))
        {
          /* LIO's count has decremented to zero */
          switch (aio->aio_liocb->lio_sigevent->sigev_notify) {
          case SIGEV_NONE:
            break;

          case SIGEV_SIGNAL:
            si.si_signo = aio->aio_liocb->lio_sigevent->sigev_signo;
            si.si_value = aio->aio_liocb->lio_sigevent->sigev_value;
            if (si.si_signo)
              sig_send (myself, si);
            break;

          case SIGEV_THREAD:
            aionotify_on_pthread (aio->aio_liocb->lio_sigevent);
            break;
          }

          free (aio->aio_liocb);
          aio->aio_liocb = NULL;
        }
    }
}

static DWORD __attribute__ ((noreturn))
aiowaiter (void *unused)
{ /* One instance, called on its own cygthread; runs until program exits */
  struct aiocb *aio;

  while (1)
    {
      /* Wait forever for at least one event to be set */
      DWORD res = WaitForMultipleObjects(AIO_MAX, evt_handles, FALSE, INFINITE);
      switch (res)
        {
          case WAIT_FAILED:
            api_fatal ("aiowaiter fatal error, %E");

          default:
            if (res < WAIT_OBJECT_0 || res >= WAIT_OBJECT_0 + AIO_MAX)
              api_fatal ("aiowaiter unexpected WFMO result %d", res);
            int slot = res - WAIT_OBJECT_0;

            /* Guard against "saw completion before request finished" gotcha */
            EnterCriticalSection (&evt_locks[slot]);
            LeaveCriticalSection (&evt_locks[slot]);

            aio = evt_aiocbs[slot];
            debug_printf ("WFMO returns %d, aio %p", res, aio);

            if (aio->aio_errno == EBUSY)
              {
                /* Capture Windows status and convert to Cygwin status */
                NTSTATUS status = (NTSTATUS) aio->aio_wincb.status;
                if (NT_SUCCESS (status))
                  {
                    aio->aio_rbytes = (ssize_t) aio->aio_wincb.info;
                    aio->aio_errno = 0;
                  }
                else
                  {
                    aio->aio_rbytes = -1;
                    aio->aio_errno = geterrno_from_nt_status (status);
                  }
              }
            else
              {
                /* Async operation was simulated; AIO status already updated */
              }

            /* Send completion signal if user requested it */
            aionotify (aio);

            /* Free up the slot used for this inline AIO.  We do this
             * manually rather than calling aiorelslot() because we
             * already have the slot number handy.
             */
            EnterCriticalSection (&slotcrit);
            evt_aiocbs[slot] = NULL;
            LeaveCriticalSection (&slotcrit);
            debug_printf ("retired aio %p; slot %d released", aio, slot);

            /* Notify workers that a slot has opened up */
            sem_post (&worksem);
        }
    }
}

static ssize_t
asyncread (struct aiocb *aio)
{ /* Try to initiate an asynchronous read, either from app or worker thread */
  ssize_t       res = 0;

  cygheap_fdget cfd (aio->aio_fildes);
  if (cfd < 0)
    res = -1; /* errno has been set to EBADF */
  else
    {
      int slot = aiogetslot (aio);
      debug_printf ("slot %d%s", slot, slot >= 0 ? " acquired" : "");
      if (slot >= 0)
        {
          EnterCriticalSection (&evt_locks[slot]);
          aio->aio_errno = EBUSY; /* Mark AIO as physically underway now */
          aio->aio_wincb.event = (void *) evt_handles[slot];
          res = cfd->pread ((void *) aio->aio_buf, aio->aio_nbytes,
                            aio->aio_offset, (void *) aio);
          LeaveCriticalSection (&evt_locks[slot]);
        }
      else
        {
          set_errno (ENOBUFS); /* Internal use only */
          res = -1;
        }
    }

  return res;
}

static ssize_t
asyncwrite (struct aiocb *aio)
{ /* Try to initiate an asynchronous write, either from app or worker thread */
  ssize_t       res = 0;

  cygheap_fdget cfd (aio->aio_fildes);
  if (cfd < 0)
    res = -1; /* errno has been set to EBADF */
  else
    {
      int slot = aiogetslot (aio);
      debug_printf ("slot %d%s", slot, slot >= 0 ? " acquired" : "");
      if (slot >= 0)
        {
          EnterCriticalSection (&evt_locks[slot]);
          aio->aio_errno = EBUSY; /* Mark AIO as physically underway now */
          aio->aio_wincb.event = (void *) evt_handles[slot];
          res = cfd->pwrite ((void *) aio->aio_buf, aio->aio_nbytes,
                             aio->aio_offset, (void *) aio);
          LeaveCriticalSection (&evt_locks[slot]);
        }
      else
        {
          set_errno (ENOBUFS); /* Internal use only */
          res = -1;
        }
    }

  return res;
}

/* Have to forward ref because of chicken v. egg situation */
static DWORD __attribute__ ((noreturn)) aioworker (void *);

static void
aioinit (void)
{
  /* First a cheap test to speed processing after initialization completes */
  if (aioinitialized >= 0)
    {
      /* Guard against multiple threads initializing at same time */
      if (0 == InterlockedExchangeAdd (&aioinitialized, 1))
        {
          int       i = AIO_MAX;
          char     *tnames = (char *) malloc (AIO_MAX * 8);

          if (!tnames)
            api_fatal ("couldn't create aioworker tname table");

          InitializeCriticalSection (&slotcrit);
          InitializeCriticalSection (&workcrit);
          sem_init (&worksem, 0, 0);
          TAILQ_INIT(&worklist);

          /* Create AIO_MAX number of aioworker threads for queued AIOs */
          while (i--)
            {
              __small_sprintf (&tnames[i * 8], "aio%d", AIO_MAX - i);
              if (!new cygthread (aioworker, NULL, &tnames[i * 8]))
                api_fatal ("couldn't create an aioworker thread, %E");
            }

          /* Initialize event handles and slot locks arrays for inline AIOs */
          for (i = 0; i < AIO_MAX; ++i)
            {
              /* Events are non-inheritable, auto-reset, init unset, unnamed */
              evt_handles[i] = CreateEvent (NULL, FALSE, FALSE, NULL);
              if (!evt_handles[i])
                api_fatal ("couldn't create an event, %E");

              InitializeCriticalSection (&evt_locks[i]);
            }

          /* Create aiowaiter thread; waits for inline AIO completion events */
          if (!new cygthread (aiowaiter, NULL, "aio"))
            api_fatal ("couldn't create aiowaiter thread, %E");

          /* Indicate we have completed initialization */
          InterlockedExchange (&aioinitialized, -1);
        }
      else
        /* If 'aioinitialized' is greater than zero, another thread is
         * initializing for us; wait until 'aioinitialized' goes negative
         */
        while (InterlockedExchangeAdd (&aioinitialized, 0) >= 0)
          yield ();
    }
}

static int
aioqueue (struct aiocb *aio)
{ /* Add an AIO to the worklist, to be serviced by a worker thread */
  if (aioinitialized >= 0)
    aioinit ();

  EnterCriticalSection (&workcrit);
  TAILQ_INSERT_TAIL(&worklist, aio, aio_chain);
  LeaveCriticalSection (&workcrit);

  debug_printf ("queued aio %p", aio);
  sem_post (&worksem);

  return 0;
}

static DWORD __attribute__ ((noreturn))
aioworker (void *unused)
{ /* Multiple instances; called on own cygthreads; runs 'til program exits */
  struct aiocb *aio;

  while (1)
    {
      /* Park here until there's work to do or a slot becomes available */
      sem_wait (&worksem);

look4work:
      EnterCriticalSection (&workcrit);
      if (TAILQ_EMPTY(&worklist))
        {
          /* Another aioworker picked up the work already */
          LeaveCriticalSection (&workcrit);
          continue;
        }

      /* Make sure a slot is available before starting this AIO */
      aio = TAILQ_FIRST(&worklist);
      int slot = aiogetslot (NULL);
      if (slot >= 0) // a slot is available
        TAILQ_REMOVE(&worklist, aio, aio_chain);
      LeaveCriticalSection (&workcrit);
      if (slot < 0) // no slot is available, so worklist unchanged and we park
        continue;

      debug_printf ("starting aio %p", aio);
      switch (aio->aio_lio_opcode)
        {
          case LIO_NOP:
            aio->aio_rbytes = 0;
            break;

          case LIO_READ:
            aio->aio_rbytes = asyncread (aio);
            break;

          case LIO_WRITE:
            aio->aio_rbytes = asyncwrite (aio);
            break;

          default:
            errno = EINVAL;
            aio->aio_rbytes = -1;
            break;
        }

      /* If operation still underway, let aiowaiter hear about its finish */
      if (aio->aio_rbytes == 0 && aio->aio_nbytes != 0) // not racy
        continue;

      /* If operation errored, save error number, else clear it */
      if (aio->aio_rbytes == -1)
        aio->aio_errno = get_errno ();
      else
        aio->aio_errno = 0;

      /* If a slot for this queued async AIO was available, but we lost out */
      if (aio->aio_errno == ENOBUFS)
        {
          aio->aio_errno = EINPROGRESS;
          aioqueue (aio); /* Re-queue the AIO */

          /* Another option would be to fail the AIO with error EAGAIN, but
           * experience with iozone showed apps might not expect to see a
           * deferred EAGAIN.  I.e. they should expect EAGAIN on their call to
           * aio_read() or aio_write() but probably not expect to see EAGAIN
           * on an aio_error() query after they'd previously seen EINPROGRESS
           * on the initial AIO call.
           */
          continue;
        }

      /* If seeks aren't permitted on given fd, or pread|pwrite not legal */
      if (aio->aio_errno == ESPIPE)
        {
          ssize_t res = 0;
          off_t curpos;

          cygheap_fdget cfd (aio->aio_fildes);
          if (cfd < 0)
            {
              res = -1;
              goto done; /* errno has been set to EBADF */
            }

          /* If we can get current file position, seek to aio_offset */
          curpos = cfd->lseek (0, SEEK_CUR);
          if (curpos < 0 || cfd->lseek (aio->aio_offset, SEEK_SET) < 0)
            {
              /* Can't seek */
              res = curpos;
              set_errno (0); /* Get rid of ESPIPE we've incurred */
              aio->aio_errno = 0; /* Here too */
            }

          /* Do the requested AIO operation manually, synchronously */
          switch (aio->aio_lio_opcode)
            {
              case LIO_READ:
                /* 2nd argument to cfd->read() is passed by reference... */
                cfd->read ((void *) aio->aio_buf, aio->aio_nbytes);
                /* ...so on return it contains the number of bytes read */
                res = aio->aio_nbytes;
                break;

              case LIO_WRITE:
                res = cfd->write ((void *) aio->aio_buf, aio->aio_nbytes);
                break;
            }

          /* If we had seeked successfully, restore original file position */
          if (curpos >= 0)
            if (cfd->lseek (curpos, SEEK_SET) < 0)
              res = -1;

done:
          /* Update AIO to reflect final result */
          aio->aio_rbytes = res;
          aio->aio_errno = res == -1 ? get_errno () : 0;

          /* Make like the requested async operation completed normally */
          for (int i = 0; i < AIO_MAX; i++)
            if (evt_aiocbs[i] == aio)
              {
                SetEvent (evt_handles[i]);
                goto truly_done;
              }

          /* Free up the slot we ended up not using */
          int slot = aiorelslot (aio);
          debug_printf ("slot %d released", slot);
        }

      /* Send completion signal if user requested it */
      aionotify (aio);

truly_done:
      debug_printf ("completed aio %p", aio);
      goto look4work;
    }
}

int
aio_cancel (int fildes, struct aiocb *aio)
{
  int           aiocount = 0;
  struct aiocb *ptr;
  siginfo_t     si = {0};
  si.si_code = SI_ASYNCIO;

  /* Note 'aio' is allowed to be NULL here; it's used as a wildcard */
restart:
  EnterCriticalSection (&workcrit);
  TAILQ_FOREACH(ptr, &worklist, aio_chain)
    {
      if (ptr->aio_fildes == fildes && (!aio || ptr == aio))
        {
          /* This queued AIO qualifies for cancellation */
          TAILQ_REMOVE(&worklist, ptr, aio_chain);
          LeaveCriticalSection (&workcrit);

          ptr->aio_errno = ECANCELED;
          ptr->aio_rbytes = -1;

          /* If signal notification wanted, send AIO-canceled signal */
          switch (ptr->aio_sigevent.sigev_notify) {
          case SIGEV_NONE:
            break;

          case SIGEV_SIGNAL:
            si.si_signo = ptr->aio_sigevent.sigev_signo;
            si.si_value = ptr->aio_sigevent.sigev_value;
            if (si.si_signo)
              sig_send (myself, si);
            break;

          case SIGEV_THREAD:
            aionotify_on_pthread (&ptr->aio_sigevent);
            break;
          }

          ++aiocount;
          goto restart;
        }
    }
  LeaveCriticalSection (&workcrit);

  /* Note that AIO_NOTCANCELED is not possible in this implementation.  That's
   * because AIOs are dequeued to execute; the worklist search above won't
   * find an AIO that's been dequeued from the worklist.
   */
  if (aiocount)
    return AIO_CANCELED;
  else
    return AIO_ALLDONE;
}

int
aio_error (const struct aiocb *aio)
{
  int res;

  if (!aio)
    {
      set_errno (EINVAL);
      return -1;
    }

  switch (aio->aio_errno)
    {
      case EBUSY:   /* This state for internal use only; not visible to app */
      case ENOBUFS: /* This state for internal use only; not visible to app */
        res = EINPROGRESS;
        break;

      default:
        res = aio->aio_errno;
    }

  return res;
}

int
aio_fsync (int mode, struct aiocb *aio)
{
  if (!aio)
    {
      set_errno (EINVAL);
      return -1;
    }

  switch (mode)
    {
#if defined(O_SYNC)
      case O_SYNC:
        aio->aio_rbytes = fsync (aio->aio_fildes);
        break;

#if defined(O_DSYNC) && O_DSYNC != O_SYNC
      case O_DSYNC:
        aio->aio_rbytes = fdatasync (aio->aio_fildes);
        break;
#endif
#endif

      default:
        set_errno (EINVAL);
        return -1;
    }

  if (aio->aio_rbytes == -1)
    aio->aio_errno = get_errno ();

  return aio->aio_rbytes;
}

int
aio_read (struct aiocb *aio)
{
  ssize_t       res = 0;

  if (!aio)
    {
      set_errno (EINVAL);
      return -1;
    }
  if (aioinitialized >= 0)
    aioinit ();
  if (aio->aio_errno == EINPROGRESS || -1 != aiochkslot (aio))
    {
      set_errno (EAGAIN);
      return -1;
    }

  aio->aio_lio_opcode = LIO_READ;
  aio->aio_errno = EINPROGRESS;
  aio->aio_rbytes = -1;

  /* Ensure zeroed (i.e. initialized but unused) aio_sigevent doesn't signal */
  if (aio->aio_sigevent.sigev_signo == 0)
    aio->aio_sigevent.sigev_notify = SIGEV_NONE;

  /* Try to launch inline async read; only on ESPIPE/ENOBUFS is it queued */
  pthread_testcancel ();
  res = asyncread (aio);

  /* If async read couldn't be launched, queue the AIO for a worker thread */
  if (res == -1)
    switch (get_errno ()) {
    case ESPIPE:
      {
        int slot = aiorelslot (aio);
        if (slot >= 0)
          debug_printf ("slot %d released", slot);
      }
      fallthrough;

    case ENOBUFS:
      aio->aio_errno = EINPROGRESS;
      aio->aio_rbytes = -1;

      res = aioqueue (aio);
      break;

    default:
      ; /* I think this is not possible */
    }

  return res < 0 ? (int) res : 0; /* return 0 on success */
}

ssize_t
aio_return (struct aiocb *aio)
{
  if (!aio)
    {
      set_errno (EINVAL);
      return -1;
    }

  switch (aio->aio_errno)
    {
      case EBUSY:       /* AIO is currently underway (internal state) */
      case ENOBUFS:     /* AIO is currently underway (internal state) */
      case EINPROGRESS: /* AIO has been queued successfully */
        set_errno (EINPROGRESS);
        return -1;

      case EINVAL:      /* aio_return() has already been called on this AIO */
        set_errno (aio->aio_errno);
        return -1;

      default:          /* AIO has completed, successfully or not */
        ;
    }

  /* This AIO has completed so grab any error status if present */
  if (aio->aio_rbytes == -1)
    set_errno (aio->aio_errno);

  /* Set this AIO's errno so later aio_return() calls on this AIO fail */
  aio->aio_errno = EINVAL;

  return aio->aio_rbytes;
}

static int
aiosuspend (const struct aiocb *const aiolist[],
         int nent, const struct timespec *timeout)
{
  /* Returns lowest list index of completed aios, else 'nent' if all completed.
   * If none completed on entry, wait for interval specified by 'timeout'.
   */
  int       res;
  sigset_t  sigmask;
  siginfo_t si;
  ULONGLONG nsecs = 0;
  ULONGLONG time0, time1;
  struct timespec to = {0};

  if (timeout)
    {
      to = *timeout;
      if (!valid_timespec (to))
        {
          set_errno (EINVAL);
          return -1;
        }
      nsecs = (NSPERSEC * to.tv_sec) + to.tv_nsec;
    }

retry:
  sigemptyset (&sigmask);
  int aiocount = 0;
  for (int i = 0; i < nent; ++i)
    if (aiolist[i] && aiolist[i]->aio_liocb)
      {
        if (aiolist[i]->aio_errno == EINPROGRESS ||
            aiolist[i]->aio_errno == ENOBUFS ||
            aiolist[i]->aio_errno == EBUSY)
          {
            ++aiocount;
            if (aiolist[i]->aio_sigevent.sigev_notify == SIGEV_SIGNAL ||
                aiolist[i]->aio_sigevent.sigev_notify == SIGEV_THREAD)
              sigaddset (&sigmask, aiolist[i]->aio_sigevent.sigev_signo);
          }
        else
          return i;
      }

  if (aiocount == 0)
    return nent;

  if (timeout && nsecs == 0)
    {
      set_errno (EAGAIN);
      return -1;
    }

  time0 = get_clock (CLOCK_MONOTONIC)->nsecs ();
  /* Note wait below is abortable even w/ empty sigmask and infinite timeout */
  res = sigtimedwait (&sigmask, &si, timeout ? &to : NULL);
  if (res == -1)
    return -1; /* Return with errno set by failed sigtimedwait() */
  time1 = get_clock (CLOCK_MONOTONIC)->nsecs ();

  /* Adjust timeout to account for time just waited */
  time1 -= time0;
  if (time1 > nsecs)
    nsecs = 0; // just in case we didn't get rescheduled very quickly
  else
    nsecs -= time1;
  to.tv_sec = nsecs / NSPERSEC;
  to.tv_nsec = nsecs % NSPERSEC;

  goto retry;
}

int
aio_suspend (const struct aiocb *const aiolist[],
             int nent, const struct timespec *timeout)
{
  int res;

  if (nent < 0)
    {
      set_errno (EINVAL);
      return -1;
    }

  pthread_testcancel ();
  res = aiosuspend (aiolist, nent, timeout);

  /* If there was an error, or no AIOs completed before or during timeout */
  if (res == -1)
    return res; /* If no AIOs completed, errno has been set to EAGAIN */

  /* Else if all AIOs have completed */
  else if (res == nent)
    return 0;

  /* Else at least one of the AIOs completed */
  else
    return 0;
}

int
aio_write (struct aiocb *aio)
{
  ssize_t       res = 0;

  if (!aio)
    {
      set_errno (EINVAL);
      return -1;
    }
  if (aioinitialized >= 0)
    aioinit ();
  if (aio->aio_errno == EINPROGRESS || -1 != aiochkslot (aio))
    {
      set_errno (EAGAIN);
      return -1;
    }

  aio->aio_lio_opcode = LIO_WRITE;
  aio->aio_errno = EINPROGRESS;
  aio->aio_rbytes = -1;

  /* Ensure zeroed (i.e. initialized but unused) aio_sigevent doesn't signal */
  if (aio->aio_sigevent.sigev_signo == 0)
    aio->aio_sigevent.sigev_notify = SIGEV_NONE;

  /* Try to launch inline async write; only on ESPIPE/ENOBUFS is it queued */
  pthread_testcancel ();
  res = asyncwrite (aio);

  /* If async write couldn't be launched, queue the AIO for a worker thread */
  if (res == -1)
    switch (get_errno ()) {
    case ESPIPE:
      {
        int slot = aiorelslot (aio);
        if (slot >= 0)
          debug_printf ("slot %d released", slot);
      }
      fallthrough;

    case ENOBUFS:
      aio->aio_errno = EINPROGRESS;
      aio->aio_rbytes = -1;

      res = aioqueue (aio);
      break;

    default:
      ; /* I think this is not possible */
    }

  return res < 0 ? (int) res : 0; /* return 0 on success */
}

int
lio_listio (int mode, struct aiocb *__restrict const aiolist[__restrict],
            int nent, struct sigevent *__restrict sig)
{
  struct aiocb *aio;
  struct __liocb *lio;

  pthread_testcancel ();

  if ((mode != LIO_WAIT && mode != LIO_NOWAIT) ||
      (nent < 0 || nent > AIO_LISTIO_MAX))
    {
      set_errno (EINVAL);
      return -1;
    }

  if (sig && nent && mode == LIO_NOWAIT)
    {
      lio = (struct __liocb *) malloc (sizeof (struct __liocb));
      if (!lio)
        {
          set_errno (ENOMEM);
          return -1;
        }

      lio->lio_count = nent;
      lio->lio_sigevent = sig;
    }
  else
    lio = NULL;

  int aiocount = 0;
  for (int i = 0; i < nent; ++i)
    {
      aio = (struct aiocb *) aiolist[i];
      if (!aio)
        {
          if (lio)
            InterlockedDecrement (&lio->lio_count);
          continue;
        }

      aio->aio_liocb = lio;
      switch (aio->aio_lio_opcode)
        {
          case LIO_NOP:
            if (lio)
              InterlockedDecrement (&lio->lio_count);
            continue;

          case LIO_READ:
            aio_read (aio);
            ++aiocount;
            continue;

          case LIO_WRITE:
            aio_write (aio);
            ++aiocount;
            continue;

          default:
            break;
        }

      if (lio)
        InterlockedDecrement (&lio->lio_count);
      aio->aio_errno = EINVAL;
      aio->aio_rbytes = -1;
    }

  /* mode is LIO_NOWAIT so return some kind of answer immediately */
  if (mode == LIO_NOWAIT)
    {
      /* At least one AIO has been launched or queued */
      if (aiocount)
        return 0;

      /* No AIOs have been launched or queued */
      set_errno (EAGAIN);
      return -1;
    }

  /* Else mode is LIO_WAIT so wait for all AIOs to complete or error */
  while (nent)
    {
      int i = aiosuspend ((const struct aiocb *const *) aiolist, nent, NULL);
      if (i >= nent)
        break;
      else
        aiolist[i]->aio_liocb = NULL; /* Avoids repeating notify on this AIO */
    }

  return 0;
}

#ifdef __cplusplus
}
#endif
