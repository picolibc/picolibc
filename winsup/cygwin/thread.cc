/* thread.cc: Locking and threading module functions

   Copyright 1998, 1999, 2000, 2001 Red Hat, Inc.

   Originally written by Marco Fuykschot <marco@ddi.nl>

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#ifdef _MT_SAFE
#include "winsup.h"
#include <limits.h>
#include <errno.h>
#include "cygerrno.h"
#include <assert.h>
#include <stdlib.h>
#include <syslog.h>
#include "sync.h"
#include "sigproc.h"
#include "pinfo.h"
#include "perprocess.h"
#include "security.h"
#include <semaphore.h>

extern int threadsafe;

#define MT_INTERFACE user_data->threadinterface

#define NOT_IMP(n)  system_printf ("not implemented %s\n", n); return 0;

struct _reent *
_reent_clib ()
{
  int tmp = GetLastError ();
  struct __reent_t *_r =
    (struct __reent_t *) TlsGetValue (MT_INTERFACE->reent_index);

#ifdef _CYG_THREAD_FAILSAFE
  if (_r == 0)
    system_printf ("local thread storage not inited");
#endif

  SetLastError (tmp);
  return _r->_clib;
}

struct _winsup_t *
_reent_winsup ()
{
  int tmp = GetLastError ();
  struct __reent_t *_r;
  _r = (struct __reent_t *) TlsGetValue (MT_INTERFACE->reent_index);
#ifdef _CYG_THREAD_FAILSAFE
  if (_r == 0)
    system_printf ("local thread storage not inited");
#endif
  SetLastError (tmp);
  return _r->_winsup;
}

inline LPCRITICAL_SECTION
ResourceLocks::Lock (int _resid)
{
#ifdef _CYG_THREAD_FAILSAFE
  if (!inited)
    system_printf ("lock called before initialization");

  thread_printf
    ("Get Resource lock %d ==> %p for %p , real : %d , threadid %d ", _resid,
     &lock, user_data, myself->pid, GetCurrentThreadId ());
#endif
  return &lock;
}

void
SetResourceLock (int _res_id, int _mode, const char *_function)
{
#ifdef _CYG_THREAD_FAILSAFE
  thread_printf ("Set resource lock %d mode %d for %s start",
		 _res_id, _mode, _function);
#endif
  EnterCriticalSection (user_data->resourcelocks->Lock (_res_id));

#ifdef _CYG_THREAD_FAILSAFE
  user_data->resourcelocks->owner = GetCurrentThreadId ();
  user_data->resourcelocks->count++;
#endif
}

void
ReleaseResourceLock (int _res_id, int _mode, const char *_function)
{
#ifdef _CYG_THREAD_FAILSAFE
  thread_printf ("Release resource lock %d mode %d for %s done", _res_id,
		 _mode, _function);

  AssertResourceOwner (_res_id, _mode);
  user_data->resourcelocks->count--;
  if (user_data->resourcelocks->count == 0)
    user_data->resourcelocks->owner = 0;
#endif

  LeaveCriticalSection (user_data->resourcelocks->Lock (_res_id));
}

#ifdef _CYG_THREAD_FAILSAFE
void
AssertResourceOwner (int _res_id, int _mode)
{

  thread_printf
    ("Assert Resource lock %d ==> for %p , real : %d , threadid %d count %d owner %d",
     _res_id, user_data, myself->pid, GetCurrentThreadId (),
     user_data->resourcelocks->count, user_data->resourcelocks->owner);
  if (user_data && (user_data->resourcelocks->owner != GetCurrentThreadId ()))
    system_printf ("assertion failed, not the resource owner");
}

#endif

void
ResourceLocks::Init ()
{
  InitializeCriticalSection (&lock);
  inited = true;

#ifdef _CYG_THREAD_FAILSAFE
  owner = 0;
  count = 0;
#endif

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
MTinterface::Init (int forked)
{
#if 0
  for (int i = 0; i < MT_MAX_ITEMS; i++)
    {
      threadlist.items[i] = NULL;
      mutexlist.items[i] = NULL;
      semalist.items[i] = NULL;
    }

  threadlist.index = 0;
  mutexlist.index = 0;
  semalist.index = 0;
#endif

  reent_index = TlsAlloc ();
  reents._clib = _impure_ptr;
  reents._winsup = &winsup_reent;

  winsup_reent._process_logmask = LOG_UPTO (LOG_DEBUG);
#if 0
  winsup_reent._grp_pos = 0;
  winsup_reent._process_ident = 0;
  winsup_reent._process_logopt = 0;
  winsup_reent._process_facility = 0;
#endif

  TlsSetValue (reent_index, &reents);
  // the static reent_data will be used in the main thread


  if (!indexallocated)
    {
      indexallocated = (-1);
      thread_self_dwTlsIndex = TlsAlloc ();
      if (thread_self_dwTlsIndex == TLS_OUT_OF_INDEXES)
	system_printf
	  ("local storage for thread couldn't be set\nThis means that we are not thread safe!\n");
    }

  if (forked)
    return;


  mainthread.win32_obj_id = myself->hProcess;
  mainthread.setThreadIdtoCurrent ();
  /* store the main thread's self pointer */
  TlsSetValue (thread_self_dwTlsIndex, &mainthread);
