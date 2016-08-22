// -*- C++ -*-
/* thread.h: Locking and threading module definitions

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

#pragma once

#define LOCK_MMAP_LIST   1

#define WRITE_LOCK 1
#define READ_LOCK  2

/* resource.cc */
extern size_t get_rlimit_stack (void);

#include <pthread.h>
#include <limits.h>
#include "security.h"
#include <errno.h>
#include "cygerrno.h"
#include "cygwait.h"

class fast_mutex
{
public:
  fast_mutex () :
    lock_counter (0), win32_obj_id (0)
  {
  }

  ~fast_mutex ()
  {
    if(win32_obj_id)
      CloseHandle (win32_obj_id);
  }

  bool init ()
  {
    lock_counter = 0;
    win32_obj_id = ::CreateEvent (&sec_none_nih, false, false, NULL);
    if (!win32_obj_id)
      {
	debug_printf ("CreateEvent failed. %E");
	return false;
      }
    return true;
  }

  void lock ()
  {
    if (InterlockedIncrement (&lock_counter) != 1)
      cygwait (win32_obj_id, cw_infinite, cw_sig | cw_sig_restart);
  }

  void unlock ()
  {
    if (InterlockedDecrement (&lock_counter))
      ::SetEvent (win32_obj_id);
  }

private:
  LONG lock_counter;
  HANDLE win32_obj_id;
};

class per_process;
class pinfo;

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
#define PTHREAD_SPINLOCK_MAGIC PTHREAD_MAGIC+11
#define PTHREAD_BARRIER_MAGIC PTHREAD_MAGIC+12
#define PTHREAD_BARRIERATTR_MAGIC PTHREAD_MAGIC+13

#define MUTEX_OWNER_ANONYMOUS ((pthread_t) -1)

typedef uint32_t thread_magic_t;

/* verifyable_object should not be defined here - it's a general purpose class */

class verifyable_object
{
public:
  thread_magic_t magic;

  verifyable_object (thread_magic_t verifyer): magic (verifyer) {}
  virtual ~verifyable_object () { magic = 0; }
};

typedef enum
{
  VALID_OBJECT,
  INVALID_OBJECT,
  VALID_STATIC_OBJECT
} verifyable_object_state;

template <class list_node> inline void
List_insert (list_node *&head, list_node *node)
{
  if (!node)
    return;
  do
    node->next = head;
  while (InterlockedCompareExchangePointer ((PVOID volatile *) &head,
					    node, node->next) != node->next);
}

template <class list_node> inline void
List_remove (fast_mutex &mx, list_node *&head, list_node *node)
{
  if (!node)
    return;
  mx.lock ();
  if (head)
    {
      if (InterlockedCompareExchangePointer ((PVOID volatile *) &head,
					     node->next, node) != node)
	{
	  list_node *cur = head;

	  while (cur->next && node != cur->next)
	    cur = cur->next;
	  if (node == cur->next)
	    cur->next = cur->next->next;
	}
    }
  mx.unlock ();
}


template <class list_node> class List
{
 public:
  List() : head(NULL)
  {
    mx_init ();
  }

  ~List()
  {
  }

  void fixup_after_fork ()
  {
    mx_init ();
  }

  void insert (list_node *node)
  {
    List_insert (head, node);
  }

  void remove (list_node *node)
  {
    List_remove (mx, head, node);
  }

  void for_each (void (list_node::*callback) ())
  {
    mx.lock ();
    list_node *cur = head;
    while (cur)
      {
	(cur->*callback) ();
	cur = cur->next;
      }
    mx.unlock ();
  }

  fast_mutex mx;
  list_node *head;

protected:
  void mx_init ()
  {
    if (!mx.init ())
      api_fatal ("Could not create mutex for list synchronisation.");
  }
};

class pthread_key: public verifyable_object
{
  DWORD tls_index;
public:
  static bool is_good_object (pthread_key_t const *);

  int set (const void *value) {TlsSetValue (tls_index, (void *) value); return 0;}
  void *get () const {return TlsGetValue (tls_index);}

  pthread_key (void (*)(void *));
  ~pthread_key ();
  static void fixup_before_fork ()
  {
    keys.for_each (&pthread_key::_fixup_before_fork);
  }

  static void fixup_after_fork ()
  {
    keys.fixup_after_fork ();
    keys.for_each (&pthread_key::_fixup_after_fork);
  }

