/* thread.cc: Locking and threading module functions

   Copyright 1998, 2000 Cygnus Solutions.

   Written by Marco Fuykschot <marco@ddi.nl>

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#ifdef _MT_SAFE
#include "winsup.h"
#include <errno.h>
#include <assert.h>
#include <stdlib.h>
#include <syslog.h>
#include "thread.h"
#include "sync.h"
#include "sigproc.h"
#include "pinfo.h"

extern int threadsafe;

#define MT_INTERFACE user_data->threadinterface

#define NOT_IMP(n)  system_printf ("not implemented %s\n", n); return 0;

#define CHECKHANDLE(rval, release) \
  if (!item->HandleOke ()) \
    { \
      if (release) \
	item->used=false; \
      return rval; \
    }

#define GETTHREAD(n) \
  if (!thread) system_printf ("thread is NULL");\
  SetResourceLock (LOCK_THREAD_LIST, READ_LOCK, n);\
  ThreadItem *item=user_data->threadinterface->GetThread (thread); \
  ReleaseResourceLock (LOCK_THREAD_LIST, READ_LOCK, n); \
  if (!item) return EINVAL; \
  CHECKHANDLE (EINVAL, 0);

#define GETMUTEX(n) \
  SetResourceLock (LOCK_MUTEX_LIST, READ_LOCK, n); \
  MutexItem* item=user_data->threadinterface->GetMutex (mutex); \
  ReleaseResourceLock (LOCK_MUTEX_LIST, READ_LOCK, n); \
  if (!item) return EINVAL;  \
  CHECKHANDLE (EINVAL, 0);

#define GETSEMA(n) \
  SetResourceLock (LOCK_SEM_LIST, READ_LOCK, n); \
  SemaphoreItem* item=user_data->threadinterface->GetSemaphore (sem); \
  ReleaseResourceLock (LOCK_SEM_LIST, READ_LOCK, n); \
  if (!item) return EINVAL; \
  CHECKHANDLE (EINVAL, 0);

#define CHECKITEM(rn, rm, fn) \
  if (!item) { \
    ReleaseResourceLock (rn, rm, fn); \
    return EINVAL;  }; \

struct _reent *
_reent_clib ()
{
  int tmp = GetLastError ();
  struct __reent_t *_r = (struct __reent_t *) TlsGetValue (MT_INTERFACE->reent_index);

#ifdef _CYG_THREAD_FAILSAFE
  if (_r == 0)
    system_printf ("local thread storage not inited");
#endif

  SetLastError (tmp);
  return _r->_clib;
};

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
};

inline LPCRITICAL_SECTION
ResourceLocks::Lock (int _resid)
{
#ifdef _CYG_THREAD_FAILSAFE
  if (!inited)
    system_printf ("lock called before initialization");

  thread_printf ("Get Resource lock %d ==> %p for %p , real : %d , threadid %d ",
		 _resid, &lock, user_data, myself->pid, GetCurrentThreadId ());
#endif
  return &lock;
};

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
  thread_printf ("Release resource lock %d mode %d for %s done", _res_id, _mode, _function);

  AssertResourceOwner (_res_id, _mode);
  user_data->resourcelocks->count--;
  if (user_data->resourcelocks->count == 0)
    user_data->resourcelocks->owner = 0;
#endif

  LeaveCriticalSection (user_data->resourcelocks->Lock (_res_id));
};

#ifdef _CYG_THREAD_FAILSAFE
void
AssertResourceOwner (int _res_id, int _mode)
{

  thread_printf ("Assert Resource lock %d ==> for %p , real : %d , threadid %d count %d owner %d",
		 _res_id, user_data, myself->pid, GetCurrentThreadId (),
		 user_data->resourcelocks->count, user_data->resourcelocks->owner);
  if (user_data && (user_data->resourcelocks->owner != GetCurrentThreadId ()))
    system_printf ("assertion failed, not the resource owner");
}

#endif

void
ResourceLocks::Init ()
{
  thread_printf ("Init resource lock %p -> %p", this, &lock);

  InitializeCriticalSection (&lock);
  inited = true;

#ifdef _CYG_THREAD_FAILSAFE
  owner = 0;
  count = 0;
#endif

  thread_printf ("Resource lock %p inited by %p , %d", &lock, user_data, myself->pid);
};

void
ResourceLocks::Delete ()
{
  if (inited)
    {
      thread_printf ("Close Resource Locks %p ", &lock);
      DeleteCriticalSection (&lock);
      inited = false;
    }
};


// Thread interface

void
MTinterface::ReleaseItem (MTitem * _item)
{
  _item->used = false;
};

MTitem *
MTinterface::Find (void *_value, int (*comp) (void *, void *), register int &_index, MTList * _list)
{
  register MTitem *current = NULL;
  for (; _index < _list->index; _index++)
    {
      current = _list->items[_index];
      if (current->used && comp (current, _value))
	break;
      current = NULL;
    }
  return current;
};

int
MTinterface::Find (MTitem & _item, MTList * _list)
{
  register MTitem *current;
  register int _index = 0;
  for (; _index < _list->index; _index++)
    {
      current = _list->items[_index];
      if (current->used && current == &_item)
	break;
    }
  return (_index == _list->index ? -1 : _index);
};

int
MTinterface::FindNextUnused (MTList * _list)
{
  register int i = 0;
  for (; i < _list->index && _list->items[i] != NULL && _list->items[i]->used && _list->items[i]->joinable != 'Y';  i++);
  return i;
};

MTitem *
MTinterface::GetItem (int _index, MTList * _list)
{
  return (_index < _list->index ? _list->items[_index] : NULL);
};

MTitem *
MTinterface::SetItem (int _index, MTitem * _item, MTList * _list)
{
  if (_index == _list->index && _list->index < MT_MAX_ITEMS)
    _list->index++;
  return (_index < _list->index ? _list->items[_index] = _item : NULL);
};

int
CmpPthreadObj (void *_i, void *_value)
{
  return ( (MTitem *) _i)->Id () == * (int *) _value;
};

int
CmpThreadId (void *_i, void *_id)
{
  return ( (ThreadItem *) _i)->thread_id == * (DWORD *) _id;
};

void
MTinterface::Init0 ()
{
  for (int i = 0; i < MT_MAX_ITEMS; i++)
    {
      threadlist.items[i] = NULL;
      mutexlist.items[i] = NULL;
      semalist.items[i] = NULL;
    }

  threadlist.index = 0;
  mutexlist.index = 0;
  semalist.index = 0;

  reent_index = TlsAlloc ();

  reents._clib = _impure_ptr;
  reents._winsup = &winsup_reent;

  winsup_reent._process_logmask = LOG_UPTO (LOG_DEBUG);
  winsup_reent._grp_pos = 0;
  winsup_reent._process_ident = 0;
  winsup_reent._process_logopt = 0;
  winsup_reent._process_facility = 0;

  TlsSetValue (reent_index, &reents);
  // the static reent_data will be used in the main thread

};

void
MTinterface::Init1 ()
{
  // create entry for main thread

  int i = FindNextUnused (&threadlist);
  assert (i == 0);
  ThreadItem *item = (ThreadItem *) GetItem (i, &threadlist);

  item = (ThreadItem *) SetItem (i, &mainthread, &threadlist);
  item->used = true;
  item->win32_obj_id = myself->hProcess;
  item->thread_id = GetCurrentThreadId ();
  item->function = NULL;

  item->sigs = NULL;
  item->sigmask = NULL;
  item->sigtodo = NULL;
};

void
MTinterface::ClearReent ()
{
  struct _reent *r = _REENT;
  memset (r, 0, sizeof (struct _reent));

  r->_errno = 0;
  r->_stdin = &r->__sf[0];
  r->_stdout = &r->__sf[1];
  r->_stderr = &r->__sf[2];

};


ThreadItem *
MTinterface::CreateThread (pthread_t * t, TFD (func), void *arg, pthread_attr_t a)
{
  AssertResourceOwner (LOCK_THREAD_LIST, WRITE_LOCK | READ_LOCK);

  int i = FindNextUnused (&threadlist);

  ThreadItem *item = (ThreadItem *) GetItem (i, &threadlist);
  if (!item)
    item = (ThreadItem *) SetItem (i, new ThreadItem (), &threadlist);
  if (!item)
    system_printf ("thread creation failed");

  item->used = true;
  item->function = func;
  item->arg = arg;
  item->attr = a;

  item->win32_obj_id = ::CreateThread (&sec_none_nih, item->attr.stacksize,
   (LPTHREAD_START_ROUTINE) thread_init_wrapper, item, 0, &item->thread_id);

  CHECKHANDLE (NULL, 1);

  *t = (pthread_t) item->win32_obj_id;

  return item;
};


MutexItem *
MTinterface::CreateMutex (pthread_mutex_t * mutex)
{
  AssertResourceOwner (LOCK_MUTEX_LIST, WRITE_LOCK | READ_LOCK);

  int i = FindNextUnused (&mutexlist);

  MutexItem *item = (MutexItem *) GetItem (i, &mutexlist);
  if (!item)
    item = (MutexItem *) SetItem (i, new MutexItem (), &mutexlist);
  if (!item)
    system_printf ("mutex creation failed");
  item->used = true;

  item->win32_obj_id = ::CreateMutex (&sec_none_nih, false, NULL);

  CHECKHANDLE (NULL, 1);

  *mutex = (pthread_mutex_t) item->win32_obj_id;

  return item;
}

ThreadItem *
MTinterface::GetCallingThread ()
{
  AssertResourceOwner (LOCK_THREAD_LIST, READ_LOCK);
  DWORD id = GetCurrentThreadId ();
  int index = 0;
  return (ThreadItem *) Find (&id, &CmpThreadId, index, &threadlist);
};

ThreadItem *
MTinterface::GetThread (pthread_t * _t)
{
  AssertResourceOwner (LOCK_THREAD_LIST, READ_LOCK);
  int index = 0;
  return (ThreadItem *) Find (_t, &CmpPthreadObj, index, &threadlist);
};

MutexItem *
MTinterface::GetMutex (pthread_mutex_t * mp)
{
  AssertResourceOwner (LOCK_MUTEX_LIST, READ_LOCK);
  int index = 0;
  return (MutexItem *) Find (mp, &CmpPthreadObj, index, &mutexlist);
}

SemaphoreItem *
MTinterface::GetSemaphore (sem_t * sp)
{
  AssertResourceOwner (LOCK_SEM_LIST, READ_LOCK);
  int index = 0;
  return (SemaphoreItem *) Find (sp, &CmpPthreadObj, index, &semalist);
}


void
MTitem::Destroy ()
{
  CloseHandle (win32_obj_id);
};

int
MutexItem::Lock ()
{
  return WaitForSingleObject (win32_obj_id, INFINITE);
};

int
MutexItem::TryLock ()
{
  return WaitForSingleObject (win32_obj_id, 0);
};

int
MutexItem::UnLock ()
{
  return ReleaseMutex (win32_obj_id);
}

SemaphoreItem *
MTinterface::CreateSemaphore (sem_t * _s, int pshared, int _v)
{
  AssertResourceOwner (LOCK_SEM_LIST, WRITE_LOCK | READ_LOCK);

  int i = FindNextUnused (&semalist);

  SemaphoreItem *item = (SemaphoreItem *) GetItem (i, &semalist);
  if (!item)
    item = (SemaphoreItem *) SetItem (i, new SemaphoreItem (), &semalist);
  if (!item)
    system_printf ("semaphore creation failed");
  item->used = true;
  item->shared = pshared;

  item->win32_obj_id = ::CreateSemaphore (&sec_none_nih, _v, _v, NULL);

  CHECKHANDLE (NULL, 1);

  *_s = (sem_t) item->win32_obj_id;

  return item;
};

int
SemaphoreItem::Wait ()
{
  return WaitForSingleObject (win32_obj_id, INFINITE);
};

int
SemaphoreItem::Post ()
{
  long pc;
  return ReleaseSemaphore (win32_obj_id, 1, &pc);
};

int
SemaphoreItem::TryWait ()
{
  return WaitForSingleObject (win32_obj_id, 0);
};


//////////////////////////  Pthreads

void *
thread_init_wrapper (void *_arg)
{
// Setup the local/global storage of this thread

  ThreadItem *thread = (ThreadItem *) _arg;
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


  if (!TlsSetValue (MT_INTERFACE->reent_index, &local_reent))
    system_printf ("local storage for thread couldn't be set");

#ifdef _CYG_THREAD_FAILSAFE
  if (_REENT == _impure_ptr)
    system_printf ("local storage for thread isn't setup correctly");
#endif


  thread_printf ("started thread %p %p %p %p %p %p", _arg, &local_clib, _impure_ptr, thread, thread->function, thread->arg);


// call the user's thread
  void *ret = thread->function (thread->arg);

// FIX ME : cleanup code

//  thread->used = false;	// release thread entry
    thread->return_ptr = ret;
  ExitThread (0);
}

int
__pthread_create (pthread_t * thread, const pthread_attr_t * attr, TFD (start_routine), void *arg)
{
  SetResourceLock (LOCK_THREAD_LIST, WRITE_LOCK | READ_LOCK, "__pthread_create");

  pthread_attr_t a;
  ThreadItem *item;

  if (attr)
    item = MT_INTERFACE->CreateThread (thread, start_routine, arg, *attr);
  else
    {
      __pthread_attr_init (&a);
      item = MT_INTERFACE->CreateThread (thread, start_routine, arg, a);
    }

  CHECKITEM (LOCK_THREAD_LIST, WRITE_LOCK | READ_LOCK, "__pthread_create")

  ReleaseResourceLock (LOCK_THREAD_LIST, WRITE_LOCK | READ_LOCK, "__pthread_create");
  return 0;
};

int
__pthread_attr_init (pthread_attr_t * attr)
{
  attr->stacksize = 0;
  return 0;
};

int
__pthread_attr_setstacksize (pthread_attr_t * attr, size_t size)
{
  attr->stacksize = size;
  return 0;
};

int
__pthread_attr_getstacksize (pthread_attr_t * attr, size_t * size)
{
  *size = attr->stacksize;
  return 0;
};

int
__pthread_attr_destroy (pthread_attr_t * /*attr*/)
{
  return 0;
};