#if 0
  item->function = NULL;

  item->sigs = NULL;
  item->sigmask = NULL;
  item->sigtodo = NULL;
#endif
}

pthread::pthread ():verifyable_object (PTHREAD_MAGIC), win32_obj_id (0)
{
}

pthread::~pthread ()
{
  if (win32_obj_id)
    CloseHandle (win32_obj_id);
}


void
pthread::create (void *(*func) (void *), pthread_attr * newattr,
		 void *threadarg)
{
  /* already running ? */
  if (win32_obj_id)
    return;

  if (newattr)
    {
      attr.joinable = newattr->joinable;
      attr.stacksize = newattr->stacksize;
    }
  function = func;
  arg = threadarg;

  win32_obj_id =::CreateThread (&sec_none_nih, attr.stacksize,
				(LPTHREAD_START_ROUTINE) thread_init_wrapper,
				this, CREATE_SUSPENDED, &thread_id);

  if (!win32_obj_id)
    magic = 0;
  else
    ResumeThread (win32_obj_id);
}

pthread_attr::pthread_attr ():verifyable_object (PTHREAD_ATTR_MAGIC),
joinable (PTHREAD_CREATE_JOINABLE), stacksize (0)
{
}

pthread_attr::~pthread_attr ()
{
}

pthread_condattr::pthread_condattr ():verifyable_object 
(PTHREAD_CONDATTR_MAGIC), shared (PTHREAD_PROCESS_PRIVATE)
{
}

pthread_condattr::~pthread_condattr ()
{
}

pthread_cond::pthread_cond (pthread_condattr * attr):verifyable_object (PTHREAD_COND_MAGIC)
{
  this->shared = attr ? attr->shared : PTHREAD_PROCESS_PRIVATE;
  this->mutex = NULL;
  this->waiting = 0;

  this->win32_obj_id =::CreateEvent (&sec_none_nih, 
	false,	/* auto signal reset - which I think is pthreads like ? */
	false,	/* start non signaled */
	NULL /* no name */ );

  if (!this->win32_obj_id)
    magic = 0;
}

pthread_cond::~pthread_cond ()
{
  if (win32_obj_id)
    CloseHandle (win32_obj_id);
}

void
pthread_cond::BroadCast ()
{
  if (!verifyable_object_isvalid (mutex, PTHREAD_MUTEX_MAGIC))
    return;
  PulseEvent (win32_obj_id);
  while (InterlockedDecrement (&waiting) != 0)
    PulseEvent (win32_obj_id);
  mutex = NULL;
}

void
pthread_cond::Signal ()
{
  if (!verifyable_object_isvalid (mutex, PTHREAD_MUTEX_MAGIC))
    return;
  PulseEvent (win32_obj_id);
}