  static void run_all_destructors ()
  {
    keys.for_each (&pthread_key::run_destructor);
  }

  /* List support calls */
  class pthread_key *next;
private:
  static List<pthread_key> keys;
  void _fixup_before_fork ();
  void _fixup_after_fork ();
  void (*destructor) (void *);
  void run_destructor ();
  void *fork_buf;
};

class pthread_attr: public verifyable_object
{
public:
  static bool is_good_object(pthread_attr_t const *);
  int joinable;
  int contentionscope;
  int inheritsched;
  struct sched_param schedparam;
  void *stackaddr;
  size_t stacksize;
  size_t guardsize;
  char *name;

  pthread_attr ();
  ~pthread_attr ();
};

class pthread_mutexattr: public verifyable_object
{
public:
  static bool is_good_object(pthread_mutexattr_t const *);
  int pshared;
  int mutextype;
  pthread_mutexattr ();
  ~pthread_mutexattr ();
};

class pthread_mutex: public verifyable_object
{
public:
  static void init_mutex ();
  static int init (pthread_mutex_t *, const pthread_mutexattr_t *attr,
		   const pthread_mutex_t);
  static bool is_good_object (pthread_mutex_t const *);
  static bool is_initializer (pthread_mutex_t const *);
  static bool is_initializer_or_object (pthread_mutex_t const *);
  static bool is_initializer_or_bad_object (pthread_mutex_t const *);

  int lock ();
  int trylock ();
  int unlock ();
  int destroy ();
  void set_type (int in_type) {type = in_type;}

  int lock_recursive ()
  {
    if (recursion_counter == UINT_MAX)
      return EAGAIN;
    recursion_counter++;
    return 0;
  }

  bool can_be_unlocked ();

  pthread_mutex (pthread_mutexattr * = NULL);
  pthread_mutex (pthread_mutex_t *, pthread_mutexattr *);
  ~pthread_mutex ();

  class pthread_mutex *next;
  static void fixup_after_fork ()
  {
    mutexes.fixup_after_fork ();
    mutexes.for_each (&pthread_mutex::_fixup_after_fork);
  }

protected:
  LONG lock_counter;
  HANDLE win32_obj_id;
  pthread_t owner;
#ifdef DEBUGGING
  DWORD tid;		/* the thread id of the owner */
#endif

  void set_shared (int in_shared) { pshared = in_shared; }
  void set_owner (pthread_t self)
  {
    recursion_counter = 1;
    owner = self;
#ifdef DEBUGGING
    tid = GetCurrentThreadId ();
#endif
  }

  static const pthread_t _new_mutex;
  static const pthread_t _unlocked_mutex;
  static const pthread_t _destroyed_mutex;

private:
  unsigned int recursion_counter;
  LONG condwaits;
  int type;
  int pshared;

  bool no_owner ();
  void _fixup_after_fork ();

  static List<pthread_mutex> mutexes;
  static fast_mutex mutex_initialization_lock;
  friend class pthread_cond;
};

class pthread_spinlock: public pthread_mutex
{
public:
  static bool is_good_object (pthread_spinlock_t const *);
  static int init (pthread_spinlock_t *, int);

  int lock ();
  int unlock ();

  pthread_spinlock (int);
};

class _cygtls;
class pthread: public verifyable_object
{
public:
  HANDLE win32_obj_id;
  class pthread_attr attr;
  void *(*function) (void *);
  void *arg;
  void *return_ptr;
  bool valid;
  bool suspended;
  bool canceled;
  int cancelstate, canceltype;
  _cygtls *cygtls;
  HANDLE cancel_event;
  pthread_t joiner;

  virtual bool create (void *(*)(void *), pthread_attr *, void *);

  pthread ();
  virtual ~pthread ();

  static void init_mainthread ();
  static bool is_good_object(pthread_t const *);
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

  virtual void exit (void *value_ptr) __attribute__ ((noreturn));

  virtual int cancel ();

  virtual void testcancel ();
  static HANDLE get_cancel_event ();
  static void static_cancel_self () __attribute__ ((noreturn));

  virtual int setcancelstate (int state, int *oldstate);
  virtual int setcanceltype (int type, int *oldtype);

  virtual void push_cleanup_handler (__pthread_cleanup_handler *handler);
  virtual void pop_cleanup_handler (int const execute);

  static pthread* self ();
  static DWORD WINAPI thread_init_wrapper (void *);

