/* thread.cc: Locking and threading module functions

   Copyright 1998, 1999, 2000, 2001 Red Hat, Inc.

   Originally written by Marco Fuykschot <marco@ddi.nl>

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

/* Implementation overview and caveats:

  Win32 puts some contraints on what can and cannot be implemented. Where possible
  we work around those contrainsts. Where we cannot work around the constraints we
  either pretend to be conformant, or return an error code.

  Some caveats: PROCESS_SHARED objects while they pretend to be process shared,
  may not actually work. Some test cases are needed to determine win32's behaviour.
  My suspicion is that the win32 handle needs to be opened with different flags for
  proper operation.

  R.Collins, April 2001.

  */

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
#include <stdio.h>

extern int threadsafe;

/* pthread_key_destructor_list class: to-be threadsafe single linked list
 * FIXME: Put me in a dedicated file, or a least a tools area !
 */

pthread_key_destructor *
pthread_key_destructor::InsertAfter (pthread_key_destructor * node)
{
  pthread_key_destructor *temp = next;
  next = node;
  return temp;
}

pthread_key_destructor *
pthread_key_destructor::UnlinkNext ()
{
  pthread_key_destructor *temp = next;
  if (next)
    next = next->Next ();
  return temp;
}

pthread_key_destructor *
pthread_key_destructor::Next ()
{
  return next;
}

void
pthread_key_destructor_list::Insert (pthread_key_destructor * node)
{
  if (!node)
    return;
  head = node->InsertAfter (head);
  if (!head)
    head = node;		/* first node special case */
}

  /* remove a given dataitem, wherever in the list it is */
pthread_key_destructor *
pthread_key_destructor_list::Remove (pthread_key * key)
{
  if (!key)
    return NULL;
  if (!head)
    return NULL;
  if (key == head->key)
    return Pop ();
  pthread_key_destructor *temp = head;
  while (temp && temp->Next () && !(key == temp->Next ()->key))
    {
      temp = temp->Next ();
    }
  if (temp)
    return temp->UnlinkNext ();
  return NULL;
}

  /* get the first item and remove at the same time */
pthread_key_destructor *
pthread_key_destructor_list::Pop ()
{
  pthread_key_destructor *temp = head;
  head = head->Next ();
  return temp;
}

pthread_key_destructor::
pthread_key_destructor (void (*thedestructor) (void *), pthread_key * key)
{
  destructor = thedestructor;
  next = NULL;
  this->key = key;
}

void
pthread_key_destructor_list::IterateNull ()
{
  pthread_key_destructor *temp = head;
  while (temp)
    {
      temp->destructor ((temp->key)->get ());
      temp = temp->Next ();
    }
}


#define MT_INTERFACE user_data->threadinterface

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

  concurrency = 0;
  threadcount = 1; /* 1 current thread when Init occurs.*/

  mainthread.win32_obj_id = myself->hProcess;
  mainthread.setThreadIdtoCurrent ();
  /* store the main thread's self pointer */
  TlsSetValue (thread_self_dwTlsIndex, &mainthread);

  if (forked)
    return;

  /* possible the atfork lists should be inited here as well */

  for (int i = 0; i < 256; i++)
    pshared_mutexs[i] = NULL;

#if 0
  item->function = NULL;

  item->sigs = NULL;
  item->sigmask = NULL;
  item->sigtodo = NULL;
#endif
}

pthread::pthread ():verifyable_object (PTHREAD_MAGIC), win32_obj_id (0),
cancelstate (0), canceltype (0)
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
      attr.contentionscope = newattr->contentionscope;
      attr.inheritsched = newattr->inheritsched;
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
    {
      /* FIXME: set the priority appropriately for system contention scope */
      if (attr.inheritsched == PTHREAD_EXPLICIT_SCHED)
	{
	  /* FIXME: set the scheduling settings for the new thread */
	  /* sched_thread_setparam (win32_obj_id, attr.schedparam); */
	}
      ResumeThread (win32_obj_id);
    }
}

pthread_attr::pthread_attr ():verifyable_object (PTHREAD_ATTR_MAGIC),
joinable (PTHREAD_CREATE_JOINABLE), contentionscope (PTHREAD_SCOPE_PROCESS),
inheritsched (PTHREAD_INHERIT_SCHED), stacksize (0)
{
  schedparam.sched_priority = 0;
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

  this->win32_obj_id =::CreateEvent (&sec_none_nih, false,	/* auto signal reset - which I think is pthreads like ? */
				     false,	/* start non signaled */
				     NULL /* no name */);

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
  // This potentially has an unfairness bug. We should
  // consider preventing the wakeups from resuming until we finish signalling.
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
  DWORD rv;
  if (os_being_run != winNT)
    {
      // FIXME: race condition (potentially drop events
      // Possible solution (single process only) - place this in a critical section.
      ReleaseMutex (mutex->win32_obj_id);
      rv = WaitForSingleObject (win32_obj_id, dwMilliseconds);
    }
  else
    rv = SignalObjectAndWait (mutex->win32_obj_id, win32_obj_id, dwMilliseconds,
			 false);
  switch (rv)
    {
    case WAIT_FAILED:
      return 0;			/* POSIX doesn't allow errors after we modify the mutex state */
    case WAIT_ABANDONED:
      return ETIMEDOUT;
    case WAIT_OBJECT_0:
      return 0;			/* we have been signaled */
    default:
      return 0;
    }
}