int
__pthread_exit (void *value_ptr)
{
  ThreadItem *item = MT_INTERFACE->GetCallingThread ();
  item->return_ptr = value_ptr;
  ExitThread (0);
  return 0;
}

int
__pthread_join (pthread_t * thread, void **return_val)
{
  ThreadItem *item=user_data->threadinterface->GetThread (thread);


  if (!item)
     return ESRCH;

  if (item->joinable == 'N')
  {
     if (return_val)
	*return_val = NULL;
     return EINVAL;
  }
  else
  {
     item->joinable = 'N';
     WaitForSingleObject ((HANDLE)*thread, INFINITE);
     if (return_val)
	*return_val = item->return_ptr;
  }/* End if*/

  return 0;
};

int
__pthread_detach (pthread_t * thread)
{
  ThreadItem *item=user_data->threadinterface->GetThread (thread);
  if (!item)
     return ESRCH;

  if (item->joinable == 'N')
  {
     item->return_ptr = NULL;
     return EINVAL;
  }

  item->joinable = 'N';
  return 0;
}

int
__pthread_suspend (pthread_t * thread)
{
  ThreadItem *item=user_data->threadinterface->GetThread (thread);
  if (!item)
     return ESRCH;

  if (item->suspended == false)
  {
     item->suspended = true;
     SuspendThread ((HANDLE)*thread);
  }

  return 0;
}