  virtual unsigned long getsequence_np();

  static int equal (pthread_t t1, pthread_t t2)
  {
    return t1 == t2;
  }

  /* List support calls */
  class pthread *next;
  static void fixup_after_fork ()
  {
    threads.fixup_after_fork ();
    threads.for_each (&pthread::_fixup_after_fork);
  }

  static void suspend_all_except_self ()
  {
    threads.for_each (&pthread::suspend_except_self);
  }

  static void resume_all ()
  {
    threads.for_each (&pthread::resume);
  }

private:
  static List<pthread> threads;
  DWORD thread_id;
  __pthread_cleanup_handler *cleanup_stack;
  pthread_mutex mutex;
  sigset_t parent_sigmask;

  void suspend_except_self ();
  void resume ();

  void _fixup_after_fork ();

  void pop_all_cleanup_handlers ();
  void precreate (pthread_attr *);
  void postcreate ();
  bool create_cancel_event ();
  void set_tls_self_pointer ();
  void cancel_self () __attribute__ ((noreturn));
  DWORD get_thread_id ();
};

class pthread_null : public pthread
{
  public:
  static pthread *get_null_pthread();
  ~pthread_null();

  /* From pthread These should never get called
  * as the ojbect is not verifyable
  */
  bool create (void *(*)(void *), pthread_attr *, void *);
  void exit (void *value_ptr) __attribute__ ((noreturn));
  int cancel ();
  void testcancel ();
  int setcancelstate (int state, int *oldstate);
  int setcanceltype (int type, int *oldtype);
  void push_cleanup_handler (__pthread_cleanup_handler *handler);
  void pop_cleanup_handler (int const execute);
  unsigned long getsequence_np();

  private:
  pthread_null ();
  static pthread_null _instance;
};

class pthread_condattr: public verifyable_object
{
public:
  static bool is_good_object(pthread_condattr_t const *);
  int shared;
  clockid_t clock_id;

  pthread_condattr ();
  ~pthread_condattr ();
};

class pthread_cond: public verifyable_object
{
public:
  static bool is_good_object (pthread_cond_t const *);
  static bool is_initializer (pthread_cond_t const *);
  static bool is_initializer_or_object (pthread_cond_t const *);
  static bool is_initializer_or_bad_object (pthread_cond_t const *);
  static void init_mutex ();
  static int init (pthread_cond_t *, const pthread_condattr_t *);

  int shared;
  clockid_t clock_id;

  LONG waiting;
  LONG pending;
  HANDLE sem_wait;

  pthread_mutex mtx_in;
  pthread_mutex mtx_out;

  pthread_mutex_t mtx_cond;

  void unblock (const bool all);
  int wait (pthread_mutex_t mutex, PLARGE_INTEGER timeout = NULL);

  pthread_cond (pthread_condattr *);
  ~pthread_cond ();

  class pthread_cond * next;
  static void fixup_after_fork ()
  {
    conds.fixup_after_fork ();
    conds.for_each (&pthread_cond::_fixup_after_fork);
  }

private:
  void _fixup_after_fork ();

  static List<pthread_cond> conds;
  static fast_mutex cond_initialization_lock;
};


class pthread_barrierattr: public verifyable_object
{
public:
  static bool is_good_object(pthread_barrierattr_t const *);
  int shared;

  pthread_barrierattr ();
  ~pthread_barrierattr ();
};


class pthread_barrier: public verifyable_object
{
public:
  static bool is_good_object(pthread_barrier_t const *);

  pthread_mutex_t mtx; /* Mutex protecting everything below. */
  pthread_cond_t cond; /* Conditional variable to wait on. */
  unsigned cnt; /* Barrier count. Threads to wait for. */
  uint64_t cyc; /* Cycle count. */
  unsigned wt; /* Already waiting threads count. */

  int init (const pthread_barrierattr_t *, unsigned);
  int wait();
  int destroy ();

  pthread_barrier ();
  ~pthread_barrier ();
};


class pthread_rwlockattr: public verifyable_object
{
public:
  static bool is_good_object(pthread_rwlockattr_t const *);
  int shared;

  pthread_rwlockattr ();
  ~pthread_rwlockattr ();
};

