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

#include <reent.h>

#include <signal.h> // sigset_t
#include <time.h> // struct timespec

void rtems_provides_crt0( void ) {}  /* dummy symbol so file always has one */

#define RTEMS_STUB(ret, func) \
ret rtems_stub_##func {}; \
ret func

/* RTEMS provides some of its own routines including a Malloc family */
RTEMS_STUB(void *,malloc(size_t s)) { return 0; }
RTEMS_STUB(void *,realloc(void* p, size_t s)) { return 0; }
RTEMS_STUB(void, free(void)) { ; }
RTEMS_STUB(_PTR, calloc(struct _reent *r, size_t s1, size_t s2)) {}

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
RTEMS_STUB(int, close (int fd)) { return -1; }
RTEMS_STUB(int, dup2(int oldfd, int newfd)) { return -1; }
RTEMS_STUB(int, fcntl( int fd, int cmd, ... /* arg */ )) { return -1; }
RTEMS_STUB(pid_t, fork(void)) { return -1; }
RTEMS_STUB(int, fstat(int fd, struct stat *buf)) { return -1; }
RTEMS_STUB(int, getdents(int fd, void *dp, int count)) { return -1; }
RTEMS_STUB(char *, getlogin(void)) { return 0; }
RTEMS_STUB(struct passwd *, getpwnam(const char *name)) { return 0; }
RTEMS_STUB(struct passwd *, getpwuid(uid_t uid)) { return 0; }
RTEMS_STUB(uid_t, getuid(void)) { return 0; }
RTEMS_STUB(int, nanosleep(const struct timespec *req, struct timespec *rem)) { return -1; }
RTEMS_STUB(_off_t, lseek(int fd, _off_t offset, int whence)) { return -1; }
RTEMS_STUB(int, lstat(const char *path, struct stat *buf)) { return -1; }
RTEMS_STUB(int, open(const char *pathname, int flags, int mode)) { return -1; }
RTEMS_STUB(int, pipe(int pipefd[2])) { return -1; }
RTEMS_STUB(_ssize_t, read(int fd, void *buf, size_t count)) { return -1; }
RTEMS_STUB(int, sigfillset(sigset_t *set)) { return -1; }
RTEMS_STUB(int, sigprocmask(int how, const sigset_t *set, sigset_t *oldset)) { return -1; }
RTEMS_STUB(int, stat(const char *path, struct stat *buf)) { return -1; }
RTEMS_STUB(int, unlink(const char *pathname)) { return -1; }
RTEMS_STUB(pid_t, vfork(void)) { return -1; }
#if !defined(_NO_POPEN) && !defined(_NO_WORDEXP)
/* pulled in by libc/sys/posix/popen.c and libc/sys/posix/word*.c */
RTEMS_STUB(int, waitpid (pid_t pid, int *status, int options)) { return -1; }
#endif
RTEMS_STUB(_ssize_t, write (int fd, const void *buf, size_t nbytes)) { return -1; }