pthread_key::pthread_key (void (*destructor) (void *)):verifyable_object (PTHREAD_KEY_MAGIC)
{
  dwTlsIndex = TlsAlloc ();
  if (dwTlsIndex == TLS_OUT_OF_INDEXES)
    magic = 0;
  else if (destructor)
    {
      MT_INTERFACE->destructors.
	Insert (new pthread_key_destructor (destructor, this));
    }
}

pthread_key::~pthread_key ()
{
  if (pthread_key_destructor * dest = MT_INTERFACE->destructors.Remove (this))
    delete dest;
  TlsFree (dwTlsIndex);
}

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

#define SYS_BASE (unsigned char) 0xC0
// Note: the order is important. This is an overloaded pthread_mutex_t from
// userland
typedef struct _pshared_mutex {
 unsigned char id;
 unsigned char reserved;
 unsigned char reserved2;
 unsigned char flags;
} pshared_mutex;

/* pshared mutexs:
 * the mutex_t (size 4) is not used as a verifyable object because we cannot
 * guarantee the same address space for all processes.
 * we use the following:
 * high bit set (never a valid address).
 * second byte is reserved for the priority.
 * third byte is reserved
 * fourth byte is the mutex id. (max 255 cygwin mutexs system wide).
 * creating mutex's does get slower and slower, but as creation is a one time
 * job, it should never become an issue
 *
 * And if you're looking at this and thinking, why not an array in cygwin for all mutexs,
 * - you incur a penalty on _every_ mutex call and you have toserialise them all.
 * ... Bad karma.
 *
 * option 2? put everything in userspace and update the ABI?
 * - bad karma as well - the HANDLE, while identical across process's,
 * Isn't duplicated, it's reopened.
 */

pthread_mutex::pthread_mutex (unsigned short id):verifyable_object (PTHREAD_MUTEX_MAGIC)
{
  //FIXME: set an appropriate security mask - probably everyone.
  if (MT_INTERFACE->pshared_mutexs[id])
    return;
  char stringbuf[29];
  snprintf (stringbuf, 29, "CYGWINMUTEX0x%0x", id & 0x000f);
  system_printf ("name of mutex to transparently open %s\n",stringbuf);
  this->win32_obj_id =::CreateMutex (&sec_none_nih, false, stringbuf);
  if (win32_obj_id==0 || (win32_obj_id && GetLastError () != ERROR_ALREADY_EXISTS))
    {
      // the mutex has been deleted or we couldn't get access.
	// the error_already_exists test is because we are only opening an
	// existint mutex here
      system_printf ("couldn't get pshared mutex %x, %d\n",win32_obj_id, GetLastError ());
      CloseHandle (win32_obj_id);
      magic = 0;
      win32_obj_id = NULL;
      return;
    }
  pshared = PTHREAD_PROCESS_SHARED;

  MT_INTERFACE->pshared_mutexs[id] = this;
}

pthread_mutex::pthread_mutex (pthread_mutex_t *mutex, pthread_mutexattr * attr):verifyable_object (PTHREAD_MUTEX_MAGIC)
{
  /* attr checked in the C call */
  if (attr && attr->pshared==PTHREAD_PROCESS_SHARED)
    {
      //FIXME: set an appropriate security mask - probably everyone.
      // This does open a D.O.S. - the name is guessable (if you are willing to run
      // thru all possible address values :]
      char stringbuf[29];
      unsigned short id = 1;
      while (id < 256)
        {
          snprintf (stringbuf, 29, "CYGWINMUTEX0x%0x", id & 0x000f);
          system_printf ("name of mutex to create %s\n",stringbuf);
          this->win32_obj_id =::CreateMutex (&sec_none_nih, false, stringbuf);
          if (this->win32_obj_id && GetLastError () != ERROR_ALREADY_EXISTS)
            {
              MT_INTERFACE->pshared_mutexs[id] = this;
	      pshared_mutex *pmutex=(pshared_mutex *)(mutex);
	      pmutex->id = id;
              pmutex->flags = SYS_BASE;
	      pshared = PTHREAD_PROCESS_SHARED;
	      condwaits = 0;
	      return;
	    }
	  id++;
	  CloseHandle (win32_obj_id);
        }
      magic = 0;
      win32_obj_id = NULL;
    }
  else
    {
      this->win32_obj_id =::CreateMutex (&sec_none_nih, false, NULL);

      if (!win32_obj_id)
        magic = 0;
      condwaits = 0;
      pshared = PTHREAD_PROCESS_PRIVATE;
    }
}

