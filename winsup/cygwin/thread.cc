/* thread.cc: Locking and threading module functions

   Copyright 1998, 1999, 2000, 2001, 2002, 2003 Red Hat, Inc.

   Originally written by Marco Fuykschot <marco@ddi.nl>
   Substantialy enhanced by Robert Collins <rbtcollins@hotmail.com>

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

/* Implementation overview and caveats:

   Win32 puts some contraints on what can and cannot be implemented.  Where
   possible we work around those contrainsts.  Where we cannot work around
   the constraints we either pretend to be conformant, or return an error
   code.

   Some caveats: PROCESS_SHARED objects while they pretend to be process
   shared, may not actually work.  Some test cases are needed to determine
   win32's behaviour.  My suspicion is that the win32 handle needs to be
   opened with different flags for proper operation.

   R.Collins, April 2001.  */

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include "winsup.h"
#include <limits.h>
#include "cygerrno.h"
#include <assert.h>
#include <stdlib.h>
#include "pinfo.h"
#include "sigproc.h"
#include "perprocess.h"
#include "security.h"
#include "cygtls.h"
#include <semaphore.h>
#include <stdio.h>
#include <sys/timeb.h>
#include <exceptions.h>
#include <sys/fcntl.h>

extern int threadsafe;

#undef __getreent
extern "C" struct _reent *
__getreent ()
{
  return &_my_tls.local_clib;
}

inline LPCRITICAL_SECTION
ResourceLocks::Lock (int _resid)
{
  return &lock;
}

void
SetResourceLock (int _res_id, int _mode, const char *_function)
{
  EnterCriticalSection (user_data->resourcelocks->Lock (_res_id));
}

void
ReleaseResourceLock (int _res_id, int _mode, const char *_function)
{
  LeaveCriticalSection (user_data->resourcelocks->Lock (_res_id));
}

void
ResourceLocks::Init ()
{
  InitializeCriticalSection (&lock);
  inited = true;
  thread_printf ("lock %p inited by %p , %d", &lock, user_data, myself->pid);
}

void
ResourceLocks::Delete ()
{
  if (inited)
    {
      thread_printf ("Close Resource Locks %p ", &lock);
      DeleteCriticalSection (&lock);
      inited = false;
    }
}

void
MTinterface::Init ()
{
  pthread_mutex::init_mutex ();
  pthread_cond::init_mutex ();
  pthread_rwlock::init_mutex ();
}

void
MTinterface::fixup_before_fork (void)
{
  pthread_key::fixup_before_fork ();
}

/* This function is called from a single threaded process */
void
MTinterface::fixup_after_fork (void)
{
  pthread_key::fixup_after_fork ();

  threadcount = 0;
  pthread::init_mainthread ();

  pthread::fixup_after_fork ();
  pthread_mutex::fixup_after_fork ();
  pthread_cond::fixup_after_fork ();
  pthread_rwlock::fixup_after_fork ();
  semaphore::fixup_after_fork ();
}

/* pthread calls */

/* static methods */
void
pthread::init_mainthread ()
{
  pthread *thread = get_tls_self_pointer ();
  if (!thread)
    {
      thread = new pthread ();
      if (!thread)
	api_fatal ("failed to create mainthread object");
    }

  thread->cygtls = &_my_tls;
  _my_tls.tid = thread;
  thread->thread_id = GetCurrentThreadId ();
  if (!DuplicateHandle (GetCurrentProcess (), GetCurrentThread (),
			GetCurrentProcess (), &thread->win32_obj_id,
			0, FALSE, DUPLICATE_SAME_ACCESS))
    api_fatal ("failed to create mainthread handle");
  if (!thread->create_cancel_event ())
    api_fatal ("couldn't create cancel event for main thread");
  thread->postcreate ();
}

pthread *
pthread::self ()
{
  pthread *thread = get_tls_self_pointer ();
  if (thread)
    return thread;
  return pthread_null::get_null_pthread ();
}

pthread *
pthread::get_tls_self_pointer ()
{
  return _my_tls.tid;
}

List<pthread> pthread::threads;

/* member methods */
pthread::pthread ():verifyable_object (PTHREAD_MAGIC), win32_obj_id (0),
		    valid (false), suspended (false),
		    cancelstate (0), canceltype (0), cancel_event (0),
		    joiner (NULL), next (NULL), cleanup_stack (NULL)
{
  if (this != pthread_null::get_null_pthread ())
    threads.insert (this);
}

pthread::~pthread ()
{
  if (win32_obj_id)
    CloseHandle (win32_obj_id);
  if (cancel_event)
    CloseHandle (cancel_event);

  if (this != pthread_null::get_null_pthread ())
    threads.remove (this);
}

bool
pthread::create_cancel_event ()
{
  cancel_event = ::CreateEvent (&sec_none_nih, TRUE, FALSE, NULL);
  if (!cancel_event)
    {
      system_printf ("couldn't create cancel event, %E");
      /* we need the event for correct behaviour */
      return false;
    }
  return true;
}

void
pthread::precreate (pthread_attr *newattr)
{
  pthread_mutex *verifyable_mutex_obj = &mutex;

  /* already running ? */
  if (win32_obj_id)
    return;

  if (newattr)
    {
      attr.joinable = newattr->joinable;
      attr.contentionscope = newattr->contentionscope;
      attr.inheritsched = newattr->inheritsched;
      attr.stacksize = newattr->stacksize;
    }

  if (!pthread_mutex::is_good_object (&verifyable_mutex_obj))
    {
      thread_printf ("New thread object access mutex is not valid. this %p",
		     this);
      magic = 0;
      return;
    }
  /* Change the mutex type to NORMAL to speed up mutex operations */
  mutex.type = PTHREAD_MUTEX_NORMAL;
  if (!create_cancel_event ())
    magic = 0;
}

void
pthread::create (void *(*func) (void *), pthread_attr *newattr,
		 void *threadarg)
{
  precreate (newattr);
  if (!magic)
    return;

  function = func;
  arg = threadarg;

  win32_obj_id = ::CreateThread (&sec_none_nih, attr.stacksize,
				thread_init_wrapper, this, CREATE_SUSPENDED,
				&thread_id);

  if (!win32_obj_id)
    {
      thread_printf ("CreateThread failed: this %p, %E", this);
      magic = 0;
    }
  else
    {
      postcreate ();
      ResumeThread (win32_obj_id);
    }
}

void
pthread::postcreate ()
{
  valid = true;

  InterlockedIncrement (&MT_INTERFACE->threadcount);
  /* FIXME: set the priority appropriately for system contention scope */
  if (attr.inheritsched == PTHREAD_EXPLICIT_SCHED)
    {
      /* FIXME: set the scheduling settings for the new thread */
      /* sched_thread_setparam (win32_obj_id, attr.schedparam); */
    }
}

void
pthread::exit (void *value_ptr)
{
  class pthread *thread = this;

  // run cleanup handlers
  pop_all_cleanup_handlers ();

  pthread_key::run_all_destructors ();

  mutex.lock ();
  // cleanup if thread is in detached state and not joined
  if (equal (joiner, thread))
    delete this;
  else
    {
      valid = false;
      return_ptr = value_ptr;
      mutex.unlock ();
    }

  (_reclaim_reent) (_REENT);


  if (InterlockedDecrement (&MT_INTERFACE->threadcount) == 0)
    ::exit (0);
  else
    {
      _my_tls.remove (INFINITE);
      ExitThread (0);
    }
}

int
pthread::cancel (void)
{
  class pthread *thread = this;
  class pthread *self = pthread::self ();

  mutex.lock ();

  if (!valid)
    {
      mutex.unlock ();
      return 0;
    }

  if (canceltype == PTHREAD_CANCEL_DEFERRED ||
      cancelstate == PTHREAD_CANCEL_DISABLE)
    {
      // cancel deferred
      mutex.unlock ();
      SetEvent (cancel_event);
      return 0;
    }
  else if (equal (thread, self))
    {
      mutex.unlock ();
      cancel_self ();
      return 0; // Never reached
    }

  // cancel asynchronous
  SuspendThread (win32_obj_id);
  if (WaitForSingleObject (win32_obj_id, 0) == WAIT_TIMEOUT)
    {
      CONTEXT context;
      context.ContextFlags = CONTEXT_CONTROL;
      GetThreadContext (win32_obj_id, &context);
      context.Eip = (DWORD) pthread::static_cancel_self;
      SetThreadContext (win32_obj_id, &context);
    }
  mutex.unlock ();
  ResumeThread (win32_obj_id);

  return 0;
/*
  TODO: insert  pthread_testcancel into the required functions
  the required function list is: *indicates done, X indicates not present in cygwin.
aio_suspend ()
*close ()
*creat ()
fcntl ()
fsync ()
getmsg ()
getpmsg ()
lockf ()
mq_receive ()
mq_send ()
msgrcv ()
msgsnd ()
msync ()
nanosleep ()
open ()
*pause ()
poll ()
pread ()
*pthread_cond_timedwait ()
*pthread_cond_wait ()
*pthread_join ()
*pthread_testcancel ()
putmsg ()
putpmsg ()
pwrite ()
read ()
readv ()
select ()
*sem_wait ()
*sigpause ()
*sigsuspend ()
sigtimedwait ()
sigwait ()
sigwaitinfo ()
*sleep ()
*system ()
tcdrain ()
*usleep ()
*wait ()
*wait3()
waitid ()
*waitpid ()
write ()
writev ()

the optional list is:
catclose ()
catgets ()
catopen ()
closedir ()
closelog ()
ctermid ()
dbm_close ()
dbm_delete ()
dbm_fetch ()
dbm_nextkey ()
dbm_open ()
dbm_store ()
dlclose ()
dlopen ()
endgrent ()
endpwent ()
endutxent ()
fclose ()
fcntl ()
fflush ()
fgetc ()
fgetpos ()
fgets ()
fgetwc ()
fgetws ()
fopen ()
fprintf ()
fputc ()
fputs ()
fputwc ()
fputws ()
fread ()
freopen ()
fscanf ()
fseek ()
fseeko ()
fsetpos ()
ftell ()
ftello ()
ftw ()
fwprintf ()
fwrite ()
fwscanf ()
getc ()
getc_unlocked ()
getchar ()
getchar_unlocked ()
getcwd ()
getdate ()
getgrent ()
getgrgid ()
getgrgid_r ()
getgrnam ()
getgrnam_r ()
getlogin ()
getlogin_r ()
getpwent ()
*getpwnam ()
*getpwnam_r ()
*getpwuid ()
*getpwuid_r ()
gets ()
getutxent ()
getutxid ()
getutxline ()
getw ()
getwc ()
getwchar ()
getwd ()
glob ()
iconv_close ()
iconv_open ()
ioctl ()
lseek ()
mkstemp ()
nftw ()
opendir ()
openlog ()
pclose ()
perror ()
popen ()
printf ()
putc ()
putc_unlocked ()
putchar ()
putchar_unlocked ()
puts ()
pututxline ()
putw ()
putwc ()
putwchar ()
readdir ()
readdir_r ()
remove ()
rename ()
rewind ()
rewinddir ()
scanf ()
seekdir ()
semop ()
setgrent ()
setpwent ()
setutxent ()
strerror ()
syslog ()
tmpfile ()
tmpnam ()
ttyname ()
ttyname_r ()
ungetc ()
ungetwc ()
unlink ()
vfprintf ()
vfwprintf ()
vprintf ()
vwprintf ()
wprintf ()
wscanf ()

Note, that for fcntl (), for any value of the cmd argument.

And we must not introduce cancellation points anywhere else that's part of the posix or
opengroup specs.
 */
}