/* stubs for functions from reent.h */
RTEMS_STUB(int, _close_r (struct _reent *r, int fd)) { return -1; }
#if defined(_NO_EXECVE)
RTEMS_STUB(int, _execve_r (struct _reent *r, char *, char **, char **)) { return -1; }
#endif
RTEMS_STUB(int, _fcntl_r (struct _reent *ptr, int fd, int cmd, int arg )) { return -1; }
#if !(defined (REENTRANT_SYSCALLS_PROVIDED) || defined (NO_EXEC))
#ifndef NO_FORK
/* cf. newlib/libc/reent/execr.c */
RTEMS_STUB(int, _fork_r (struct _reent *r)) { return -1; }
#endif
#endif
RTEMS_STUB(int, _fstat_r (struct _reent *r, int fd, struct stat *buf)) { return -1; }
RTEMS_STUB(int, _getpid_r (struct _reent *r)) { return -1; }
RTEMS_STUB(int, _gettimeofday_r(struct _reent *r, struct timeval *tp, void *tzp)) { return 0; }
RTEMS_STUB(int, _isatty_r (struct _reent *r, int fd)) { return isatty( fd ); }
RTEMS_STUB(int, _kill_r (struct _reent *r, int pid, int sig )) { return -1; }
#if !defined(REENTRANT_SYSCALLS_PROVIDED)
/* cf. newlib/libc/reent/linkr.c */
RTEMS_STUB(int, _link_r (struct _reent *, const char *, const char *)) { return -1; }
#endif
RTEMS_STUB(_off_t, _lseek_r ( struct _reent *ptr, int fd, _off_t offset, int whence )) { return -1; }
RTEMS_STUB(int, _open_r (struct _reent *r, const char *buf, int flags, int mode)) { return -1; }
RTEMS_STUB(_ssize_t, _read_r (struct _reent *r, int fd, void *buf, size_t nbytes)) { return -1; }
RTEMS_STUB(int, _rename_r (struct _reent *r, const char *a, const char *b)){ return -1; }
#if !(defined (REENTRANT_SYSCALLS_PROVIDED) || defined (MALLOC_PROVIDED))
/* cf. newlib/libc/reent/sbrkr.c */
RTEMS_STUB(void *,_sbrk_r (struct _reent *r, ptrdiff_t)) { return -1; }
#endif
RTEMS_STUB(int, _stat_r (struct _reent *r, const char *path, struct stat *buf)) { return -1; }
RTEMS_STUB(_CLOCK_T_, _times_r (struct _reent *r, struct tms *ptms)) { return -1; }
RTEMS_STUB(int, _unlink_r (struct _reent *r, const char *path)) { return -1; }
#if !(defined (REENTRANT_SYSCALLS_PROVIDED) || defined (NO_EXEC))
/* cf. newlib/libc/reent/execr.c */
RTEMS_STUB(int, _wait_r (struct _reent *r, int *)) { return -1; }
#endif
RTEMS_STUB(_ssize_t, _write_r (struct _reent *r, int fd, const void *buf, size_t nbytes)) { return -1; }


RTEMS_STUB(int, _execve(const char *path, char * const *argv, char * const *envp)) { return -1; }
RTEMS_STUB(void, _exit(int status)) { while(1); }

/* Pulled in by newlib/libc/posix/glob.c */
#ifndef _NO_GLOB
#ifndef __NETBSD_SYSCALLS
RTEMS_STUB(int, issetugid (void)) { return 0; }
#endif
#endif

/* stdlib.h */
RTEMS_STUB(_PTR, _realloc_r(struct _reent *r, _PTR p, size_t s)) {}
RTEMS_STUB(_PTR, _calloc_r(struct _reent *r, size_t s1, size_t s2)) {}
RTEMS_STUB(_PTR, _malloc_r(struct _reent *r, size_t s)) {}
RTEMS_STUB(_VOID, _free_r(struct _reent *r, _PTR p)) {}

/* stubs for functions required by libc/stdlib */
RTEMS_STUB(void, __assert_func(const char *file, int line, const char *failedexpr)) {}

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

/*  The hppa expects this to be defined in the real crt0.s. 
 *  Also for some reason, the hppa1.1 does not find atexit()
 *  during the AC_PROG_CC tests.
 */

#if defined(__hppa__)
/*
  asm ( ".subspa \$GLOBAL\$,QUAD=1,ALIGN=8,ACCESS=0x1f,SORT=40");
  asm ( ".export \$global\$" );
  asm ( "\$global\$:");
*/

  asm (".text");
  asm (".global");
  asm (".EXPORT $$dyncall,ENTRY");
  asm ("$$dyncall:");
  int atexit(void (*function)(void)) { return 0; }
#endif


/*
 *  The AMD a29k generates code expecting the following.
 */

#if defined(_AM29000) || defined(_AM29K)
asm (".global V_SPILL, V_FILL" );
asm (".global V_EPI_OS, V_BSD_OS" );

asm (".equ    V_SPILL, 64" );
asm (".equ    V_FILL, 65" );

asm (".equ    V_BSD_OS, 66" );
asm (".equ    V_EPI_OS, 69" );
#endif

#if defined(__AVR__)
/*
 * Initial stack pointer address "__stack"
 *  hard coded into GCC instead of providing it through ldscripts
 */
const char* __stack ;
#endif
