/* thread.h: Locking and threading module definitions

   Copyright 1998, 1999, 2000, 2001, 2002, 2003 Red Hat, Inc.

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
#include <limits.h>
#include <errno.h>
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
  struct __group16 _grp;
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
  char _username[UNLEN + 1];

  /* net.cc */
  char *_ntoa_buf;
  struct protoent *_protoent_buf;
  struct servent *_servent_buf;
  struct hostent *_hostent_buf;
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

class nativeMutex
{
public:
  bool init ();
  bool lock ();
  void unlock ();
private:
  HANDLE theHandle;
};

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
#define PTHREAD_ONCE_MAGIC PTHREAD_MAGIC+8
#define PTHREAD_RWLOCK_MAGIC PTHREAD_MAGIC+9
#define PTHREAD_RWLOCKATTR_MAGIC PTHREAD_MAGIC+10

#define MUTEX_OWNER_ANONYMOUS        ((pthread_t) -1)

/* verifyable_object should not be defined here - it's a general purpose class */

class verifyable_object
{
public:
  long magic;

  verifyable_object (long);
  virtual ~verifyable_object ();
};

typedef enum
{
  VALID_OBJECT,
  INVALID_OBJECT,
  VALID_STATIC_OBJECT
} verifyable_object_state;

verifyable_object_state verifyable_object_isvalid (void const *, long);
verifyable_object_state verifyable_object_isvalid (void const *, long, void *);

/* interface */
template <class ListNode> class List {
public:
  List();
  void Insert (ListNode *aNode);
  ListNode *Remove ( ListNode *aNode);
  ListNode *Pop ();
  void forEach (void (*)(ListNode *aNode));
protected:
  ListNode *head;
};

class pthread_key:public verifyable_object
{
public:
  static bool isGoodObject (pthread_key_t const *);
  static void runAllDestructors ();

  DWORD dwTlsIndex;

  int set (const void *);
  void *get () const;

  pthread_key (void (*)(void *));
  ~pthread_key ();
  static void fixup_before_fork();
  static void fixup_after_fork();

  /* List support calls */
  class pthread_key *next;
private:
  // lists of objects. USE THREADSAFE INSERTS AND DELETES.
  static List<pthread_key> keys;
  static void saveAKey (pthread_key *);
  static void restoreAKey (pthread_key *);
  static void destroyAKey (pthread_key *);
  void saveKeyToBuffer ();
  void recreateKeyFromBuffer ();
  void (*destructor) (void *);
  void run_destructor ();
  void *fork_buf;
};

/* implementation */
template <class ListNode>
List<ListNode>::List<ListNode> () : head(NULL)
{
}
template <class ListNode> void
List<ListNode>::Insert (ListNode *aNode)
{
  if (!aNode)
  return;
  aNode->next = (ListNode *) InterlockedExchangePointer (&head, aNode);
}
template <class ListNode> ListNode *
List<ListNode>::Remove ( ListNode *aNode)
{
  if (!aNode)
  return NULL;
  if (!head)
  return NULL;
  if (aNode == head)
  return Pop ();
  ListNode *resultPrev = head;
  while (resultPrev && resultPrev->next && !(aNode == resultPrev->next))
  resultPrev = resultPrev->next;
  if (resultPrev)
  return (ListNode *)InterlockedExchangePointer (&resultPrev->next, resultPrev->next->next);
  return NULL;
}
template <class ListNode> ListNode *
List<ListNode>::Pop ()
{
  return (ListNode *) InterlockedExchangePointer (&head, head->next);
}
/* poor mans generic programming. */
template <class ListNode> void
List<ListNode>::forEach (void (*callback)(ListNode *))
{
  ListNode *aNode = head;
  while (aNode)
  {
    callback (aNode);
    aNode = aNode->next;
  }
}

class pthread_attr:public verifyable_object
{
public:
  static bool isGoodObject(pthread_attr_t const *);
  int joinable;
  int contentionscope;
  int inheritsched;
  struct sched_param schedparam;
  size_t stacksize;

  pthread_attr ();
  ~pthread_attr ();
};

class pthread_mutexattr:public verifyable_object
{
public:
  static bool isGoodObject(pthread_mutexattr_t const *);
  int pshared;
  int mutextype;
  pthread_mutexattr ();
  ~pthread_mutexattr ();
};

