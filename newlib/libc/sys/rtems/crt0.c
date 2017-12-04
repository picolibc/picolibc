/*
 *  RTEMS Fake crt0
 *
 *  Each RTEMS BSP provides its own crt0 and linker script.  Unfortunately
 *  this means that crt0 and the linker script are not available as
 *  each tool is configured.  Without a crt0 and linker script, some
 *  targets do not successfully link "conftest.c" during the configuration 
 *  process.  So this fake crt0.c provides all the symbols required to
 *  successfully link a program.  The resulting program will not run
 *  but this is enough to satisfy the autoconf macro AC_PROG_CC.
 */

#include <sys/lock.h>
#include <sys/stat.h>
#include <sys/uio.h>
#include <reent.h>
#include <signal.h>
#include <time.h>
#include <unistd.h>
#include <machine/_arc4random.h>
#include <machine/_libatomic.h>

void rtems_provides_crt0( void ) {}  /* dummy symbol so file always has one */

#define RTEMS_STUB(ret, func, body) \
ret rtems_stub_##func body; \
ret func body

/* RTEMS provides some of its own routines including a Malloc family */
RTEMS_STUB(void *,malloc(size_t s), { return 0; })
RTEMS_STUB(void *,realloc(void* p, size_t s), { return 0; })
RTEMS_STUB(void, free(void* ptr), { })
RTEMS_STUB(void *, calloc(size_t s1, size_t s2), { return 0; })
RTEMS_STUB(int, posix_memalign(void **p, size_t si, size_t s2), { return -1; })

/* Stubs for routines from RTEMS <sys/lock.h> */
RTEMS_STUB(void, _Mutex_Acquire(struct _Mutex_Control *p), { })
RTEMS_STUB(int,  _Mutex_Acquire_timed(struct _Mutex_Control *p1, const struct timespec *p2), { return -1; })
RTEMS_STUB(int,  _Mutex_Try_Acquire(struct _Mutex_Control *p), { return -1; })
RTEMS_STUB(void, _Mutex_Release(struct _Mutex_Control *p), { })

RTEMS_STUB(void, _Mutex_recursive_Acquire(struct _Mutex_recursive_Control *p), { })
RTEMS_STUB(int,  _Mutex_recursive_Acquire_timed(struct _Mutex_recursive_Control *p1, const struct timespec *p2), { return -1; })
RTEMS_STUB(int,  _Mutex_recursive_Try_acquire(struct _Mutex_recursive_Control *p), { return -1; })
RTEMS_STUB(void, _Mutex_recursive_Release(struct _Mutex_recursive_Control *p), { })

RTEMS_STUB(void, _Condition_Wait(struct _Condition_Control *p1, struct _Mutex_Control *p2), { })
RTEMS_STUB(int,  _Condition_Wait_timed(struct _Condition_Control *p1, struct _Mutex_Control *p2, const struct timespec *p3), { return -1; })
RTEMS_STUB(void, _Condition_Wait_recursive(struct _Condition_Control *p1, struct _Mutex_recursive_Control *p2), { })
RTEMS_STUB(int,  _Condition_Wait_recursive_timed(struct _Condition_Control *p1, struct _Mutex_recursive_Control *p2, const struct timespec *p3), { return -1; })
RTEMS_STUB(void, _Condition_Signal(struct _Condition_Control *p), { })
RTEMS_STUB(void, _Condition_Broadcast(struct _Condition_Control *p), { })

RTEMS_STUB(void, _Semaphore_Wait(struct _Semaphore_Control *p), { })
RTEMS_STUB(void, _Semaphore_Post(struct _Semaphore_Control *p), { })

RTEMS_STUB(int, _Futex_Wait(struct _Futex_Control *p1, int *p2, int i), { return -1; })
RTEMS_STUB(int, _Futex_Wake(struct _Futex_Control *p, int i), { return -1; })

RTEMS_STUB(int, _Sched_Count(void), { return -1; })
RTEMS_STUB(int, _Sched_Index(void), { return -1; })
RTEMS_STUB(int, _Sched_Name_to_index(const char *p, size_t s), { return -1; })
RTEMS_STUB(int, _Sched_Processor_count(int i), { return 1; })

/* Stubs for routines from RTEMS <machine/_libatomic.h> */
RTEMS_STUB(__uint32_t, _Libatomic_Protect_start(void *ptr), { return 0; });
RTEMS_STUB(void, _Libatomic_Protect_end(void *ptr, __uint32_t isr_level), { });
RTEMS_STUB(void, _Libatomic_Lock_n(void *ptr, __size_t n), { });
RTEMS_STUB(void, _Libatomic_Unlock_n(void *ptr, __size_t n), { });

/* Stubs for routines for arc4random (from <unistd.h> and <machine/_arc4random.h> */
RTEMS_STUB(int,  getentropy(void *ptr, __size_t n), { return -1; });
RTEMS_STUB(void, _arc4random_getentropy_fail(void), { });