void
pthread::testcancel (void)
{
  if (cancelstate == PTHREAD_CANCEL_DISABLE)
    return;

  if (WaitForSingleObject (cancel_event, 0) == WAIT_OBJECT_0)
    cancel_self ();
}

void
pthread::static_cancel_self (void)
{
  pthread::self ()->cancel_self ();
}


DWORD
pthread::cancelable_wait (HANDLE object, DWORD timeout, const bool do_cancel)
{
  DWORD res;
  HANDLE wait_objects[2];
  pthread_t thread = self ();

  if (!is_good_object (&thread) || thread->cancelstate == PTHREAD_CANCEL_DISABLE)
    return WaitForSingleObject (object, timeout);

  // Do not change the wait order
  // The object must have higher priority than the cancel event,
  // because WaitForMultipleObjects will return the smallest index
  // if both objects are signaled
  wait_objects[0] = object;
  wait_objects[1] = thread->cancel_event;

  res = WaitForMultipleObjects (2, wait_objects, FALSE, timeout);
  if (do_cancel && res == WAIT_CANCELED)
    pthread::static_cancel_self ();
  return res;
}

int
pthread::setcancelstate (int state, int *oldstate)
{
  int result = 0;

  mutex.lock ();

  if (state != PTHREAD_CANCEL_ENABLE && state != PTHREAD_CANCEL_DISABLE)
    result = EINVAL;
  else
    {
      if (oldstate)
	*oldstate = cancelstate;
      cancelstate = state;
    }

  mutex.unlock ();

  return result;
}

int
pthread::setcanceltype (int type, int *oldtype)
{
  int result = 0;

  mutex.lock ();

  if (type != PTHREAD_CANCEL_DEFERRED && type != PTHREAD_CANCEL_ASYNCHRONOUS)
    result = EINVAL;
  else
    {
      if (oldtype)
	*oldtype = canceltype;
      canceltype = type;
    }

  mutex.unlock ();

  return result;
}

void
pthread::push_cleanup_handler (__pthread_cleanup_handler *handler)
{
  if (this != self ())
    // TODO: do it?
    api_fatal ("Attempt to push a cleanup handler across threads");
  handler->next = cleanup_stack;
  cleanup_stack = handler;
}

void
pthread::pop_cleanup_handler (int const execute)
{
  if (this != self ())
    // TODO: send a signal or something to the thread ?
    api_fatal ("Attempt to execute a cleanup handler across threads");

  mutex.lock ();

  if (cleanup_stack != NULL)
    {
      __pthread_cleanup_handler *handler = cleanup_stack;

      if (execute)
	(*handler->function) (handler->arg);
      cleanup_stack = handler->next;
    }

  mutex.unlock ();
}

void
pthread::pop_all_cleanup_handlers ()
{
  while (cleanup_stack != NULL)
    pop_cleanup_handler (1);
}

void
pthread::cancel_self ()
{
  exit (PTHREAD_CANCELED);
}

DWORD
pthread::get_thread_id ()
{
  return thread_id;
}

void
pthread::_fixup_after_fork ()
{
  /* set thread to not running if it is not the forking thread */
  if (this != pthread::self ())
    {
      magic = 0;
      valid = false;
      win32_obj_id = NULL;
      cancel_event = NULL;
    }
}

void
pthread::suspend_except_self ()
{
  if (valid && this != pthread::self ())
    SuspendThread (win32_obj_id);
}

void
pthread::resume ()
{
  if (valid)
    ResumeThread (win32_obj_id);
}

/* static members */
bool
pthread_attr::is_good_object (pthread_attr_t const *attr)
{
  if (verifyable_object_isvalid (attr, PTHREAD_ATTR_MAGIC) != VALID_OBJECT)
    return false;
  return true;
}

/* instance members */

pthread_attr::pthread_attr ():verifyable_object (PTHREAD_ATTR_MAGIC),
joinable (PTHREAD_CREATE_JOINABLE), contentionscope (PTHREAD_SCOPE_PROCESS),
inheritsched (PTHREAD_INHERIT_SCHED), stacksize (0)
{
  schedparam.sched_priority = 0;
}

pthread_attr::~pthread_attr ()
{
}

bool
pthread_condattr::is_good_object (pthread_condattr_t const *attr)
{
  if (verifyable_object_isvalid (attr, PTHREAD_CONDATTR_MAGIC) != VALID_OBJECT)
    return false;
  return true;
}

pthread_condattr::pthread_condattr ():verifyable_object
  (PTHREAD_CONDATTR_MAGIC), shared (PTHREAD_PROCESS_PRIVATE)
{
}

pthread_condattr::~pthread_condattr ()
{
}

List<pthread_cond> pthread_cond::conds;

/* This is used for cond creation protection within a single process only */
fast_mutex NO_COPY pthread_cond::cond_initialization_lock;

/* We can only be called once.
   TODO: (no rush) use a non copied memory section to
   hold an initialization flag.  */
void
pthread_cond::init_mutex ()
{
  if (!cond_initialization_lock.init ())
    api_fatal ("Could not create win32 Mutex for pthread cond static initializer support.");
}

pthread_cond::pthread_cond (pthread_condattr *attr) :
  verifyable_object (PTHREAD_COND_MAGIC),
  shared (0), waiting (0), pending (0), sem_wait (NULL),
  mtx_cond(NULL), next (NULL)
{
  pthread_mutex *verifyable_mutex_obj;

  if (attr)
    if (attr->shared != PTHREAD_PROCESS_PRIVATE)
      {
	magic = 0;
	return;
      }

  verifyable_mutex_obj = &mtx_in;
  if (!pthread_mutex::is_good_object (&verifyable_mutex_obj))
    {
      thread_printf ("Internal cond mutex is not valid. this %p", this);
      magic = 0;
      return;
    }
  /*
   * Change the mutex type to NORMAL.
   * This mutex MUST be of type normal
  */
  mtx_in.type = PTHREAD_MUTEX_NORMAL;

  verifyable_mutex_obj = &mtx_out;
  if (!pthread_mutex::is_good_object (&verifyable_mutex_obj))
    {
      thread_printf ("Internal cond mutex is not valid. this %p", this);
      magic = 0;
      return;
    }
  /* Change the mutex type to NORMAL to speed up mutex operations */
  mtx_out.type = PTHREAD_MUTEX_NORMAL;

  sem_wait = ::CreateSemaphore (&sec_none_nih, 0, LONG_MAX, NULL);
  if (!sem_wait)
    {
      debug_printf ("CreateSemaphore failed. %E");
      magic = 0;
      return;
    }

  conds.insert (this);
}

pthread_cond::~pthread_cond ()
{
  if (sem_wait)
    CloseHandle (sem_wait);

  conds.remove (this);
}

void
pthread_cond::unblock (const bool all)
{
  unsigned long releaseable;

  /*
   * Block outgoing threads (and avoid simultanous unblocks)
   */
  mtx_out.lock ();

  releaseable = waiting - pending;
  if (releaseable)
    {
      unsigned long released;

      if (!pending)
	{
	  /*
	   * Block incoming threads until all waiting threads are released.
	   */
	  mtx_in.lock ();

	  /*
	   * Calculate releaseable again because threads can enter until
	   * the semaphore has been taken, but they can not leave, therefore pending
	   * is unchanged and releaseable can only get higher
	   */
	  releaseable = waiting - pending;
	}

      released = all ? releaseable : 1;
      pending += released;
      /*
       * Signal threads
       */
      ::ReleaseSemaphore (sem_wait, released, NULL);
    }

  /*
   * And let the threads release.
   */
  mtx_out.unlock ();
}

int
pthread_cond::wait (pthread_mutex_t mutex, DWORD dwMilliseconds)
{
  DWORD rv;

  mtx_in.lock ();
  if (InterlockedIncrement ((long *)&waiting) == 1)
    mtx_cond = mutex;
  else if (mtx_cond != mutex)
    {
      InterlockedDecrement ((long *)&waiting);
      mtx_in.unlock ();
      return EINVAL;
    }
  mtx_in.unlock ();

  /*
   * Release the mutex and wait on semaphore
   */
  ++mutex->condwaits;
  mutex->unlock ();

  rv = pthread::cancelable_wait (sem_wait, dwMilliseconds, false);

  mtx_out.lock ();

  if (rv != WAIT_OBJECT_0)
    {
      /*
       * It might happen that a signal is sent while the thread got canceled
       * or timed out. Try to take one.
       * If the thread gets one than a signal|broadcast is in progress.
       */
      if (WaitForSingleObject (sem_wait, 0) == WAIT_OBJECT_0)
	/*
	 * thread got cancelled ot timed out while a signalling is in progress.
	 * Set wait result back to signaled
	 */
	rv = WAIT_OBJECT_0;
    }

  InterlockedDecrement ((long *)&waiting);

  if (rv == WAIT_OBJECT_0 && --pending == 0)
    /*
     * All signaled threads are released,
     * new threads can enter Wait
     */
    mtx_in.unlock ();

  mtx_out.unlock ();

  mutex->lock ();
  --mutex->condwaits;

  if (rv == WAIT_CANCELED)
    pthread::static_cancel_self ();
  else if (rv == WAIT_TIMEOUT)
    return ETIMEDOUT;

  return 0;
}

