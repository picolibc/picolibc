/* thread.cc: Locking and threading module functions

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

/* Implementation overview and caveats:

   Win32 puts some contraints on what can and cannot be implemented.  Where
   possible we work around those contrainsts.  Where we cannot work around
   the constraints we either pretend to be conformant, or return an error
   code.

   Some caveats: PROCESS_SHARED objects, while they pretend to be process
   shared, may not actually work.  Some test cases are needed to determine
   win32's behaviour.  My suspicion is that the win32 handle needs to be
   opened with different flags for proper operation.

   R.Collins, April 2001.  */

#include "winsup.h"
#include "miscfuncs.h"
#include "path.h"
#include <sched.h>
#include <stdlib.h>
#include "sigproc.h"
#include "fhandler.h"
#include "dtable.h"
#include "cygheap.h"
#include "ntdll.h"
#include "cygwait.h"
#include "exception.h"

extern "C" void __fp_lock_all ();
extern "C" void __fp_unlock_all ();
extern "C" bool valid_sched_parameters(const struct sched_param *);
extern "C" int sched_get_thread_priority(HANDLE thread);
extern "C" int sched_set_thread_priority(HANDLE thread, int priority);
static inline verifyable_object_state
  verifyable_object_isvalid (void const * objectptr, thread_magic_t magic,
			     void *static_ptr1 = NULL,
			     void *static_ptr2 = NULL,
			     void *static_ptr3 = NULL);

extern int threadsafe;

const pthread_t pthread_mutex::_new_mutex = (pthread_t) 1;
const pthread_t pthread_mutex::_unlocked_mutex = (pthread_t) 2;
const pthread_t pthread_mutex::_destroyed_mutex = (pthread_t) 3;


template <typename T>
static inline
void
delete_and_clear (T * * const ptr)
{
  delete *ptr;
  *ptr = 0;
}


inline bool
pthread_mutex::no_owner()
{
    int res;
    if (!owner)
      {
	debug_printf ("NULL owner value");
	res = 1;
      }
    else if (owner == _destroyed_mutex)
      {
	paranoid_printf ("attempt to use destroyed mutex");
	res = 1;
      }
    else if (owner == _new_mutex || owner == _unlocked_mutex)
      res = 1;
    else
      res = 0;
    return res;
}

#undef __getreent
extern "C" struct _reent *
__getreent ()
{
  return &_my_tls.local_clib;
}

extern "C" void
__cygwin_lock_init (_LOCK_T *lock)
{
  *lock = _LOCK_T_INITIALIZER;
}

extern "C" void
__cygwin_lock_init_recursive (_LOCK_T *lock)
{
  *lock = _LOCK_T_RECURSIVE_INITIALIZER;
}

extern "C" void
__cygwin_lock_fini (_LOCK_T *lock)
{
  pthread_mutex_destroy ((pthread_mutex_t*) lock);
}

extern "C" void
__cygwin_lock_lock (_LOCK_T *lock)
{
  paranoid_printf ("threadcount %d.  locking", MT_INTERFACE->threadcount);
  pthread_mutex_lock ((pthread_mutex_t*) lock);
}

extern "C" int
__cygwin_lock_trylock (_LOCK_T *lock)
{
  return pthread_mutex_trylock ((pthread_mutex_t*) lock);
}


extern "C" void
__cygwin_lock_unlock (_LOCK_T *lock)
{
  pthread_mutex_unlock ((pthread_mutex_t*) lock);
  paranoid_printf ("threadcount %d.  unlocked", MT_INTERFACE->threadcount);
}

static inline verifyable_object_state
verifyable_object_isvalid (void const *objectptr, thread_magic_t magic, void *static_ptr1,
			   void *static_ptr2, void *static_ptr3)
{
  verifyable_object_state state = INVALID_OBJECT;

  __try
    {
      if (!objectptr || !(*(const char **) objectptr))
	__leave;

      verifyable_object **object = (verifyable_object **) objectptr;

      if ((static_ptr1 && *object == static_ptr1) ||
	  (static_ptr2 && *object == static_ptr2) ||
	  (static_ptr3 && *object == static_ptr3))
	state = VALID_STATIC_OBJECT;
      else if ((*object)->magic == magic)
	state = VALID_OBJECT;
    }
  __except (NO_ERROR) {}
  __endtry
  return state;
}

/* static members */
inline bool
pthread_attr::is_good_object (pthread_attr_t const *attr)
{
  if (verifyable_object_isvalid (attr, PTHREAD_ATTR_MAGIC) != VALID_OBJECT)
    return false;
  return true;
}

inline bool
pthread_condattr::is_good_object (pthread_condattr_t const *attr)
{
  if (verifyable_object_isvalid (attr, PTHREAD_CONDATTR_MAGIC) != VALID_OBJECT)
    return false;
  return true;
}

inline bool
pthread_rwlockattr::is_good_object (pthread_rwlockattr_t const *attr)
{
  if (verifyable_object_isvalid (attr, PTHREAD_RWLOCKATTR_MAGIC) != VALID_OBJECT)
    return false;
  return true;
}

inline bool
pthread_key::is_good_object (pthread_key_t const *key)
{
  if (verifyable_object_isvalid (key, PTHREAD_KEY_MAGIC) != VALID_OBJECT)
    return false;
  return true;
}

inline bool
pthread_spinlock::is_good_object (pthread_spinlock_t const *mutex)
{
  if (verifyable_object_isvalid (mutex, PTHREAD_SPINLOCK_MAGIC) != VALID_OBJECT)
    return false;
  return true;
}

inline bool
pthread_mutex::is_good_object (pthread_mutex_t const *mutex)
{
  if (verifyable_object_isvalid (mutex, PTHREAD_MUTEX_MAGIC) != VALID_OBJECT)
    return false;
  return true;
}

inline bool
pthread_mutex::is_initializer (pthread_mutex_t const *mutex)
{
  if (verifyable_object_isvalid (mutex, PTHREAD_MUTEX_MAGIC,
				 PTHREAD_RECURSIVE_MUTEX_INITIALIZER_NP,
				 PTHREAD_NORMAL_MUTEX_INITIALIZER_NP,
				 PTHREAD_ERRORCHECK_MUTEX_INITIALIZER_NP) != VALID_STATIC_OBJECT)
    return false;
  return true;
}

inline bool
pthread_mutex::is_initializer_or_object (pthread_mutex_t const *mutex)
{
  if (verifyable_object_isvalid (mutex, PTHREAD_MUTEX_MAGIC,
				 PTHREAD_RECURSIVE_MUTEX_INITIALIZER_NP,
				 PTHREAD_NORMAL_MUTEX_INITIALIZER_NP,
				 PTHREAD_ERRORCHECK_MUTEX_INITIALIZER_NP) == INVALID_OBJECT)
    return false;
  return true;
}

/* FIXME: Accommodate PTHREAD_MUTEX_ERRORCHECK */
inline bool
pthread_mutex::can_be_unlocked ()
{
  pthread_t self = pthread::self ();
  /* Check if the mutex is owned by the current thread and can be unlocked.
   * Also check for the ANONYMOUS owner to cover NORMAL mutexes as well. */
  bool res = type == PTHREAD_MUTEX_NORMAL || no_owner ()
	     || (recursion_counter == 1 && pthread::equal (owner, self));
  pthread_printf ("recursion_counter %u res %d", recursion_counter, res);
  return res;
}

inline bool
pthread_mutexattr::is_good_object (pthread_mutexattr_t const * attr)
{
  if (verifyable_object_isvalid (attr, PTHREAD_MUTEXATTR_MAGIC) != VALID_OBJECT)
    return false;
  return true;
}

inline bool __attribute__ ((used))
pthread::is_good_object (pthread_t const *thread)
{
  if (verifyable_object_isvalid (thread, PTHREAD_MAGIC) != VALID_OBJECT)
    return false;
  return true;
}

/* Thread synchronisation */
inline bool
pthread_cond::is_good_object (pthread_cond_t const *cond)
{
  if (verifyable_object_isvalid (cond, PTHREAD_COND_MAGIC) != VALID_OBJECT)
    return false;
  return true;
}

inline bool
pthread_cond::is_initializer (pthread_cond_t const *cond)
{
  if (verifyable_object_isvalid (cond, PTHREAD_COND_MAGIC, PTHREAD_COND_INITIALIZER) != VALID_STATIC_OBJECT)
    return false;
  return true;
}

inline bool
pthread_cond::is_initializer_or_object (pthread_cond_t const *cond)
{
  if (verifyable_object_isvalid (cond, PTHREAD_COND_MAGIC, PTHREAD_COND_INITIALIZER) == INVALID_OBJECT)
    return false;
  return true;
}

inline bool
pthread_barrierattr::is_good_object (pthread_barrierattr_t const *cond)
{
  if (verifyable_object_isvalid (cond, PTHREAD_BARRIERATTR_MAGIC)
      != VALID_OBJECT)
    return false;
  return true;
}

inline bool
pthread_barrier::is_good_object (pthread_barrier_t const *cond)
{
  if (verifyable_object_isvalid (cond, PTHREAD_BARRIER_MAGIC) != VALID_OBJECT)
    return false;
  return true;
}

/* RW locks */
inline bool
pthread_rwlock::is_good_object (pthread_rwlock_t const *rwlock)
{
  if (verifyable_object_isvalid (rwlock, PTHREAD_RWLOCK_MAGIC) != VALID_OBJECT)
    return false;
  return true;
}

inline bool
pthread_rwlock::is_initializer (pthread_rwlock_t const *rwlock)
{
  if (verifyable_object_isvalid (rwlock, PTHREAD_RWLOCK_MAGIC, PTHREAD_RWLOCK_INITIALIZER) != VALID_STATIC_OBJECT)
    return false;
  return true;
}

inline bool
pthread_rwlock::is_initializer_or_object (pthread_rwlock_t const *rwlock)
{
  if (verifyable_object_isvalid (rwlock, PTHREAD_RWLOCK_MAGIC, PTHREAD_RWLOCK_INITIALIZER) == INVALID_OBJECT)
    return false;
  return true;
}

inline bool
semaphore::is_good_object (sem_t const * sem)
{
  if (verifyable_object_isvalid (sem, SEM_MAGIC) != VALID_OBJECT)
    return false;
  return true;
}

void
MTinterface::Init ()
{
  pthread_mutex::init_mutex ();
  pthread_cond::init_mutex ();
  pthread_rwlock::init_mutex ();
}

void
MTinterface::fixup_before_fork ()
{
  pthread_key::fixup_before_fork ();
  semaphore::fixup_before_fork ();
}