pthread_mutex::pthread_mutex (pthread_mutexattr * attr):verifyable_object (PTHREAD_MUTEX_MAGIC)
{
  /* attr checked in the C call */
  if (attr && attr->pshared==PTHREAD_PROCESS_SHARED)
    {
      /* for pshared mutex's we need the mutex address */
      magic = 0;
      return;
    }

  this->win32_obj_id =::CreateMutex (&sec_none_nih, false, NULL);

  if (!win32_obj_id)
    magic = 0;
  condwaits = 0;
  pshared = PTHREAD_PROCESS_PRIVATE;
}

pthread_mutex::~pthread_mutex ()
{
  if (win32_obj_id)
    CloseHandle (win32_obj_id);
  win32_obj_id = NULL;
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

pthread_mutex **
__pthread_mutex_getpshared (pthread_mutex_t *mutex)
{
  if ((((pshared_mutex *)(mutex))->flags & SYS_BASE) != SYS_BASE)
    return (pthread_mutex **) mutex;
  pshared_mutex *pmutex=(pshared_mutex *)(mutex);
  if ((MT_INTERFACE->pshared_mutexs[pmutex->id]) != NULL)
    return &(MT_INTERFACE->pshared_mutexs[pmutex->id]);
  /* attempt to get the existing mutex */
  pthread_mutex * newmutex;
  newmutex = new pthread_mutex (pmutex->id);
  if (!verifyable_object_isvalid (newmutex, PTHREAD_MUTEX_MAGIC))
  {
    delete (newmutex);
    MT_INTERFACE->pshared_mutexs[pmutex->id] = NULL;
    return &(MT_INTERFACE->pshared_mutexs[0]);
  }
  return &(MT_INTERFACE->pshared_mutexs[pmutex->id]);
}

pthread_mutexattr::pthread_mutexattr ():verifyable_object (PTHREAD_MUTEXATTR_MAGIC),
pshared (PTHREAD_PROCESS_PRIVATE), mutextype (PTHREAD_MUTEX_DEFAULT)
{
}

pthread_mutexattr::~pthread_mutexattr ()
{
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
// it's all now in __pthread_exit ();
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
  InterlockedIncrement (&MT_INTERFACE->threadcount);

  return 0;
}

int
__pthread_once (pthread_once_t * once_control, void (*init_routine) (void))
{
  pthread_mutex_lock (&once_control->mutex);
  /* Here we must set a cancellation handler to unlock the mutex if needed */
  /* but a cancellation handler is not the right thing. We need this in the thread
   * cleanup routine. Assumption: a thread can only be in one pthread_once routine
   * at a time. Stote a mutex_t * in the pthread_structure. if that's non null unlock
   * on pthread_exit ();
   */
  if (once_control->state == 0)
    {
      init_routine ();
      once_control->state = 1;
    }
  /* Here we must remove our cancellation handler */
  pthread_mutex_unlock (&once_control->mutex);
  return 0;
}

/* Cancelability states */


/* Perform the actual cancel */
void
__pthread_cleanup (pthread_t thread)
{
}


int
__pthread_cancel (pthread_t thread)
{
  if (!verifyable_object_isvalid (thread, PTHREAD_MAGIC))
    return ESRCH;
  if (thread->cancelstate == PTHREAD_CANCEL_ENABLE)
    {
#if 0
      /* once all the functions call testcancel (), we will do this */
      if (thread->canceltype == PTHREAD_CANCEL_DEFERRED)
	{
	}
      else
	{
	  /* possible FIXME: this function is meant to return asynchronously
	   * from the cancellation routine actually firing. So we may need some sort
	   * of signal to be sent that is immediately recieved and acted on.
	   */
	  __pthread_cleanup (thread);
	}
#endif
    }
/*  return 0;
*/

  return ESRCH;
/*
  we return ESRCH until all the required functions call testcancel ();
  this will give applications predictable behaviour.

  the required function list is: * indicates done, X indicates not present in cygwin.
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
pause ()
poll ()
pread ()
pthread_cond_timedwait ()
pthread_cond_wait ()
*pthread_join ()
pthread_testcancel ()
putmsg ()
putpmsg ()
pwrite ()
read ()
readv ()
select ()
sem_wait ()
sigpause ()
sigsuspend ()
sigtimedwait ()
sigwait ()
sigwaitinfo ()
*sleep ()
system ()
tcdrain ()
*usleep ()
wait ()
wait3()
waitid ()
waitpid ()
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
* getpwnam ()
* getpwnam_r ()
* getpwuid ()
* getpwuid_r ()
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

/* no races in these three functions: they are all current-thread-only */
int
__pthread_setcancelstate (int state, int *oldstate)
{
  class pthread *thread = __pthread_self ();
  if (state != PTHREAD_CANCEL_ENABLE && state != PTHREAD_CANCEL_DISABLE)
    return EINVAL;
  *oldstate = thread->cancelstate;
  thread->cancelstate = state;
  return 0;
}

int
__pthread_setcanceltype (int type, int *oldtype)
{
  class pthread *thread = __pthread_self ();
  if (type != PTHREAD_CANCEL_DEFERRED && type != PTHREAD_CANCEL_ASYNCHRONOUS)
    return EINVAL;
  *oldtype = thread->canceltype;
  thread->canceltype = type;
  return 0;
}

/* deferred cancellation request handler */
void
__pthread_testcancel (void)
{
  class pthread *thread = __pthread_self ();
  if (thread->cancelstate == PTHREAD_CANCEL_DISABLE)
    return;
  /* check the cancellation event object here - not neededuntil pthread_cancel actually
   * does something*/
}

/*
 * Races in pthread_atfork:
 * We are race safe in that any additions to the lists are made via
 * InterlockedExchangePointer.
 * However, if the user application doesn't perform syncronisation of some sort
 * It's not guaranteed that a near simultaneous call to pthread_atfork and fork
 * will result in the new atfork handlers being calls.
 * More rigorous internal syncronisation isn't needed as the user program isn't
 * guaranteeing their own state.
 *
 * as far as multiple calls to pthread_atfork, the worst case is simultaneous calls
 * will result in an indeterminate order for parent and child calls (what gets inserted
 * first isn't guaranteed.)
 *
 * There is one potential race... Does the result of InterlockedExchangePointer
 * get committed to the return location _before_ any context switches can occur?
 * If yes, we're safe, if no, we're not.
 */
void
__pthread_atforkprepare (void)
{
  callback *cb = MT_INTERFACE->pthread_prepare;
  while (cb)
    {
      cb->cb ();
      cb = cb->next;
    }
}

void
__pthread_atforkparent (void)
{
  callback *cb = MT_INTERFACE->pthread_parent;
  while (cb)
    {
      cb->cb ();
      cb = cb->next;
    }
}

void
__pthread_atforkchild (void)
{
  callback *cb = MT_INTERFACE->pthread_child;
  while (cb)
    {
      cb->cb ();
      cb = cb->next;
    }
}

/* FIXME: implement InterlockExchangePointer and get rid of the silly typecasts below
 */
#define InterlockedExchangePointer InterlockedExchange

/* Register a set of functions to run before and after fork.
 * prepare calls are called in LI-FC order.
 * parent and child calls are called in FI-FC order.
 */
int
__pthread_atfork (void (*prepare)(void), void (*parent)(void), void (*child)(void))
{
  callback * prepcb = NULL, * parentcb = NULL, * childcb = NULL;
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
    prepcb->next=(callback *)InterlockedExchangePointer ((LONG *) &MT_INTERFACE->pthread_prepare, (long int) prepcb);
  }
  if (parentcb)
  {
    parentcb->cb = parent;
    callback ** t = &MT_INTERFACE->pthread_parent;
    while (*t)
      t = &(*t)->next;
    /* t = pointer to last next in the list */
    parentcb->next=(callback *)InterlockedExchangePointer ((LONG *) t, (long int) parentcb);
  }
  if (childcb)
  {
    childcb->cb = child;
    callback ** t = &MT_INTERFACE->pthread_child;
    while (*t)
      t = &(*t)->next;
    /* t = pointer to last next in the list */
    childcb->next=(callback *)InterlockedExchangePointer ((LONG *) t, (long int) childcb);
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
__pthread_attr_getinheritsched (const pthread_attr_t * attr,
				int *inheritsched)
{
  if (!verifyable_object_isvalid (*attr, PTHREAD_ATTR_MAGIC))
    return EINVAL;
  *inheritsched = (*attr)->inheritsched;
  return 0;
}

int
__pthread_attr_getschedparam (const pthread_attr_t * attr,
			      struct sched_param *param)
{
  if (!verifyable_object_isvalid (*attr, PTHREAD_ATTR_MAGIC))
    return EINVAL;
  *param = (*attr)->schedparam;
  return 0;
}

/* From a pure code point of view, this should call a helper in sched.cc,
 * to allow for someone adding scheduler policy changes to win32 in the future.
 * However that's extremely unlikely, so short and sweet will do us
 */
int
__pthread_attr_getschedpolicy (const pthread_attr_t * attr, int *policy)
{
  if (!verifyable_object_isvalid (*attr, PTHREAD_ATTR_MAGIC))
    return EINVAL;
  *policy = SCHED_FIFO;
  return 0;
}


int
__pthread_attr_getscope (const pthread_attr_t * attr, int *contentionscope)
{
  if (!verifyable_object_isvalid (*attr, PTHREAD_ATTR_MAGIC))
    return EINVAL;
  *contentionscope = (*attr)->contentionscope;
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
__pthread_attr_setinheritsched (pthread_attr_t * attr, int inheritsched)
{
  if (!verifyable_object_isvalid (*attr, PTHREAD_ATTR_MAGIC))
    return EINVAL;
  if (inheritsched != PTHREAD_INHERIT_SCHED
      && inheritsched != PTHREAD_EXPLICIT_SCHED)
    return ENOTSUP;
  (*attr)->inheritsched = inheritsched;
  return 0;
}

int
__pthread_attr_setschedparam (pthread_attr_t * attr,
			      const struct sched_param *param)
{
  if (!verifyable_object_isvalid (*attr, PTHREAD_ATTR_MAGIC))
    return EINVAL;
  if (!valid_sched_parameters (param))
    return ENOTSUP;
  (*attr)->schedparam = *param;
  return 0;
}

/* See __pthread_attr_getschedpolicy for some notes */
int
__pthread_attr_setschedpolicy (pthread_attr_t * attr, int policy)
{
  if (!verifyable_object_isvalid (*attr, PTHREAD_ATTR_MAGIC))
    return EINVAL;
  if (policy != SCHED_FIFO)
    return ENOTSUP;
  return 0;
}

int
__pthread_attr_setscope (pthread_attr_t * attr, int contentionscope)
{
  if (!verifyable_object_isvalid (*attr, PTHREAD_ATTR_MAGIC))
    return EINVAL;
  if (contentionscope != PTHREAD_SCOPE_SYSTEM
      && contentionscope != PTHREAD_SCOPE_PROCESS)
    return EINVAL;
  /* In future, we may be able to support system scope by escalating the thread
   * priority to exceed the priority class. For now we only support PROCESS scope. */
  if (contentionscope != PTHREAD_SCOPE_PROCESS)
    return ENOTSUP;
  (*attr)->contentionscope = contentionscope;
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
__pthread_attr_getstacksize (const pthread_attr_t * attr, size_t * size)
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

  MT_INTERFACE->destructors.IterateNull ();

  thread->return_ptr = value_ptr;
  if (InterlockedDecrement (&MT_INTERFACE->threadcount) == 0)
    exit (0);
  else
    ExitThread (0);
}

int
__pthread_join (pthread_t * thread, void **return_val)
{
  /* FIXME: wait on the thread cancellation event as well - we are a cancellation point*/
  if (!verifyable_object_isvalid (*thread, PTHREAD_MAGIC))
    return ESRCH;

  if ((*thread)->attr.joinable == PTHREAD_CREATE_DETACHED)
    {
      if (return_val)
	*return_val = NULL;
      return EINVAL;
    }
  else
    {
      (*thread)->attr.joinable = PTHREAD_CREATE_DETACHED;
      WaitForSingleObject ((*thread)->win32_obj_id, INFINITE);
      if (return_val)
	*return_val = (*thread)->return_ptr;
    }	/* End if */

  pthread_testcancel ();

  return 0;
}

int
__pthread_detach (pthread_t * thread)
{
  if (!verifyable_object_isvalid (*thread, PTHREAD_MAGIC))
    return ESRCH;

  if ((*thread)->attr.joinable == PTHREAD_CREATE_DETACHED)
    {
      (*thread)->return_ptr = NULL;
      return EINVAL;
    }

  (*thread)->attr.joinable = PTHREAD_CREATE_DETACHED;
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

/* provided for source level compatability.
 * See http://www.opengroup.org/onlinepubs/007908799/xsh/pthread_getconcurrency.html
 */
int
__pthread_getconcurrency (void)
{
  return MT_INTERFACE->concurrency;
}

/* keep this in sync with sched.cc */
int
__pthread_getschedparam (pthread_t thread, int *policy,
			 struct sched_param *param)
{
  if (!verifyable_object_isvalid (thread, PTHREAD_MAGIC))
    return ESRCH;
  *policy = SCHED_FIFO;
  /* we don't return the current effective priority, we return the current requested
   * priority */
  *param = thread->attr.schedparam;
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

  *key = new pthread_key (destructor);

  if (!verifyable_object_isvalid (*key, PTHREAD_KEY_MAGIC))
    {
      delete (*key);
      *key = NULL;
      return EAGAIN;
    }
  return 0;
}

int
__pthread_key_delete (pthread_key_t key)
{
  if (!verifyable_object_isvalid (key, PTHREAD_KEY_MAGIC))
    return EINVAL;

  delete (key);
  return 0;
}

/* provided for source level compatability.
 * See http://www.opengroup.org/onlinepubs/007908799/xsh/pthread_getconcurrency.html
 */
int
__pthread_setconcurrency (int new_level)
{
  if (new_level < 0)
    return EINVAL;
  MT_INTERFACE->concurrency = new_level;
  return 0;
}

/* keep syncronised with sched.cc */
int
__pthread_setschedparam (pthread_t thread, int policy,
			 const struct sched_param *param)
{
  if (!verifyable_object_isvalid (thread, PTHREAD_MAGIC))
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

// FIXME: pshared mutexs have the cond count in the shared memory area.
// We need to accomodate that.
int
__pthread_cond_timedwait (pthread_cond_t * cond, pthread_mutex_t * mutex,
			  const struct timespec *abstime)
{
  int rv;
  if (!abstime)
    return EINVAL;
  pthread_mutex **themutex = NULL;
  if (*mutex == PTHREAD_MUTEX_INITIALIZER)
    __pthread_mutex_init (mutex, NULL);
  if ((((pshared_mutex *)(mutex))->flags & SYS_BASE == SYS_BASE))
    // a pshared mutex
    themutex = __pthread_mutex_getpshared (mutex);

  if (!verifyable_object_isvalid (*themutex, PTHREAD_MUTEX_MAGIC))
    return EINVAL;
  if (!verifyable_object_isvalid (*cond, PTHREAD_COND_MAGIC))
    return EINVAL;

  if ((*cond)->waiting)
    if ((*cond)->mutex && ((*cond)->mutex != (*themutex)))
      return EINVAL;
  InterlockedIncrement (&((*cond)->waiting));

  (*cond)->mutex = (*themutex);
  InterlockedIncrement (&((*themutex)->condwaits));
  rv = (*cond)->TimedWait (abstime->tv_sec * 1000);
  (*cond)->mutex->Lock ();
  if (InterlockedDecrement (&((*cond)->waiting)) == 0)
    (*cond)->mutex = NULL;
  InterlockedDecrement (&((*themutex)->condwaits));

  return rv;
}

int
__pthread_cond_wait (pthread_cond_t * cond, pthread_mutex_t * mutex)
{
  int rv;
  pthread_mutex_t *themutex = mutex;
  if (*mutex == PTHREAD_MUTEX_INITIALIZER)
    __pthread_mutex_init (mutex, NULL);
  if ((((pshared_mutex *)(mutex))->flags & SYS_BASE == SYS_BASE))
    // a pshared mutex
    themutex = __pthread_mutex_getpshared (mutex);
  if (!verifyable_object_isvalid (*themutex, PTHREAD_MUTEX_MAGIC))
    return EINVAL;
  if (!verifyable_object_isvalid (*cond, PTHREAD_COND_MAGIC))
    return EINVAL;

  if ((*cond)->waiting)
    if ((*cond)->mutex && ((*cond)->mutex != (*themutex)))
      return EINVAL;
  InterlockedIncrement (&((*cond)->waiting));

  (*cond)->mutex = (*themutex);
  InterlockedIncrement (&((*themutex)->condwaits));
  rv = (*cond)->TimedWait (INFINITE);
  (*cond)->mutex->Lock ();
  if (InterlockedDecrement (&((*cond)->waiting)) == 0)
    (*cond)->mutex = NULL;
  InterlockedDecrement (&((*themutex)->condwaits));

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
  /* shared cond vars not currently supported */
  if (pshared != PTHREAD_PROCESS_PRIVATE)
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
__pthread_kill (pthread_t thread, int sig)
{
// lock myself, for the use of thread2signal
  // two differ kills might clash: FIXME

  if (!verifyable_object_isvalid (thread, PTHREAD_MAGIC))
    return EINVAL;

  if (thread->sigs)
    myself->setthread2signal (thread);

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
pthread_t
__pthread_self ()
{
  return (pthread *) TlsGetValue (MT_INTERFACE->thread_self_dwTlsIndex);
}

int
__pthread_equal (pthread_t * t1, pthread_t * t2)
{
  return (*t1 - *t2);
}

/* Mutexes  */

/* FIXME: there's a potential race with PTHREAD_MUTEX_INITALIZER:
 * the mutex is not actually inited until the first use.
 * So two threads trying to lock/trylock may collide.
 * Solution: we need a global mutex on mutex creation, or possibly simply
 * on all constructors that allow INITIALIZER macros.
 * the lock should be very small: only around the init routine, not
 * every test, or all mutex access will be synchronised.
 */

int
__pthread_mutex_init (pthread_mutex_t * mutex,
		      const pthread_mutexattr_t * attr)
{
  if ((((pshared_mutex *)(mutex))->flags & SYS_BASE == SYS_BASE))
    // a pshared mutex
    return EBUSY;
  if (attr && !verifyable_object_isvalid (*attr, PTHREAD_MUTEXATTR_MAGIC))
    return EINVAL;

  if (verifyable_object_isvalid (*mutex, PTHREAD_MUTEX_MAGIC))
    return EBUSY;

  if (attr && (*attr)->pshared == PTHREAD_PROCESS_SHARED)
    {
      pthread_mutex_t throwaway = new pthread_mutex (mutex, (*attr));
      mutex = __pthread_mutex_getpshared ((pthread_mutex_t *) mutex);

      if (!verifyable_object_isvalid (*mutex, PTHREAD_MUTEX_MAGIC))
        {
          delete throwaway;
          *mutex = NULL;
          return EAGAIN;
        }
      return 0;
    }
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
__pthread_mutex_getprioceiling (const pthread_mutex_t * mutex,
				int *prioceiling)
{
  pthread_mutex_t *themutex=(pthread_mutex_t *) mutex;
  if (*mutex == PTHREAD_MUTEX_INITIALIZER)
    __pthread_mutex_init ((pthread_mutex_t *) mutex, NULL);
  if ((((pshared_mutex *)(mutex))->flags & SYS_BASE == SYS_BASE))
    // a pshared mutex
    themutex = __pthread_mutex_getpshared ((pthread_mutex_t *) mutex);
  if (!verifyable_object_isvalid (*themutex, PTHREAD_MUTEX_MAGIC))
    return EINVAL;
  /* We don't define _POSIX_THREAD_PRIO_PROTECT because we do't currently support
   * mutex priorities.
   *
   * We can support mutex priorities in the future though:
   * Store a priority with each mutex.
   * When the mutex is optained, set the thread priority as appropriate
   * When the mutex is released, reset the thre priority.
   */
  return ENOSYS;
}

int
__pthread_mutex_lock (pthread_mutex_t * mutex)
{
  pthread_mutex_t *themutex = mutex;
  if (*mutex == PTHREAD_MUTEX_INITIALIZER)
    __pthread_mutex_init (mutex, NULL);
  if ((((pshared_mutex *)(mutex))->flags & SYS_BASE) == SYS_BASE)
    // a pshared mutex
    themutex = __pthread_mutex_getpshared (mutex);
  if (!verifyable_object_isvalid (*themutex, PTHREAD_MUTEX_MAGIC))
    return EINVAL;
  (*themutex)->Lock ();
  return 0;
}

int
__pthread_mutex_trylock (pthread_mutex_t * mutex)
{
  pthread_mutex_t *themutex = mutex;
  if (*mutex == PTHREAD_MUTEX_INITIALIZER)
    __pthread_mutex_init (mutex, NULL);
  if ((((pshared_mutex *)(mutex))->flags & SYS_BASE) == SYS_BASE)
    // a pshared mutex
    themutex = __pthread_mutex_getpshared (mutex);
  if (!verifyable_object_isvalid (*themutex, PTHREAD_MUTEX_MAGIC))
    return EINVAL;
  if ((*themutex)->TryLock () == WAIT_TIMEOUT)
    return EBUSY;
  return 0;
}

int
__pthread_mutex_unlock (pthread_mutex_t * mutex)
{
  if (*mutex == PTHREAD_MUTEX_INITIALIZER)
    __pthread_mutex_init (mutex, NULL);
  if ((((pshared_mutex *)(mutex))->flags & SYS_BASE) == SYS_BASE)
    // a pshared mutex
    mutex = __pthread_mutex_getpshared (mutex);
  if (!verifyable_object_isvalid (*mutex, PTHREAD_MUTEX_MAGIC))
    return EINVAL;
  (*mutex)->UnLock ();
  return 0;
}

int
__pthread_mutex_destroy (pthread_mutex_t * mutex)
{
  if (*mutex == PTHREAD_MUTEX_INITIALIZER)
    return 0;
  if ((((pshared_mutex *)(mutex))->flags & SYS_BASE) == SYS_BASE)
    // a pshared mutex
    mutex = __pthread_mutex_getpshared (mutex);
  if (!verifyable_object_isvalid (*mutex, PTHREAD_MUTEX_MAGIC))
    return EINVAL;

  /* reading a word is atomic */
  if ((*mutex)->condwaits)
    return EBUSY;

  delete (*mutex);
  *mutex = NULL;
  return 0;
}

int
__pthread_mutex_setprioceiling (pthread_mutex_t * mutex, int prioceiling,
				int *old_ceiling)
{
  pthread_mutex_t *themutex = mutex;
  if (*mutex == PTHREAD_MUTEX_INITIALIZER)
    __pthread_mutex_init (mutex, NULL);
  if ((((pshared_mutex *)(mutex))->flags & SYS_BASE == SYS_BASE))
    // a pshared mutex
    themutex = __pthread_mutex_getpshared (mutex);
  if (!verifyable_object_isvalid (*themutex, PTHREAD_MUTEX_MAGIC))
    return EINVAL;
  return ENOSYS;
}

/* Win32 doesn't support mutex priorities - see __pthread_mutex_getprioceiling
 * for more detail */
int
__pthread_mutexattr_getprotocol (const pthread_mutexattr_t * attr,
				 int *protocol)
{
  if (!verifyable_object_isvalid (*attr, PTHREAD_MUTEX_MAGIC))
    return EINVAL;
  return ENOSYS;
}

int
__pthread_mutexattr_getpshared (const pthread_mutexattr_t * attr,
				int *pshared)
{
  if (!verifyable_object_isvalid (*attr, PTHREAD_MUTEX_MAGIC))
    return EINVAL;
  *pshared = (*attr)->pshared;
  return 0;
}

/* Win32 mutex's are equivalent to posix RECURSIVE mutexs.
 * We need to put glue in place to support other types of mutex's. We map
 * PTHREAD_MUTEX_DEFAULT to PTHREAD_MUTEX_RECURSIVE and return EINVAL for other types.
 */
int
__pthread_mutexattr_gettype (const pthread_mutexattr_t * attr, int *type)
{
  if (!verifyable_object_isvalid (*attr, PTHREAD_MUTEX_MAGIC))
    return EINVAL;
  *type = (*attr)->mutextype;
  return 0;
}

/* Currently pthread_mutex_init ignores the attr variable, this is because
 * none of the variables have any impact on it's behaviour.
 *
 * FIXME: write and test process shared mutex's.
 */
int
__pthread_mutexattr_init (pthread_mutexattr_t * attr)
{
  if (verifyable_object_isvalid (*attr, PTHREAD_MUTEXATTR_MAGIC))
    return EBUSY;

  *attr = new pthread_mutexattr ();
  if (!verifyable_object_isvalid (*attr, PTHREAD_MUTEXATTR_MAGIC))
    {
      delete (*attr);
      *attr = NULL;
      return ENOMEM;
    }
  return 0;
}

int
__pthread_mutexattr_destroy (pthread_mutexattr_t * attr)
{
  if (!verifyable_object_isvalid (*attr, PTHREAD_MUTEXATTR_MAGIC))
    return EINVAL;
  delete (*attr);
  *attr = NULL;
  return 0;
}


/* Win32 doesn't support mutex priorities */
int
__pthread_mutexattr_setprotocol (pthread_mutexattr_t * attr, int protocol)
{
  if (!verifyable_object_isvalid (*attr, PTHREAD_MUTEXATTR_MAGIC))
    return EINVAL;
  return ENOSYS;
}

/* Win32 doesn't support mutex priorities */
int
__pthread_mutexattr_setprioceiling (pthread_mutexattr_t * attr,
				    int prioceiling)
{
  if (!verifyable_object_isvalid (*attr, PTHREAD_MUTEXATTR_MAGIC))
    return EINVAL;
  return ENOSYS;
}

int
__pthread_mutexattr_getprioceiling (const pthread_mutexattr_t * attr,
				    int *prioceiling)
{
  if (!verifyable_object_isvalid (*attr, PTHREAD_MUTEXATTR_MAGIC))
    return EINVAL;
  return ENOSYS;
}

int
__pthread_mutexattr_setpshared (pthread_mutexattr_t * attr, int pshared)
{
  if (!verifyable_object_isvalid (*attr, PTHREAD_MUTEXATTR_MAGIC))
    return EINVAL;
  /* we don't use pshared for anything as yet. We need to test PROCESS_SHARED
   * functionality
   */
  if (pshared != PTHREAD_PROCESS_PRIVATE && pshared != PTHREAD_PROCESS_SHARED)
    return EINVAL;
  (*attr)->pshared = pshared;
  return 0;
}

/* see __pthread_mutex_gettype */
int
__pthread_mutexattr_settype (pthread_mutexattr_t * attr, int type)
{
  if (!verifyable_object_isvalid (*attr, PTHREAD_MUTEXATTR_MAGIC))
    return EINVAL;
  if (type != PTHREAD_MUTEX_RECURSIVE)
    return EINVAL;
  (*attr)->mutextype = type;
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

#endif // MT_SAFE
