/* thread.h: Locking and threading module definitions

   Copyright 1998, 1999, 2000 Cygnus Solutions.
   Copyright 2001 Red Hat, Inc.

   Written by Marco Fuykschot <marco@ddi.nl>
   Major update 2001 Robert Collins <rbtcollins@hotmail.com>
   
This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

#ifndef _CYGNUS_THREADS_
#define _CYGNUS_THREADS_

#define LOCK_FD_LIST     1
#define LOCK_MEMORY_LIST 2
#define LOCK_MMAP_LIST   3
#define LOCK_DLL_LIST    4

#define WRITE_LOCK 1
#define READ_LOCK  2

extern "C"
{
#if defined (_CYG_THREAD_FAILSAFE) && defined (_MT_SAFE)
void AssertResourceOwner (int, int);
#else
#define AssertResourceOwner(i,ii)
#endif
}

#ifndef _MT_SAFE

#define SetResourceLock(i,n,c)
#define ReleaseResourceLock(i,n,c)

#else

//#include <pthread.h>
/* FIXME: these are defined in pthread.h, but pthread.h defines symbols it shouldn't -
 * all the types.
 */
#define PTHREAD_PROCESS_PRIVATE 0
#define PTHREAD_PROCESS_SHARED  1
#define PTHREAD_DESTRUCTOR_ITERATIONS 1
/* Tls has 64 items for pre win2000 - and we don't want to use them all :]
 * Before committing discuss this with the list
 */
#define PTHREAD_KEYS_MAX 32
#define PTHREAD_CREATE_DETACHED 1
/* the default : joinable */
#define PTHREAD_CREATE_JOINABLE 0

#include <signal.h>
#include <pwd.h>
#include <grp.h>
#define _NOMNTENT_FUNCS
#include <mntent.h>

extern "C"
{

struct _winsup_t
{
  /*
  Needed for the group functions
  */
  struct group _grp;
  char *_namearray[2];
  int _grp_pos;

  /* console.cc */
  unsigned _rarg;

  /* dlfcn.cc */
  int _dl_error;
  char _dl_buffer[256];

  /* passwd.cc */
  struct passwd _res;
  char _pass[_PASSWORD_LEN];
  int _pw_pos;

  /* path.cc */
  struct mntent mntbuf;
  int _iteration;
  DWORD available_drives;

  /* strerror */
  char _strerror_buf[20];

  /* sysloc.cc */
  char *_process_ident;
  int _process_logopt;
  int _process_facility;
  int _process_logmask;

  /* times.cc */
  char timezone_buf[20];
  struct tm _localtime_buf;

  /* uinfo.cc */
  char _username[MAX_USER_NAME];
};


struct __reent_t
{
  struct _reent *_clib;
  struct _winsup_t *_winsup;
};

_reent *_reent_clib ();
_winsup_t *_reent_winsup ();
void SetResourceLock (int, int, const char *) __attribute__ ((regparm (3)));
void ReleaseResourceLock (int, int, const char *)
  __attribute__ ((regparm (3)));

#ifdef _CYG_THREAD_FAILSAFE
void AssertResourceOwner (int, int);
#else
#define AssertResourceOwner(i,ii)
#endif
}

class per_process;
class pinfo;

class ResourceLocks
{
public:
  ResourceLocks () {}
  LPCRITICAL_SECTION Lock (int);
  void Init ();
  void Delete ();
#ifdef _CYG_THREAD_FAILSAFE
  DWORD owner;
  DWORD count;
#endif
private:
  CRITICAL_SECTION lock;
  bool inited;
};

#define PTHREAD_MAGIC 0xdf0df045
#define PTHREAD_MUTEX_MAGIC PTHREAD_MAGIC+1
#define PTHREAD_KEY_MAGIC PTHREAD_MAGIC+2
#define PTHREAD_ATTR_MAGIC PTHREAD_MAGIC+3
#define PTHREAD_MUTEXATTR_MAGIC PTHREAD_MAGIC+4
#define PTHREAD_COND_MAGIC PTHREAD_MAGIC+5
#define PTHREAD_CONDATTR_MAGIC PTHREAD_MAGIC+6
#define SEM_MAGIC PTHREAD_MAGIC+7

class verifyable_object
{
public:
  long magic;

  verifyable_object (long);
  ~verifyable_object ();
};

int verifyable_object_isvalid (verifyable_object *, long);

class pthread_attr:public verifyable_object
{
public:
  int joinable;
  size_t stacksize;

  pthread_attr ();
  ~pthread_attr ();
};

class pthread:public verifyable_object
{
public:
  HANDLE win32_obj_id;
  class pthread_attr attr;
  void *(*function) (void *);
  void *arg;
  void *return_ptr;
  bool suspended;
  int joinable;
  DWORD GetThreadId ()
  {
    return thread_id;
  }
  void setThreadIdtoCurrent ()
  {
    thread_id = GetCurrentThreadId ();
  }

  /* signal handling */
  struct sigaction *sigs;
  sigset_t *sigmask;
  LONG *sigtodo;
  void create (void *(*)(void *), pthread_attr *, void *);

  pthread ();
  ~pthread ();

private:
  DWORD thread_id;
};

class pthread_mutexattr:public verifyable_object
{
public:
  pthread_mutexattr ();
  ~pthread_mutexattr ();
};

class pthread_mutex:public verifyable_object
{
public:
  HANDLE win32_obj_id;
  LONG condwaits;