/* This function is called from a single threaded process */
void
MTinterface::fixup_after_fork ()
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
  pthread *thread = _my_tls.tid;
  if (!thread)
    {
      thread = new pthread ();
      if (!thread)
	api_fatal ("failed to create mainthread object");
    }

  thread->set_tls_self_pointer ();
  thread->thread_id = GetCurrentThreadId ();
  if (!DuplicateHandle (GetCurrentProcess (), GetCurrentThread (),
			GetCurrentProcess (), &thread->win32_obj_id,
			0, FALSE, DUPLICATE_SAME_ACCESS))
    api_fatal ("failed to create mainthread handle");
  if (!thread->create_cancel_event ())
    api_fatal ("couldn't create cancel event for main thread");
  VerifyHandle (thread->win32_obj_id);
  /* Make sure the pthread mutex is recursive.  See comment in
     pthread::precreate (called only for subsequent pthreads)
     for a description. */
  thread->mutex.set_type (PTHREAD_MUTEX_RECURSIVE);
  thread->postcreate ();
}

pthread *
pthread::self ()
{
  pthread *thread = _my_tls.tid;
  if (!thread)
    {
      thread = pthread_null::get_null_pthread ();
      thread->set_tls_self_pointer ();
    }
  return thread;
}

void
pthread::set_tls_self_pointer ()
{
  cygtls = &_my_tls;
  _my_tls.tid = this;
}

List<pthread> pthread::threads;

/* member methods */
pthread::pthread ():verifyable_object (PTHREAD_MAGIC), win32_obj_id (0),
		    valid (false), suspended (false), canceled (false),
		    cancelstate (0), canceltype (0), cancel_event (0),
		    joiner (NULL), next (NULL), cleanup_stack (NULL)
{
  if (this != pthread_null::get_null_pthread ())
    threads.insert (this);
  sigprocmask (SIG_SETMASK, NULL, &parent_sigmask);
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
  cancel_event = ::CreateEvent (&sec_none_nih, true, false, NULL);
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
      attr.stackaddr = newattr->stackaddr;
      attr.stacksize = newattr->stacksize;
      attr.guardsize = newattr->guardsize;
    }

  if (!pthread_mutex::is_good_object (&verifyable_mutex_obj))
    {
      thread_printf ("New thread object access mutex is not valid. this %p",
		     this);
      magic = 0;
      return;
    }
  /* This mutex MUST be recursive.  Consider the following scenario:
     - The thread installs a cleanup handler.
     - The cleanup handler calls a function which itself installs a
       cleanup handler.
     - pthread_cancel is called for this thread.
     - The thread's cleanup handler is called under mutex lock condition.
     - The cleanup handler calls the subsequent function with cleanup handler.
     - The function runs to completion, so it calls pthread_cleanup_pop.
     - pthread_cleanup_pop calls pthread::pop_cleanup_handler which will again
       try to lock the mutex.
     - Deadlock. */
  mutex.set_type (PTHREAD_MUTEX_RECURSIVE);
  if (!create_cancel_event ())
    magic = 0;
}

bool
pthread::create (void *(*func) (void *), pthread_attr *newattr,
		 void *threadarg)
{
  bool retval;

  precreate (newattr);
  if (!magic)
    return false;

  function = func;
  arg = threadarg;

  mutex.lock ();
  /* stackaddr holds the uppermost stack address.  See the comments in
     pthread_attr_setstack and pthread_attr_setstackaddr for a description. */
  ULONG stacksize = attr.stacksize ?: get_rlimit_stack ();
  PVOID stackaddr = attr.stackaddr ? ((caddr_t) attr.stackaddr - stacksize)
				   : NULL;
  win32_obj_id = CygwinCreateThread (thread_init_wrapper, this, stackaddr,
				     stacksize, attr.guardsize, 0, &thread_id);

  if (!win32_obj_id)
    {
      thread_printf ("CreateThread failed: this %p, %E", this);
      magic = 0;
    }
  else
    {
      postcreate ();
      while (!cygtls)
	yield ();
    }
  retval = magic;
  mutex.unlock ();
  return retval;
}

void
pthread::postcreate ()
{
  valid = true;

  InterlockedIncrement (&MT_INTERFACE->threadcount);

  /* Per POSIX the new thread inherits the sched priority from its caller
     thread if PTHREAD_INHERIT_SCHED is set.
     FIXME: set the priority appropriately for system contention scope */
  if (attr.inheritsched == PTHREAD_INHERIT_SCHED)
    attr.schedparam.sched_priority
	= sched_get_thread_priority (GetCurrentThread ());
  if (attr.schedparam.sched_priority)
    sched_set_thread_priority (win32_obj_id, attr.schedparam.sched_priority);
}

void
pthread::exit (void *value_ptr)
{
  class pthread *thread = this;
  _cygtls *tls = cygtls;	/* Save cygtls before deleting this. */

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

  if (_my_tls.local_clib.__sdidinit < 0)
    _my_tls.local_clib.__sdidinit = 0;
  _reclaim_reent (_REENT);

  if (InterlockedDecrement (&MT_INTERFACE->threadcount) == 0)
    ::exit (0);
  else
    {
      if (tls == _main_tls)
	{
	  cygheap->find_tls (tls); /* Lock _main_tls mutex. */
	  _cygtls *dummy = (_cygtls *) malloc (sizeof (_cygtls));
	  *dummy = *_main_tls;
	  _main_tls = dummy;
	  _main_tls->initialized = 0;
	}
      /* This also unlocks and closes the _main_tls mutex. */
      tls->remove (INFINITE);
      ExitThread (0);
    }
}

int
pthread::cancel ()
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
      canceled = true;
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
      /* The OS is not foolproof in terms of asynchronous thread cancellation
	 and tends to hang infinitely if we change the instruction pointer.
	 So just don't cancel asynchronously if the thread is currently
	 executing Windows code.  Rely on deferred cancellation in this case. */
      threadlist_t *tl_entry = cygheap->find_tls (cygtls);
      if (!cygtls->inside_kernel (&context))
	{
#ifdef __x86_64__
	  context.Rip = (ULONG_PTR) pthread::static_cancel_self;
#else
	  context.Eip = (DWORD) pthread::static_cancel_self;
#endif
	  SetThreadContext (win32_obj_id, &context);
	}
      cygheap->unlock_tls (tl_entry);
    }
  mutex.unlock ();
  /* See above.  For instance, a thread which waits for a semaphore in sem_wait
     will call cygwait which in turn calls WFMO.  While this WFMO call
     is cancelable by setting the thread's cancel_event object, the OS
     apparently refuses to set the thread's context and continues to wait for
     the WFMO conditions.  This is *not* reflected in the return value of
     SetThreadContext or ResumeThread, btw.
     So, what we do here is to set the cancel_event as well to allow at least
     a deferred cancel. */
  canceled = true;
  SetEvent (cancel_event);
  ResumeThread (win32_obj_id);

  return 0;
}

/* TODO: Insert pthread_testcancel into the required functions.

   Here are the lists of required and optional functions per POSIX.1-2001
   and POSIX.1-2008. A star (*) indicates that the Cygwin function already
   is a cancellation point (aka "calls pthread_testcancel"), an o (o)
   indicates that the function is not implemented in Cygwin.

   Required cancellation points:

    * accept ()
    * aio_suspend ()
    * clock_nanosleep ()
    * close ()
    * connect ()
    * creat ()
    * fcntl () F_SETLKW
    * fdatasync ()
    * fsync ()
    o getmsg ()
    o getpmsg ()
    * lockf () F_LOCK
    * mq_receive ()
    * mq_send ()
    * mq_timedreceive ()
    * mq_timedsend ()
      msgrcv ()
      msgsnd ()
    * msync ()
    * nanosleep ()
    * open ()
    * openat ()
    * pause ()
    * poll ()
    * pread ()
    * pselect ()
    * pthread_cond_timedwait ()
    * pthread_cond_wait ()
    * pthread_join ()
    * pthread_testcancel ()
    o putmsg ()
    o putpmsg ()
    * pwrite ()
    * read ()
    * readv ()
    * recv ()
    * recvfrom ()
    * recvmsg ()
    * select ()
    * sem_timedwait ()
    * sem_wait ()
    * send ()
    * sendmsg ()
    * sendto ()
    * sigpause ()
    * sigsuspend ()
    * sigtimedwait ()
    * sigwait ()
    * sigwaitinfo ()
    * sleep ()
    * system ()
    * tcdrain ()
    * usleep ()
    * wait ()
    * wait3()
    o waitid ()
    * waitpid ()
    * write ()
    * writev ()

   Optional cancellation points:

      access ()
      asctime ()
      asctime_r ()
      catclose ()	Implemented externally: libcatgets
      catgets ()	Implemented externally: libcatgets
      catopen ()	Implemented externally: libcatgets
      chmod ()
      chown ()
      closedir ()
      closelog ()
      ctermid ()
      ctime ()
      ctime_r ()
      dbm_close ()	Implemented externally: libgdbm
      dbm_delete ()	Implemented externally: libgdbm
      dbm_fetch ()	Implemented externally: libgdbm
      dbm_nextkey ()	Implemented externally: libgdbm
      dbm_open ()	Implemented externally: libgdbm
      dbm_store ()	Implemented externally: libgdbm
      dlclose ()
      dlopen ()
      dprintf ()
      endgrent ()
      endhostent ()
    o endnetent ()
      endprotoent ()
      endpwent ()
      endservent ()
      endutxent ()
      faccessat ()
      fchmod ()
      fchmodat ()
      fchown ()
      fchownat ()
    * fclose ()
    * fcntl () (any value)
      fflush ()
      fgetc ()
      fgetpos ()
      fgets ()
      fgetwc ()
      fgetws ()
    o fmtmsg ()
      fopen ()
      fpathconf ()
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
      fstat ()
      fstatat ()
      ftell ()
      ftello ()
      ftw ()
      futimens ()
      fwprintf ()
      fwrite ()
      fwscanf ()
      getaddrinfo ()
      getc ()
      getc_unlocked ()
      getchar ()
      getchar_unlocked ()
      getcwd ()
    o getdate ()
      getdelim ()
      getgrent ()
      getgrgid ()
      getgrgid_r ()
      getgrnam ()
      getgrnam_r ()
      gethostbyaddr ()
      gethostbyname ()
      gethostent ()
      gethostid ()
      gethostname ()
      getline ()
      getlogin ()
      getlogin_r ()
      getnameinfo ()
    o getnetbyaddr ()
    o getnetbyname ()
    o getnetent ()
      getopt () (if opterr is nonzero)
      getprotobyname ()
      getprotobynumber ()
      getprotoent ()
      getpwent ()
    * getpwnam ()
    * getpwnam_r ()
    * getpwuid ()
    * getpwuid_r ()
      gets ()
      getservbyname ()
      getservbyport ()
      getservent ()
      getutxent ()
      getutxid ()
      getutxline ()
      getwc ()
      getwchar ()
      getwd ()
      glob ()
      iconv_close ()	Implemented externally: libiconv
      iconv_open ()	Implemented externally: libiconv
      ioctl ()
      link ()
      linkat ()
    * lio_listio ()
      localtime ()
      localtime_r ()
    * lockf ()
      lseek ()
      lstat ()
      mkdir ()
      mkdirat ()
      mkdtemp ()
      mkfifo ()
      mkfifoat ()
      mknod ()
      mknodat ()
      mkstemp ()
      mktime ()
      nftw ()
      opendir ()
      openlog ()
      pathconf ()
      pclose ()
      perror ()
      popen ()
      posix_fadvise ()
      posix_fallocate ()
      posix_madvise ()
      posix_openpt ()
      posix_spawn ()
      posix_spawnp ()
    o posix_trace_clear ()
    o posix_trace_close ()
    o posix_trace_create ()
    o posix_trace_create_withlog ()
    o posix_trace_eventtypelist_getnext_id ()
    o posix_trace_eventtypelist_rewind ()
    o posix_trace_flush ()
    o posix_trace_get_attr ()
    o posix_trace_get_filter ()
    o posix_trace_get_status ()
    o posix_trace_getnext_event ()
    o posix_trace_open ()
    o posix_trace_rewind ()
    o posix_trace_set_filter ()
    o posix_trace_shutdown ()
    o posix_trace_timedgetnext_event ()
    o posix_typed_mem_open ()
      printf ()
      psiginfo ()
      psignal ()
      pthread_rwlock_rdlock ()
    o pthread_rwlock_timedrdlock ()
    o pthread_rwlock_timedwrlock ()
      pthread_rwlock_wrlock ()
      putc ()
      putc_unlocked ()
      putchar ()
      putchar_unlocked ()
      puts ()
      pututxline ()
      putwc ()
      putwchar ()
      readdir ()
      readdir_r ()
      readlink ()
      readlinkat ()
      remove ()
      rename ()
      renameat ()
      rewind ()
      rewinddir ()
      scandir ()
      scanf ()
      seekdir ()
      semop ()
      setgrent ()
      sethostent ()
    o setnetent ()
      setprotoent ()
      setpwent ()
      setservent ()
      setutxent ()
      sigpause ()
      stat ()
      strerror ()
      strerror_r ()
      strftime ()
      symlink ()
      symlinkat ()
      sync ()
      syslog ()
      tmpfile ()
      tmpnam ()
      ttyname ()
      ttyname_r ()
      tzset ()
      ungetc ()
      ungetwc ()
      unlink ()
      unlinkat ()
      utime ()
      utimensat ()
      utimes ()
      vdprintf ()
      vfprintf ()
      vfwprintf ()
      vprintf ()
      vwprintf ()
      wcsftime ()
      wordexp ()
      wprintf ()
      wscanf ()

   An implementation may also mark other functions not specified in the
   standard as cancellation points.  In particular, an implementation is
   likely to mark any nonstandard function that may block as a
   cancellation point. */