int
__pthread_continue (pthread_t * thread)
{
  ThreadItem *item=user_data->threadinterface->GetThread (thread);
  if (!item)
     return ESRCH;

  if (item->suspended == true)
	ResumeThread ((HANDLE)*thread);
  item->suspended = false;

  return 0;
}




unsigned long
__pthread_getsequence_np (pthread_t * thread)
{
  GETTHREAD ("__pthread_getsequence_np");
  return item->GetThreadId ();
};

/* Thread SpecificData */
int
__pthread_key_create (pthread_key_t */*key*/)
{
  NOT_IMP ("_p_key_create\n");
};

int
__pthread_key_delete (pthread_key_t */*key*/)
{
  NOT_IMP ("_p_key_delete\n");
};
int
__pthread_setspecific (pthread_key_t */*key*/, const void */*value*/)
{
  NOT_IMP ("_p_key_setsp\n");
};
void *
__pthread_getspecific (pthread_key_t */*key*/)
{
  NOT_IMP ("_p_key_getsp\n");
};

/* Thread signal */
int
__pthread_kill (pthread_t * thread, int sig)
{
// lock myself, for the use of thread2signal
  // two differ kills might clash: FIX ME
  GETTHREAD ("__pthread_kill");

  if (item->sigs)
    myself->setthread2signal (item);

  int rval = _kill (myself->pid, sig);

// unlock myself
  return rval;

};

