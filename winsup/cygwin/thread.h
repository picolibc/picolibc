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

#include <pthread.h>
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
  char mnt_type[80];
  char mnt_opts[80];
  char mnt_fsname[MAX_PATH];
  char mnt_dir[MAX_PATH];

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
  ResourceLocks ()
  {
  }
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
#define PTHREAD_ONCE_MAGIC PTHREAD_MAGIC+8;

/* verifyable_object should not be defined here - it's a general purpose class */

class verifyable_object
{
public:
  long magic;

    verifyable_object (long);
   ~verifyable_object ();
};

int verifyable_object_isvalid (verifyable_object *, long);

class pthread_key:public verifyable_object
{
public:

  DWORD dwTlsIndex;
  int set (const void *);
  void *get ();

    pthread_key (void (*)(void *));
   ~pthread_key ();
};

/* FIXME: test using multiple inheritance and merging key_destructor into pthread_key
 * for efficiency */
class pthread_key_destructor
{
public:
  void (*destructor) (void *);
  pthread_key_destructor *InsertAfter (pthread_key_destructor * node);
  pthread_key_destructor *UnlinkNext ();
  pthread_key_destructor *Next ();

    pthread_key_destructor (void (*thedestructor) (void *), pthread_key * key);
  pthread_key_destructor *next;
  pthread_key *key;
};

class pthread_key_destructor_list
{
public:
  void Insert (pthread_key_destructor * node);
/* remove a given dataitem, wherever in the list it is */
  pthread_key_destructor *Remove (pthread_key_destructor * item);
/* get the first item and remove at the same time */
  pthread_key_destructor *Pop ();
  pthread_key_destructor *Remove (pthread_key * key);
  void IterateNull ();
private:
    pthread_key_destructor * head;
};


class pthread_attr:public verifyable_object
{
public:
  int joinable;
  int contentionscope;
  int inheritsched;
  struct sched_param schedparam;
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
  int cancelstate, canceltype;
  // int joinable;

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
  int pshared;
  int mutextype;
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

class pthread_once
{
public:
  pthread_mutex_t mutex;
  int state;
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

class callback
{
public:
  void (*cb)(void);
  class callback * next;
};

class MTinterface
{
public:
  // General
  DWORD reent_index;
  DWORD thread_self_dwTlsIndex;
  /* we may get 0 for the Tls index.. grrr */
  int indexallocated;
  int concurrency;
  long int threadcount;

  // Used for main thread data, and sigproc thread
  struct __reent_t reents;
  struct _winsup_t winsup_reent;
  pthread mainthread;

  pthread_key_destructor_list destructors;
  callback *pthread_prepare;
  callback *pthread_child;
  callback *pthread_parent;

  void Init (int);

    MTinterface ():reent_index (0), indexallocated (0), threadcount (1)
  {
    pthread_prepare = NULL;
    pthread_child   = NULL;
    pthread_parent  = NULL;
  }
};

void __pthread_atforkprepare(void);
void __pthread_atforkparent(void);
void __pthread_atforkchild(void);

extern "C"
{
void *thread_init_wrapper (void *);

/*  ThreadCreation */
int __pthread_create (pthread_t * thread, const pthread_attr_t * attr,
		      void *(*start_routine) (void *), void *arg);
int __pthread_once (pthread_once_t *, void (*)(void));
int __pthread_atfork(void (*)(void), void (*)(void), void (*)(void));

int __pthread_attr_init (pthread_attr_t * attr);
int __pthread_attr_destroy (pthread_attr_t * attr);
int __pthread_attr_setdetachstate (pthread_attr_t *, int);
int __pthread_attr_getdetachstate (const pthread_attr_t *, int *);
int __pthread_attr_setstacksize (pthread_attr_t * attr, size_t size);
int __pthread_attr_getstacksize (const pthread_attr_t * attr, size_t * size);

int __pthread_attr_getinheritsched (const pthread_attr_t *, int *);
int __pthread_attr_getschedparam (const pthread_attr_t *,
				  struct sched_param *);
int __pthread_attr_getschedpolicy (const pthread_attr_t *, int *);
int __pthread_attr_getscope (const pthread_attr_t *, int *);
int __pthread_attr_getstackaddr (const pthread_attr_t *, void **);
int __pthread_attr_setinheritsched (pthread_attr_t *, int);
int __pthread_attr_setschedparam (pthread_attr_t *,
				  const struct sched_param *);
int __pthread_attr_setschedpolicy (pthread_attr_t *, int);
int __pthread_attr_setscope (pthread_attr_t *, int);
int __pthread_attr_setstackaddr (pthread_attr_t *, void *);



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
int __pthread_key_delete (pthread_key_t key);
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
int __pthread_kill (pthread_t thread, int sig);
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
int __pthread_mutex_setprioceiling (pthread_mutex_t * mutex,
				    int prioceiling, int *old_ceiling);
int __pthread_mutex_getprioceiling (const pthread_mutex_t * mutex,
				    int *prioceiling);


int __pthread_mutexattr_destroy (pthread_mutexattr_t *);
int __pthread_mutexattr_getprioceiling (const pthread_mutexattr_t *, int *);
int __pthread_mutexattr_getprotocol (const pthread_mutexattr_t *, int *);
int __pthread_mutexattr_getpshared (const pthread_mutexattr_t *, int *);
int __pthread_mutexattr_gettype (const pthread_mutexattr_t *, int *);
int __pthread_mutexattr_init (pthread_mutexattr_t *);
int __pthread_mutexattr_setprioceiling (pthread_mutexattr_t *, int);
int __pthread_mutexattr_setprotocol (pthread_mutexattr_t *, int);
int __pthread_mutexattr_setpshared (pthread_mutexattr_t *, int);
int __pthread_mutexattr_settype (pthread_mutexattr_t *, int);


/* Scheduling */
int __pthread_getconcurrency (void);
int __pthread_setconcurrency (int new_level);
int __pthread_getschedparam (pthread_t thread, int *policy,
			     struct sched_param *param);
int __pthread_setschedparam (pthread_t thread, int policy,
			     const struct sched_param *param);

/* cancelability states */
int __pthread_cancel (pthread_t thread);
int __pthread_setcancelstate (int state, int *oldstate);
int __pthread_setcanceltype (int type, int *oldtype);
void __pthread_testcancel (void);


/* Semaphores */
int __sem_init (sem_t * sem, int pshared, unsigned int value);
int __sem_destroy (sem_t * sem);
int __sem_wait (sem_t * sem);
int __sem_trywait (sem_t * sem);
int __sem_post (sem_t * sem);
};

#endif // MT_SAFE

#endif // _CYGNUS_THREADS_