void
pthread_cond::_fixup_after_fork ()
{
  waiting = pending = 0;
  mtx_cond = NULL;

  /* Unlock eventually locked mutexes */
  mtx_in.unlock ();
  mtx_out.unlock ();

  sem_wait = ::CreateSemaphore (&sec_none_nih, 0, LONG_MAX, NULL);
  if (!sem_wait)
    api_fatal ("pthread_cond::_fixup_after_fork () failed to recreate win32 semaphore");
}

bool
pthread_rwlockattr::is_good_object (pthread_rwlockattr_t const *attr)
{
  if (verifyable_object_isvalid (attr, PTHREAD_RWLOCKATTR_MAGIC) != VALID_OBJECT)
    return false;
  return true;
}

pthread_rwlockattr::pthread_rwlockattr ():verifyable_object
  (PTHREAD_RWLOCKATTR_MAGIC), shared (PTHREAD_PROCESS_PRIVATE)
{
}

pthread_rwlockattr::~pthread_rwlockattr ()
{
}

List<pthread_rwlock> pthread_rwlock::rwlocks;

/* This is used for rwlock creation protection within a single process only */
fast_mutex NO_COPY pthread_rwlock::rwlock_initialization_lock;

/* We can only be called once.
   TODO: (no rush) use a non copied memory section to
   hold an initialization flag.  */
void
pthread_rwlock::init_mutex ()
{
  if (!rwlock_initialization_lock.init ())
    api_fatal ("Could not create win32 Mutex for pthread rwlock static initializer support.");
}

pthread_rwlock::pthread_rwlock (pthread_rwlockattr *attr) :
  verifyable_object (PTHREAD_RWLOCK_MAGIC),
  shared (0), waiting_readers (0), waiting_writers (0), writer (NULL),
  readers (NULL), readers_mx (), mtx (NULL), cond_readers (NULL), cond_writers (NULL),
  next (NULL)
{
  pthread_mutex *verifyable_mutex_obj = &mtx;
  pthread_cond *verifyable_cond_obj;

  if (!readers_mx.init ())
    {
      thread_printf ("Internal rwlock synchronisation mutex is not valid. this %p", this);
      magic = 0;
      return;
    }

  if (attr)
    if (attr->shared != PTHREAD_PROCESS_PRIVATE)
      {
	magic = 0;
	return;
      }

  if (!pthread_mutex::is_good_object (&verifyable_mutex_obj))
    {
      thread_printf ("Internal rwlock mutex is not valid. this %p", this);
      magic = 0;
      return;
    }
  /* Change the mutex type to NORMAL to speed up mutex operations */
  mtx.type = PTHREAD_MUTEX_NORMAL;

  verifyable_cond_obj = &cond_readers;
  if (!pthread_cond::is_good_object (&verifyable_cond_obj))
    {
      thread_printf ("Internal rwlock readers cond is not valid. this %p", this);
      magic = 0;
      return;
    }

  verifyable_cond_obj = &cond_writers;
  if (!pthread_cond::is_good_object (&verifyable_cond_obj))
    {
      thread_printf ("Internal rwlock writers cond is not valid. this %p", this);
      magic = 0;
      return;
    }


  rwlocks.insert (this);
}

pthread_rwlock::~pthread_rwlock ()
{
  rwlocks.remove (this);
}

int
pthread_rwlock::rdlock ()
{
  int result = 0;
  struct RWLOCK_READER *reader;
  pthread_t self = pthread::self ();

  mtx.lock ();

  if (lookup_reader (self))
    {
      result = EDEADLK;
      goto DONE;
    }

  reader = new struct RWLOCK_READER;
  if (!reader)
    {
      result = EAGAIN;
      goto DONE;
    }

  while (writer || waiting_writers)
    {
      pthread_cleanup_push (pthread_rwlock::rdlock_cleanup, this);

      ++waiting_readers;
      cond_readers.wait (&mtx);
      --waiting_readers;

      pthread_cleanup_pop (0);
    }

  reader->thread = self;
  add_reader (reader);

 DONE:
  mtx.unlock ();

  return result;
}

int
pthread_rwlock::tryrdlock ()
{
  int result = 0;
  pthread_t self = pthread::self ();

  mtx.lock ();

  if (writer || waiting_writers || lookup_reader (self))
    result = EBUSY;
  else
    {
      struct RWLOCK_READER *reader = new struct RWLOCK_READER;
      if (reader)
	{
	  reader->thread = self;
	  add_reader (reader);
	}
      else
	result = EAGAIN;
    }

  mtx.unlock ();

  return result;
}

int
pthread_rwlock::wrlock ()
{
  int result = 0;
  pthread_t self = pthread::self ();

  mtx.lock ();

  if (writer == self || lookup_reader (self))
    {
      result = EDEADLK;
      goto DONE;
    }

  while (writer || readers)
    {
      pthread_cleanup_push (pthread_rwlock::wrlock_cleanup, this);

      ++waiting_writers;
      cond_writers.wait (&mtx);
      --waiting_writers;

      pthread_cleanup_pop (0);
    }

  writer = self;

 DONE:
  mtx.unlock ();

  return result;
}

int
pthread_rwlock::trywrlock ()
{
  int result = 0;
  pthread_t self = pthread::self ();

  mtx.lock ();

  if (writer || readers)
    result = EBUSY;
  else
    writer = self;

  mtx.unlock ();

  return result;
}

int
pthread_rwlock::unlock ()
{
  int result = 0;
  pthread_t self = pthread::self ();

  mtx.lock ();

  if (writer)
    {
      if (writer != self)
	{
	  result = EPERM;
	  goto DONE;
	}

      writer = NULL;
    }
  else
    {
      struct RWLOCK_READER *reader = lookup_reader (self);

      if (!reader)
	{
	  result = EPERM;
	  goto DONE;
	}

      remove_reader (reader);
      delete reader;
    }

  release ();

 DONE:
  mtx.unlock ();

  return result;
}

void
pthread_rwlock::add_reader (struct RWLOCK_READER *rd)
{
  List_insert (readers, rd);
}

void
pthread_rwlock::remove_reader (struct RWLOCK_READER *rd)
{
  List_remove (readers_mx, readers, rd);
}

struct pthread_rwlock::RWLOCK_READER *
pthread_rwlock::lookup_reader (pthread_t thread)
{
  readers_mx.lock ();

  struct RWLOCK_READER *cur = readers;

  while (cur && cur->thread != thread)
    cur = cur->next;

  readers_mx.unlock ();

  return cur;
}

void
pthread_rwlock::rdlock_cleanup (void *arg)
{
  pthread_rwlock *rwlock = (pthread_rwlock *) arg;

  --(rwlock->waiting_readers);
  rwlock->release ();
  rwlock->mtx.unlock ();
}

void
pthread_rwlock::wrlock_cleanup (void *arg)
{
  pthread_rwlock *rwlock = (pthread_rwlock *) arg;

  --(rwlock->waiting_writers);
  rwlock->release ();
  rwlock->mtx.unlock ();
}

void
pthread_rwlock::_fixup_after_fork ()
{
  pthread_t self = pthread::self ();
  struct RWLOCK_READER **temp = &readers;

  waiting_readers = 0;
  waiting_writers = 0;

  if (!readers_mx.init ())
    api_fatal ("pthread_rwlock::_fixup_after_fork () failed to recreate mutex");

  /* Unlock eventually locked mutex */
  mtx.unlock ();
  /*
   * Remove all readers except self
   */
  while (*temp)
    {
      if ((*temp)->thread == self)
	temp = &((*temp)->next);
      else
	{
	  struct RWLOCK_READER *cur = *temp;
	  *temp = (*temp)->next;
	  delete cur;
	}
    }
}

/* pthread_key */
/* static members */
/* This stores pthread_key information across fork() boundaries */
List<pthread_key> pthread_key::keys;

bool
pthread_key::is_good_object (pthread_key_t const *key)
{
  if (verifyable_object_isvalid (key, PTHREAD_KEY_MAGIC) != VALID_OBJECT)
    return false;
  return true;
}

/* non-static members */

pthread_key::pthread_key (void (*aDestructor) (void *)):verifyable_object (PTHREAD_KEY_MAGIC), destructor (aDestructor)
{
  tls_index = TlsAlloc ();
  if (tls_index == TLS_OUT_OF_INDEXES)
    magic = 0;
  else
    keys.insert (this);
}

pthread_key::~pthread_key ()
{
  /* We may need to make the list code lock the list during operations
   */
  if (magic != 0)
    {
      keys.remove (this);
      TlsFree (tls_index);
    }
}

int
pthread_key::set (const void *value)
{
  /* the OS function doesn't perform error checking */
  TlsSetValue (tls_index, (void *) value);
  return 0;
}

void *
pthread_key::get () const
{
  int saved_error = ::GetLastError ();
  void *result = TlsGetValue (tls_index);
  ::SetLastError (saved_error);
  return result;
}

void
pthread_key::_fixup_before_fork ()
{
  fork_buf = get ();
}

void
pthread_key::_fixup_after_fork ()
{
  tls_index = TlsAlloc ();
  if (tls_index == TLS_OUT_OF_INDEXES)
    api_fatal ("pthread_key::recreate_key_from_buffer () failed to reallocate Tls storage");
  set (fork_buf);
}

void
pthread_key::run_destructor ()
{
  if (destructor)
    {
      void *oldValue = get ();
      if (oldValue)
	{
	  set (NULL);
	  destructor (oldValue);
	}
    }
}

