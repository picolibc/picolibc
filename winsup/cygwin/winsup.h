/* winsup.h: main Cygwin header file.

   Copyright 1996, 1997, 1998, 1999, 2000 Cygnus Solutions.

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#define __INSIDE_CYGWIN__

#define alloca(x) __builtin_alloca (x)
#define strlen __builtin_strlen
#define strcpy __builtin_strcpy
#define memcpy __builtin_memcpy
#define memcmp __builtin_memcmp
#ifdef HAVE_BUILTIN_MEMSET
# define memset __builtin_memset
#endif

#include <sys/types.h>
#include <sys/strace.h>
#include <sys/resource.h>
#include <setjmp.h>
#include <signal.h>
#include <string.h>

#undef strchr
#define strchr cygwin_strchr
extern inline char * strchr(const char * s, int c)
{
register char * __res;
__asm__ __volatile__(
	"movb %%al,%%ah\n"
	"1:\tmovb (%1),%%al\n\t"
	"cmpb %%ah,%%al\n\t"
	"je 2f\n\t"
	"incl %1\n\t"
	"testb %%al,%%al\n\t"
	"jne 1b\n\t"
	"xorl %1,%1\n"
	"2:\tmovl %1,%0\n\t"
	:"=a" (__res), "=r" (s)
	:"0" (c),      "1"  (s));
return __res;
}

#include <windows.h>
#include <wincrypt.h>

/* Used for runtime OS check/decisions. */
enum os_type {winNT = 1, win95, win98, win32s, unknown};
extern os_type os_being_run;

/* Used to check if Cygwin DLL is dynamically loaded. */
extern int dynamically_loaded;

#include <cygwin/version.h>

#define TITLESIZE 1024
#define MAX_USER_NAME 20
#define DEFAULT_UID 500
#define DEFAULT_GID 544

#define MAX_SID_LEN 40
#define MAX_HOST_NAME 256

