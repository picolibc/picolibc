/* thread.h: Locking and threading module definitions

   Copyright 1998, 1999, 2000 Cygnus Solutions.

   Written by Marco Fuykschot <marco@ddi.nl>

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
#define LOCK_THREAD_LIST 5
#define LOCK_MUTEX_LIST  6
#define LOCK_SEM_LIST    7

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
#include <pwd.h>
#include <grp.h>
#define _NOMNTENT_FUNCS
#include <mntent.h>

extern "C" {

struct _winsup_t
{
/*
 Needed for the group functions
*/
  struct group _grp;
  char *_namearray[2];
  char _linebuf[100];
  int _grp_pos;

/* console.cc */
  unsigned _rarg;
  char _my_title_buf[TITLESIZE + 1];

/* dlfcn.cc */
  int _dl_error;
  char _dl_buffer[256];

/* passwd.cc */
  struct passwd _res;
  char _tmpbuf[100];
  char _pass[_PASSWORD_LEN];
  int _pw_pos;

/* path.cc */
  struct mntent _ret;
  int _iteration;

/* strerror */
  char _strerror_buf[20];

/* syscalls.cc */
  char _dacl_buf[1024];
  char _sacl_buf[1024];
  char _ownr_buf[1024];
  char _grp_buf[1024];

/* sysloc.cc */
  char *_process_ident;
  int _process_logopt;
  int _process_facility;
  int _process_logmask;

/* times.cc */
  char _b[20];
  struct tm _localtime_buf;
  char _buf1[33];
  char _buf2[33];

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
void SetResourceLock (int, int, const char *);
void ReleaseResourceLock (int, int, const char *);

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
ResourceLocks ():inited (false) {};
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


#define MT_MAX_ITEMS 128

// thread classes\lists

class MTitem
{
public:
HANDLE win32_obj_id;
UINT return_value;
bool used;
char joinable;       // for thread only
bool HandleOke () {return win32_obj_id;};
virtual void Destroy ();
virtual int Id () {return (int) win32_obj_id;};
};

class ThreadItem:public MTitem
{
public:
pthread_attr_t attr;
TFD (function);
void *arg;
void *return_ptr;
bool suspended;
DWORD thread_id;
DWORD GetThreadId () {return thread_id;};

/* signal handling */
struct sigaction *sigs;
sigset_t *sigmask;
LONG *sigtodo;
};

class MutexItem:public MTitem
{
public:
int Lock ();
int TryLock ();
int UnLock ();
};

class SemaphoreItem:public MTitem
{
public:
int shared;
int Wait ();
int Post ();
int TryWait ();
};


typedef struct
{
MTitem *items[MT_MAX_ITEMS];
int index;
}
MTList;

class MTinterface
{
public:
// General
DWORD reent_index;
DWORD thread_key;

// Used for main thread data, and sigproc thread
struct __reent_t reents;
struct _winsup_t winsup_reent;
ThreadItem mainthread;

void Init0 ();
void Init1 ();
void ClearReent ();

void ReleaseItem (MTitem *);

// Thread functions
ThreadItem *CreateThread (pthread_t *, TFD (func), void *, pthread_attr_t);
ThreadItem *GetCallingThread ();
ThreadItem *GetThread (pthread_t *);

// Mutex functions
MutexItem *CreateMutex (pthread_mutex_t *);
MutexItem *GetMutex (pthread_mutex_t *);

// Semaphore functions
SemaphoreItem *CreateSemaphore (sem_t *, int, int);
SemaphoreItem *GetSemaphore (sem_t * t);

private:
// General Administration
MTitem * Find (void *, int (*compare) (void *, void *), int &, MTList *);
MTitem *GetItem (int, MTList *);
MTitem *SetItem (int, MTitem *, MTList *);
int Find (MTitem &, MTList *);
int FindNextUnused (MTList *);

MTList threadlist;
MTList mutexlist;
MTList semalist;
};


extern "C"
{

void *thread_init_wrapper (void *);

/*  ThreadCreation */
int __pthread_create (pthread_t * thread, const pthread_attr_t * attr, TFD (start_routine), void *arg);
int __pthread_attr_init (pthread_attr_t * attr);
int __pthread_attr_destroy (pthread_attr_t * attr);
int __pthread_attr_setstacksize (pthread_attr_t * attr, size_t size);
int __pthread_attr_getstacksize (pthread_attr_t * attr, size_t * size);
/*
__pthread_attr_setstackaddr(...);
__pthread_attr_getstackaddr(...);
*/

/* Thread Exit */
int __pthread_exit (void *value_ptr);
int __pthread_join(pthread_t *thread, void **return_val);
int __pthread_detach(pthread_t *thread);

/* Thread suspend */

int __pthread_suspend(pthread_t *thread);
int __pthread_continue(pthread_t *thread);

unsigned long __pthread_getsequence_np (pthread_t * thread);

/* Thread SpecificData */
int __pthread_key_create (pthread_key_t * key);
int __pthread_key_delete (pthread_key_t * key);
int __pthread_setspecific (pthread_key_t * key, const void *value);
void *__pthread_getspecific (pthread_key_t * key);


/* Thread signal */
int __pthread_kill (pthread_t * thread, int sig);
int __pthread_sigmask (int operation, const sigset_t * set, sigset_t * old_set);

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