  int Lock ();
  int TryLock ();
  int UnLock ();

  pthread_mutex (pthread_mutexattr *);
  ~pthread_mutex ();
};

class pthread_key:public verifyable_object
{
public:

  DWORD dwTlsIndex;
  int set (const void *);
  void *get ();

  pthread_key ();
  ~pthread_key ();
};

class pthread_condattr:public verifyable_object
{
public:
  int shared;

  pthread_condattr ();
  ~pthread_condattr ();
};

class pthread_cond:public verifyable_object
{
public:
  int shared;
  LONG waiting;
  pthread_mutex *mutex;
  HANDLE win32_obj_id;
  int TimedWait (DWORD dwMilliseconds);
  void BroadCast ();
  void Signal ();

  pthread_cond (pthread_condattr *);
  ~pthread_cond ();
};

/* shouldn't be here */
class semaphore:public verifyable_object
{
public:
  HANDLE win32_obj_id;
  int shared;
  void Wait ();
  void Post ();
  int TryWait ();

  semaphore (int, unsigned int);
  ~semaphore ();
};

typedef class pthread *pthread_t;
typedef class pthread_mutex *pthread_mutex_t;
/* sem routines belong in semaphore.cc */
typedef class semaphore *sem_t;

typedef class pthread_key *pthread_key_t;
typedef class pthread_attr *pthread_attr_t;
typedef class pthread_mutexattr *pthread_mutexattr_t;
typedef class pthread_condattr *pthread_condattr_t;
typedef class pthread_cond *pthread_cond_t;

class MTinterface
{
public:
  // General
  DWORD reent_index;
  DWORD thread_self_dwTlsIndex;
  /* we may get 0 for the Tls index.. grrr */
  int indexallocated;

  // Used for main thread data, and sigproc thread
  struct __reent_t reents;
  struct _winsup_t winsup_reent;
  pthread mainthread;

  void Init (int);

  MTinterface ():reent_index (0), indexallocated (0)
  {}
};


extern "C"
{
  void *thread_init_wrapper (void *);

/*  ThreadCreation */
  int __pthread_create (pthread_t * thread, const pthread_attr_t * attr,
			void *(*start_routine) (void *), void *arg);
  int __pthread_attr_init (pthread_attr_t * attr);
  int __pthread_attr_destroy (pthread_attr_t * attr);
  int __pthread_attr_setdetachstate (pthread_attr_t *, int);
  int __pthread_attr_getdetachstate (const pthread_attr_t *, int *);
  int __pthread_attr_setstacksize (pthread_attr_t * attr, size_t size);
  int __pthread_attr_getstacksize (pthread_attr_t * attr, size_t * size);
/*
__pthread_attr_setstackaddr(...);
__pthread_attr_getstackaddr(...);
*/

/* Thread Exit */
  void __pthread_exit (void *value_ptr);
  int __pthread_join (pthread_t * thread, void **return_val);
  int __pthread_detach (pthread_t * thread);

/* Thread suspend */

  int __pthread_suspend (pthread_t * thread);
  int __pthread_continue (pthread_t * thread);

  unsigned long __pthread_getsequence_np (pthread_t * thread);

/* Thread SpecificData */
  int __pthread_key_create (pthread_key_t * key, void (*destructor) (void *));
  int __pthread_key_delete (pthread_key_t * key);
  int __pthread_setspecific (pthread_key_t key, const void *value);
  void *__pthread_getspecific (pthread_key_t key);

/* Thead synchroniation */
  int __pthread_cond_destroy (pthread_cond_t * cond);
  int __pthread_cond_init (pthread_cond_t * cond,
			   const pthread_condattr_t * attr);
  int __pthread_cond_signal (pthread_cond_t * cond);
  int __pthread_cond_broadcast (pthread_cond_t * cond);
  int __pthread_cond_timedwait (pthread_cond_t * cond,
				pthread_mutex_t * mutex,
				const struct timespec *abstime);
  int __pthread_cond_wait (pthread_cond_t * cond, pthread_mutex_t * mutex);
  int __pthread_condattr_init (pthread_condattr_t * condattr);
  int __pthread_condattr_destroy (pthread_condattr_t * condattr);
  int __pthread_condattr_getpshared (const pthread_condattr_t * attr,
				     int *pshared);
  int __pthread_condattr_setpshared (pthread_condattr_t * attr, int pshared);

/* Thread signal */
  int __pthread_kill (pthread_t * thread, int sig);
  int __pthread_sigmask (int operation, const sigset_t * set,
			 sigset_t * old_set);

/*  ID */
  pthread_t __pthread_self ();
  int __pthread_equal (pthread_t * t1, pthread_t * t2);


/* Mutexes  */
  int __pthread_mutex_init (pthread_mutex_t *, const pthread_mutexattr_t *);
  int __pthread_mutex_lock (pthread_mutex_t *);
  int __pthread_mutex_trylock (pthread_mutex_t *);
  int __pthread_mutex_unlock (pthread_mutex_t *);
  int __pthread_mutex_destroy (pthread_mutex_t *);

/* Semaphores */
  int __sem_init (sem_t * sem, int pshared, unsigned int value);
  int __sem_destroy (sem_t * sem);
  int __sem_wait (sem_t * sem);
  int __sem_trywait (sem_t * sem);
  int __sem_post (sem_t * sem);
};

#endif // MT_SAFE

#endif // _CYGNUS_THREADS_