#if defined(__GNUC__)
/*
 * stubs for libstdc++ rtems-threads support functions from gcc/gthr-rtems.h
 */
int rtems_gxx_once() { return -1; }
int rtems_gxx_key_create() { return -1; }
int rtems_gxx_key_delete() { return -1; }
void *rtems_gxx_getspecific() { return 0; }
int rtems_gxx_setspecific() { return -1; }

void rtems_gxx_mutex_init() { }
int rtems_gxx_mutex_lock() { return -1; }
int rtems_gxx_mutex_trylock() { return -1; }
int rtems_gxx_mutex_unlock() { return -1; }

void rtems_gxx_recursive_mutex_init() { }
int rtems_gxx_recursive_mutex_lock() { return -1; }
int rtems_gxx_recursive_mutex_trylock() { return -1; }
int rtems_gxx_recursive_mutex_unlock() { return -1; }
#endif

/* stubs for functions RTEMS provides */
RTEMS_STUB(int, access(const char *pathname, int mode), { return -1; })
RTEMS_STUB(int, clock_gettime(clockid_t clk_id, struct timespec *tp), { return -1; })
RTEMS_STUB(int, close (int fd), { return -1; })
RTEMS_STUB(int, dup2(int oldfd, int newfd), { return -1; })
RTEMS_STUB(int, fchmod(int fd, mode_t mode ), { return -1; })
RTEMS_STUB(int, fcntl( int fd, int cmd, ... /* arg */ ), { return -1; })
RTEMS_STUB(pid_t, fork(void), { return -1; })
RTEMS_STUB(int, fstat(int fd, struct stat *buf), { return -1; })
RTEMS_STUB(int, getdents(int fd, void *dp, int count), { return -1; })
RTEMS_STUB(char *, getlogin(void), { return 0; })
RTEMS_STUB(int, gettimeofday(struct timeval *__restrict tv, struct timezone *__restrict tz), { return -1; })
RTEMS_STUB(struct passwd *, getpwnam(const char *name), { return 0; })
RTEMS_STUB(struct passwd *, getpwuid(uid_t uid), { return 0; })
RTEMS_STUB(uid_t, getuid(void), { return 0; })
RTEMS_STUB(int, nanosleep(const struct timespec *req, struct timespec *rem), { return -1; })
RTEMS_STUB(int, ftruncate(int fd, off_t length), { return -1; })
RTEMS_STUB(_off_t, lseek(int fd, _off_t offset, int whence), { return -1; })
RTEMS_STUB(int, lstat(const char *path, struct stat *buf), { return -1; })
RTEMS_STUB(int, open(const char *pathname, int flags, int mode), { return -1; })
RTEMS_STUB(int, pipe(int pipefd[2]), { return -1; })
RTEMS_STUB(_ssize_t, read(int fd, void *buf, size_t count), { return -1; })
RTEMS_STUB(ssize_t, readv (int fd, const struct iovec *iov, int iovcnt), { return -1; })
RTEMS_STUB(int, sched_yield(void), { return -1; })
RTEMS_STUB(int, sigfillset(sigset_t *set), { return -1; })
RTEMS_STUB(int, sigprocmask(int how, const sigset_t *set, sigset_t *oldset), { return -1; })
RTEMS_STUB(int, stat(const char *path, struct stat *buf), { return -1; })
RTEMS_STUB(long, sysconf(int name), { return -1; })
RTEMS_STUB(int, unlink(const char *pathname), { return -1; })
RTEMS_STUB(pid_t, vfork(void), { return -1; })
#if !defined(_NO_POPEN) && !defined(_NO_WORDEXP)
/* pulled in by libc/sys/posix/popen.c and libc/sys/posix/word*.c */
RTEMS_STUB(int, waitpid (pid_t pid, int *status, int options), { return -1; })
#endif
RTEMS_STUB(_ssize_t, write (int fd, const void *buf, size_t nbytes), { return -1; })
RTEMS_STUB(ssize_t, writev (int fd, const struct iovec *iov, int iovcnt), { return -1; })