void
pthread::testcancel ()
{
  if (cancelstate == PTHREAD_CANCEL_DISABLE)
    return;

  /* We check for the canceled flag first.  This allows to use the
     pthread_testcancel function a lot without adding the overhead of
     an OS call.  Only if the thread is marked as canceled, we wait for
     cancel_event being really set, on the off-chance that pthread_cancel
     gets interrupted before calling SetEvent. */
  if (canceled)
    {
      WaitForSingleObject (cancel_event, INFINITE);
      cancel_self ();
    }
}

/* Return cancel event handle if it exists *and* cancel is not disabled.
   This function is supposed to be used from other functions which are
   cancelable and need the cancel event in a WFMO call. */
HANDLE
pthread::get_cancel_event ()
{
  pthread_t thread = pthread::self ();

  return (thread && thread->cancel_event
	  && thread->cancelstate != PTHREAD_CANCEL_DISABLE)
	  ? thread->cancel_event : NULL;
}

void
pthread::static_cancel_self ()
{
  pthread::self ()->cancel_self ();
}

int
pthread::setcancelstate (int state, int *oldstate)
{
  if (state != PTHREAD_CANCEL_ENABLE && state != PTHREAD_CANCEL_DISABLE)
    return EINVAL;

  if (oldstate)
    *oldstate = cancelstate;
  cancelstate = state;

  return 0;
}

int
pthread::setcanceltype (int type, int *oldtype)
{
  if (type != PTHREAD_CANCEL_DEFERRED && type != PTHREAD_CANCEL_ASYNCHRONOUS)
    return EINVAL;

  if (oldtype)
    *oldtype = canceltype;
  canceltype = type;

  return 0;
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
  /* We will no honor cancels since the thread is exiting.  */
  cancelstate = PTHREAD_CANCEL_DISABLE;

  while (cleanup_stack != NULL)
    pop_cleanup_handler (1);
}