int
pthread_cond::TimedWait (DWORD dwMilliseconds)
{
  DWORD rv =
    SignalObjectAndWait (mutex->win32_obj_id, win32_obj_id, dwMilliseconds,
			 false);
  switch (rv)
    {
    case WAIT_FAILED:
      return 0;		/* POSIX doesn't allow errors after we modify the mutex state */
    case WAIT_ABANDONED:
      return ETIMEDOUT;
    case WAIT_OBJECT_0:
      return 0;		/* we have been signaled */
    default:
      return 0;
    }
}

pthread_key::pthread_key ():verifyable_object (PTHREAD_KEY_MAGIC)
{
  dwTlsIndex = TlsAlloc ();
  if (dwTlsIndex == TLS_OUT_OF_INDEXES)
    magic = 0;
}

pthread_key::~pthread_key ()
{
/* FIXME: New feature completeness.
 * bracketed code is to called when the thread exists, not when delete is called
 * if (destructor && TlsGetValue(dwTlsIndex))
 *   destructor (TlsGetValue(dwTlsIndex));
 */
  TlsFree (dwTlsIndex);
};

int
pthread_key::set (const void *value)
{
  /* the OS function doesn't perform error checking */
  TlsSetValue (dwTlsIndex, (void *) value);
  return 0;
}

void *
pthread_key::get ()
{
  set_errno (0);
  return TlsGetValue (dwTlsIndex);
}

pthread_mutex::pthread_mutex (pthread_mutexattr * attr):verifyable_object (PTHREAD_MUTEX_MAGIC)
{
  this->win32_obj_id =::CreateMutex (&sec_none_nih, false, NULL);
  if (!this->win32_obj_id)
    magic = 0;
  condwaits = 0;
};

pthread_mutex::~pthread_mutex ()
{
  if (win32_obj_id)
    CloseHandle (win32_obj_id);
}

int
pthread_mutex::Lock ()
{
  return WaitForSingleObject (win32_obj_id, INFINITE);
}

int
pthread_mutex::TryLock ()
{
  return WaitForSingleObject (win32_obj_id, 0);
}

int
pthread_mutex::UnLock ()
{
  return ReleaseMutex (win32_obj_id);
}

semaphore::semaphore (int pshared, unsigned int value):verifyable_object (SEM_MAGIC)
{
  this->win32_obj_id =::CreateSemaphore (&sec_none_nih, value, LONG_MAX,
					 NULL);
  if (!this->win32_obj_id)
    magic = 0;
  this->shared = pshared;
}

semaphore::~semaphore ()
{
  if (win32_obj_id)
    CloseHandle (win32_obj_id);
}

void
semaphore::Post ()
{
  long pc;
  ReleaseSemaphore (win32_obj_id, 1, &pc);
}

int
semaphore::TryWait ()
{
  /* FIXME: signals should be able to interrupt semaphores...
   * We probably need WaitForMultipleObjects here.
   */
  if (WaitForSingleObject (win32_obj_id, 0) == WAIT_TIMEOUT)
    return EAGAIN;
  else
    return 0;
}