/* stubs for functions from reent.h */
RTEMS_STUB(int, _close_r (struct _reent *r, int fd), { return -1; })
#if defined(_NO_EXECVE)
RTEMS_STUB(int, _execve_r (struct _reent *r, char *, char **, char **), { return -1; })
#endif
RTEMS_STUB(int, _fcntl_r (struct _reent *ptr, int fd, int cmd, int arg ), { return -1; })
#if !(defined (REENTRANT_SYSCALLS_PROVIDED) || defined (NO_EXEC))
#ifndef NO_FORK
/* cf. newlib/libc/reent/execr.c */
RTEMS_STUB(int, _fork_r (struct _reent *r), { return -1; })
#endif
#endif
RTEMS_STUB(int, _fstat_r (struct _reent *r, int fd, struct stat *buf), { return -1; })
RTEMS_STUB(uid_t, geteuid (), { return -1; })
RTEMS_STUB(gid_t, getgid (), { return -1; })
RTEMS_STUB(gid_t, _getgid_r (struct _reent *r), { return -1; })
RTEMS_STUB(struct _reent *, __getreent (void), { return 0; })
RTEMS_STUB(pid_t, getpid (), { return -1; })
RTEMS_STUB(pid_t, getppid (), { return -1; })
RTEMS_STUB(pid_t, _getpid_r (struct _reent *r), { return -1; })
RTEMS_STUB(int, _gettimeofday_r(struct _reent *r, struct timeval *tp, void *tzp), { return 0; })
RTEMS_STUB(int, _isatty_r (struct _reent *r, int fd), { return isatty( fd ); })
RTEMS_STUB(int, _kill_r (struct _reent *r, int pid, int sig ), { return -1; })
#if !defined(REENTRANT_SYSCALLS_PROVIDED)
/* cf. newlib/libc/reent/linkr.c */
RTEMS_STUB(int, _link_r (struct _reent *r, const char *oldpath, const char *newpath), { return -1; })
#endif
RTEMS_STUB(_off_t, _lseek_r ( struct _reent *ptr, int fd, _off_t offset, int whence ), { return -1; })
RTEMS_STUB(int, _open_r (struct _reent *r, const char *buf, int flags, int mode), { return -1; })
RTEMS_STUB(_ssize_t, _read_r (struct _reent *r, int fd, void *buf, size_t nbytes), { return -1; })
RTEMS_STUB(int, _rename_r (struct _reent *r, const char *a, const char *b), { return -1; })
#if !(defined (REENTRANT_SYSCALLS_PROVIDED) || defined (MALLOC_PROVIDED))
/* cf. newlib/libc/reent/sbrkr.c */
RTEMS_STUB(void *,_sbrk_r (struct _reent *r, ptrdiff_t addr), { return 0; })
#endif
RTEMS_STUB(int, _stat_r (struct _reent *r, const char *path, struct stat *buf), { return -1; })
RTEMS_STUB(_CLOCK_T_, _times_r (struct _reent *r, struct tms *ptms), { return -1; })
RTEMS_STUB(int, _unlink_r (struct _reent *r, const char *path), { return -1; })
#if !(defined (REENTRANT_SYSCALLS_PROVIDED) || defined (NO_EXEC))
/* cf. newlib/libc/reent/execr.c */
RTEMS_STUB(int, _wait_r (struct _reent *r, int *status), { return -1; })
#endif
RTEMS_STUB(_ssize_t, _write_r (struct _reent *r, int fd, const void *buf, size_t nbytes), { return -1; })


RTEMS_STUB(int, _execve(const char *path, char * const *argv, char * const *envp), { return -1; })
RTEMS_STUB(void, _exit(int status), { while(1); })

/* Pulled in by newlib/libc/posix/glob.c */
#ifndef _NO_GLOB
#ifndef __NETBSD_SYSCALLS
RTEMS_STUB(int, issetugid (void), { return 0; })
#endif
#endif

/* stdlib.h */
RTEMS_STUB(void *, _realloc_r(struct _reent *r, void *p, size_t s), { return 0; })
RTEMS_STUB(void *, _calloc_r(struct _reent *r, size_t s1, size_t s2), { return 0; })
RTEMS_STUB(void *, _malloc_r(struct _reent * r, size_t s), { return 0; })
RTEMS_STUB(void, _free_r(struct _reent *r, void **p), { })

/* stubs for functions required by libc/stdlib */
RTEMS_STUB(void, __assert_func(const char *file, int line, const char *failedexpr), { })

#if defined(__arm__)
RTEMS_STUB(void, __aeabi_read_tp(void), { })
RTEMS_STUB(void *, __tls_get_addr(const void *ti), { })
#endif

/* The PowerPC expects certain symbols to be defined in the linker script. */

#if defined(__PPC__)
  int __SDATA_START__;  int __SDATA2_START__;
  int __GOT_START__;    int __GOT_END__;
  int __GOT2_START__;   int __GOT2_END__;
  int __SBSS_END__;     int __SBSS2_END__;
  int __FIXUP_START__;  int __FIXUP_END__;
  int __EXCEPT_START__; int __EXCEPT_END__;
  int __init;           int __fini;
  int __CTOR_LIST__;    int __CTOR_END__;
  int __DTOR_LIST__;    int __DTOR_END__;
#endif

/* The SH expects certain symbols to be defined in the linker script. */

#if defined(__sh__)
int __EH_FRAME_BEGIN__;
#endif

#if defined(__AVR__)
/*
 * Initial stack pointer address "__stack"
 *  hard coded into GCC instead of providing it through ldscripts
 */
const char* __stack ;
#endif