/* pshared mutexs:

   REMOVED FROM CURRENT. These can be reinstated with the daemon, when all the
   gymnastics can be a lot easier.

   the mutex_t (size 4) is not used as a verifyable object because we cannot
   guarantee the same address space for all processes.
   we use the following:
   high bit set (never a valid address).
   second byte is reserved for the priority.
   third byte is reserved
   fourth byte is the mutex id. (max 255 cygwin mutexs system wide).
   creating mutex's does get slower and slower, but as creation is a one time
   job, it should never become an issue

   And if you're looking at this and thinking, why not an array in cygwin for all mutexs,
   - you incur a penalty on _every_ mutex call and you have toserialise them all.
   ... Bad karma.

   option 2? put everything in userspace and update the ABI?
   - bad karma as well - the HANDLE, while identical across process's,
   Isn't duplicated, it's reopened. */

/* static members */
bool
pthread_mutex::is_good_object (pthread_mutex_t const *mutex)
{
  if (verifyable_object_isvalid (mutex, PTHREAD_MUTEX_MAGIC) != VALID_OBJECT)
    return false;
  return true;
}

bool
pthread_mutex::is_good_initializer (pthread_mutex_t const *mutex)
{
  if (verifyable_object_isvalid (mutex, PTHREAD_MUTEX_MAGIC, PTHREAD_MUTEX_INITIALIZER) != VALID_STATIC_OBJECT)
    return false;
  return true;
}

bool
pthread_mutex::is_good_initializer_or_object (pthread_mutex_t const *mutex)
{
  if (verifyable_object_isvalid (mutex, PTHREAD_MUTEX_MAGIC, PTHREAD_MUTEX_INITIALIZER) == INVALID_OBJECT)
    return false;
  return true;
}

bool
pthread_mutex::is_good_initializer_or_bad_object (pthread_mutex_t const *mutex)
{
  verifyable_object_state objectState = verifyable_object_isvalid (mutex, PTHREAD_MUTEX_MAGIC, PTHREAD_MUTEX_INITIALIZER);
  if (objectState == VALID_OBJECT)
	return false;
  return true;
}

bool
pthread_mutex::can_be_unlocked (pthread_mutex_t const *mutex)
{
  pthread_t self = pthread::self ();

  if (!is_good_object (mutex))
    return false;
  /*
   * Check if the mutex is owned by the current thread and can be unlocked
   */
  return ((*mutex)->recursion_counter == 1 && pthread::equal ((*mutex)->owner, self));
}

List<pthread_mutex> pthread_mutex::mutexes;

/* This is used for mutex creation protection within a single process only */
fast_mutex NO_COPY pthread_mutex::mutex_initialization_lock;

/* We can only be called once.
   TODO: (no rush) use a non copied memory section to
   hold an initialization flag.  */
void
pthread_mutex::init_mutex ()
{
  if (!mutex_initialization_lock.init ())
    api_fatal ("Could not create win32 Mutex for pthread mutex static initializer support.");
}

pthread_mutex::pthread_mutex (pthread_mutexattr *attr) :
  verifyable_object (PTHREAD_MUTEX_MAGIC),
  lock_counter (0),
  win32_obj_id (NULL), recursion_counter (0),
  condwaits (0), owner (NULL), type (PTHREAD_MUTEX_DEFAULT),
  pshared (PTHREAD_PROCESS_PRIVATE)
{
  win32_obj_id = ::CreateSemaphore (&sec_none_nih, 0, LONG_MAX, NULL);
  if (!win32_obj_id)
    {
      magic = 0;
      return;
    }
  /*attr checked in the C call */
  if (attr)
    {
      if (attr->pshared == PTHREAD_PROCESS_SHARED)
	{
	  // fail
	  magic = 0;
	  return;
	}

      type = attr->mutextype;
    }

  mutexes.insert (this);
}

pthread_mutex::~pthread_mutex ()
{
  if (win32_obj_id)
    CloseHandle (win32_obj_id);

  mutexes.remove (this);
}

int
pthread_mutex::_lock (pthread_t self)
{
  int result = 0;

  if (InterlockedIncrement ((long *)&lock_counter) == 1)
    set_owner (self);
  else if (type != PTHREAD_MUTEX_NORMAL && pthread::equal (owner, self))
    {
      InterlockedDecrement ((long *) &lock_counter);
      if (type == PTHREAD_MUTEX_RECURSIVE)
	result = lock_recursive ();
      else
	result = EDEADLK;
    }
  else
    {
      WaitForSingleObject (win32_obj_id, INFINITE);
      set_owner (self);
    }

  return result;
}

int
pthread_mutex::_trylock (pthread_t self)
{
  int result = 0;

  if (InterlockedCompareExchange ((long *) &lock_counter, 1, 0) == 0)
    set_owner (self);
  else if (type == PTHREAD_MUTEX_RECURSIVE && pthread::equal (owner, self))
    result = lock_recursive ();
  else
    result = EBUSY;

  return result;
}

int
pthread_mutex::_unlock (pthread_t self)
{
  if (!pthread::equal (owner, self))
    return EPERM;

  if (--recursion_counter == 0)
    {
      owner = NULL;
      if (InterlockedDecrement ((long *)&lock_counter))
	// Another thread is waiting
	::ReleaseSemaphore (win32_obj_id, 1, NULL);
    }

  return 0;
}

int
pthread_mutex::_destroy (pthread_t self)
{
  if (condwaits || _trylock (self))
    // Do not destroy a condwaited or locked mutex
    return EBUSY;
  else if (recursion_counter != 1)
    {
      // Do not destroy a recursive locked mutex
      --recursion_counter;
      return EBUSY;
    }

  delete this;
  return 0;
}

void
pthread_mutex::_fixup_after_fork ()
{
  debug_printf ("mutex %x in _fixup_after_fork", this);
  if (pshared != PTHREAD_PROCESS_PRIVATE)
    api_fatal ("pthread_mutex::_fixup_after_fork () doesn'tunderstand PROCESS_SHARED mutex's");

  if (owner == NULL)
    /* mutex has no owner, reset to initial */
    lock_counter = 0;
  else if (lock_counter != 0)
    /* All waiting threads are gone after a fork */
    lock_counter = 1;

  win32_obj_id = ::CreateSemaphore (&sec_none_nih, 0, LONG_MAX, NULL);
  if (!win32_obj_id)
    api_fatal ("pthread_mutex::_fixup_after_fork () failed to recreate win32 semaphore for mutex");

  condwaits = 0;
}

bool
pthread_mutexattr::is_good_object (pthread_mutexattr_t const * attr)
{
  if (verifyable_object_isvalid (attr, PTHREAD_MUTEXATTR_MAGIC) != VALID_OBJECT)
    return false;
  return true;
}

pthread_mutexattr::pthread_mutexattr ():verifyable_object (PTHREAD_MUTEXATTR_MAGIC),
pshared (PTHREAD_PROCESS_PRIVATE), mutextype (PTHREAD_MUTEX_DEFAULT)
{
}

pthread_mutexattr::~pthread_mutexattr ()
{
}

List<semaphore> semaphore::semaphores;

semaphore::semaphore (int pshared, unsigned int value)
: verifyable_object (SEM_MAGIC),
  shared (pshared),
  currentvalue (value),
  name (NULL)
{
  SECURITY_ATTRIBUTES sa = (pshared != PTHREAD_PROCESS_PRIVATE)
			   ? sec_all : sec_none_nih;
  this->win32_obj_id = ::CreateSemaphore (&sa, value, LONG_MAX, NULL);
  if (!this->win32_obj_id)
    magic = 0;

  semaphores.insert (this);
}

semaphore::semaphore (const char *sem_name, int oflag, mode_t mode,
				  unsigned int value)
: verifyable_object (SEM_MAGIC),
  shared (PTHREAD_PROCESS_SHARED),
  currentvalue (value), 		/* Unused for named semaphores. */
  name (NULL)
{
  if (oflag & O_CREAT)
    {
      SECURITY_ATTRIBUTES sa = sec_all;
      security_descriptor sd;
      if (allow_ntsec)
	set_security_attribute (mode, &sa, sd);
      this->win32_obj_id = ::CreateSemaphore (&sa, value, LONG_MAX, sem_name);
      if (!this->win32_obj_id)
	magic = 0;
      if (GetLastError () == ERROR_ALREADY_EXISTS && (oflag & O_EXCL))
	{
	  __seterrno ();
	  CloseHandle (this->win32_obj_id);
	  magic = 0;
	}
    }
  else
    {
      this->win32_obj_id = ::OpenSemaphore (SEMAPHORE_ALL_ACCESS, FALSE,
					    sem_name);
      if (!this->win32_obj_id)
	{
	  __seterrno ();
	  magic = 0;
	}
    }
  if (magic)
    {
      name = new char [strlen (sem_name + 1)];
      if (!name)
	{
	  set_errno (ENOSPC);
	  CloseHandle (this->win32_obj_id);
	  magic = 0;
	}
      else
	strcpy (name, sem_name);
    }

  semaphores.insert (this);
}

semaphore::~semaphore ()
{
  if (win32_obj_id)
    CloseHandle (win32_obj_id);

  delete [] name;

  semaphores.remove (this);
}

void
semaphore::_post ()
{
  if (ReleaseSemaphore (win32_obj_id, 1, &currentvalue))
    currentvalue++;
}

int
semaphore::_getvalue (int *sval)
{
  long val;

  switch (WaitForSingleObject (win32_obj_id, 0))
    {
      case WAIT_OBJECT_0:
	ReleaseSemaphore (win32_obj_id, 1, &val);
	*sval = val + 1;
	break;
      case WAIT_TIMEOUT:
	*sval = 0;
	break;
      default:
	set_errno (EAGAIN);
	return -1;
    }
  return 0;
}

int
semaphore::_trywait ()
{
  /* FIXME: signals should be able to interrupt semaphores...
   *We probably need WaitForMultipleObjects here.
   */
  if (WaitForSingleObject (win32_obj_id, 0) == WAIT_TIMEOUT)
    {
      set_errno (EAGAIN);
      return -1;
    }
  currentvalue--;
  return 0;
}