void
semaphore::Wait ()
{
  WaitForSingleObject (win32_obj_id, INFINITE);
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
check_valid_pointer (void *pointer)
{
  MEMORY_BASIC_INFORMATION m;
  if (!pointer || !VirtualQuery (pointer, &m, sizeof (m))
      || (m.State != MEM_COMMIT))
    return EFAULT;
  return 0;
}

int
verifyable_object_isvalid (verifyable_object * object, long magic)
{
  if (!object)
    return 0;
  if (check_valid_pointer (object))
    return 0;
  if (object->magic != magic)
    return 0;
  return -1;
}

/*  Pthreads */
void *
thread_init_wrapper (void *_arg)
{
  // Setup the local/global storage of this thread

  pthread *thread = (pthread *) _arg;
  struct __reent_t local_reent;
  struct _winsup_t local_winsup;
  struct _reent local_clib;

  struct sigaction _sigs[NSIG];
  sigset_t _sig_mask;		/* one set for everything to ignore. */
  LONG _sigtodo[NSIG + __SIGOFFSET];

  // setup signal structures
  thread->sigs = _sigs;
  thread->sigmask = &_sig_mask;
  thread->sigtodo = _sigtodo;

  memset (&local_clib, 0, sizeof (struct _reent));
  memset (&local_winsup, 0, sizeof (struct _winsup_t));

  local_clib._errno = 0;
  local_clib._stdin = &local_clib.__sf[0];
  local_clib._stdout = &local_clib.__sf[1];
  local_clib._stderr = &local_clib.__sf[2];

  local_reent._clib = &local_clib;
  local_reent._winsup = &local_winsup;

  local_winsup._process_logmask = LOG_UPTO (LOG_DEBUG);

  /* This is not checked by the OS !! */
  if (!TlsSetValue (MT_INTERFACE->reent_index, &local_reent))
    system_printf ("local storage for thread couldn't be set");

  /* the OS doesn't check this for <=64 Tls entries (pre win2k) */
  TlsSetValue (MT_INTERFACE->thread_self_dwTlsIndex, thread);

#ifdef _CYG_THREAD_FAILSAFE
  if (_REENT == _impure_ptr)
    system_printf ("local storage for thread isn't setup correctly");
#endif

  thread_printf ("started thread %p %p %p %p %p %p", _arg, &local_clib,
		 _impure_ptr, thread, thread->function, thread->arg);

  // call the user's thread
  void *ret = thread->function (thread->arg);

  __pthread_exit (ret);

#if 0
// ??? This code only runs if the thread exits by returning.
// it's all now in __pthread_exit();
#endif
  /* never reached */
  return 0;
}

int
__pthread_create (pthread_t * thread, const pthread_attr_t * attr,
		  void *(*start_routine) (void *), void *arg)
{
  if (attr && !verifyable_object_isvalid (*attr, PTHREAD_ATTR_MAGIC))
    return EINVAL;

  *thread = new pthread ();
  (*thread)->create (start_routine, attr ? *attr : NULL, arg);
  if (!verifyable_object_isvalid (*thread, PTHREAD_MAGIC))
    {
      delete (*thread);
      *thread = NULL;
      return EAGAIN;
    }

  return 0;
}

int
__pthread_attr_init (pthread_attr_t * attr)
{
  *attr = new pthread_attr;
  if (!verifyable_object_isvalid (*attr, PTHREAD_ATTR_MAGIC))
    {
      delete (*attr);
      *attr = NULL;
      return EAGAIN;
    }
  return 0;
}

int
__pthread_attr_setdetachstate (pthread_attr_t * attr, int detachstate)
{
  if (!verifyable_object_isvalid (*attr, PTHREAD_ATTR_MAGIC))
    return EINVAL;
  if (detachstate < 0 || detachstate > 1)
    return EINVAL;
  (*attr)->joinable = detachstate;
  return 0;
}

int
__pthread_attr_getdetachstate (const pthread_attr_t * attr, int *detachstate)
{
  if (!verifyable_object_isvalid (*attr, PTHREAD_ATTR_MAGIC))
    return EINVAL;
  *detachstate = (*attr)->joinable;
  return 0;
}

int
__pthread_attr_setstacksize (pthread_attr_t * attr, size_t size)
{
  if (!verifyable_object_isvalid (*attr, PTHREAD_ATTR_MAGIC))
    return EINVAL;
  (*attr)->stacksize = size;
  return 0;
}

int
__pthread_attr_getstacksize (pthread_attr_t * attr, size_t * size)
{
  if (!verifyable_object_isvalid (*attr, PTHREAD_ATTR_MAGIC))
    return EINVAL;
  *size = (*attr)->stacksize;
  return 0;
}

int
__pthread_attr_destroy (pthread_attr_t * attr)
{
  if (!verifyable_object_isvalid (*attr, PTHREAD_ATTR_MAGIC))
    return EINVAL;
  delete (*attr);
  *attr = NULL;
  return 0;
}

void
__pthread_exit (void *value_ptr)
{
  class pthread *thread = __pthread_self ();

// FIXME: run the destructors of thread_key items here

  thread->return_ptr = value_ptr;
  ExitThread (0);
}

int
__pthread_join (pthread_t * thread, void **return_val)
{
  if (!verifyable_object_isvalid (*thread, PTHREAD_MAGIC))
    return ESRCH;

  if ((*thread)->joinable == PTHREAD_CREATE_DETACHED)
    {
      if (return_val)
	*return_val = NULL;
      return EINVAL;
    }
  else
    {
      (*thread)->joinable = PTHREAD_CREATE_DETACHED;
      WaitForSingleObject ((*thread)->win32_obj_id, INFINITE);
      if (return_val)
	*return_val = (*thread)->return_ptr;
    }				/* End if */

  return 0;
}

int
__pthread_detach (pthread_t * thread)
{
  if (!verifyable_object_isvalid (*thread, PTHREAD_MAGIC))
    return ESRCH;

  if ((*thread)->joinable == PTHREAD_CREATE_DETACHED)
    {
      (*thread)->return_ptr = NULL;
      return EINVAL;
    }

  (*thread)->joinable = PTHREAD_CREATE_DETACHED;
  return 0;
}

int
__pthread_suspend (pthread_t * thread)
{
  if (!verifyable_object_isvalid (*thread, PTHREAD_MAGIC))
    return ESRCH;

  if ((*thread)->suspended == false)
    {
      (*thread)->suspended = true;
      SuspendThread ((*thread)->win32_obj_id);
    }

  return 0;
}


int
__pthread_continue (pthread_t * thread)
{
  if (!verifyable_object_isvalid (*thread, PTHREAD_MAGIC))
    return ESRCH;

  if ((*thread)->suspended == true)
    ResumeThread ((*thread)->win32_obj_id);
  (*thread)->suspended = false;

  return 0;
}

unsigned long
__pthread_getsequence_np (pthread_t * thread)
{
  if (!verifyable_object_isvalid (*thread, PTHREAD_MAGIC))
    return EINVAL;
  return (*thread)->GetThreadId ();
}

/* Thread SpecificData */
int
__pthread_key_create (pthread_key_t * key, void (*destructor) (void *))
{
  /* The opengroup docs don't define if we should check this or not,
   * but creation is relatively rare.. 
   */
  if (verifyable_object_isvalid (*key, PTHREAD_KEY_MAGIC))
    return EBUSY;

  *key = new pthread_key ();

  if (!verifyable_object_isvalid (*key, PTHREAD_KEY_MAGIC))
    {
      delete (*key);
      *key = NULL;
      return EAGAIN;
    }
  return 0;
}

int
__pthread_key_delete (pthread_key_t * key)
{
  if (!verifyable_object_isvalid (*key, PTHREAD_KEY_MAGIC))
    return EINVAL;

  delete (*key);
  return 0;
}


int
__pthread_setspecific (pthread_key_t key, const void *value)
{
  if (!verifyable_object_isvalid (key, PTHREAD_KEY_MAGIC))
    return EINVAL;
  (key)->set (value);
  return 0;
}

void *
__pthread_getspecific (pthread_key_t key)
{
  if (!verifyable_object_isvalid (key, PTHREAD_KEY_MAGIC))
    return NULL;

  return (key)->get ();

}

/* Thread synchronisation */

int
__pthread_cond_destroy (pthread_cond_t * cond)
{
  if (!verifyable_object_isvalid (*cond, PTHREAD_COND_MAGIC))
    return EINVAL;

  /* reads are atomic */
  if ((*cond)->waiting)
    return EBUSY;

  delete (*cond);
  *cond = NULL;

  return 0;
}

int
__pthread_cond_init (pthread_cond_t * cond, const pthread_condattr_t * attr)
{
  if (attr && !verifyable_object_isvalid (*attr, PTHREAD_CONDATTR_MAGIC))
    return EINVAL;

  if (verifyable_object_isvalid (*cond, PTHREAD_COND_MAGIC))
    return EBUSY;

  *cond = new pthread_cond (attr ? (*attr) : NULL);

  if (!verifyable_object_isvalid (*cond, PTHREAD_COND_MAGIC))
    {
      delete (*cond);
      *cond = NULL;
      return EAGAIN;
    }

  return 0;
}

int
__pthread_cond_broadcast (pthread_cond_t * cond)
{
  if (!verifyable_object_isvalid (*cond, PTHREAD_COND_MAGIC))
    return EINVAL;

  (*cond)->BroadCast ();

  return 0;
}

int
__pthread_cond_signal (pthread_cond_t * cond)
{
  if (!verifyable_object_isvalid (*cond, PTHREAD_COND_MAGIC))
    return EINVAL;

  (*cond)->Signal ();

  return 0;
}

int
__pthread_cond_timedwait (pthread_cond_t * cond, pthread_mutex_t * mutex,
			  const struct timespec *abstime)
{
  int rv;
  if (!abstime)
    return EINVAL;
  if (!verifyable_object_isvalid (*mutex, PTHREAD_MUTEX_MAGIC))
    return EINVAL;
  if (!verifyable_object_isvalid (*cond, PTHREAD_COND_MAGIC))
    return EINVAL;

  if ((*cond)->waiting)
    if ((*cond)->mutex && ((*cond)->mutex != (*mutex)))
      return EINVAL;
  InterlockedIncrement (&((*cond)->waiting));

  (*cond)->mutex = (*mutex);
  InterlockedIncrement (&((*mutex)->condwaits));
  rv = (*cond)->TimedWait (abstime->tv_sec * 1000);
  (*cond)->mutex->Lock ();
  if (InterlockedDecrement (&((*cond)->waiting)) == 0)
    (*cond)->mutex = NULL;
  InterlockedDecrement (&((*mutex)->condwaits));

  return rv;
}

int
__pthread_cond_wait (pthread_cond_t * cond, pthread_mutex_t * mutex)
{
  int rv;
  if (!verifyable_object_isvalid (*mutex, PTHREAD_MUTEX_MAGIC))
    return EINVAL;
  if (!verifyable_object_isvalid (*cond, PTHREAD_COND_MAGIC))
    return EINVAL;

  if ((*cond)->waiting)
    if ((*cond)->mutex && ((*cond)->mutex != (*mutex)))
      return EINVAL;
  InterlockedIncrement (&((*cond)->waiting));

  (*cond)->mutex = (*mutex);
  InterlockedIncrement (&((*mutex)->condwaits));
  rv = (*cond)->TimedWait (INFINITE);
  (*cond)->mutex->Lock ();
  if (InterlockedDecrement (&((*cond)->waiting)) == 0)
    (*cond)->mutex = NULL;
  InterlockedDecrement (&((*mutex)->condwaits));

  return rv;
}

int
__pthread_condattr_init (pthread_condattr_t * condattr)
{
  *condattr = new pthread_condattr;
  if (!verifyable_object_isvalid (*condattr, PTHREAD_CONDATTR_MAGIC))
    {
      delete (*condattr);
      *condattr = NULL;
      return EAGAIN;
    }
  return 0;
}

int
__pthread_condattr_getpshared (const pthread_condattr_t * attr, int *pshared)
{
  if (!verifyable_object_isvalid (*attr, PTHREAD_CONDATTR_MAGIC))
    return EINVAL;
  *pshared = (*attr)->shared;
  return 0;
}

int
__pthread_condattr_setpshared (pthread_condattr_t * attr, int pshared)
{
  if (!verifyable_object_isvalid (*attr, PTHREAD_CONDATTR_MAGIC))
    return EINVAL;
  if ((pshared < 0) || (pshared > 1))
    return EINVAL;
  (*attr)->shared = pshared;
  return 0;
}

int
__pthread_condattr_destroy (pthread_condattr_t * condattr)
{
  if (!verifyable_object_isvalid (*condattr, PTHREAD_CONDATTR_MAGIC))
    return EINVAL;
  delete (*condattr);
  *condattr = NULL;
  return 0;
}

/* Thread signal */
int
__pthread_kill (pthread_t * thread, int sig)
{
// lock myself, for the use of thread2signal
  // two differ kills might clash: FIXME

  if (!verifyable_object_isvalid (*thread, PTHREAD_MAGIC))
    return EINVAL;

  if ((*thread)->sigs)
    myself->setthread2signal (*thread);

  int rval = _kill (myself->pid, sig);

  // unlock myself
  return rval;
}

int
__pthread_sigmask (int operation, const sigset_t * set, sigset_t * old_set)
{
  pthread *thread = __pthread_self ();

  // lock this myself, for the use of thread2signal
  // two differt kills might clash: FIXME

  if (thread->sigs)
    myself->setthread2signal (thread);

  int rval = sigprocmask (operation, set, old_set);

  // unlock this myself

  return rval;
}

/*  ID */
pthread_t __pthread_self ()
{
  return (pthread *) TlsGetValue (MT_INTERFACE->thread_self_dwTlsIndex);
}

int
__pthread_equal (pthread_t * t1, pthread_t * t2)
{
  return (*t1 - *t2);
}

/* Mutexes  */

int
__pthread_mutex_init (pthread_mutex_t * mutex,
		      const pthread_mutexattr_t * attr)
{
  if (attr && !verifyable_object_isvalid (*attr, PTHREAD_MUTEXATTR_MAGIC))
    return EINVAL;

  if (verifyable_object_isvalid (*mutex, PTHREAD_MUTEX_MAGIC))
    return EBUSY;

  *mutex = new pthread_mutex (attr ? (*attr) : NULL);
  if (!verifyable_object_isvalid (*mutex, PTHREAD_MUTEX_MAGIC))
    {
      delete (*mutex);
      *mutex = NULL;
      return EAGAIN;
    }
  return 0;
}

int
__pthread_mutex_lock (pthread_mutex_t * mutex)
{
  if (!verifyable_object_isvalid (*mutex, PTHREAD_MUTEX_MAGIC))
    return EINVAL;
  (*mutex)->Lock ();
  return 0;
}

int
__pthread_mutex_trylock (pthread_mutex_t * mutex)
{
  if (!verifyable_object_isvalid (*mutex, PTHREAD_MUTEX_MAGIC))
    return EINVAL;
  if ((*mutex)->TryLock () == WAIT_TIMEOUT)
    return EBUSY;
  return 0;
}

int
__pthread_mutex_unlock (pthread_mutex_t * mutex)
{
  if (!verifyable_object_isvalid (*mutex, PTHREAD_MUTEX_MAGIC))
    return EINVAL;
  (*mutex)->UnLock ();
  return 0;
}

int
__pthread_mutex_destroy (pthread_mutex_t * mutex)
{
  if (!verifyable_object_isvalid (*mutex, PTHREAD_MUTEX_MAGIC))
    return EINVAL;

  /* reading a word is atomic */
  if ((*mutex)->condwaits)
    return EBUSY;

  delete (*mutex);
  *mutex = NULL;
  return 0;
}

/* Semaphores */
int
__sem_init (sem_t * sem, int pshared, unsigned int value)
{
  /* opengroup calls this undefined */
  if (verifyable_object_isvalid (*sem, SEM_MAGIC))
    return EBUSY;

  if (value > SEM_VALUE_MAX)
    return EINVAL;

  *sem = new semaphore (pshared, value);

  if (!verifyable_object_isvalid (*sem, SEM_MAGIC))
    {
      delete (*sem);
      *sem = NULL;
      return EAGAIN;
    }
  return 0;
}

int
__sem_destroy (sem_t * sem)
{
  if (!verifyable_object_isvalid (*sem, SEM_MAGIC))
    return EINVAL;

  /* FIXME - new feature - test for busy against threads... */

  delete (*sem);
  *sem = NULL;
  return 0;
}

int
__sem_wait (sem_t * sem)
{
  if (!verifyable_object_isvalid (*sem, SEM_MAGIC))
    return EINVAL;

  (*sem)->Wait ();
  return 0;
}

int
__sem_trywait (sem_t * sem)
{
  if (!verifyable_object_isvalid (*sem, SEM_MAGIC))
    return EINVAL;

  return (*sem)->TryWait ();
}

int
__sem_post (sem_t * sem)
{
  if (!verifyable_object_isvalid (*sem, SEM_MAGIC))
    return EINVAL;

  (*sem)->Post ();
  return 0;
}


#else

// empty functions needed when makeing the dll without mt_safe support
extern "C"
{
  int __pthread_create (pthread_t *, const pthread_attr_t *,
			TFD (start_routine), void *arg)
  {
    return -1;
  }
  int __pthread_attr_init (pthread_attr_t * attr)
  {
    return -1;
  }
  int __pthread_attr_destroy (pthread_attr_t * attr)
  {
    return -1;
  }
  int __pthread_attr_setdetachstate (pthread_attr_t * attr, int detachstate)
  {
    return -1;
  }
  int
    __pthread_attr_getdetachstate (const pthread_attr_t * attr,
				   int *detachstate)
  {
    return -1;
  }
  int __pthread_attr_setstacksize (pthread_attr_t * attr, size_t size)
  {
    return -1;
  }
  int __pthread_attr_getstacksize (pthread_attr_t * attr, size_t * size)
  {
    return -1;
  }
/* these cannot be supported on win32 - the os allocates it's own stack space..
   __pthread_attr_setstackaddr (...){ return -1; };
   __pthread_attr_getstackaddr (...){ return -1; };
 */
  int __pthread_exit (void *value_ptr)
  {
    return -1;
  }

  int __pthread_join (pthread_t thread_id, void **return_val)
  {
    return -1;
  }

  unsigned long __pthread_getsequence_np (pthread_t * thread)
  {
    return 0;
  }
  int __pthread_key_create (pthread_key_t * key)
  {
    return -1;
  }
  int __pthread_key_delete (pthread_key_t * key)
  {
    return -1;
  }
  int __pthread_setspecific (pthread_key_t * key, const void *value)
  {
    return -1;
  }
  void *__pthread_getspecific (pthread_key_t * key)
  {
    return NULL;
  }
  int __pthread_kill (pthread_t * thread, int sig)
  {
    return -1;
  }
  int __pthread_sigmask (int operation, const sigset_t * set,
			 sigset_t * old_set)
  {
    return -1;
  }
  pthread_t __pthread_self ()
  {
    return -1;
  }
  int __pthread_equal (pthread_t * t1, pthread_t * t2)
  {
    return -1;
  }
  int __pthread_mutex_init (pthread_mutex_t *, const pthread_mutexattr_t *)
  {
    return -1;
  }
  int __pthread_mutex_lock (pthread_mutex_t *)
  {
    return -1;
  }
  int __pthread_mutex_trylock (pthread_mutex_t *)
  {
    return -1;
  }
  int __pthread_mutex_unlock (pthread_mutex_t *)
  {
    return -1;
  }
  int __pthread_mutex_destroy (pthread_mutex_t *)
  {
    return -1;
  }
  int __pthread_cond_destroy (pthread_cond_t *)
  {
    return -1;
  }
  int __pthread_cond_init (pthread_cond_t *, const pthread_condattr_t *)
  {
    return -1;
  }
  int __pthread_cond_signal (pthread_cond_t *)
  {
    return -1;
  }
  int __pthread_cond_broadcast (pthread_cond_t *)
  {
    return -1;
  }
  int __pthread_cond_timedwait (pthread_cond_t *, pthread_mutex_t *,
				const struct timespec *)
  {
    return -1;
  }
  int __pthread_cond_wait (pthread_cond_t *, pthread_mutex_t *)
  {
    return -1;
  }
  int __pthread_condattr_init (pthread_condattr_t *)
  {
    return -1;
  }
  int __pthread_condattr_destroy (pthread_condattr_t *)
  {
    return -1;
  }
  int __pthread_condattr_getpshared (pthread_condattr_t *, int *)
  {
    return -1;
  }
  int __pthread_condattr_setpshared (pthread_condattr_t *, int)
  {
    return -1;
  }
  int __sem_init (sem_t * sem, int pshared, unsigned int value)
  {
    return -1;
  }
  int __sem_destroy (sem_t * sem)
  {
    return -1;
  }
  int __sem_wait (sem_t * sem)
  {
    return -1;
  }
  int __sem_trywait (sem_t * sem)
  {
    return -1;
  }
  int __sem_post (sem_t * sem)
  {
    return -1;
  }
  struct _reent *_reent_clib ()
  {
    return NULL;
  }
}

#endif				// MT_SAFE