class pthread_mutex:public verifyable_object
{
public:
  static bool isGoodObject (pthread_mutex_t const *);
  static bool isGoodInitializer (pthread_mutex_t const *);
  static bool isGoodInitializerOrObject (pthread_mutex_t const *);
  static bool isGoodInitializerOrBadObject (pthread_mutex_t const *mutex);
  static bool canBeUnlocked (pthread_mutex_t const *mutex);
  static void initMutex ();
  static int init (pthread_mutex_t *, const pthread_mutexattr_t *);

  unsigned long lock_counter;
  HANDLE win32_obj_id;
  unsigned int recursion_counter;
  LONG condwaits;
  pthread_t owner;
  int type;
  int pshared;
  class pthread_mutex * next;

  pthread_t GetPthreadSelf () const
  {
    return PTHREAD_MUTEX_NORMAL == type ? MUTEX_OWNER_ANONYMOUS :
      ::pthread_self ();
  }

  int Lock ()
  {
    return _Lock (GetPthreadSelf ());
  }
  int TryLock ()
  {
    return _TryLock (GetPthreadSelf ());
  }
  int UnLock ()
  {
    return _UnLock (GetPthreadSelf ());
  }
  int Destroy ()
  {
    return _Destroy (GetPthreadSelf ());
  }

  void SetOwner (pthread_t self)
  {
    recursion_counter = 1;
    owner = self;
  }

  int LockRecursive ()
  {
    if (UINT_MAX == recursion_counter)
      return EAGAIN;
    ++recursion_counter;
    return 0;
  }

  void fixup_after_fork ();

  pthread_mutex (pthread_mutexattr * = NULL);
  pthread_mutex (pthread_mutex_t *, pthread_mutexattr *);
  ~pthread_mutex ();

private:
  int _Lock (pthread_t self);
  int _TryLock (pthread_t self);
  int _UnLock (pthread_t self);
  int _Destroy (pthread_t self);

  static nativeMutex mutexInitializationLock;
};

#define WAIT_CANCELED   (WAIT_OBJECT_0 + 1)

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
  HANDLE cancel_event;
  pthread_t joiner;
  // int joinable;

  /* signal handling */
  struct sigaction *sigs;
  sigset_t *sigmask;
  LONG *sigtodo;
  virtual void create (void *(*)(void *), pthread_attr *, void *);

  pthread ();
  virtual ~pthread ();

  static void initMainThread (bool);
  static bool isGoodObject(pthread_t const *);
  static void atforkprepare();
  static void atforkparent();
  static void atforkchild();

  /* API calls */
  static int cancel (pthread_t);
  static int join (pthread_t * thread, void **return_val);
  static int detach (pthread_t * thread);
  static int create (pthread_t * thread, const pthread_attr_t * attr,
			      void *(*start_routine) (void *), void *arg);
  static int once (pthread_once_t *, void (*)(void));
  static int atfork(void (*)(void), void (*)(void), void (*)(void));
  static int suspend (pthread_t * thread);
  static int resume (pthread_t * thread);

  virtual void exit (void *value_ptr);

  virtual int cancel ();

  virtual void testcancel ();
  static void static_cancel_self ();

  static DWORD cancelable_wait (HANDLE object, DWORD timeout, const bool do_cancel = true);

  virtual int setcancelstate (int state, int *oldstate);
  virtual int setcanceltype (int type, int *oldtype);

  virtual void push_cleanup_handler (__pthread_cleanup_handler *handler);
  virtual void pop_cleanup_handler (int const execute);

  static pthread* self ();
  static void *thread_init_wrapper (void *);

  virtual unsigned long getsequence_np();

private:
  DWORD thread_id;
  __pthread_cleanup_handler *cleanup_stack;
  pthread_mutex mutex;

  void pop_all_cleanup_handlers (void);
  void precreate (pthread_attr *);
  void postcreate ();
  void setThreadIdtoCurrent ();
  static void setTlsSelfPointer (pthread *);
  static pthread *getTlsSelfPointer ();
  void cancel_self ();
  DWORD getThreadId ();
  void initCurrentThread ();
};

class pthreadNull : public pthread
{
  public:
  static pthread *getNullpthread();
  ~pthreadNull();

  /* From pthread These should never get called
  * as the ojbect is not verifyable
  */
  void create (void *(*)(void *), pthread_attr *, void *);
  void exit (void *value_ptr);
  int cancel ();
  void testcancel ();
  int setcancelstate (int state, int *oldstate);
  int setcanceltype (int type, int *oldtype);
  void push_cleanup_handler (__pthread_cleanup_handler *handler);
  void pop_cleanup_handler (int const execute);
  unsigned long getsequence_np();