int
semaphore::_timedwait (const struct timespec *abstime)
{
  struct timeval tv;
  long waitlength;

  if (__check_invalid_read_ptr (abstime, sizeof *abstime))
    {
      /* According to SUSv3, abstime need not be checked for validity,
	 if the semaphore can be locked immediately. */
      if (!_trywait ())
	return 0;
      set_errno (EINVAL);
      return -1;
    }

  gettimeofday (&tv, NULL);
  waitlength = abstime->tv_sec * 1000 + abstime->tv_nsec / (1000 * 1000);
  waitlength -= tv.tv_sec * 1000 + tv.tv_usec / 1000;
  if (waitlength < 0)
    waitlength = 0;
  switch (pthread::cancelable_wait (win32_obj_id, waitlength))
    {
    case WAIT_OBJECT_0:
      currentvalue--;
      break;
    case WAIT_TIMEOUT:
      set_errno (ETIMEDOUT);
      return -1;
    default:
      debug_printf ("cancelable_wait failed. %E");
      __seterrno ();
      return -1;
    }
  return 0;
}

void
semaphore::_wait ()
{
  switch (pthread::cancelable_wait (win32_obj_id, INFINITE))
    {
    case WAIT_OBJECT_0:
      currentvalue--;
      break;
    default:
      debug_printf ("cancelable_wait failed. %E");
      return;
    }
}

void
semaphore::_fixup_after_fork ()
{
  if (shared == PTHREAD_PROCESS_PRIVATE)
    {
      debug_printf ("sem %x in _fixup_after_fork", this);
      /* FIXME: duplicate code here and in the constructor. */
      this->win32_obj_id = ::CreateSemaphore (&sec_none_nih, currentvalue,
					      LONG_MAX, NULL);
      if (!win32_obj_id)
	api_fatal ("failed to create new win32 semaphore, error %d");
    }
}

verifyable_object::verifyable_object (long verifyer):
magic (verifyer)
{
}

verifyable_object::~verifyable_object ()
{
  magic = 0;
}

/* Generic memory acccess routine - where should it live ? */
int __stdcall
check_valid_pointer (void const *pointer)
{
  if (!pointer || IsBadWritePtr ((void *) pointer, sizeof (verifyable_object)))
    return EFAULT;
  return 0;
}

verifyable_object_state
verifyable_object_isvalid (void const * objectptr, long magic, void *static_ptr)
{
  verifyable_object **object = (verifyable_object **)objectptr;
  if (check_valid_pointer (object))
    return INVALID_OBJECT;
  if (static_ptr && *object == static_ptr)
    return VALID_STATIC_OBJECT;
  if (!*object)
    return INVALID_OBJECT;
  if (check_valid_pointer (*object))
    return INVALID_OBJECT;
  if ((*object)->magic != magic)
    return INVALID_OBJECT;
  return VALID_OBJECT;
}

verifyable_object_state
verifyable_object_isvalid (void const * objectptr, long magic)
{
  return verifyable_object_isvalid (objectptr, magic, NULL);
}

DWORD WINAPI
pthread::thread_init_wrapper (void *arg)
{
  pthread *thread = (pthread *) arg;
  _my_tls.tid = thread;

  thread->mutex.lock ();

  // if thread is detached force cleanup on exit
  if (thread->attr.joinable == PTHREAD_CREATE_DETACHED && thread->joiner == NULL)
    thread->joiner = thread;
  thread->mutex.unlock ();

  thread_printf ("started thread %p %p %p %p %p %p", arg, &_my_tls.local_clib,
		 _impure_ptr, thread, thread->function, thread->arg);

  // call the user's thread
  void *ret = thread->function (thread->arg);

  thread->exit (ret);

  return 0;	// just for show.  Never returns.
}

bool
pthread::is_good_object (pthread_t const *thread)
{
  if (verifyable_object_isvalid (thread, PTHREAD_MAGIC) != VALID_OBJECT)
    return false;
  return true;
}

unsigned long
pthread::getsequence_np ()
{
  return get_thread_id ();
}

int
pthread::create (pthread_t *thread, const pthread_attr_t *attr,
		  void *(*start_routine) (void *), void *arg)
{
  if (attr && !pthread_attr::is_good_object (attr))
    return EINVAL;

  *thread = new pthread ();
  (*thread)->create (start_routine, attr ? *attr : NULL, arg);
  if (!is_good_object (thread))
    {
      delete (*thread);
      *thread = NULL;
      return EAGAIN;
    }

  return 0;
}

int
pthread::once (pthread_once_t *once_control, void (*init_routine) (void))
{
  // already done ?
  if (once_control->state)
    return 0;

  pthread_mutex_lock (&once_control->mutex);
  /* Here we must set a cancellation handler to unlock the mutex if needed */
  /* but a cancellation handler is not the right thing. We need this in the thread
   *cleanup routine. Assumption: a thread can only be in one pthread_once routine
   *at a time. Stote a mutex_t *in the pthread_structure. if that's non null unlock
   *on pthread_exit ();
   */
  if (!once_control->state)
    {
      init_routine ();
      once_control->state = 1;
    }
  /* Here we must remove our cancellation handler */
  pthread_mutex_unlock (&once_control->mutex);
  return 0;
}

int
pthread::cancel (pthread_t thread)
{
  if (!is_good_object (&thread))
    return ESRCH;

  return thread->cancel ();
}

void
pthread::atforkprepare (void)
{
  MT_INTERFACE->fixup_before_fork ();

  callback *cb = MT_INTERFACE->pthread_prepare;
  while (cb)
    {
      cb->cb ();
      cb = cb->next;
    }
}

void
pthread::atforkparent (void)
{
  callback *cb = MT_INTERFACE->pthread_parent;
  while (cb)
    {
      cb->cb ();
      cb = cb->next;
    }
}

void
pthread::atforkchild (void)
{
  MT_INTERFACE->fixup_after_fork ();

  callback *cb = MT_INTERFACE->pthread_child;
  while (cb)
    {
      cb->cb ();
      cb = cb->next;
    }
}

/* Register a set of functions to run before and after fork.
   prepare calls are called in LI-FC order.
   parent and child calls are called in FI-FC order.  */
int
pthread::atfork (void (*prepare)(void), void (*parent)(void), void (*child)(void))
{
  callback *prepcb = NULL, *parentcb = NULL, *childcb = NULL;
  if (prepare)
    {
      prepcb = new callback;
      if (!prepcb)
	return ENOMEM;
    }
  if (parent)
    {
      parentcb = new callback;
      if (!parentcb)
	{
	  if (prepcb)
	    delete prepcb;
	  return ENOMEM;
	}
    }
  if (child)
    {
      childcb = new callback;
      if (!childcb)
	{
	  if (prepcb)
	    delete prepcb;
	  if (parentcb)
	    delete parentcb;
	  return ENOMEM;
	}
    }

  if (prepcb)
  {
    prepcb->cb = prepare;
    List_insert (MT_INTERFACE->pthread_prepare, prepcb);
  }
  if (parentcb)
  {
    parentcb->cb = parent;
    callback **t = &MT_INTERFACE->pthread_parent;
    while (*t)
      t = &(*t)->next;
    /* t = pointer to last next in the list */
    List_insert (*t, parentcb);
  }
  if (childcb)
  {
    childcb->cb = child;
    callback **t = &MT_INTERFACE->pthread_child;
    while (*t)
      t = &(*t)->next;
    /* t = pointer to last next in the list */
    List_insert (*t, childcb);
  }
  return 0;
}

extern "C" int
pthread_attr_init (pthread_attr_t *attr)
{
  if (pthread_attr::is_good_object (attr))
    return EBUSY;

  *attr = new pthread_attr;
  if (!pthread_attr::is_good_object (attr))
    {
      delete (*attr);
      *attr = NULL;
      return ENOMEM;
    }
  return 0;
}

extern "C" int
pthread_attr_getinheritsched (const pthread_attr_t *attr,
				int *inheritsched)
{
  if (!pthread_attr::is_good_object (attr))
    return EINVAL;
  *inheritsched = (*attr)->inheritsched;
  return 0;
}

extern "C" int
pthread_attr_getschedparam (const pthread_attr_t *attr,
			      struct sched_param *param)
{
  if (!pthread_attr::is_good_object (attr))
    return EINVAL;
  *param = (*attr)->schedparam;
  return 0;
}

/* From a pure code point of view, this should call a helper in sched.cc,
   to allow for someone adding scheduler policy changes to win32 in the future.
   However that's extremely unlikely, so short and sweet will do us */
extern "C" int
pthread_attr_getschedpolicy (const pthread_attr_t *attr, int *policy)
{
  if (!pthread_attr::is_good_object (attr))
    return EINVAL;
  *policy = SCHED_FIFO;
  return 0;
}


extern "C" int
pthread_attr_getscope (const pthread_attr_t *attr, int *contentionscope)
{
  if (!pthread_attr::is_good_object (attr))
    return EINVAL;
  *contentionscope = (*attr)->contentionscope;
  return 0;
}

extern "C" int
pthread_attr_setdetachstate (pthread_attr_t *attr, int detachstate)
{
  if (!pthread_attr::is_good_object (attr))
    return EINVAL;
  if (detachstate < 0 || detachstate > 1)
    return EINVAL;
  (*attr)->joinable = detachstate;
  return 0;
}

extern "C" int
pthread_attr_getdetachstate (const pthread_attr_t *attr, int *detachstate)
{
  if (!pthread_attr::is_good_object (attr))
    return EINVAL;
  *detachstate = (*attr)->joinable;
  return 0;
}

extern "C" int
pthread_attr_setinheritsched (pthread_attr_t *attr, int inheritsched)
{
  if (!pthread_attr::is_good_object (attr))
    return EINVAL;
  if (inheritsched != PTHREAD_INHERIT_SCHED
      && inheritsched != PTHREAD_EXPLICIT_SCHED)
    return ENOTSUP;
  (*attr)->inheritsched = inheritsched;
  return 0;
}