int
__pthread_sigmask (int operation, const sigset_t * set, sigset_t * old_set)
{
  SetResourceLock (LOCK_THREAD_LIST, READ_LOCK, "__pthread_sigmask");
  ThreadItem *item = MT_INTERFACE->GetCallingThread ();
  ReleaseResourceLock (LOCK_THREAD_LIST, READ_LOCK, "__pthread_sigmask");

// lock this myself, for the use of thread2signal
  // two differt kills might clash: FIX ME

  if (item->sigs)
    myself->setthread2signal (item);

  int rval = sigprocmask (operation, set, old_set);

// unlock this myself

  return rval;
};

/*  ID */
pthread_t
__pthread_self ()
{
  SetResourceLock (LOCK_THREAD_LIST, READ_LOCK, "__pthread_self");

  ThreadItem *item = MT_INTERFACE->GetCallingThread ();

  ReleaseResourceLock (LOCK_THREAD_LIST, READ_LOCK, "__pthread_self");
  return (pthread_t) item->Id ();

};

int
__pthread_equal (pthread_t * t1, pthread_t * t2)
{
  return (*t1 - *t2);
};

/* Mutexes  */

int
__pthread_mutex_init (pthread_mutex_t * mutex, const pthread_mutexattr_t */*_attr*/)
{
  SetResourceLock (LOCK_MUTEX_LIST, WRITE_LOCK | READ_LOCK, "__pthread_mutex_init");

  MutexItem *item = MT_INTERFACE->CreateMutex (mutex);

  CHECKITEM (LOCK_MUTEX_LIST, WRITE_LOCK | READ_LOCK, "__pthread_mutex_init");

  ReleaseResourceLock (LOCK_MUTEX_LIST, WRITE_LOCK | READ_LOCK, "__pthread_mutex_init");
  return 0;
};