  private:
  pthreadNull ();
  static pthreadNull _instance;
};

class pthread_condattr:public verifyable_object
{
public:
  static bool isGoodObject(pthread_condattr_t const *);
  int shared;

  pthread_condattr ();
  ~pthread_condattr ();
};

class pthread_cond:public verifyable_object
{
public:
  static bool isGoodObject (pthread_cond_t const *);
  static bool isGoodInitializer (pthread_cond_t const *);
  static bool isGoodInitializerOrObject (pthread_cond_t const *);
  static bool isGoodInitializerOrBadObject (pthread_cond_t const *);
  static void initMutex ();
  static int init (pthread_cond_t *, const pthread_condattr_t *);

  int shared;

  unsigned long waiting;
  unsigned long pending;
  HANDLE semWait;

  pthread_mutex mtxIn;
  pthread_mutex mtxOut;

  pthread_mutex_t mtxCond;

  class pthread_cond * next;

  void UnBlock (const bool all);
  int Wait (pthread_mutex_t mutex, DWORD dwMilliseconds = INFINITE);
  void fixup_after_fork ();

  pthread_cond (pthread_condattr *);
  ~pthread_cond ();

private:
  static nativeMutex condInitializationLock;
};

class pthread_rwlockattr:public verifyable_object
{
public:
  static bool isGoodObject(pthread_rwlockattr_t const *);
  int shared;

  pthread_rwlockattr ();
  ~pthread_rwlockattr ();
};

class pthread_rwlock:public verifyable_object
{
public:
  static bool isGoodObject (pthread_rwlock_t const *);
  static bool isGoodInitializer (pthread_rwlock_t const *);
  static bool isGoodInitializerOrObject (pthread_rwlock_t const *);
  static bool isGoodInitializerOrBadObject (pthread_rwlock_t const *);
  static void initMutex ();
  static int init (pthread_rwlock_t *, const pthread_rwlockattr_t *);

  int shared;

  unsigned long waitingReaders;
  unsigned long waitingWriters;
  pthread_t writer;
  struct RWLOCK_READER
  {
    struct RWLOCK_READER *next;
    pthread_t thread;
  } *readers;

  int RdLock ();
  int TryRdLock ();

  int WrLock ();
  int TryWrLock ();

  int UnLock ();

  pthread_mutex mtx;
  pthread_cond condReaders;
  pthread_cond condWriters;

  class pthread_rwlock * next;

  void fixup_after_fork ();

  pthread_rwlock (pthread_rwlockattr *);
  ~pthread_rwlock ();

private:
  void addReader (struct RWLOCK_READER *rd);
  void removeReader (struct RWLOCK_READER *rd);
  struct RWLOCK_READER *lookupReader (pthread_t thread);

  static void RdLockCleanup (void *arg);
  static void WrLockCleanup (void *arg);

  static nativeMutex rwlockInitializationLock;
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
  static bool isGoodObject(sem_t const *);
  /* API calls */
  static int init (sem_t * sem, int pshared, unsigned int value);
  static int destroy (sem_t * sem);
  static int wait (sem_t * sem);
  static int trywait (sem_t * sem);
  static int post (sem_t * sem);

  HANDLE win32_obj_id;
  class semaphore * next;
  int shared;
  long currentvalue;
  void Wait ();
  void Post ();
  int TryWait ();
  void fixup_after_fork ();

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
  int concurrency;
  long int threadcount;

  // Used for main thread data, and sigproc thread
  struct __reent_t reents;
  struct _winsup_t winsup_reent;

  callback *pthread_prepare;
  callback *pthread_child;
  callback *pthread_parent;

  // lists of pthread objects. USE THREADSAFE INSERTS AND DELETES.
  class pthread_mutex * mutexs;
  class pthread_cond  * conds;
  class pthread_rwlock * rwlocks;
  class semaphore     * semaphores;

  pthread_key reent_key;
  pthread_key thread_self_key;

  void Init (int);
  void fixup_before_fork (void);
  void fixup_after_fork (void);

  MTinterface () :
    concurrency (0), threadcount (1),
    pthread_prepare (NULL), pthread_child (NULL), pthread_parent (NULL),
    mutexs (NULL), conds (NULL), rwlocks (NULL), semaphores (NULL),
    reent_key (NULL), thread_self_key (NULL)
  {
  }
};

#define MT_INTERFACE user_data->threadinterface

#endif // MT_SAFE

#endif // _CYGNUS_THREADS_