extern "C" int
pthread_attr_setschedparam (pthread_attr_t *attr,
			      const struct sched_param *param)
{
  if (!pthread_attr::is_good_object (attr))
    return EINVAL;
  if (!valid_sched_parameters (param))
    return ENOTSUP;
  (*attr)->schedparam = *param;
  return 0;
}

/* See __pthread_attr_getschedpolicy for some notes */
extern "C" int
pthread_attr_setschedpolicy (pthread_attr_t *attr, int policy)
{
  if (!pthread_attr::is_good_object (attr))
    return EINVAL;
  if (policy != SCHED_FIFO)
    return ENOTSUP;
  return 0;
}

extern "C" int
pthread_attr_setscope (pthread_attr_t *attr, int contentionscope)
{
  if (!pthread_attr::is_good_object (attr))
    return EINVAL;
  if (contentionscope != PTHREAD_SCOPE_SYSTEM
      && contentionscope != PTHREAD_SCOPE_PROCESS)
    return EINVAL;
  /* In future, we may be able to support system scope by escalating the thread
     priority to exceed the priority class. For now we only support PROCESS scope. */
  if (contentionscope != PTHREAD_SCOPE_PROCESS)
    return ENOTSUP;
  (*attr)->contentionscope = contentionscope;
  return 0;
}

extern "C" int
pthread_attr_setstacksize (pthread_attr_t *attr, size_t size)
{
  if (!pthread_attr::is_good_object (attr))
    return EINVAL;
  (*attr)->stacksize = size;
  return 0;
}

extern "C" int
pthread_attr_getstacksize (const pthread_attr_t *attr, size_t *size)
{
  if (!pthread_attr::is_good_object (attr))
    return EINVAL;
  *size = (*attr)->stacksize;
  return 0;
}

extern "C" int
pthread_attr_destroy (pthread_attr_t *attr)
{
  if (!pthread_attr::is_good_object (attr))
    return EINVAL;
  delete (*attr);
  *attr = NULL;
  return 0;
}

int
pthread::join (pthread_t *thread, void **return_val)
{
   pthread_t joiner = self ();

   joiner->testcancel ();

   // Initialize return val with NULL
   if (return_val)
     *return_val = NULL;

   if (!is_good_object (&joiner))
     return EINVAL;

  if (!is_good_object (thread))
    return ESRCH;

  if (equal (*thread,joiner))
    return EDEADLK;

  (*thread)->mutex.lock ();

  if ((*thread)->attr.joinable == PTHREAD_CREATE_DETACHED)
    {
      (*thread)->mutex.unlock ();
      return EINVAL;
    }
  else
    {
      (*thread)->joiner = joiner;
      (*thread)->attr.joinable = PTHREAD_CREATE_DETACHED;
      (*thread)->mutex.unlock ();

      switch (cancelable_wait ((*thread)->win32_obj_id, INFINITE, false))
	{
	case WAIT_OBJECT_0:
	  if (return_val)
	    *return_val = (*thread)->return_ptr;
	  delete (*thread);
	  break;
	case WAIT_CANCELED:
	  // set joined thread back to joinable since we got canceled
	  (*thread)->joiner = NULL;
	  (*thread)->attr.joinable = PTHREAD_CREATE_JOINABLE;
	  joiner->cancel_self ();
	  // never reached
	  break;
	default:
	  // should never happen
	  return EINVAL;
	}
    }

  return 0;
}

int
pthread::detach (pthread_t *thread)
{
  if (!is_good_object (thread))
    return ESRCH;

  (*thread)->mutex.lock ();
  if ((*thread)->attr.joinable == PTHREAD_CREATE_DETACHED)
    {
      (*thread)->mutex.unlock ();
      return EINVAL;
    }

  // check if thread is still alive
  if ((*thread)->valid && WaitForSingleObject ((*thread)->win32_obj_id, 0) == WAIT_TIMEOUT)
    {
      // force cleanup on exit
      (*thread)->joiner = *thread;
      (*thread)->attr.joinable = PTHREAD_CREATE_DETACHED;
      (*thread)->mutex.unlock ();
    }
  else
    {
      // thread has already terminated.
      (*thread)->mutex.unlock ();
      delete (*thread);
    }

  return 0;
}

int
pthread::suspend (pthread_t *thread)
{
  if (!is_good_object (thread))
    return ESRCH;

  if ((*thread)->suspended == false)
    {
      (*thread)->suspended = true;
      SuspendThread ((*thread)->win32_obj_id);
    }

  return 0;
}


int
pthread::resume (pthread_t *thread)
{
  if (!is_good_object (thread))
    return ESRCH;

  if ((*thread)->suspended == true)
    ResumeThread ((*thread)->win32_obj_id);
  (*thread)->suspended = false;

  return 0;
}

/* provided for source level compatability.
   See http://www.opengroup.org/onlinepubs/007908799/xsh/pthread_getconcurrency.html
*/
extern "C" int
pthread_getconcurrency (void)
{
  return MT_INTERFACE->concurrency;
}

/* keep this in sync with sched.cc */
extern "C" int
pthread_getschedparam (pthread_t thread, int *policy,
			 struct sched_param *param)
{
  if (!pthread::is_good_object (&thread))
    return ESRCH;
  *policy = SCHED_FIFO;
  /* we don't return the current effective priority, we return the current
     requested priority */
  *param = thread->attr.schedparam;
  return 0;
}

/* Thread Specific Data */
extern "C" int
pthread_key_create (pthread_key_t *key, void (*destructor) (void *))
{
  /* The opengroup docs don't define if we should check this or not,
     but creation is relatively rare.  */
  if (pthread_key::is_good_object (key))
    return EBUSY;

  *key = new pthread_key (destructor);

  if (!pthread_key::is_good_object (key))
    {
      delete (*key);
      *key = NULL;
      return EAGAIN;
    }
  return 0;
}

extern "C" int
pthread_key_delete (pthread_key_t key)
{
  if (!pthread_key::is_good_object (&key))
    return EINVAL;

  delete (key);
  return 0;
}

/* provided for source level compatability.  See
http://www.opengroup.org/onlinepubs/007908799/xsh/pthread_getconcurrency.html
*/
extern "C" int
pthread_setconcurrency (int new_level)
{
  if (new_level < 0)
    return EINVAL;
  MT_INTERFACE->concurrency = new_level;
  return 0;
}

/* keep syncronised with sched.cc */
extern "C" int
pthread_setschedparam (pthread_t thread, int policy,
			 const struct sched_param *param)
{
  if (!pthread::is_good_object (&thread))
    return ESRCH;
  if (policy != SCHED_FIFO)
    return ENOTSUP;
  if (!param)
    return EINVAL;
  int rv =
    sched_set_thread_priority (thread->win32_obj_id, param->sched_priority);
  if (!rv)
    thread->attr.schedparam.sched_priority = param->sched_priority;
  return rv;
}


extern "C" int
pthread_setspecific (pthread_key_t key, const void *value)
{
  if (!pthread_key::is_good_object (&key))
    return EINVAL;
  (key)->set (value);
  return 0;
}

extern "C" void *
pthread_getspecific (pthread_key_t key)
{
  if (!pthread_key::is_good_object (&key))
    return NULL;

  return (key)->get ();

}

/* Thread synchronisation */
bool
pthread_cond::is_good_object (pthread_cond_t const *cond)
{
  if (verifyable_object_isvalid (cond, PTHREAD_COND_MAGIC) != VALID_OBJECT)
    return false;
  return true;
}

bool
pthread_cond::is_good_initializer (pthread_cond_t const *cond)
{
  if (verifyable_object_isvalid (cond, PTHREAD_COND_MAGIC, PTHREAD_COND_INITIALIZER) != VALID_STATIC_OBJECT)
    return false;
  return true;
}

bool
pthread_cond::is_good_initializer_or_object (pthread_cond_t const *cond)
{
  if (verifyable_object_isvalid (cond, PTHREAD_COND_MAGIC, PTHREAD_COND_INITIALIZER) == INVALID_OBJECT)
    return false;
  return true;
}

bool
pthread_cond::is_good_initializer_or_bad_object (pthread_cond_t const *cond)
{
  verifyable_object_state objectState = verifyable_object_isvalid (cond, PTHREAD_COND_MAGIC, PTHREAD_COND_INITIALIZER);
  if (objectState == VALID_OBJECT)
	return false;
  return true;
}

extern "C" int
pthread_cond_destroy (pthread_cond_t *cond)
{
  if (pthread_cond::is_good_initializer (cond))
    return 0;
  if (!pthread_cond::is_good_object (cond))
    return EINVAL;

  /* reads are atomic */
  if ((*cond)->waiting)
    return EBUSY;

  delete (*cond);
  *cond = NULL;

  return 0;
}

int
pthread_cond::init (pthread_cond_t *cond, const pthread_condattr_t *attr)
{
  if (attr && !pthread_condattr::is_good_object (attr))
    return EINVAL;

  cond_initialization_lock.lock ();

  if (!is_good_initializer_or_bad_object (cond))
    {
      cond_initialization_lock.unlock ();
      return EBUSY;
    }

  *cond = new pthread_cond (attr ? (*attr) : NULL);
  if (!is_good_object (cond))
    {
      delete (*cond);
      *cond = NULL;
      cond_initialization_lock.unlock ();
      return EAGAIN;
    }
  cond_initialization_lock.unlock ();
  return 0;
}

extern "C" int
pthread_cond_broadcast (pthread_cond_t *cond)
{
  if (pthread_cond::is_good_initializer (cond))
    return 0;
  if (!pthread_cond::is_good_object (cond))
    return EINVAL;

  (*cond)->unblock (true);

  return 0;
}

extern "C" int
pthread_cond_signal (pthread_cond_t *cond)
{
  if (pthread_cond::is_good_initializer (cond))
    return 0;
  if (!pthread_cond::is_good_object (cond))
    return EINVAL;

  (*cond)->unblock (false);

  return 0;
}