void
pthread::cancel_self ()
{
  /* Can someone explain why the pthread:: is needed here?  g++ complains
     without it. */
  pthread::exit (PTHREAD_CANCELED);
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
      canceled = false;
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

/* instance members */

pthread_attr::pthread_attr ():verifyable_object (PTHREAD_ATTR_MAGIC),
joinable (PTHREAD_CREATE_JOINABLE), contentionscope (PTHREAD_SCOPE_PROCESS),
inheritsched (PTHREAD_INHERIT_SCHED), stackaddr (NULL), stacksize (0),
guardsize (wincap.def_guard_page_size ()), name (NULL)
{
  schedparam.sched_priority = 0;
}

pthread_attr::~pthread_attr ()
{
}

pthread_condattr::pthread_condattr ():verifyable_object
  (PTHREAD_CONDATTR_MAGIC), shared (PTHREAD_PROCESS_PRIVATE),
  clock_id (CLOCK_REALTIME)
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
  shared (0), clock_id (CLOCK_REALTIME), waiting (0), pending (0),
  sem_wait (NULL), mtx_cond(NULL), next (NULL)
{
  pthread_mutex *verifyable_mutex_obj;

  if (attr)
    {
      clock_id = attr->clock_id;

      if (attr->shared != PTHREAD_PROCESS_PRIVATE)
	{
	  magic = 0;
	  return;
	}
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
  mtx_in.set_type (PTHREAD_MUTEX_NORMAL);

  verifyable_mutex_obj = &mtx_out;
  if (!pthread_mutex::is_good_object (&verifyable_mutex_obj))
    {
      thread_printf ("Internal cond mutex is not valid. this %p", this);
      magic = 0;
      return;
    }
  /* Change the mutex type to NORMAL to speed up mutex operations */
  mtx_out.set_type (PTHREAD_MUTEX_NORMAL);

  sem_wait = ::CreateSemaphore (&sec_none_nih, 0, INT32_MAX, NULL);
  if (!sem_wait)
    {
      pthread_printf ("CreateSemaphore failed. %E");
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
  LONG releaseable;

  /*
   * Block outgoing threads (and avoid simultanous unblocks)
   */
  mtx_out.lock ();

  releaseable = waiting - pending;
  if (releaseable)
    {
      LONG released;

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
pthread_cond::wait (pthread_mutex_t mutex, PLARGE_INTEGER timeout)
{
  DWORD rv;

  mtx_in.lock ();
  if (InterlockedIncrement (&waiting) == 1)
    mtx_cond = mutex;
  else if (mtx_cond != mutex)
    {
      InterlockedDecrement (&waiting);
      mtx_in.unlock ();
      return EINVAL;
    }
  mtx_in.unlock ();

  /*
   * Release the mutex and wait on semaphore
   */
  ++mutex->condwaits;
  mutex->unlock ();

  rv = cygwait (sem_wait, timeout, cw_cancel | cw_sig_restart);

  mtx_out.lock ();

  if (rv != WAIT_OBJECT_0 && WaitForSingleObject (sem_wait, 0) == WAIT_OBJECT_0)
    /* Thread got cancelled ot timed out while a signalling is in progress.
       Set wait result back to signaled */
    rv = WAIT_OBJECT_0;

  InterlockedDecrement (&waiting);

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

  sem_wait = ::CreateSemaphore (&sec_none_nih, 0, INT32_MAX, NULL);
  if (!sem_wait)
    api_fatal ("pthread_cond::_fixup_after_fork () failed to recreate win32 semaphore");
}

pthread_barrierattr::pthread_barrierattr ()
  : verifyable_object (PTHREAD_BARRIERATTR_MAGIC)
  , shared (PTHREAD_PROCESS_PRIVATE)
{
}

pthread_barrierattr::~pthread_barrierattr ()
{
}

pthread_barrier::pthread_barrier ()
  : verifyable_object (PTHREAD_BARRIER_MAGIC)
{
}

pthread_barrier::~pthread_barrier ()
{
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
  mtx.set_type (PTHREAD_MUTEX_NORMAL);

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
pthread_rwlock::rdlock (PLARGE_INTEGER timeout)
{
  int result = 0;
  struct RWLOCK_READER *reader;

  mtx.lock ();

  reader = lookup_reader ();
  if (reader)
    {
      if (reader->n < UINT32_MAX)
	++reader->n;
      else
	result = EAGAIN;
      goto DONE;
    }

  while (writer || waiting_writers)
    {
      int ret;

      pthread_cleanup_push (pthread_rwlock::rdlock_cleanup, this);

      ++waiting_readers;
      ret = cond_readers.wait (&mtx, timeout);
      --waiting_readers;

      pthread_cleanup_pop (0);

      if (ret == ETIMEDOUT)
	{
	  result = ETIMEDOUT;
	  goto DONE;
	}
    }

  if ((reader = add_reader ()))
    ++reader->n;
  else
    {
      result = EAGAIN;
      goto DONE;
    }

 DONE:
  mtx.unlock ();

  return result;
}

int
pthread_rwlock::tryrdlock ()
{
  int result = 0;

  mtx.lock ();

  if (writer || waiting_writers)
    result = EBUSY;
  else
    {
      RWLOCK_READER *reader = lookup_reader ();
      if (!reader)
	reader = add_reader ();
      if (reader && reader->n < UINT32_MAX)
	++reader->n;
      else
	result = EAGAIN;
    }

  mtx.unlock ();

  return result;
}

int
pthread_rwlock::wrlock (PLARGE_INTEGER timeout)
{
  int result = 0;
  pthread_t self = pthread::self ();

  mtx.lock ();

  if (writer == self || lookup_reader ())
    {
      result = EDEADLK;
      goto DONE;
    }

  while (writer || readers)
    {
      int ret;

      pthread_cleanup_push (pthread_rwlock::wrlock_cleanup, this);

      ++waiting_writers;
      ret = cond_writers.wait (&mtx, timeout);
      --waiting_writers;

      pthread_cleanup_pop (0);

      if (ret == ETIMEDOUT)
	{
	  result = ETIMEDOUT;
	  goto DONE;
	}
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

  mtx.lock ();

  if (writer)
    {
      if (writer != pthread::self ())
	{
	  result = EPERM;
	  goto DONE;
	}

      writer = NULL;
    }
  else
    {
      struct RWLOCK_READER *reader = lookup_reader ();

      if (!reader)
	{
	  result = EPERM;
	  goto DONE;
	}
      if (--reader->n > 0)
	goto DONE;

      remove_reader (reader);
      delete reader;
    }

  release ();

 DONE:
  mtx.unlock ();

  return result;
}

pthread_rwlock::RWLOCK_READER *
pthread_rwlock::add_reader ()
{
  RWLOCK_READER *rd = new RWLOCK_READER;
  if (rd)
    List_insert_nolock (readers, rd);
  return rd;
}

void
pthread_rwlock::remove_reader (struct RWLOCK_READER *rd)
{
  List_remove (readers_mx, readers, rd);
}

struct pthread_rwlock::RWLOCK_READER *
pthread_rwlock::lookup_reader ()
{
  readers_mx.lock ();
  pthread_t thread = pthread::self ();

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

bool pthread_key::iterate_dtors_once_more;

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
	  if (get ())
	    iterate_dtors_once_more = true;
	}
    }
}

/* pshared mutexs */

/* static members */

List<pthread_mutex> pthread_mutex::mutexes;

/* This is used for mutex creation protection within a single process only */
fast_mutex NO_COPY pthread_mutex::mutex_initialization_lock;

void
pthread_mutex::init_mutex ()
{
  if (!mutex_initialization_lock.init ())
    api_fatal ("Could not create win32 Mutex for pthread mutex static initializer support.");
}

pthread_mutex::pthread_mutex (pthread_mutexattr *attr) :
  verifyable_object (0),	/* set magic to zero initially */
  lock_counter (0),
  win32_obj_id (NULL), owner (_new_mutex),
#ifdef DEBUGGING
  tid (0),
#endif
  recursion_counter (0), condwaits (0),
  type (PTHREAD_MUTEX_NORMAL),
  pshared (PTHREAD_PROCESS_PRIVATE)
{
  win32_obj_id = ::CreateEvent (&sec_none_nih, false, false, NULL);
  if (!win32_obj_id)
    return;
  /*attr checked in the C call */
  if (!attr)
    /* handled in the caller */;
  else if (attr->pshared != PTHREAD_PROCESS_SHARED)
    type = attr->mutextype;
  else
    return;		/* Not implemented */

  magic = PTHREAD_MUTEX_MAGIC;
  mutexes.insert (this);
}

pthread_mutex::~pthread_mutex ()
{
  if (win32_obj_id)
    {
      CloseHandle (win32_obj_id);
      win32_obj_id = NULL;
    }

  mutexes.remove (this);
  owner = _destroyed_mutex;
  magic = 0;
}

int
pthread_mutex::lock (PLARGE_INTEGER timeout)
{
  pthread_t self = ::pthread_self ();
  int result = 0;

  if (InterlockedIncrement (&lock_counter) == 1)
    set_owner (self);
  else if (type == PTHREAD_MUTEX_NORMAL /* potentially causes deadlock */
	   || !pthread::equal (owner, self))
    {
      if (cygwait (win32_obj_id, timeout, cw_sig | cw_sig_restart)
	  != WAIT_TIMEOUT)
	set_owner (self);
      else
	{
	  InterlockedDecrement (&lock_counter);
	  result = ETIMEDOUT;
	}
    }
  else
    {
      InterlockedDecrement (&lock_counter);
      if (type == PTHREAD_MUTEX_RECURSIVE)
	result = lock_recursive ();
      else
	result = EDEADLK;
    }

  pthread_printf ("mutex %p, self %p, owner %p, lock_counter %d, recursion_counter %u",
		  this, self, owner, lock_counter, recursion_counter);
  return result;
}

int
pthread_mutex::unlock ()
{
  int res = 0;
  pthread_t self = ::pthread_self ();
  if (type == PTHREAD_MUTEX_NORMAL)
    /* no error checking */;
  else if (no_owner ())
    res = type == PTHREAD_MUTEX_ERRORCHECK ? EPERM : 0;
  else if (!pthread::equal (owner, self))
    res = EPERM;
  if (!res && recursion_counter > 0 && --recursion_counter == 0)
    /* Don't try to unlock anything if recursion_counter == 0.
       This means the mutex was never locked or that we've forked. */
    {
      owner = (pthread_t) _unlocked_mutex;
#ifdef DEBUGGING
      tid = 0;		// thread-id
#endif
      if (InterlockedDecrement (&lock_counter))
	::SetEvent (win32_obj_id); // Another thread is waiting
      res = 0;
    }

  pthread_printf ("mutex %p, owner %p, self %p, lock_counter %d, recursion_counter %u, type %d, res %d",
		  this, owner, self, lock_counter, recursion_counter, type, res);
  return res;
}

int
pthread_mutex::trylock ()
{
  pthread_t self = ::pthread_self ();
  int result = 0;

  if (InterlockedCompareExchange (&lock_counter, 1, 0) == 0)
    set_owner (self);
  else if (type == PTHREAD_MUTEX_RECURSIVE && pthread::equal (owner, self))
    result = lock_recursive ();
  else
    result = EBUSY;

  return result;
}

int
pthread_mutex::destroy ()
{
  if (condwaits || trylock ())
    // Do not destroy a condwaited or locked mutex
    return EBUSY;
  else if (recursion_counter > 1)
    {
      // Do not destroy a recursive locked mutex
      recursion_counter--;
      return EBUSY;
    }

  delete this;
  return 0;
}

void
pthread_mutex::_fixup_after_fork ()
{
  pthread_printf ("mutex %p", this);
  if (pshared != PTHREAD_PROCESS_PRIVATE)
    api_fatal ("pthread_mutex::_fixup_after_fork () doesn't understand PROCESS_SHARED mutex's");

  /* All waiting threads are gone after a fork */
  recursion_counter = 0;
  lock_counter = 0;
  condwaits = 0;
#ifdef DEBUGGING
  tid = 0xffffffff;	/* Don't know the tid after a fork */
#endif
  win32_obj_id = ::CreateEvent (&sec_none_nih, false, false, NULL);
  if (!win32_obj_id)
    api_fatal ("pthread_mutex::_fixup_after_fork () failed to recreate win32 event for mutex");
}

pthread_mutexattr::pthread_mutexattr ():verifyable_object (PTHREAD_MUTEXATTR_MAGIC),
pshared (PTHREAD_PROCESS_PRIVATE), mutextype (PTHREAD_MUTEX_NORMAL)
{
}

pthread_mutexattr::~pthread_mutexattr ()
{
}

/* pshared spinlocks

   The infrastructure is provided by the underlying pthread_mutex class.
   The rest is a simplification implementing spin locking. */

pthread_spinlock::pthread_spinlock (int pshared) :
  pthread_mutex (NULL)
{
  magic = PTHREAD_SPINLOCK_MAGIC;
  set_type (PTHREAD_MUTEX_NORMAL);
  set_shared (pshared);
}

int
pthread_spinlock::lock ()
{
  pthread_t self = ::pthread_self ();
  int result = -1;
  unsigned spins = 0;

  /*
    We want to spin using 'pause' instruction on multi-core system but we
    want to avoid this on single-core systems.

    The limit of 1000 spins is semi-arbitrary. Microsoft suggests (in their
    InitializeCriticalSectionAndSpinCount documentation on MSDN) they are
    using spin count limit 4000 for their heap manager critical
    sections. Other source suggest spin count as small as 200 for fast path
    of mutex locking.
   */
  unsigned const FAST_SPINS_LIMIT = wincap.cpu_count () != 1 ? 1000 : 0;

  do
    {
      if (InterlockedExchange (&lock_counter, 1) == 0)
	{
	  set_owner (self);
	  result = 0;
	}
      else if (unlikely(pthread::equal (owner, self)))
	result = EDEADLK;
      else if (spins < FAST_SPINS_LIMIT)
        {
          ++spins;
          __asm__ volatile ("pause":::);
        }
      else
	{
	  /* Minimal timeout to minimize CPU usage while still spinning. */
	  LARGE_INTEGER timeout;
	  timeout.QuadPart = -10000LL;
	  /* FIXME: no cancel? */
	  cygwait (win32_obj_id, &timeout, cw_sig);
	}
    }
  while (result == -1);
  pthread_printf ("spinlock %p, self %p, owner %p", this, self, owner);
  return result;
}

int
pthread_spinlock::unlock ()
{
  pthread_t self = ::pthread_self ();
  int result = 0;

  if (!pthread::equal (owner, self))
    result = EPERM;
  else
    {
      owner = (pthread_t) _unlocked_mutex;
#ifdef DEBUGGING
      tid = 0;		// thread-id
#endif
      InterlockedExchange (&lock_counter, 0);
      ::SetEvent (win32_obj_id);
      result = 0;
    }
  pthread_printf ("spinlock %p, owner %p, self %p, res %d",
		  this, owner, self, result);
  return result;
}

DWORD WINAPI
pthread::thread_init_wrapper (void *arg)
{
  exception protect;
  pthread *thread = (pthread *) arg;
  /* This *must* be set prior to calling set_tls_self_pointer or there is
     a race with the signal processing code which may miss the signal mask
     settings. */
  _my_tls.sigmask = thread->parent_sigmask;
  thread->set_tls_self_pointer ();

  // Give thread default name
  SetThreadName (GetCurrentThreadId (), program_invocation_short_name);

  thread->mutex.lock ();

  // if thread is detached force cleanup on exit
  if (thread->attr.joinable == PTHREAD_CREATE_DETACHED && thread->joiner == NULL)
    thread->joiner = thread;
  thread->mutex.unlock ();

  debug_printf ("tid %p", &_my_tls);
  thread_printf ("started thread %p %p %p %p %p %p", arg, &_my_tls.local_clib,
		 _impure_ptr, thread, thread->function, thread->arg);

  // call the user's thread
  void *ret = thread->function (thread->arg);

  thread->exit (ret);

  return 0;	// just for show.  Never returns.
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
  if (!(*thread)->create (start_routine, attr ? *attr : NULL, arg))
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
pthread::atforkprepare ()
{
  callback *cb = MT_INTERFACE->pthread_prepare;
  while (cb)
    {
      cb->cb ();
      cb = cb->next;
    }

  __fp_lock_all ();

  MT_INTERFACE->fixup_before_fork ();
}

void
pthread::atforkparent ()
{
  __fp_unlock_all ();

  callback *cb = MT_INTERFACE->pthread_parent;
  while (cb)
    {
      cb->cb ();
      cb = cb->next;
    }
}

void
pthread::atforkchild ()
{
  MT_INTERFACE->fixup_after_fork ();

  __fp_unlock_all ();

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
    List_insert_nolock (MT_INTERFACE->pthread_prepare, prepcb);
  }
  if (parentcb)
  {
    parentcb->cb = parent;
    callback **t = &MT_INTERFACE->pthread_parent;
    while (*t)
      t = &(*t)->next;
    /* t = pointer to last next in the list */
    List_insert_nolock (*t, parentcb);
  }
  if (childcb)
  {
    childcb->cb = child;
    callback **t = &MT_INTERFACE->pthread_child;
    while (*t)
      t = &(*t)->next;
    /* t = pointer to last next in the list */
    List_insert_nolock (*t, childcb);
  }
  return 0;
}

extern "C" int
pthread_attr_init (pthread_attr_t *attr)
{
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
pthread_attr_setstack (pthread_attr_t *attr, void *addr, size_t size)
{
  if (!pthread_attr::is_good_object (attr))
    return EINVAL;
  if (addr == NULL)
    return EINVAL;
  if (size < PTHREAD_STACK_MIN)
    return EINVAL;
  /* The incoming address addr points to the lowest addressable byte of a
     buffer of size bytes.  Due to the way pthread_attr_setstackaddr is defined
     on Linux, the lowest address ot the stack can't be reliably computed when
     using pthread_attr_setstackaddr/pthread_attr_setstacksize.  Therefore we
     store the uppermost address of the stack in stackaddr.  See also the
     comment in pthread_attr_setstackaddr. */
  (*attr)->stackaddr = (caddr_t) addr + size;
  (*attr)->stacksize = size;
  return 0;
}

extern "C" int
pthread_attr_getstack (const pthread_attr_t *attr, void **addr, size_t *size)
{
  if (!pthread_attr::is_good_object (attr))
    return EINVAL;
  /* stackaddr holds the uppermost stack address.  See the comment in
     pthread_attr_setstack. */
  *addr = (caddr_t) (*attr)->stackaddr - (*attr)->stacksize;
  *size = (*attr)->stacksize;
  return 0;
}

extern "C" int
pthread_attr_setstackaddr (pthread_attr_t *attr, void *addr)
{
  if (!pthread_attr::is_good_object (attr))
    return EINVAL;
  if (addr == NULL)
    return EINVAL;
  /* This function is deprecated in SUSv4, but SUSv3 didn't define
     if the incoming stack address is the lowest address of the memory
     area defined as stack, or if it's the start address of the stack
     at which it begins its growth.  On Linux it's the latter which
     means the uppermost stack address on x86 based systems.  See comment
     in pthread_attr_setstack as well. */
  (*attr)->stackaddr = addr;
  return 0;
}

extern "C" int
pthread_attr_getstackaddr (const pthread_attr_t *attr, void **addr)
{
  if (!pthread_attr::is_good_object (attr))
    return EINVAL;
  /* See comment in pthread_attr_setstackaddr. */
  *addr = (*attr)->stackaddr;
  return 0;
}

extern "C" int
pthread_attr_setstacksize (pthread_attr_t *attr, size_t size)
{
  if (!pthread_attr::is_good_object (attr))
    return EINVAL;
  if (size < PTHREAD_STACK_MIN)
    return EINVAL;
  (*attr)->stacksize = size;
  return 0;
}

extern "C" int
pthread_attr_getstacksize (const pthread_attr_t *attr, size_t *size)
{
  if (!pthread_attr::is_good_object (attr))
    return EINVAL;
  /* If the stacksize has not been set by the application, return the
     default stacksize.  Note that this is different from what
     pthread_attr_getstack returns. */
  *size = (*attr)->stacksize ?: get_rlimit_stack ();
  return 0;
}

extern "C" int
pthread_attr_setguardsize (pthread_attr_t *attr, size_t size)
{
  if (!pthread_attr::is_good_object (attr))
    return EINVAL;
  /* We don't support a guardsize of more than 1 Meg. */
  if (size > 1024 * 1024)
    return EINVAL;
  (*attr)->guardsize = size;
  return 0;
}

extern "C" int
pthread_attr_getguardsize (const pthread_attr_t *attr, size_t *size)
{
  if (!pthread_attr::is_good_object (attr))
    return EINVAL;
  *size = (*attr)->guardsize;
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
pthread::join (pthread_t *thread, void **return_val, PLARGE_INTEGER timeout)
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

      switch (cygwait ((*thread)->win32_obj_id, timeout,
		       cw_sig | cw_sig_restart | cw_cancel))
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
	case WAIT_TIMEOUT:
	  // set joined thread back to joinable since we got canceled
	  (*thread)->joiner = NULL;
	  (*thread)->attr.joinable = PTHREAD_CREATE_JOINABLE;
	  return (timeout && timeout->QuadPart == 0LL) ? EBUSY : ETIMEDOUT;
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

static inline int
pthread_convert_abstime (clockid_t clock_id, const struct timespec *abstime,
			 PLARGE_INTEGER timeout)
{
  struct timespec tp;

  /* According to SUSv3, the abstime value must be checked for validity. */
  if (!valid_timespec (*abstime))
    return EINVAL;

  /* Check for immediate timeout before converting */
  clock_gettime (clock_id, &tp);
  if (tp.tv_sec > abstime->tv_sec
      || (tp.tv_sec == abstime->tv_sec
	  && tp.tv_nsec > abstime->tv_nsec))
    return ETIMEDOUT;

  timeout->QuadPart = abstime->tv_sec * NS100PERSEC
		     + (abstime->tv_nsec + (NSPERSEC/NS100PERSEC) - 1)
		       / (NSPERSEC/NS100PERSEC);
  switch (clock_id)
    {
    case CLOCK_REALTIME_COARSE:
    case CLOCK_REALTIME:
      timeout->QuadPart += FACTOR;
      break;
    default:
      /* other clocks must be handled as relative timeout */
      timeout->QuadPart -= tp.tv_sec * NS100PERSEC + tp.tv_nsec
			   / (NSPERSEC/NS100PERSEC);
      timeout->QuadPart *= -1LL;
      break;
    }
  return 0;
}

extern "C" int
pthread_join (pthread_t thread, void **return_val)
{
  return pthread::join (&thread, (void **) return_val, NULL);
}

extern "C" int
pthread_tryjoin_np (pthread_t thread, void **return_val)
{
  LARGE_INTEGER timeout = { QuadPart:0LL };

  return pthread::join (&thread, (void **) return_val, &timeout);
}

extern "C" int
pthread_timedjoin_np (pthread_t thread, void **return_val,
		      const struct timespec *abstime)
{
  LARGE_INTEGER timeout;

  int err = pthread_convert_abstime (CLOCK_REALTIME, abstime, &timeout);
  if (err)
    return err;
  return pthread::join (&thread, (void **) return_val, &timeout);
}

extern "C" int
pthread_getaffinity_np (pthread_t thread, size_t sizeof_set, cpu_set_t *set)
{
  if (!pthread::is_good_object (&thread))
    return ESRCH;

  return sched_get_thread_affinity (thread->win32_obj_id, sizeof_set, set);
}

extern "C" int
pthread_setaffinity_np (pthread_t thread, size_t sizeof_set, const cpu_set_t *set)
{
  if (!pthread::is_good_object (&thread))
    return ESRCH;

  return sched_set_thread_affinity (thread->win32_obj_id, sizeof_set, set);
}

extern "C" int
pthread_getattr_np (pthread_t thread, pthread_attr_t *attr)
{
  THREAD_BASIC_INFORMATION tbi;
  NTSTATUS status;

  if (!pthread::is_good_object (&thread))
    return ESRCH;

  /* attr may not be pre-initialized */
  if (!pthread_attr::is_good_object (attr))
  {
    int rv = pthread_attr_init (attr);
    if (rv != 0)
      return rv;
  }

  (*attr)->joinable = thread->attr.joinable;
  (*attr)->contentionscope = thread->attr.contentionscope;
  (*attr)->inheritsched = thread->attr.inheritsched;
  (*attr)->schedparam = thread->attr.schedparam;
  (*attr)->guardsize = thread->attr.guardsize;

  status = NtQueryInformationThread (thread->win32_obj_id,
				     ThreadBasicInformation,
				     &tbi, sizeof (tbi), NULL);
  if (NT_SUCCESS (status))
    {
      PTEB teb = (PTEB) tbi.TebBaseAddress;
      /* stackaddr holds the uppermost stack address.  See the comments
	 in pthread_attr_setstack and pthread_attr_setstackaddr for a
	 description. */
      (*attr)->stackaddr = teb->Tib.StackBase;
      (*attr)->stacksize = (uintptr_t) teb->Tib.StackBase
	       - (uintptr_t) (teb->DeallocationStack ?: teb->Tib.StackLimit);
    }
  else
    {
      debug_printf ("NtQueryInformationThread(ThreadBasicInformation), "
		    "status %y", status);
      (*attr)->stackaddr = thread->attr.stackaddr;
      (*attr)->stacksize = thread->attr.stacksize;
    }

  return 0;
}

/* For Linux compatibility, the length of a thread name is 16 characters. */
#define THRNAMELEN 16

extern "C" int
pthread_getname_np (pthread_t thread, char *buf, size_t buflen)
{
  char *name;

  if (!pthread::is_good_object (&thread))
    return ESRCH;

  if (!thread->attr.name)
    name = program_invocation_short_name;
  else
    name = thread->attr.name;

  /* Return ERANGE if the provided buffer is less than THRNAMELEN.  Truncate
     and zero-terminate the name to fit in buf.  This means we always return
     something if the buffer is THRNAMELEN or larger, but there is no way to
     tell if we have the whole name. */
  if (buflen < THRNAMELEN)
    return ERANGE;

  int ret = 0;
  __try
    {
      strlcpy (buf, name, buflen);
    }
  __except (NO_ERROR)
    {
      ret = EFAULT;
    }
  __endtry

  return ret;
}

extern "C" int
pthread_setname_np (pthread_t thread, const char *name)
{
  char *oldname, *cp;

  if (!pthread::is_good_object (&thread))
    return ESRCH;

  if (strlen (name) > THRNAMELEN)
    return ERANGE;

  cp = strdup (name);
  if (!cp)
    return ENOMEM;

  oldname = thread->attr.name;
  thread->attr.name = cp;

  SetThreadName (GetThreadId (thread->win32_obj_id), thread->attr.name);

  if (oldname)
    free (oldname);

  return 0;
}

/* Returns running thread's name; works for both cygthreads and pthreads */
char *
mythreadname (void)
{
  char *result = (char *) cygthread::name ();

  if (result == _my_tls.locals.unknown_thread_name)
    {
      result[0] = '\0';
      pthread_getname_np (pthread_self (), result, (size_t) THRNAMELEN);
    }

  return result;
}
#undef THRNAMELEN

/* provided for source level compatability.
   See http://www.opengroup.org/onlinepubs/007908799/xsh/pthread_getconcurrency.html
*/
extern "C" int
pthread_getconcurrency ()
{
  return MT_INTERFACE->concurrency;
}

extern "C" int
pthread_getcpuclockid (pthread_t thread, clockid_t *clk_id)
{
  if (!pthread::is_good_object (&thread))
    return (ESRCH);
  *clk_id = (clockid_t) THREADID_TO_CLOCKID (thread->getsequence_np ());
  return 0;
}

/* keep this in sync with sched.cc */
extern "C" int
pthread_getschedparam (pthread_t thread, int *policy,
			 struct sched_param *param)
{
  if (!pthread::is_good_object (&thread))
    return ESRCH;
  *policy = SCHED_FIFO;
  param->sched_priority = sched_get_thread_priority (thread->win32_obj_id);
  return 0;
}

/* Thread Specific Data */
extern "C" int
pthread_key_create (pthread_key_t *key, void (*destructor) (void *))
{
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
pthread_setschedprio (pthread_t thread, int priority)
{
  if (!pthread::is_good_object (&thread))
    return ESRCH;
  int rv =
    sched_set_thread_priority (thread->win32_obj_id, priority);
  if (!rv)
    thread->attr.schedparam.sched_priority = priority;
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

extern "C" int
pthread_cond_destroy (pthread_cond_t *cond)
{
  if (pthread_cond::is_initializer (cond))
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
  pthread_cond_t new_cond;

  if (attr && !pthread_condattr::is_good_object (attr))
    return EINVAL;

  cond_initialization_lock.lock ();

  new_cond = new pthread_cond (attr ? (*attr) : NULL);
  if (!is_good_object (&new_cond))
    {
      delete new_cond;
      cond_initialization_lock.unlock ();
      return EAGAIN;
    }

  int ret = 0;

  __try
    {
      *cond = new_cond;
    }
  __except (NO_ERROR)
    {
      delete new_cond;
      ret = EINVAL;
    }
  __endtry
  cond_initialization_lock.unlock ();
  return ret;
}

extern "C" int
pthread_cond_broadcast (pthread_cond_t *cond)
{
  if (pthread_cond::is_initializer (cond))
    return 0;
  if (!pthread_cond::is_good_object (cond))
    return EINVAL;

  (*cond)->unblock (true);

  return 0;
}

extern "C" int
pthread_cond_signal (pthread_cond_t *cond)
{
  if (pthread_cond::is_initializer (cond))
    return 0;
  if (!pthread_cond::is_good_object (cond))
    return EINVAL;

  (*cond)->unblock (false);

  return 0;
}

static int
__pthread_cond_wait_init (pthread_cond_t *cond, pthread_mutex_t *mutex)
{
  if (!pthread_mutex::is_good_object (mutex))
    return EINVAL;
  if (!(*mutex)->can_be_unlocked ())
    return EPERM;

  if (pthread_cond::is_initializer (cond))
    pthread_cond::init (cond, NULL);
  if (!pthread_cond::is_good_object (cond))
    return EINVAL;

  return 0;
}

static int
__pthread_cond_clockwait (pthread_cond_t *cond, pthread_mutex_t *mutex,
			  clockid_t clock_id, const struct timespec *abstime)
{
  int err = 0;
  LARGE_INTEGER timeout;

  do
    {
      err = pthread_convert_abstime (clock_id, abstime, &timeout);
      if (err)
	break;

      err = (*cond)->wait (*mutex, &timeout);
    }
  while (err == ETIMEDOUT);
  return err;
}

extern "C" int
pthread_cond_clockwait (pthread_cond_t *cond, pthread_mutex_t *mutex,
			clockid_t clock_id, const struct timespec *abstime)
{
  int err = 0;

  pthread_testcancel ();

  __try
    {
      err = __pthread_cond_wait_init (cond, mutex);
      if (err)
	__leave;
      err = __pthread_cond_clockwait (cond, mutex, clock_id, abstime);
    }
  __except (NO_ERROR)
    {
      return EINVAL;
    }
  __endtry
  return err;
}

extern "C" int
pthread_cond_timedwait (pthread_cond_t *cond, pthread_mutex_t *mutex,
			const struct timespec *abstime)
{
  int err = 0;

  pthread_testcancel ();

  __try
    {
      err = __pthread_cond_wait_init (cond, mutex);
      if (err)
	__leave;
      err = __pthread_cond_clockwait (cond, mutex, (*cond)->clock_id, abstime);
    }
  __except (NO_ERROR)
    {
      return EINVAL;
    }
  __endtry
  return err;
}

extern "C" int
pthread_cond_wait (pthread_cond_t *cond, pthread_mutex_t *mutex)
{
  pthread_testcancel ();

  int err = __pthread_cond_wait_init (cond, mutex);
  if (err)
    return err;
  return (*cond)->wait (*mutex, NULL);
}

extern "C" int
pthread_condattr_init (pthread_condattr_t *condattr)
{
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
pthread_condattr_getclock (const pthread_condattr_t *attr, clockid_t *clock_id)
{
  if (!pthread_condattr::is_good_object (attr))
    return EINVAL;
  *clock_id = (*attr)->clock_id;
  return 0;
}

extern "C" int
pthread_condattr_setclock (pthread_condattr_t *attr, clockid_t clock_id)
{
  if (!pthread_condattr::is_good_object (attr))
    return EINVAL;
  if (CLOCKID_IS_PROCESS (clock_id) || CLOCKID_IS_THREAD (clock_id)
      || clock_id >= MAX_CLOCKS)
    return EINVAL;
  (*attr)->clock_id = clock_id;
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

extern "C" int
pthread_rwlock_destroy (pthread_rwlock_t *rwlock)
{
  if (pthread_rwlock::is_initializer (rwlock))
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
  pthread_rwlock_t new_rwlock;

  if (attr && !pthread_rwlockattr::is_good_object (attr))
    return EINVAL;

  rwlock_initialization_lock.lock ();

  new_rwlock = new pthread_rwlock (attr ? (*attr) : NULL);
  if (!is_good_object (&new_rwlock))
    {
      delete new_rwlock;
      rwlock_initialization_lock.unlock ();
      return EAGAIN;
    }

  int ret = 0;

  __try
    {
      *rwlock = new_rwlock;
    }
  __except (NO_ERROR)
    {
      delete new_rwlock;
      ret = EINVAL;
    }
  __endtry
  rwlock_initialization_lock.unlock ();
  return ret;
}

extern "C" int
pthread_rwlock_rdlock (pthread_rwlock_t *rwlock)
{
  pthread_testcancel ();

  if (pthread_rwlock::is_initializer (rwlock))
    pthread_rwlock::init (rwlock, NULL);
  if (!pthread_rwlock::is_good_object (rwlock))
    return EINVAL;

  return (*rwlock)->rdlock ();
}

extern "C" int
pthread_rwlock_clockrdlock (pthread_rwlock_t *rwlock, clockid_t clock_id,
			    const struct timespec *abstime)
{
  LARGE_INTEGER timeout;

  pthread_testcancel ();

  if (pthread_rwlock::is_initializer (rwlock))
    pthread_rwlock::init (rwlock, NULL);
  if (!pthread_rwlock::is_good_object (rwlock))
    return EINVAL;

  /* According to SUSv3, abstime need not be checked for validity,
     if the rwlock can be locked immediately. */
  if (!(*rwlock)->tryrdlock ())
    return 0;

  __try
    {
      int err = pthread_convert_abstime (clock_id, abstime, &timeout);
      if (err)
	return err;

      return (*rwlock)->rdlock (&timeout);
    }
  __except (NO_ERROR) {}
  __endtry
  return EINVAL;
}

extern "C" int
pthread_rwlock_timedrdlock (pthread_rwlock_t *rwlock,
			    const struct timespec *abstime)
{
  return pthread_rwlock_clockrdlock (rwlock, CLOCK_REALTIME, abstime);
}

extern "C" int
pthread_rwlock_tryrdlock (pthread_rwlock_t *rwlock)
{
  if (pthread_rwlock::is_initializer (rwlock))
    pthread_rwlock::init (rwlock, NULL);
  if (!pthread_rwlock::is_good_object (rwlock))
    return EINVAL;

  return (*rwlock)->tryrdlock ();
}

extern "C" int
pthread_rwlock_wrlock (pthread_rwlock_t *rwlock)
{
  pthread_testcancel ();

  if (pthread_rwlock::is_initializer (rwlock))
    pthread_rwlock::init (rwlock, NULL);
  if (!pthread_rwlock::is_good_object (rwlock))
    return EINVAL;

  return (*rwlock)->wrlock ();
}

extern "C" int
pthread_rwlock_clockwrlock (pthread_rwlock_t *rwlock, clockid_t clock_id,
			    const struct timespec *abstime)
{
  LARGE_INTEGER timeout;

  pthread_testcancel ();

  if (pthread_rwlock::is_initializer (rwlock))
    pthread_rwlock::init (rwlock, NULL);
  if (!pthread_rwlock::is_good_object (rwlock))
    return EINVAL;

  /* According to SUSv3, abstime need not be checked for validity,
     if the rwlock can be locked immediately. */
  if (!(*rwlock)->trywrlock ())
    return 0;

  __try
    {
      int err = pthread_convert_abstime (clock_id, abstime, &timeout);
      if (err)
	return err;

      return (*rwlock)->wrlock (&timeout);
    }
  __except (NO_ERROR) {}
  __endtry
  return EINVAL;
}

extern "C" int
pthread_rwlock_timedwrlock (pthread_rwlock_t *rwlock,
			    const struct timespec *abstime)
{
  return pthread_rwlock_clockwrlock (rwlock, CLOCK_REALTIME, abstime);
}

extern "C" int
pthread_rwlock_trywrlock (pthread_rwlock_t *rwlock)
{
  if (pthread_rwlock::is_initializer (rwlock))
    pthread_rwlock::init (rwlock, NULL);
  if (!pthread_rwlock::is_good_object (rwlock))
    return EINVAL;

  return (*rwlock)->trywrlock ();
}

extern "C" int
pthread_rwlock_unlock (pthread_rwlock_t *rwlock)
{
  if (pthread_rwlock::is_initializer (rwlock))
    return 0;
  if (!pthread_rwlock::is_good_object (rwlock))
    return EINVAL;

  return (*rwlock)->unlock ();
}

extern "C" int
pthread_rwlockattr_init (pthread_rwlockattr_t *rwlockattr)
{
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

  siginfo_t si = {0};
  si.si_signo = sig;
  si.si_code = SI_USER;
  si.si_pid = myself->pid;
  si.si_uid = myself->uid;
  int rval;
  if (!thread->valid)
    rval = ESRCH;
  else if (sig)
    {
      rval = (int) sig_send (NULL, si, thread->cygtls);
      if (rval == -1)
	rval = get_errno ();
    }
  else
    switch (WaitForSingleObject (thread->win32_obj_id, 0))
      {
      case WAIT_TIMEOUT:
	rval = 0;
	break;
      default:
	rval = ESRCH;
	break;
      }

  // unlock myself
  return rval;
}

extern "C" int
pthread_sigmask (int operation, const sigset_t *set, sigset_t *old_set)
{
  int res = handle_sigprocmask (operation, set, old_set, _my_tls.sigmask);
  syscall_printf ("%d = pthread_sigmask(%d, %p, %p)",
		  res, operation, set, old_set);
  return res;
}

extern "C" int
pthread_sigqueue (pthread_t *thread, int sig, const union sigval value)
{
  siginfo_t si = {0};

  if (!pthread::is_good_object (thread))
    return EINVAL;
  if (!(*thread)->valid)
    return ESRCH;

  si.si_signo = sig;
  si.si_code = SI_QUEUE;
  si.si_value = value;
  si.si_pid = myself->pid;
  si.si_uid = myself->uid;
  return (int) sig_send (NULL, si, (*thread)->cygtls);
}

/* ID */

extern "C" int
pthread_equal (pthread_t t1, pthread_t t2)
{
  return pthread::equal (t1, t2);
}

/* Mutexes  */

int
pthread_mutex::init (pthread_mutex_t *mutex,
		     const pthread_mutexattr_t *attr,
		     const pthread_mutex_t initializer)
{
  if (attr && !pthread_mutexattr::is_good_object (attr))
    return EINVAL;

  mutex_initialization_lock.lock ();
  if (initializer == NULL || pthread_mutex::is_initializer (mutex))
    {
      pthread_mutex_t new_mutex = new pthread_mutex (attr ? (*attr) : NULL);
      if (!is_good_object (&new_mutex))
	{
	  delete new_mutex;
	  mutex_initialization_lock.unlock ();
	  return EAGAIN;
	}

      if (!attr && initializer)
	{
	  if (initializer == PTHREAD_RECURSIVE_MUTEX_INITIALIZER_NP)
	    new_mutex->type = PTHREAD_MUTEX_RECURSIVE;
	  else if (initializer == PTHREAD_NORMAL_MUTEX_INITIALIZER_NP)
	    new_mutex->type = PTHREAD_MUTEX_NORMAL;
	  else if (initializer == PTHREAD_ERRORCHECK_MUTEX_INITIALIZER_NP)
	    new_mutex->type = PTHREAD_MUTEX_ERRORCHECK;
	}

      __try
	{
	  *mutex = new_mutex;
	}
      __except (NO_ERROR)
	{
	  delete new_mutex;
	  mutex_initialization_lock.unlock ();
	  return EINVAL;
	}
      __endtry
    }
  mutex_initialization_lock.unlock ();
  pthread_printf ("*mutex %p, attr %p, initializer %p", *mutex, attr, initializer);

  return 0;
}

extern "C" int
pthread_mutex_getprioceiling (const pthread_mutex_t *mutex,
				int *prioceiling)
{
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
  if (pthread_mutex::is_initializer (mutex))
    pthread_mutex::init (mutex, NULL, *mutex);
  if (!pthread_mutex::is_good_object (mutex))
    return EINVAL;
  return (*mutex)->lock ();
}

extern "C" int
pthread_mutex_clocklock (pthread_mutex_t *mutex, clockid_t clock_id,
			 const struct timespec *abstime)
{
  LARGE_INTEGER timeout;

  if (pthread_mutex::is_initializer (mutex))
    pthread_mutex::init (mutex, NULL, *mutex);
  if (!pthread_mutex::is_good_object (mutex))
    return EINVAL;

  /* According to SUSv3, abstime need not be checked for validity,
     if the mutex can be locked immediately. */
  if (!(*mutex)->trylock ())
    return 0;

  __try
    {
      int err = pthread_convert_abstime (clock_id, abstime, &timeout);
      if (err)
	return err;

      return (*mutex)->lock (&timeout);
    }
  __except (NO_ERROR) {}
  __endtry
  return EINVAL;
}

extern "C" int
pthread_mutex_timedlock (pthread_mutex_t *mutex, const struct timespec *abstime)
{
  return pthread_mutex_clocklock (mutex, CLOCK_REALTIME, abstime);
}

extern "C" int
pthread_mutex_trylock (pthread_mutex_t *mutex)
{
  if (pthread_mutex::is_initializer (mutex))
    pthread_mutex::init (mutex, NULL, *mutex);
  if (!pthread_mutex::is_good_object (mutex))
    return EINVAL;
  return (*mutex)->trylock ();
}

extern "C" int
pthread_mutex_unlock (pthread_mutex_t *mutex)
{
  if (pthread_mutex::is_initializer (mutex))
    pthread_mutex::init (mutex, NULL, *mutex);
  if (!pthread_mutex::is_good_object (mutex))
    return EINVAL;
  return (*mutex)->unlock ();
}

extern "C" int
pthread_mutex_destroy (pthread_mutex_t *mutex)
{
  int rv;

  if (pthread_mutex::is_initializer (mutex))
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
  return ENOSYS;
}

/* Spinlocks  */

int
pthread_spinlock::init (pthread_spinlock_t *spinlock, int pshared)
{
  pthread_spinlock_t new_spinlock = new pthread_spinlock (pshared);
  if (!is_good_object (&new_spinlock))
    {
      delete new_spinlock;
      return EAGAIN;
    }

  __try
    {
      *spinlock = new_spinlock;
    }
  __except (NO_ERROR)
    {
      delete new_spinlock;
      return EINVAL;
    }
  __endtry
  pthread_printf ("*spinlock %p, pshared %d", *spinlock, pshared);
  return 0;
}

extern "C" int
pthread_spin_lock (pthread_spinlock_t *spinlock)
{
  if (!pthread_spinlock::is_good_object (spinlock))
    return EINVAL;
  return (*spinlock)->lock ();
}

extern "C" int
pthread_spin_trylock (pthread_spinlock_t *spinlock)
{
  if (!pthread_spinlock::is_good_object (spinlock))
    return EINVAL;
  return (*spinlock)->trylock ();
}

extern "C" int
pthread_spin_unlock (pthread_spinlock_t *spinlock)
{
  if (!pthread_spinlock::is_good_object (spinlock))
    return EINVAL;
  return (*spinlock)->unlock ();
}

extern "C" int
pthread_spin_destroy (pthread_spinlock_t *spinlock)
{
  if (!pthread_spinlock::is_good_object (spinlock))
    return EINVAL;
  return (*spinlock)->destroy ();
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

List<semaphore> semaphore::semaphores;

semaphore::semaphore (int pshared, unsigned int value)
: verifyable_object (SEM_MAGIC),
  shared (pshared),
  currentvalue (-1),
  startvalue (value),
  fd (-1),
  hash (0ULL),
  sem (NULL)
{
  SECURITY_ATTRIBUTES sa = (pshared != PTHREAD_PROCESS_PRIVATE)
			   ? sec_all : sec_none_nih;
  this->win32_obj_id = ::CreateSemaphore (&sa, value, INT32_MAX, NULL);
  if (!this->win32_obj_id)
    magic = 0;

  semaphores.insert (this);
}

semaphore::semaphore (unsigned long long shash, LUID sluid, int sfd,
		      sem_t *ssem, int oflag, mode_t mode, unsigned int value)
: verifyable_object (SEM_MAGIC),
  shared (PTHREAD_PROCESS_SHARED),
  currentvalue (-1),		/* Unused for named semaphores. */
  startvalue (value),
  fd (sfd),
  hash (shash),
  luid (sluid),
  sem (ssem)
{
  char name[MAX_PATH];

  __small_sprintf (name, "semaphore/%016X%08x%08x",
		   hash, luid.HighPart, luid.LowPart);
  this->win32_obj_id = ::CreateSemaphore (&sec_all, value, INT32_MAX, name);
  if (!this->win32_obj_id)
    magic = 0;
  if (GetLastError () == ERROR_ALREADY_EXISTS && (oflag & O_EXCL))
    {
      __seterrno ();
      CloseHandle (this->win32_obj_id);
      magic = 0;
    }

  semaphores.insert (this);
}

semaphore::~semaphore ()
{
  if (win32_obj_id)
    CloseHandle (win32_obj_id);

  semaphores.remove (this);
}

void
semaphore::_post ()
{
  LONG dummy;
  ReleaseSemaphore (win32_obj_id, 1, &dummy);
}

int
semaphore::_getvalue (int *sval)
{
  NTSTATUS status;
  SEMAPHORE_BASIC_INFORMATION sbi;

  status = NtQuerySemaphore (win32_obj_id, SemaphoreBasicInformation, &sbi,
			     sizeof sbi, NULL);
  int res;
  if (NT_SUCCESS (status))
    {
      *sval = sbi.CurrentCount;
      res = 0;
    }
  else
    {
      *sval = startvalue;
      __seterrno_from_nt_status (status);
      res = -1;
    }
  return res;
}

int
semaphore::_trywait ()
{
  /* FIXME: signals should be able to interrupt semaphores...
    We probably need WaitForMultipleObjects here.  */
  if (WaitForSingleObject (win32_obj_id, 0) == WAIT_TIMEOUT)
    {
      set_errno (EAGAIN);
      return -1;
    }
  return 0;
}

int
semaphore::_wait (PLARGE_INTEGER timeout)
{
  __try
    {
      switch (cygwait (win32_obj_id, timeout,
		       cw_cancel | cw_cancel_self | cw_sig_eintr))
	{
	case WAIT_OBJECT_0:
	  break;
	case WAIT_SIGNALED:
	  set_errno (EINTR);
	  return -1;
	case WAIT_TIMEOUT:
	  set_errno (ETIMEDOUT);
	  return -1;
	default:
	  pthread_printf ("cygwait failed. %E");
	  __seterrno ();
	  return -1;
	}
    }
  __except (NO_ERROR) {}
  __endtry
  return 0;
}

void
semaphore::_fixup_before_fork ()
{
  NTSTATUS status;
  SEMAPHORE_BASIC_INFORMATION sbi;

  status = NtQuerySemaphore (win32_obj_id, SemaphoreBasicInformation, &sbi,
			     sizeof sbi, NULL);
  if (NT_SUCCESS (status))
    currentvalue = sbi.CurrentCount;
  else
    currentvalue = startvalue;
}

void
semaphore::_fixup_after_fork ()
{
  if (shared == PTHREAD_PROCESS_PRIVATE)
    {
      pthread_printf ("sem %p", this);
      win32_obj_id = ::CreateSemaphore (&sec_none_nih, currentvalue,
					INT32_MAX, NULL);
      if (!win32_obj_id)
	api_fatal ("failed to create new win32 semaphore, "
		   "currentvalue %ld, %E", currentvalue);
    }
}

void
semaphore::_terminate ()
{
  int _sem_close (sem_t *, bool);

  if (sem)
    _sem_close (sem, false);
}

/* static members */

int
semaphore::init (sem_t *sem, int pshared, unsigned int value)
{
  /*
     We can't tell the difference between reinitialising an
     existing semaphore and initialising a semaphore who's
     contents happen to be a valid pointer
   */
  if (is_good_object (sem))
    paranoid_printf ("potential attempt to reinitialise a semaphore");

  if (value > SEM_VALUE_MAX)
    {
      set_errno(EINVAL);
      return -1;
    }

  *sem = new semaphore (pshared, value);

  if (!is_good_object (sem))
    {
      delete (*sem);
      *sem = NULL;
      set_errno(EAGAIN);
      return -1;
    }
  return 0;
}

int
semaphore::destroy (sem_t *sem)
{
  if (!is_good_object (sem))
    {
      set_errno(EINVAL);
      return -1;
    }

  /* It's invalid to destroy a semaphore not opened with sem_init. */
  if ((*sem)->fd != -1)
    {
      set_errno(EINVAL);
      return -1;
    }

  /* FIXME - new feature - test for busy against threads... */

  delete (*sem);
  *sem = NULL;
  return 0;
}

int
semaphore::close (sem_t *sem)
{
  if (!is_good_object (sem))
    {
      set_errno(EINVAL);
      return -1;
    }

  /* It's invalid to close a semaphore not opened with sem_open. */
  if ((*sem)->fd == -1)
    {
      set_errno(EINVAL);
      return -1;
    }

  delete (*sem);
  delete sem;
  return 0;
}

sem_t *
semaphore::open (unsigned long long hash, LUID luid, int fd, int oflag,
		 mode_t mode, unsigned int value, bool &wasopen)
{
  if (value > SEM_VALUE_MAX)
    {
      set_errno (EINVAL);
      return NULL;
    }

  /* sem_open is supposed to return the same pointer, if the same named
     semaphore is opened multiple times in the same process, as long as
     the semaphore hasn't been closed or unlinked in the meantime. */
  semaphores.mx.lock ();
  for (semaphore *sema = semaphores.head; sema; sema = sema->next)
    if (sema->fd >= 0 && sema->hash == hash
	&& sema->luid.HighPart == luid.HighPart
	&& sema->luid.LowPart == luid.LowPart)
      {
	wasopen = true;
	semaphores.mx.unlock ();
	return sema->sem;
      }
  semaphores.mx.unlock ();

  wasopen = false;
  sem_t *sem = new sem_t;
  if (!sem)
    {
      set_errno (ENOMEM);
      return NULL;
    }

  *sem = new semaphore (hash, luid, fd, sem, oflag, mode, value);

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

  return (*sem)->_wait ();
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
semaphore::clockwait (sem_t *sem, clockid_t clock_id,
		      const struct timespec *abstime)
{
  LARGE_INTEGER timeout;

  if (!is_good_object (sem))
    {
      set_errno (EINVAL);
      return -1;
    }

  /* According to SUSv3, abstime need not be checked for validity,
     if the semaphore can be locked immediately. */
  if (!(*sem)->_trywait ())
    return 0;

  __try
    {
      int err = pthread_convert_abstime (clock_id, abstime, &timeout);
      if (err)
	return err;

      return (*sem)->_wait (&timeout);
    }
  __except (NO_ERROR) {}
  __endtry
  return EINVAL;
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
  __try
    {
      if (is_good_object (sem))
	return (*sem)->_getvalue (sval);
    }
  __except (NO_ERROR) {}
  __endtry
  set_errno (EINVAL);
  return -1;
}

int
semaphore::getinternal (sem_t *sem, int *sfd, unsigned long long *shash,
			LUID *sluid, unsigned int *sval)
{
  __try
    {
      if (!is_good_object (sem))
	__leave;
      if ((*sfd = (*sem)->fd) < 0)
	__leave;
      *shash = (*sem)->hash;
      *sluid = (*sem)->luid;
      /* POSIX defines the value in calls to sem_init/sem_open as unsigned,
	 but the sem_getvalue gets a pointer to int to return the value.
	 Go figure! */
      return (*sem)->_getvalue ((int *)sval);
    }
  __except (NO_ERROR) {}
  __endtry
  set_errno (EINVAL);
  return -1;
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

bool
pthread_null::create (void *(*)(void *), pthread_attr *, void *)
{
  return true;
}

void
pthread_null::exit (void *value_ptr)
{
  _my_tls.remove (INFINITE);
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


extern "C"
int
pthread_barrierattr_init (pthread_barrierattr_t * battr)
{
  if (unlikely (battr == NULL))
    return EINVAL;

  *battr = new pthread_barrierattr;
  (*battr)->shared = PTHREAD_PROCESS_PRIVATE;

  return 0;
}


extern "C"
int
pthread_barrierattr_setpshared (pthread_barrierattr_t * battr, int shared)
{
  if (unlikely (! pthread_barrierattr::is_good_object (battr)))
    return EINVAL;

  if (unlikely (shared != PTHREAD_PROCESS_SHARED
                && shared != PTHREAD_PROCESS_PRIVATE))
    return EINVAL;

  (*battr)->shared = shared;
  return 0;
}


extern "C"
int
pthread_barrierattr_getpshared (const pthread_barrierattr_t * battr,
                                int * shared)
{
  if (unlikely (! pthread_barrierattr::is_good_object (battr)
                || shared == NULL))
    return EINVAL;

  *shared = (*battr)->shared;
  return 0;
}


extern "C"
int
pthread_barrierattr_destroy (pthread_barrierattr_t * battr)
{
  if (unlikely (! pthread_barrierattr::is_good_object (battr)))
    return EINVAL;

  delete_and_clear (battr);
  return 0;
}


extern "C"
int
pthread_barrier_init (pthread_barrier_t * bar,
                      const pthread_barrierattr_t * attr, unsigned count)
{
  if (unlikely (bar == NULL))
    return EINVAL;

  *bar = new pthread_barrier;
  return (*bar)->init (attr, count);
}


int
pthread_barrier::init (const pthread_barrierattr_t * attr, unsigned count)
{
  pthread_mutex_t * mutex = NULL;

  if (unlikely ((attr != NULL
                 && (! pthread_barrierattr::is_good_object (attr)
                     || (*attr)->shared == PTHREAD_PROCESS_SHARED))
                || count == 0))
    return EINVAL;

  int retval = pthread_mutex_init (&mtx, NULL);
  if (unlikely (retval != 0))
    return retval;

  retval = pthread_cond_init (&cond, NULL);
  if (unlikely (retval != 0))
    {
      int ret = pthread_mutex_destroy (mutex);
      if (ret != 0)
        api_fatal ("pthread_mutex_destroy (%p) = %d", mutex, ret);

      mtx = NULL;
      return retval;
    }

  cnt = count;
  cyc = 0;
  wt = 0;

  return 0;
}


extern "C"
int
pthread_barrier_destroy (pthread_barrier_t * bar)
{
  if (unlikely (! pthread_barrier::is_good_object (bar)))
    return EINVAL;

  int ret;
  ret = (*bar)->destroy ();
  if (ret == 0)
    delete_and_clear (bar);

  return ret;
}


int
pthread_barrier::destroy ()
{
  if (unlikely (wt != 0))
    return EBUSY;

  int retval = pthread_cond_destroy (&cond);
  if (unlikely (retval != 0))
    return retval;
  else
    cond = NULL;

  retval = pthread_mutex_destroy (&mtx);
  if (unlikely (retval != 0))
    return retval;
  else
    mtx = NULL;

  cnt = 0;
  cyc = 0;
  wt = 0;

  return 0;
}


extern "C"
int
pthread_barrier_wait (pthread_barrier_t * bar)
{
  if (unlikely (! pthread_barrier::is_good_object (bar)))
    return EINVAL;

  return (*bar)->wait ();
}


int
pthread_barrier::wait ()
{
  int retval = pthread_mutex_lock (&mtx);
  if (unlikely (retval != 0))
    return retval;

  if (unlikely (wt >= cnt))
    {
      api_fatal ("wt >= cnt (%u >= %u)", wt, cnt);
      return EINVAL;
    }

  if (unlikely (++wt == cnt))
    {
      ++cyc;
      /* This is the last thread to reach the barrier. Signal the waiting
         threads to wake up and continue.  */
      retval = pthread_cond_broadcast (&cond);
      if (unlikely (retval != 0))
        goto cond_error;

      wt = 0;
      retval = pthread_mutex_unlock (&mtx);
      if (unlikely (retval != 0))
        abort ();

      return PTHREAD_BARRIER_SERIAL_THREAD;
    }
  else
    {
      uint64_t cycle = cyc;
      do
        {
          retval = pthread_cond_wait (&cond, &mtx);
          if (unlikely (retval != 0))
            goto cond_error;
        }
      while (unlikely (cycle == cyc));

      retval = pthread_mutex_unlock (&mtx);
      if (unlikely (retval != 0))
        api_fatal ("pthread_mutex_unlock (%p) = %d", &mtx, retval);

      return 0;
    }

 cond_error:
  {
    --wt;
    int ret = pthread_mutex_unlock (&mtx);
    if (unlikely (ret != 0))
        api_fatal ("pthread_mutex_unlock (%p) = %d", &mtx, ret);

    return retval;
  }
}