int
__pthread_mutex_lock (pthread_mutex_t * mutex)
{
  GETMUTEX ("_ptherad_mutex_lock");

  item->Lock ();

  return 0;
};

int
__pthread_mutex_trylock (pthread_mutex_t * mutex)
{
  GETMUTEX ("_ptherad_mutex_lock");

  if (item->TryLock () == WAIT_TIMEOUT)
    return EBUSY;

  return 0;
};

int
__pthread_mutex_unlock (pthread_mutex_t * mutex)
{
  GETMUTEX ("_ptherad_mutex_lock");

  item->UnLock ();

  return 0;
};

int
__pthread_mutex_destroy (pthread_mutex_t * mutex)
{
  SetResourceLock (LOCK_MUTEX_LIST, READ_LOCK | WRITE_LOCK, "__pthread_mutex_destroy");

  MutexItem *item = MT_INTERFACE->GetMutex (mutex);

  CHECKITEM (LOCK_MUTEX_LIST, WRITE_LOCK | READ_LOCK, "__pthread_mutex_init");

  item->Destroy ();

  MT_INTERFACE->ReleaseItem (item);

  ReleaseResourceLock (LOCK_MUTEX_LIST, READ_LOCK | WRITE_LOCK, "__pthread_mutex_destroy");
  return 0;
};

/* Semaphores */
int
__sem_init (sem_t * sem, int pshared, unsigned int value)
{
  SetResourceLock (LOCK_SEM_LIST, READ_LOCK | WRITE_LOCK, "__sem_init");

  SemaphoreItem *item = MT_INTERFACE->CreateSemaphore (sem, pshared, value);

  CHECKITEM (LOCK_SEM_LIST, READ_LOCK | WRITE_LOCK, "__sem_init");

  ReleaseResourceLock (LOCK_SEM_LIST, READ_LOCK | WRITE_LOCK, "__sem_init");
  return 0;
};

int
__sem_destroy (sem_t * sem)
{
  SetResourceLock (LOCK_SEM_LIST, READ_LOCK | WRITE_LOCK, "__sem_destroy");

  SemaphoreItem *item = MT_INTERFACE->GetSemaphore (sem);

  CHECKITEM (LOCK_SEM_LIST, READ_LOCK | WRITE_LOCK, "__sem_init");

  item->Destroy ();

  MT_INTERFACE->ReleaseItem (item);

  ReleaseResourceLock (LOCK_SEM_LIST, READ_LOCK | WRITE_LOCK, "__sem_destroy");
  return 0;
};

int
__sem_wait (sem_t * sem)
{
  GETSEMA ("__sem_wait");

  item->Wait ();

  return 0;
};

int
__sem_trywait (sem_t * sem)
{
  GETSEMA ("__sem_trywait");

  if (item->TryWait () == WAIT_TIMEOUT)
    return EAGAIN;

  return 0;
};

int
__sem_post (sem_t * sem)
{
  GETSEMA ("__sem_post");

  item->Post ();

  return 0;
};


#else

// empty functions needed when makeing the dll without mt_safe support
extern "C"
{
  int __pthread_create (pthread_t *, const pthread_attr_t *, TFD (start_routine), void *arg)
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
  int __pthread_attr_setstacksize (pthread_attr_t * attr, size_t size)
  {
    return -1;
  }
  int __pthread_attr_getstacksize (pthread_attr_t * attr, size_t * size)
  {
    return -1;
  }
/*
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
  int __pthread_sigmask (int operation, const sigset_t * set, sigset_t * old_set)
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

#endif // MT_SAFE