static int
__pthread_cond_dowait (pthread_cond_t *cond, pthread_mutex_t *mutex,
		       DWORD waitlength)
{
  if (!pthread_mutex::is_good_object (mutex))
    return EINVAL;
  if (!pthread_mutex::can_be_unlocked (mutex))
    return EPERM;

  if (pthread_cond::is_good_initializer (cond))
    pthread_cond::init (cond, NULL);
  if (!pthread_cond::is_good_object (cond))
    return EINVAL;

  return (*cond)->wait (*mutex, waitlength);
}

extern "C" int
pthread_cond_timedwait (pthread_cond_t *cond, pthread_mutex_t *mutex,
			const struct timespec *abstime)
{
  struct timeval tv;
  long waitlength;

  pthread_testcancel ();

  if (check_valid_pointer (abstime))
    return EINVAL;

  gettimeofday (&tv, NULL);
  waitlength = abstime->tv_sec * 1000 + abstime->tv_nsec / (1000 * 1000);
  waitlength -= tv.tv_sec * 1000 + tv.tv_usec / 1000;
  if (waitlength < 0)
    return ETIMEDOUT;
  return __pthread_cond_dowait (cond, mutex, waitlength);
}

extern "C" int
pthread_cond_wait (pthread_cond_t *cond, pthread_mutex_t *mutex)
{
  pthread_testcancel ();

  return __pthread_cond_dowait (cond, mutex, INFINITE);
}

extern "C" int
pthread_condattr_init (pthread_condattr_t *condattr)
{
  if (pthread_condattr::is_good_object (condattr))
    return EBUSY;

  *condattr = new pthread_condattr;
  if (!pthread_condattr::is_good_object (condattr))
    {
      delete (*condattr);
      *condattr = NULL;
      return ENOMEM;
    }
  return 0;
}

extern "C" int
pthread_condattr_getpshared (const pthread_condattr_t *attr, int *pshared)
{
  if (!pthread_condattr::is_good_object (attr))
    return EINVAL;
  *pshared = (*attr)->shared;
  return 0;
}

extern "C" int
pthread_condattr_setpshared (pthread_condattr_t *attr, int pshared)
{
  if (!pthread_condattr::is_good_object (attr))
    return EINVAL;
  if ((pshared < 0) || (pshared > 1))
    return EINVAL;
  /* shared cond vars not currently supported */
  if (pshared != PTHREAD_PROCESS_PRIVATE)
    return EINVAL;
  (*attr)->shared = pshared;
  return 0;
}

extern "C" int
pthread_condattr_destroy (pthread_condattr_t *condattr)
{
  if (!pthread_condattr::is_good_object (condattr))
    return EINVAL;
  delete (*condattr);
  *condattr = NULL;
  return 0;
}

/* RW locks */
bool
pthread_rwlock::is_good_object (pthread_rwlock_t const *rwlock)
{
  if (verifyable_object_isvalid (rwlock, PTHREAD_RWLOCK_MAGIC) != VALID_OBJECT)
    return false;
  return true;
}

bool
pthread_rwlock::is_good_initializer (pthread_rwlock_t const *rwlock)
{
  if (verifyable_object_isvalid (rwlock, PTHREAD_RWLOCK_MAGIC, PTHREAD_RWLOCK_INITIALIZER) != VALID_STATIC_OBJECT)
    return false;
  return true;
}

bool
pthread_rwlock::is_good_initializer_or_object (pthread_rwlock_t const *rwlock)
{
  if (verifyable_object_isvalid (rwlock, PTHREAD_RWLOCK_MAGIC, PTHREAD_RWLOCK_INITIALIZER) == INVALID_OBJECT)
    return false;
  return true;
}

bool
pthread_rwlock::is_good_initializer_or_bad_object (pthread_rwlock_t const *rwlock)
{
  verifyable_object_state objectState = verifyable_object_isvalid (rwlock, PTHREAD_RWLOCK_MAGIC, PTHREAD_RWLOCK_INITIALIZER);
  if (objectState == VALID_OBJECT)
	return false;
  return true;
}

extern "C" int
pthread_rwlock_destroy (pthread_rwlock_t *rwlock)
{
  if (pthread_rwlock::is_good_initializer (rwlock))
    return 0;
  if (!pthread_rwlock::is_good_object (rwlock))
    return EINVAL;

  if ((*rwlock)->writer || (*rwlock)->readers ||
      (*rwlock)->waiting_readers || (*rwlock)->waiting_writers)
    return EBUSY;

  delete (*rwlock);
  *rwlock = NULL;

  return 0;
}

int
pthread_rwlock::init (pthread_rwlock_t *rwlock, const pthread_rwlockattr_t *attr)
{
  if (attr && !pthread_rwlockattr::is_good_object (attr))
    return EINVAL;

  rwlock_initialization_lock.lock ();

  if (!is_good_initializer_or_bad_object (rwlock))
    {
      rwlock_initialization_lock.unlock ();
      return EBUSY;
    }

  *rwlock = new pthread_rwlock (attr ? (*attr) : NULL);
  if (!is_good_object (rwlock))
    {
      delete (*rwlock);
      *rwlock = NULL;
      rwlock_initialization_lock.unlock ();
      return EAGAIN;
    }
  rwlock_initialization_lock.unlock ();
  return 0;
}

extern "C" int
pthread_rwlock_rdlock (pthread_rwlock_t *rwlock)
{
  pthread_testcancel ();

  if (pthread_rwlock::is_good_initializer (rwlock))
    pthread_rwlock::init (rwlock, NULL);
  if (!pthread_rwlock::is_good_object (rwlock))
    return EINVAL;

  return (*rwlock)->rdlock ();
}

extern "C" int
pthread_rwlock_tryrdlock (pthread_rwlock_t *rwlock)
{
  if (pthread_rwlock::is_good_initializer (rwlock))
    pthread_rwlock::init (rwlock, NULL);
  if (!pthread_rwlock::is_good_object (rwlock))
    return EINVAL;

  return (*rwlock)->tryrdlock ();
}

extern "C" int
pthread_rwlock_wrlock (pthread_rwlock_t *rwlock)
{
  pthread_testcancel ();

  if (pthread_rwlock::is_good_initializer (rwlock))
    pthread_rwlock::init (rwlock, NULL);
  if (!pthread_rwlock::is_good_object (rwlock))
    return EINVAL;

  return (*rwlock)->wrlock ();
}

extern "C" int
pthread_rwlock_trywrlock (pthread_rwlock_t *rwlock)
{
  if (pthread_rwlock::is_good_initializer (rwlock))
    pthread_rwlock::init (rwlock, NULL);
  if (!pthread_rwlock::is_good_object (rwlock))
    return EINVAL;

  return (*rwlock)->trywrlock ();
}

extern "C" int
pthread_rwlock_unlock (pthread_rwlock_t *rwlock)
{
  if (pthread_rwlock::is_good_initializer (rwlock))
    return 0;
  if (!pthread_rwlock::is_good_object (rwlock))
    return EINVAL;

  return (*rwlock)->unlock ();
}

extern "C" int
pthread_rwlockattr_init (pthread_rwlockattr_t *rwlockattr)
{
  if (pthread_rwlockattr::is_good_object (rwlockattr))
    return EBUSY;

  *rwlockattr = new pthread_rwlockattr;
  if (!pthread_rwlockattr::is_good_object (rwlockattr))
    {
      delete (*rwlockattr);
      *rwlockattr = NULL;
      return ENOMEM;
    }
  return 0;
}

extern "C" int
pthread_rwlockattr_getpshared (const pthread_rwlockattr_t *attr, int *pshared)
{
  if (!pthread_rwlockattr::is_good_object (attr))
    return EINVAL;
  *pshared = (*attr)->shared;
  return 0;
}

extern "C" int
pthread_rwlockattr_setpshared (pthread_rwlockattr_t *attr, int pshared)
{
  if (!pthread_rwlockattr::is_good_object (attr))
    return EINVAL;
  if ((pshared < 0) || (pshared > 1))
    return EINVAL;
  /* shared rwlock vars not currently supported */
  if (pshared != PTHREAD_PROCESS_PRIVATE)
    return EINVAL;
  (*attr)->shared = pshared;
  return 0;
}

extern "C" int
pthread_rwlockattr_destroy (pthread_rwlockattr_t *rwlockattr)
{
  if (!pthread_rwlockattr::is_good_object (rwlockattr))
    return EINVAL;
  delete (*rwlockattr);
  *rwlockattr = NULL;
  return 0;
}

/* Thread signal */
extern "C" int
pthread_kill (pthread_t thread, int sig)
{
  // lock myself, for the use of thread2signal
  // two different kills might clash: FIXME

  if (!pthread::is_good_object (&thread))
    return EINVAL;

  int rval = sig ? sig_send (NULL, sig, thread->cygtls) : 0;

  // unlock myself
  return rval;
}

extern "C" int
pthread_sigmask (int operation, const sigset_t *set, sigset_t *old_set)
{
  return handle_sigprocmask (operation, set, old_set, _my_tls.sigmask);
}

/* ID */

extern "C" int
pthread_equal (pthread_t t1, pthread_t t2)
{
  return pthread::equal (t1, t2);
}

/* Mutexes  */

/* FIXME: there's a potential race with PTHREAD_MUTEX_INITALIZER:
   the mutex is not actually inited until the first use.
   So two threads trying to lock/trylock may collide.
   Solution: we need a global mutex on mutex creation, or possibly simply
   on all constructors that allow INITIALIZER macros.
   the lock should be very small: only around the init routine, not
   every test, or all mutex access will be synchronised.  */

int
pthread_mutex::init (pthread_mutex_t *mutex,
		      const pthread_mutexattr_t *attr)
{
  if (attr && !pthread_mutexattr::is_good_object (attr) || check_valid_pointer (mutex))
    return EINVAL;

  mutex_initialization_lock.lock ();

  if (!is_good_initializer_or_bad_object (mutex))
    {
      mutex_initialization_lock.unlock ();
      return EBUSY;
    }

  *mutex = new pthread_mutex (attr ? (*attr) : NULL);
  if (!is_good_object (mutex))
    {
      delete (*mutex);
      *mutex = NULL;
      mutex_initialization_lock.unlock ();
      return EAGAIN;
    }
  mutex_initialization_lock.unlock ();
  return 0;
}