/* status bit manipulation */
#define __ISSETF(what, x, prefix) \
  ((what)->status & prefix##_##x)
#define __SETF(what, x, prefix) \
  ((what)->status |= prefix##_##x)
#define __CLEARF(what, x, prefix) \
  ((what)->status &= ~prefix##_##x)
#define __CONDSETF(n, what, x, prefix) \
  ((n) ? __SETF (what, x, prefix) : __CLEARF (what, x, prefix))

#include "thread.h"
#include "shared.h"

extern HANDLE hMainThread;
extern HANDLE hMainProc;

/* Now that pinfo has been defined, include... */
#include "debug.h"
#include "sync.h"
#include "sigproc.h"
#include "fhandler.h"
#include "path.h"
#include <sys/cygwin.h>

/********************** Application Interface **************************/

extern "C" per_process __cygwin_user_data; /* Pointer into application's static data */
#define user_data (&__cygwin_user_data)

/* We use the following to test that sizeof hasn't changed.  When adding
   or deleting members, insert fillers or use the reserved entries.
   Do not change this value. */
#define SIZEOF_PER_PROCESS (42 * 4)

class hinfo
{
  fhandler_base **fds;
  fhandler_base **fds_on_hold;
  int first_fd_for_open;
public:
  size_t size;
  hinfo () {first_fd_for_open = 3;}
  int vfork_child_dup ();
  void vfork_parent_restore ();
  fhandler_base *dup_worker (fhandler_base *oldfh);
  int extend (int howmuch);
  void fixup_after_fork (HANDLE parent);
  fhandler_base *build_fhandler (int fd, DWORD dev, const char *name,
				 int unit = -1);
  fhandler_base *build_fhandler (int fd, const char *name, HANDLE h);
  int not_open (int n);
  int find_unused_handle (int start);
  int find_unused_handle () { return find_unused_handle (first_fd_for_open);}
  void release (int fd);
  void init_std_file_from_handle (int fd, HANDLE handle, DWORD access, const char *name);
  int dup2 (int oldfd, int newfd);
  int linearize_fd_array (unsigned char *buf, int buflen);
  LPBYTE de_linearize_fd_array (LPBYTE buf);
  fhandler_base *operator [](int fd) { return fds[fd]; }
  select_record *select_read (int fd, select_record *s);
  select_record *select_write (int fd, select_record *s);
  select_record *select_except (int fd, select_record *s);
};

/******************* Host-dependent constants **********************/
/* Portions of the cygwin DLL require special constants whose values
   are dependent on the host system.  Rather than dynamically
   determine those values whenever they are required, initialize these
   values once at process start-up. */

class host_dependent_constants
{
 public:
  void init (void);

  /* Used by fhandler_disk_file::lock which needs a platform-specific
     upper word value for locking entire files. */
  DWORD win32_upper;

  /* fhandler_base::open requires host dependent file sharing
     attributes. */
  int shared;
};

extern host_dependent_constants host_dependent;

/* Events/mutexes */
extern HANDLE pinfo_mutex;
extern HANDLE title_mutex;



/*************************** Per Thread ******************************/

#define PER_THREAD_FORK_CLEAR ((void *)0xffffffff)
class per_thread
{
  DWORD tls;
  int clear_on_fork_p;
public:
  per_thread (int forkval = 1) {tls = TlsAlloc (); clear_on_fork_p = forkval;}
  DWORD get_tls () {return tls;}
  int clear_on_fork () {return clear_on_fork_p;}

  virtual void *get () {return TlsGetValue (get_tls ());}
  virtual size_t size () {return 0;}
  virtual void set (void *s = NULL);
  virtual void set (int n) {TlsSetValue (get_tls (), (void *)n);}
  virtual void *create ()
  {
    void *s = new char [size ()];
    memset (s, 0, size ());
    set (s);
    return s;
  }
};

class per_thread_waitq : public per_thread
{
public:
  per_thread_waitq () : per_thread (0) {}
  void *get () {return (waitq *) this->per_thread::get ();}
  void *create () {return (waitq *) this->per_thread::create ();}
  size_t size () {return sizeof (waitq);}
};

struct vfork_save
{
  int pid;
  jmp_buf j;
  char **vfork_ebp;
  char *caller_ebp;
  char *retaddr;
  int is_active () { return pid < 0; }
};

class per_thread_vfork : public per_thread
{
public:
  vfork_save *val () { return (vfork_save *) this->per_thread::get (); }
  vfork_save *create () {return (vfork_save *) this->per_thread::create ();}
  size_t size () {return sizeof (vfork_save);}
};

extern "C" {
struct signal_dispatch
{
  int arg;
  void (*func) (int);
  int sig;
  int saved_errno;
  DWORD oldmask;
  DWORD retaddr;
  DWORD *retaddr_on_stack;
};
};

struct per_thread_signal_dispatch : public per_thread
{
  signal_dispatch *get () { return (signal_dispatch *) this->per_thread::get (); }
  signal_dispatch *create () {return (signal_dispatch *) this->per_thread::create ();}
  size_t size () {return sizeof (signal_dispatch);}
};

extern per_thread_waitq waitq_storage;
extern per_thread_vfork vfork_storage;
extern per_thread_signal_dispatch signal_dispatch_storage;

extern per_thread *threadstuff[];

/**************************** Convenience ******************************/

#define NO_COPY __attribute__((section(".data_cygwin_nocopy")))

/* Used when treating / and \ as equivalent. */
#define SLASH_P(ch) \
    ({ \
	char __c = (ch); \
	((__c) == '/' || (__c) == '\\'); \
    })

/* Convert a signal to a signal mask */
#define SIGTOMASK(sig)	(1<<((sig) - signal_shift_subtract))
extern unsigned int signal_shift_subtract;

#ifdef NOSTRACE
#define MARK() 0
#else
#define MARK() mark (__FILE__,__LINE__)
#endif

#define api_fatal(fmt, args...) \
  __api_fatal ("%P: *** " fmt, ## args)

#undef issep
#define issep(ch) (strchr (" \t\n\r", (ch)) != NULL)

#define isdirsep SLASH_P
#define isabspath(p) \
  (isdirsep (*(p)) || (isalpha (*(p)) && (p)[1] == ':' && (!(p)[2] || isdirsep ((p)[2]))))

/******************** Initialization/Termination **********************/

/* cygwin .dll initialization */
void dll_crt0 (per_process *);
extern "C" void __stdcall _dll_crt0 ();

/* dynamically loaded dll initialization */
extern "C" int dll_dllcrt0 (HMODULE, per_process*);

/* dynamically loaded dll initialization for non-cygwin apps */
extern "C" int dll_noncygwin_dllcrt0 (HMODULE, per_process *);

/* exit the program */
extern "C" void __stdcall do_exit (int) __attribute__ ((noreturn));

/* Initialize the environment */
void environ_init (int);

/* Heap management. */
void heap_init (void);
void malloc_init (void);

/* fd table */
void dtable_init (void);
void hinfo_init (void);
extern hinfo dtable;

/* UID/GID */
void uinfo_init (void);

/* various events */
void events_init (void);
void events_terminate (void);

void __stdcall close_all_files (void);

extern class strace strace;

/* Invisible window initialization/termination. */
HWND __stdcall gethwnd (void);
void __stdcall window_terminate (void);

/* Globals that handle initialization of winsock in a child process. */
extern HANDLE wsock32_handle;

/* Globals that handle initialization of netapi in a child process. */
extern HANDLE netapi32_handle;

/* debug_on_trap support. see exceptions.cc:try_to_debug() */
extern "C" void error_start_init (const char*);
extern "C" int try_to_debug ();

/**************************** Miscellaneous ******************************/

const char * __stdcall find_exec (const char *name, path_conv& buf, const char *winenv = "PATH=",
			int null_if_notfound = 0, const char **known_suffix = NULL);

/* File manipulation */
int __stdcall set_process_privileges ();
int __stdcall get_file_attribute (int, const char *, int *,
                                  uid_t * = NULL, gid_t * = NULL);
int __stdcall set_file_attribute (int, const char *, int);
int __stdcall set_file_attribute (int, const char *, uid_t, gid_t, int, const char *);
void __stdcall set_std_handle (int);
int __stdcall writable_directory (const char *file);
int __stdcall stat_dev (DWORD, int, unsigned long, struct stat *);
extern BOOL allow_ntsec;

/* `lookup_name' should be called instead of LookupAccountName.
 * logsrv may be NULL, in this case only the local system is used for lookup.
 * The buffer for ret_sid (40 Bytes) has to be allocated by the caller! */
BOOL __stdcall lookup_name (const char *, const char *, PSID);
char *__stdcall convert_sid_to_string_sid (PSID, char *);
PSID __stdcall convert_string_sid_to_sid (PSID, const char *);
BOOL __stdcall get_pw_sid (PSID, struct passwd *);

unsigned long __stdcall hash_path_name (unsigned long hash, const char *name);
void __stdcall nofinalslash (const char *src, char *dst);
extern "C" char *__stdcall rootdir (char *full_path);

void __stdcall mark (const char *, int);

extern "C" int _spawnve (HANDLE hToken, int mode, const char *path,
			 const char *const *argv, const char *const *envp);
int __stdcall spawn_guts (HANDLE hToken, const char *prog_arg,
		   const char *const *argv, const char *const envp[],
		   pinfo *child, int mode);

/* For mmaps across fork(). */
int __stdcall recreate_mmaps_after_fork (void *);
void __stdcall set_child_mmap_ptr (pinfo *);

/* String manipulation */
char *__stdcall strccpy (char *s1, const char **s2, char c);
int __stdcall strcasematch (const char *s1, const char *s2);
int __stdcall strncasematch (const char *s1, const char *s2, size_t n);
char *__stdcall strcasestr (const char *searchee, const char *lookfor);

/* Time related */
void __stdcall totimeval (struct timeval *dst, FILETIME * src, int sub, int flag);
long __stdcall to_time_t (FILETIME * ptr);

/* pinfo table manipulation */
#ifndef lock_pinfo_for_update
int __stdcall lock_pinfo_for_update (DWORD timeout);
#endif
void unlock_pinfo (void);
pinfo *__stdcall set_myself (pinfo *);

/* Retrieve a security descriptor that allows all access */
SECURITY_DESCRIPTOR *__stdcall get_null_sd (void);

int __stdcall get_id_from_sid (PSID, BOOL);
extern inline int get_uid_from_sid (PSID psid) { return get_id_from_sid (psid, FALSE);}
extern inline int get_gid_from_sid (PSID psid) { return get_id_from_sid (psid, TRUE); }

int __stdcall NTReadEA (const char *file, const char *attrname, char *buf, int len);
BOOL __stdcall NTWriteEA (const char *file, const char *attrname, char *buf, int len);

void __stdcall set_console_title (char *);
void set_console_handler ();

void __stdcall fill_rusage (struct rusage *, HANDLE);
void __stdcall add_rusage (struct rusage *, struct rusage *);

void set_winsock_errno ();

/**************************** Exports ******************************/

extern "C" {
int cygwin_select (int , fd_set *, fd_set *, fd_set *,
		   struct timeval *to);
int cygwin_gethostname (char *__name, size_t __len);

int kill_pgrp (pid_t, int);
int _kill (int, int);
int _raise (int sig);

int getdtablesize ();
void setdtablesize (int);

extern char _data_start__, _data_end__, _bss_start__, _bss_end__;
extern void (*__CTOR_LIST__) (void);
extern void (*__DTOR_LIST__) (void);
};

/*************************** Unsorted ******************************/

#define WM_ASYNCIO	0x8000		// WM_APP

/* Note that MAX_PATH is defined in the windows headers */
/* There is also PATH_MAX and MAXPATHLEN.
   PATH_MAX is from Posix and does *not* include the trailing NUL.
   MAXPATHLEN is from Unix.

   Thou shalt use MAX_PATH throughout.  It avoids the NUL vs no-NUL
   issue and is neither of the Unixy ones [so we can punt on which
   one is the right one to use].  */

/* Initial and increment values for cygwin's fd table */
#define NOFILE_INCR    32

#ifdef __cplusplus
extern "C" {
#endif
#include <sys/reent.h>

#define STD_RBITS (S_IRUSR | S_IRGRP | S_IROTH)
#define STD_WBITS (S_IWUSR)
#define STD_XBITS (S_IXUSR | S_IXGRP | S_IXOTH)

#define O_NOSYMLINK 0x080000
#define O_DIROPEN   0x100000

#ifdef __cplusplus
}
#endif

/*************************** Environment ******************************/

/* The structure below is used to control conversion to/from posix-style
 * file specs.  Currently, only PATH and HOME are converted, but PATH
 * needs to use a "convert path list" function while HOME needs a simple
 * "convert to posix/win32".  For the simple case, where a calculated length
 * is required, just return MAX_PATH.  *FIXME*
 */
struct win_env
  {
    const char *name;
    size_t namelen;
    char *posix;
    char *native;
    int (*toposix) (const char *, char *);
    int (*towin32) (const char *, char *);
    int (*posix_len) (const char *);
    int (*win32_len) (const char *);
    void add_cache (const char *in_posix, const char *in_native = NULL);
    const char * get_native () {return native ? native + namelen : NULL;}
  };

win_env * __stdcall getwinenv (const char *name, const char *posix = NULL);

char * __stdcall winenv (const char * const *, int);
extern char **__cygwin_environ;

/* The title on program start. */
extern char *old_title;
extern BOOL display_title;


/*************************** errno manipulation ******************************/

void seterrno_from_win_error (const char *file, int line, int code);
void seterrno (const char *, int line);

#define __seterrno() seterrno (__FILE__, __LINE__)
#define __seterrno_from_win_error(val) seterrno_from_win_error (__FILE__, __LINE__, val)
#undef errno
#define errno dont_use_this_since_were_in_a_shared library
#define set_errno(val) (_impure_ptr->_errno = (val))
#define get_errno()  (_impure_ptr->_errno)
extern "C" void __stdcall set_sig_errno (int e);

class save_errno
  {
    int saved;
  public:
    save_errno () {saved = get_errno ();}
    save_errno (int what) {saved = get_errno (); set_errno (what); }
    void set (int what) {set_errno (what); saved = what;}
    void reset () {saved = get_errno ();}
    ~save_errno () {set_errno (saved);}
  };

extern const char *__sp_fn;
extern int __sp_ln;