class pthread_rwlock: public verifyable_object
{
public:
  static bool is_good_object (pthread_rwlock_t const *);
  static bool is_initializer (pthread_rwlock_t const *);
  static bool is_initializer_or_object (pthread_rwlock_t const *);
  static bool is_initializer_or_bad_object (pthread_rwlock_t const *);
  static void init_mutex ();
  static int init (pthread_rwlock_t *, const pthread_rwlockattr_t *);

  int shared;

  uint32_t waiting_readers;
  uint32_t waiting_writers;
  pthread_t writer;
  struct RWLOCK_READER
  {
    struct RWLOCK_READER *next;
    pthread_t thread;
    uint32_t n;
    RWLOCK_READER (): next (NULL), thread (pthread::self ()), n (0) {}
  } *readers;
  fast_mutex readers_mx;

  int rdlock ();
  int tryrdlock ();

  int wrlock ();
  int trywrlock ();

  int unlock ();

  pthread_mutex mtx;
  pthread_cond cond_readers;
  pthread_cond cond_writers;

  pthread_rwlock (pthread_rwlockattr *);
  ~pthread_rwlock ();

  class pthread_rwlock * next;
  static void fixup_after_fork ()
  {
    rwlocks.fixup_after_fork ();
    rwlocks.for_each (&pthread_rwlock::_fixup_after_fork);
  }

private:
  static List<pthread_rwlock> rwlocks;

  RWLOCK_READER *add_reader ();
  void remove_reader (struct RWLOCK_READER *rd);
  struct RWLOCK_READER *lookup_reader ();

  void release ()
  {
    if (waiting_writers)
      {
	if (!readers)
	  cond_writers.unblock (false);
      }
    else if (waiting_readers)
      cond_readers.unblock (true);
  }


  static void rdlock_cleanup (void *arg);
  static void wrlock_cleanup (void *arg);

  void _fixup_after_fork ();

  static fast_mutex rwlock_initialization_lock;
};

class pthread_once
{
public:
  pthread_mutex_t mutex;
  int state;
};

/* shouldn't be here */
class semaphore: public verifyable_object
{
public:
  static bool is_good_object(sem_t const *);
  /* API calls */
  static int init (sem_t *sem, int pshared, unsigned int value);
  static int destroy (sem_t *sem);
  static sem_t *open (unsigned long long hash, LUID luid, int fd, int oflag,
		      mode_t mode, unsigned int value, bool &wasopen);
  static int close (sem_t *sem);
  static int wait (sem_t *sem);
  static int post (sem_t *sem);
  static int getvalue (sem_t *sem, int *sval);
  static int trywait (sem_t *sem);
  static int timedwait (sem_t *sem, const struct timespec *abstime);

  static int getinternal (sem_t *sem, int *sfd, unsigned long long *shash,
			  LUID *sluid, unsigned int *sval);

  HANDLE win32_obj_id;
  int shared;
  LONG currentvalue;
  LONG startvalue;
  int fd;
  unsigned long long hash;
  LUID luid;
  sem_t *sem;

  semaphore (int, unsigned int);
  semaphore (unsigned long long, LUID, int, sem_t *, int, mode_t, unsigned int);
  ~semaphore ();

  class semaphore * next;
  static void fixup_before_fork ()
  {
    semaphores.for_each (&semaphore::_fixup_before_fork);
  }
  static void fixup_after_fork ()
  {
    semaphores.fixup_after_fork ();
    semaphores.for_each (&semaphore::_fixup_after_fork);
  }
  static void terminate ()
  {
    save_errno save;
    semaphores.for_each (&semaphore::_terminate);
  }

private:
  int _wait ();
  void _post ();
  int _getvalue (int *sval);
  int _trywait ();
  int _timedwait (const struct timespec *abstime);

  void _fixup_before_fork ();
  void _fixup_after_fork ();
  void _terminate ();

  static List<semaphore> semaphores;
};

class callback
{
public:
  void (*cb)(void);
  class callback * next;
};

struct MTinterface
{
  // General
  int concurrency;
  LONG threadcount;

  callback *pthread_prepare;
  callback *pthread_child;
  callback *pthread_parent;

  void Init ();
  void fixup_before_fork ();
  void fixup_after_fork ();

#if 0 // avoid initialization since zero is implied and
  MTinterface () :
    concurrency (0), threadcount (0),
    pthread_prepare (NULL), pthread_child (NULL), pthread_parent (NULL)
  {
  }
#endif
};

#define MT_INTERFACE user_data->threadinterface