extern "C" int
pthread_mutex_getprioceiling (const pthread_mutex_t *mutex,
				int *prioceiling)
{
  pthread_mutex_t *themutex = (pthread_mutex_t *) mutex;
  if (pthread_mutex::is_good_initializer (mutex))
    pthread_mutex::init ((pthread_mutex_t *) mutex, NULL);
  if (!pthread_mutex::is_good_object (themutex))
    return EINVAL;
  /* We don't define _POSIX_THREAD_PRIO_PROTECT because we do't currently support
     mutex priorities.

     We can support mutex priorities in the future though:
     Store a priority with each mutex.
     When the mutex is optained, set the thread priority as appropriate
     When the mutex is released, reset the thread priority.  */
  return ENOSYS;
}

extern "C" int
pthread_mutex_lock (pthread_mutex_t *mutex)
{
  pthread_mutex_t *themutex = mutex;
  /* This could be simplified via is_good_initializer_or_object
     and is_good_initializer, but in a performance critical call like this....
     no.  */
  switch (verifyable_object_isvalid (themutex, PTHREAD_MUTEX_MAGIC, PTHREAD_MUTEX_INITIALIZER))
    {
    case INVALID_OBJECT:
      return EINVAL;
      break;
    case VALID_STATIC_OBJECT:
      if (pthread_mutex::is_good_initializer (mutex))
	{
	  int rv = pthread_mutex::init (mutex, NULL);
	  if (rv && rv != EBUSY)
	    return rv;
	}
      /* No else needed. If it's been initialized while we waited,
	 we can just attempt to lock it */
      break;
    case VALID_OBJECT:
      break;
    }
  return (*themutex)->lock ();
}

extern "C" int
pthread_mutex_trylock (pthread_mutex_t *mutex)
{
  pthread_mutex_t *themutex = mutex;
  if (pthread_mutex::is_good_initializer (mutex))
    pthread_mutex::init (mutex, NULL);
  if (!pthread_mutex::is_good_object (themutex))
    return EINVAL;
  return (*themutex)->trylock ();
}

extern "C" int
pthread_mutex_unlock (pthread_mutex_t *mutex)
{
  if (pthread_mutex::is_good_initializer (mutex))
    pthread_mutex::init (mutex, NULL);
  if (!pthread_mutex::is_good_object (mutex))
    return EINVAL;
  return (*mutex)->unlock ();
}

extern "C" int
pthread_mutex_destroy (pthread_mutex_t *mutex)
{
  int rv;

  if (pthread_mutex::is_good_initializer (mutex))
    return 0;
  if (!pthread_mutex::is_good_object (mutex))
    return EINVAL;

  rv = (*mutex)->destroy ();
  if (rv)
    return rv;

  *mutex = NULL;
  return 0;
}

extern "C" int
pthread_mutex_setprioceiling (pthread_mutex_t *mutex, int prioceiling,
				int *old_ceiling)
{
  pthread_mutex_t *themutex = mutex;
  if (pthread_mutex::is_good_initializer (mutex))
    pthread_mutex::init (mutex, NULL);
  if (!pthread_mutex::is_good_object (themutex))
    return EINVAL;
  return ENOSYS;
}

/* Win32 doesn't support mutex priorities - see __pthread_mutex_getprioceiling
   for more detail */
extern "C" int
pthread_mutexattr_getprotocol (const pthread_mutexattr_t *attr,
				 int *protocol)
{
  if (!pthread_mutexattr::is_good_object (attr))
    return EINVAL;
  return ENOSYS;
}

extern "C" int
pthread_mutexattr_getpshared (const pthread_mutexattr_t *attr,
				int *pshared)
{
  if (!pthread_mutexattr::is_good_object (attr))
    return EINVAL;
  *pshared = (*attr)->pshared;
  return 0;
}

extern "C" int
pthread_mutexattr_gettype (const pthread_mutexattr_t *attr, int *type)
{
  if (!pthread_mutexattr::is_good_object (attr))
    return EINVAL;
  *type = (*attr)->mutextype;
  return 0;
}

/* FIXME: write and test process shared mutex's.  */
extern "C" int
pthread_mutexattr_init (pthread_mutexattr_t *attr)
{
  if (pthread_mutexattr::is_good_object (attr))
    return EBUSY;

  *attr = new pthread_mutexattr ();
  if (!pthread_mutexattr::is_good_object (attr))
    {
      delete (*attr);
      *attr = NULL;
      return ENOMEM;
    }
  return 0;
}

extern "C" int
pthread_mutexattr_destroy (pthread_mutexattr_t *attr)
{
  if (!pthread_mutexattr::is_good_object (attr))
    return EINVAL;
  delete (*attr);
  *attr = NULL;
  return 0;
}


/* Win32 doesn't support mutex priorities */
extern "C" int
pthread_mutexattr_setprotocol (pthread_mutexattr_t *attr, int protocol)
{
  if (!pthread_mutexattr::is_good_object (attr))
    return EINVAL;
  return ENOSYS;
}

/* Win32 doesn't support mutex priorities */
extern "C" int
pthread_mutexattr_setprioceiling (pthread_mutexattr_t *attr,
				    int prioceiling)
{
  if (!pthread_mutexattr::is_good_object (attr))
    return EINVAL;
  return ENOSYS;
}

extern "C" int
pthread_mutexattr_getprioceiling (const pthread_mutexattr_t *attr,
				    int *prioceiling)
{
  if (!pthread_mutexattr::is_good_object (attr))
    return EINVAL;
  return ENOSYS;
}

extern "C" int
pthread_mutexattr_setpshared (pthread_mutexattr_t *attr, int pshared)
{
  if (!pthread_mutexattr::is_good_object (attr))
    return EINVAL;
  /* we don't use pshared for anything as yet. We need to test PROCESS_SHARED
   *functionality
   */
  if (pshared != PTHREAD_PROCESS_PRIVATE)
    return EINVAL;
  (*attr)->pshared = pshared;
  return 0;
}

/* see pthread_mutex_gettype */
extern "C" int
pthread_mutexattr_settype (pthread_mutexattr_t *attr, int type)
{
  if (!pthread_mutexattr::is_good_object (attr))
    return EINVAL;

  switch (type)
    {
    case PTHREAD_MUTEX_ERRORCHECK:
    case PTHREAD_MUTEX_RECURSIVE:
    case PTHREAD_MUTEX_NORMAL:
      (*attr)->mutextype = type;
      break;
    default:
      return EINVAL;
    }

  return 0;
}

/* Semaphores */

/* static members */
bool
semaphore::is_good_object (sem_t const * sem)
{
  if (verifyable_object_isvalid (sem, SEM_MAGIC) != VALID_OBJECT)
    return false;
  return true;
}

int
semaphore::init (sem_t *sem, int pshared, unsigned int value)
{
  /* opengroup calls this undefined */
  if (is_good_object (sem))
    return EBUSY;

  if (value > SEM_VALUE_MAX)
    return EINVAL;

  *sem = new semaphore (pshared, value);

  if (!is_good_object (sem))
    {
      delete (*sem);
      *sem = NULL;
      return EAGAIN;
    }
  return 0;
}

int
semaphore::destroy (sem_t *sem)
{
  if (!is_good_object (sem))
    return EINVAL;

  /* FIXME - new feature - test for busy against threads... */

  delete (*sem);
  *sem = NULL;
  return 0;
}

sem_t *
semaphore::open (const char *name, int oflag, mode_t mode, unsigned int value)
{
  if (value > SEM_VALUE_MAX)
    {
      set_errno (EINVAL);
      return NULL;
    }

  sem_t *sem = new sem_t;
  if (!sem)
    {
      set_errno (ENOMEM);
      return NULL;
    }

  *sem = new semaphore (name, oflag, mode, value);

  if (!is_good_object (sem))
    {
      delete *sem;
      delete sem;
      return NULL;
    }
  return sem;
}

int
semaphore::wait (sem_t *sem)
{
  pthread_testcancel ();

  if (!is_good_object (sem))
    {
      set_errno (EINVAL);
      return -1;
    }

  (*sem)->_wait ();
  return 0;
}

int
semaphore::trywait (sem_t *sem)
{
  if (!is_good_object (sem))
    {
      set_errno (EINVAL);
      return -1;
    }

  return (*sem)->_trywait ();
}

int
semaphore::timedwait (sem_t *sem, const struct timespec *abstime)
{
  if (!is_good_object (sem))
    {
      set_errno (EINVAL);
      return -1;
    }

  return (*sem)->_timedwait (abstime);
}

int
semaphore::post (sem_t *sem)
{
  if (!is_good_object (sem))
    {
      set_errno (EINVAL);
      return -1;
    }

  (*sem)->_post ();
  return 0;
}

int
semaphore::getvalue (sem_t *sem, int *sval)
{

  if (!is_good_object (sem)
      || __check_null_invalid_struct (sval, sizeof (int)))
    {
      set_errno (EINVAL);
      return -1;
    }

  return (*sem)->_getvalue (sval);
}

/* pthread_null */
pthread *
pthread_null::get_null_pthread ()
{
  /* because of weird entry points */
  _instance.magic = 0;
  return &_instance;
}

pthread_null::pthread_null ()
{
  attr.joinable = PTHREAD_CREATE_DETACHED;
  /* Mark ourselves as invalid */
  magic = 0;
}

pthread_null::~pthread_null ()
{
}

void
pthread_null::create (void *(*)(void *), pthread_attr *, void *)
{
}

void
pthread_null::exit (void *value_ptr)
{
  ExitThread (0);
}

int
pthread_null::cancel ()
{
  return 0;
}

void
pthread_null::testcancel ()
{
}

int
pthread_null::setcancelstate (int state, int *oldstate)
{
  return EINVAL;
}

int
pthread_null::setcanceltype (int type, int *oldtype)
{
  return EINVAL;
}

void
pthread_null::push_cleanup_handler (__pthread_cleanup_handler *handler)
{
}

void
pthread_null::pop_cleanup_handler (int const execute)
{
}

unsigned long
pthread_null::getsequence_np ()
{
  return 0;
}

pthread_null pthread_null::_instance;
