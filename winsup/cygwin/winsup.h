/* winsup.h: main Cygwin header file.

   Copyright 1996, 1997, 1998, 1999, 2000, 2001, 2002, 2003, 2004,
   2005, 2006, 2007 Red Hat, Inc.

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

#ifdef DEBUGIT
#define spf(a, b, c) small_printf (a, b, c)
#else
#define spf(a, b, c) do {} while (0)
#endif

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#define __INSIDE_CYGWIN__

#define strlen __builtin_strlen
#define strcmp __builtin_strcmp
#define strcpy __builtin_strcpy
#define memcpy __builtin_memcpy
#define memcmp __builtin_memcmp
#ifdef HAVE_BUILTIN_MEMSET
# define memset __builtin_memset
#endif

#define NO_COPY __attribute__((nocommon)) __attribute__((section(".data_cygwin_nocopy")))
#define NO_COPY_INIT __attribute__((section(".data_cygwin_nocopy")))

#define EXPORT_ALIAS(sym,symalias) extern "C" __typeof (sym) symalias __attribute__ ((alias(#sym)));

#define WINVER 0x0600
#define _NO_W32_PSEUDO_MODIFIERS

#include <sys/types.h>
#include <sys/strace.h>

/* Declarations for functions used in C and C++ code. */
#ifdef __cplusplus
extern "C" {
#endif
extern __uid32_t getuid32 (void);
extern __uid32_t geteuid32 (void);
extern int seteuid32 (__uid32_t);
extern __gid32_t getegid32 (void);
extern struct passwd *getpwuid32 (__uid32_t);
extern struct passwd *getpwnam (const char *);
extern struct __sFILE64 *fopen64 (const char *, const char *);
extern struct hostent *cygwin_gethostbyname (const char *name);
extern unsigned long cygwin_inet_addr (const char *cp);

#ifdef __cplusplus
}
#endif

/* Note that MAX_PATH is defined in the windows headers */
/* There is also PATH_MAX and MAXPATHLEN.
   PATH_MAX is from Posix and does include the trailing NUL.
   MAXPATHLEN is from Unix.

   Thou shalt *not* use CYG_MAX_PATH anymore.  Use NT_MAX_PATH or
   dynamic allocation instead when accessing real files.  Use
   MAX_PATH in case you need a convenient small buffer when creating
   names for synchronization objects or named pipes. */
#define CYG_MAX_PATH (MAX_PATH)

/* There's no define for the maximum path length the NT kernel can handle.
   That's why we define our own to use in path length test and for path
   buffer sizes.  As MAX_PATH and PATH_MAX, this is defined including the
   trailing 0.  Internal buffers and internal path routines should use
   NT_MAX_PATH.  PATH_MAX as defined in limits.h is the maximum length of
   application provided path strings we handle. */
#define NT_MAX_PATH 32768

#ifdef __cplusplus

extern const char case_folded_lower[];
#define cyg_tolower(c) (case_folded_lower[(unsigned char)(c)])
extern const char case_folded_upper[];
#define cyg_toupper(c) (case_folded_upper[(unsigned char)(c)])

#ifndef MALLOC_DEBUG
#define cfree newlib_cfree_dont_use
#endif

#define WIN32_LEAN_AND_MEAN 1
#define _WINGDI_H
#define _WINUSER_H
#define _WINNLS_H
#define _WINVER_H
#define _WINNETWK_H
#define _WINSVC_H
#include <windows.h>
#include <wincrypt.h>
#include <lmcons.h>
#include <ntdef.h>
#undef _WINGDI_H
#undef _WINUSER_H
#undef _WINNLS_H
#undef _WINVER_H
#undef _WINNETWK_H
#undef _WINSVC_H

#include "wincap.h"

/* The one function we use from winuser.h most of the time */
extern "C" DWORD WINAPI GetLastError (void);

/* Codepage and multibyte string specific stuff. */
enum codepage_type {ansi_cp, oem_cp, utf8_cp};
extern codepage_type current_codepage;
extern UINT active_codepage;

void codepage_init (const char *buf);
UINT get_cp ();
bool is_cp_multibyte (UINT cp);
const unsigned char *next_char (UINT cp, const unsigned char *str,
				const unsigned char *end);

/* Used as type by sys_wcstombs_alloc and sys_mbstowcs_alloc.  For a
   description see there. */
#define HEAP_NOTHEAP -1

int __stdcall sys_wcstombs (char *, int, const PWCHAR, int = -1)
  __attribute__ ((regparm(3)));
int __stdcall sys_wcstombs_alloc (char **, int, const PWCHAR, int = -1)
  __attribute__ ((regparm(3)));

int __stdcall sys_mbstowcs (PWCHAR, int, const char *, int = -1)
  __attribute__ ((regparm(3)));
int __stdcall sys_mbstowcs_alloc (PWCHAR *, int, const char *, int = -1)
  __attribute__ ((regparm(3)));

/* Used to check if Cygwin DLL is dynamically loaded. */
extern int dynamically_loaded;

extern int cygserver_running;

#define _MT_SAFE	// DELETEME someday

#define TITLESIZE 1024

#include "debug.h"

/* Events/mutexes */
extern HANDLE tty_mutex;

/**************************** Convenience ******************************/

/* Used to define status flag accessor methods */
#define IMPLEMENT_STATUS_FLAG(type,flag) \
  void flag (type val) { status.flag = (val); } \
  type flag () const { return (type) status.flag; }

/* Used when treating / and \ as equivalent. */
#define iswdirsep(ch) \
    ({ \
	WCHAR __c = (ch); \
	((__c) == L'/' || (__c) == L'\\'); \
    })

#define isdirsep(ch) \
    ({ \
	char __c = (ch); \
	((__c) == '/' || (__c) == '\\'); \
    })

/* Convert a signal to a signal mask */
#define SIGTOMASK(sig)	(1 << ((sig) - 1))

extern int __api_fatal_exit_val;
#define set_api_fatal_return(n) do {extern int __api_fatal_exit_val; __api_fatal_exit_val = (n);} while (0)
#define api_fatal(fmt, args...) __api_fatal (fmt,## args)

#undef issep
#define issep(ch) (strchr (" \t\n\r", (ch)) != NULL)

/* Every path beginning with / or \, as well as every path being X:
   or starting with X:/ or X:\ */
#define isabspath_u(p) \
  ((p)->Length && \
   (iswdirsep ((p)->Buffer[0]) || \
    ((p)->Length > sizeof (WCHAR) && iswalpha ((p)->Buffer[0]) \
    && (p)->Buffer[1] == L':' && \
    ((p)->Length == 2 * sizeof (WCHAR) || iswdirsep ((p)->Buffer[2])))))

#define iswabspath(p) \
  (iswdirsep (*(p)) || (iswalpha (*(p)) && (p)[1] == L':' && (!(p)[2] || iswdirsep ((p)[2]))))

#define isabspath(p) \
  (isdirsep (*(p)) || (isalpha (*(p)) && (p)[1] == ':' && (!(p)[2] || isdirsep ((p)[2]))))

/******************** Initialization/Termination **********************/

class per_process;
/* cygwin .dll initialization */
void dll_crt0 (per_process *) __asm__ ("_dll_crt0__FP11per_process");
extern "C" void __stdcall _dll_crt0 ();
extern void dll_crt0_1 (void *);
extern void dll_dllcrt0_1 (void *);
extern int __stdcall spawn_guts (const char * prog_arg, const char *const *argv,
				 const char *const envp[], int mode,
				 int __stdin = -1, int __stdout = -1);

/* dynamically loaded dll initialization */
extern "C" int dll_dllcrt0 (HMODULE, per_process *);

/* dynamically loaded dll initialization for non-cygwin apps */
extern "C" int dll_noncygwin_dllcrt0 (HMODULE, per_process *);

/* exit the program */

enum exit_states
  {
    ES_NOT_EXITING = 0,
    ES_PROCESS_LOCKED,
    ES_GLOBAL_DTORS,
    ES_EVENTS_TERMINATE,
    ES_THREADTERM,
    ES_SIGNAL,
    ES_CLOSEALL,
    ES_HUP_PGRP,
    ES_HUP_SID,
    ES_EXEC_EXIT,
    ES_TITLE,
    ES_TTY_TERMINATE,
    ES_FINAL
  };

extern exit_states exit_state;
void __stdcall do_exit (int) __attribute__ ((regparm (1), noreturn));

/* UID/GID */
void uinfo_init ();

#define ILLEGAL_UID16 ((__uid16_t)-1)
#define ILLEGAL_UID ((__uid32_t)-1)
#define ILLEGAL_GID16 ((__gid16_t)-1)
#define ILLEGAL_GID ((__gid32_t)-1)
#define ILLEGAL_SEEK ((_off64_t)-1)

#define uid16touid32(u16)  ((u16)==ILLEGAL_UID16?ILLEGAL_UID:(__uid32_t)(u16))
#define gid16togid32(g16)  ((g16)==ILLEGAL_GID16?ILLEGAL_GID:(__gid32_t)(g16))

/* Convert LARGE_INTEGER into long long */
#define get_ll(pl)  (((long long) (pl).HighPart << 32) | (pl).LowPart)

/* various events */
void events_init ();
void events_terminate ();

void __stdcall close_all_files (bool = false);

/* debug_on_trap support. see exceptions.cc:try_to_debug() */
extern "C" void error_start_init (const char*);
extern "C" int try_to_debug (bool waitloop = 1);

void ld_preload ();
const char *find_first_notloaded_dll (class path_conv &);

void set_file_api_mode (codepage_type);

extern bool cygwin_finished_initializing;

/**************************** Miscellaneous ******************************/

void __stdcall set_std_handle (int);
int __stdcall stat_dev (DWORD, int, unsigned long, struct __stat64 *);

__ino64_t __stdcall hash_path_name (__ino64_t hash, PUNICODE_STRING name) __attribute__ ((regparm(2)));
__ino64_t __stdcall hash_path_name (__ino64_t hash, PCWSTR name) __attribute__ ((regparm(2)));
__ino64_t __stdcall hash_path_name (__ino64_t hash, const char *name) __attribute__ ((regparm(2)));
void __stdcall nofinalslash (const char *src, char *dst) __attribute__ ((regparm(2)));

/* String manipulation */
extern "C" char *__stdcall strccpy (char *s1, const char **s2, char c);

void *hook_or_detect_cygwin (const char *, const void *, WORD&) __attribute__ ((regparm (3)));

/* Time related */
void __stdcall totimeval (struct timeval *, FILETIME *, int, int);
long __stdcall to_time_t (FILETIME *);
void __stdcall to_timestruc_t (FILETIME *, timestruc_t *);
void __stdcall time_as_timestruc_t (timestruc_t *);
void __stdcall timeval_to_filetime (const struct timeval *, FILETIME *);

/* Console related */
void __stdcall set_console_title (char *);
void init_console_handler (bool);

void init_global_security ();

int __stdcall check_invalid_virtual_addr (const void *s, unsigned sz) __attribute__ ((regparm(2)));

ssize_t check_iovec (const struct iovec *, int, bool) __attribute__ ((regparm(3)));
#define check_iovec_for_read(a, b) check_iovec ((a), (b), false)
#define check_iovec_for_write(a, b) check_iovec ((a), (b), true)

#define set_winsock_errno() __set_winsock_errno (__FUNCTION__, __LINE__)
void __set_winsock_errno (const char *fn, int ln) __attribute__ ((regparm(2)));

extern bool wsock_started;

/* Printf type functions */
extern "C" void __api_fatal (const char *, ...) __attribute__ ((noreturn));
extern "C" int __small_sprintf (char *dst, const char *fmt, ...) /*__attribute__ ((regparm (2)))*/;
extern "C" int __small_vsprintf (char *dst, const char *fmt, va_list ap) /*__attribute__ ((regparm (3)))*/;
extern "C" int __small_swprintf (PWCHAR dst, const WCHAR *fmt, ...) /*__attribute__ ((regparm (2)))*/;
extern "C" int __small_vswprintf (PWCHAR dst, const WCHAR *fmt, va_list ap) /*__attribute__ ((regparm (3)))*/;
extern void multiple_cygwin_problem (const char *, unsigned, unsigned);

extern "C" void vklog (int priority, const char *message, va_list ap);
extern "C" void klog (int priority, const char *message, ...);
bool child_copy (HANDLE, bool, ...);

int symlink_worker (const char *, const char *, bool, bool)
  __attribute__ ((regparm (3)));

class path_conv;

int fcntl_worker (int fd, int cmd, void *arg);
int __stdcall stat_worker (path_conv &pc, struct __stat64 *buf) __attribute__ ((regparm (2)));

__ino64_t __stdcall readdir_get_ino (const char *path, bool dot_dot) __attribute__ ((regparm (2)));

extern "C" int low_priority_sleep (DWORD) __attribute__ ((regparm (1)));
#define SLEEP_0_STAY_LOW INFINITE

/* Returns the real page size, not the allocation size. */
size_t getsystempagesize ();

/* mmap functions. */
enum mmap_region_status
  {
    MMAP_NONE,
    MMAP_RAISE_SIGBUS,
    MMAP_NORESERVE_COMMITED
  };
mmap_region_status mmap_is_attached_or_noreserve (void *addr, size_t len);

int winprio_to_nice (DWORD) __attribute__ ((regparm (1)));
DWORD nice_to_winprio (int &) __attribute__ ((regparm (1)));

bool __stdcall create_pipe (PHANDLE, PHANDLE, LPSECURITY_ATTRIBUTES, DWORD)
  __attribute__ ((regparm (3)));
#define CreatePipe create_pipe

inline bool flush_file_buffers (HANDLE h)
{
  return (GetFileType (h) != FILE_TYPE_PIPE) ? FlushFileBuffers (h) : true;
}
#define FlushFileBuffers flush_file_buffers

/**************************** Exports ******************************/

extern "C" {
int cygwin_select (int , fd_set *, fd_set *, fd_set *,
		   struct timeval *to);
int cygwin_gethostname (char *__name, size_t __len);

extern DWORD binmode;
extern char _data_start__, _data_end__, _bss_start__, _bss_end__;
extern void (*__CTOR_LIST__) (void);
extern void (*__DTOR_LIST__) (void);
extern SYSTEM_INFO system_info;
};

/*************************** Unsorted ******************************/

#define WM_ASYNCIO	0x8000		// WM_APP


#define STD_RBITS (S_IRUSR | S_IRGRP | S_IROTH)
#define STD_WBITS (S_IWUSR)
#define STD_XBITS (S_IXUSR | S_IXGRP | S_IXOTH)
#define NO_W ~(S_IWUSR | S_IWGRP | S_IWOTH)
#define NO_R ~(S_IRUSR | S_IRGRP | S_IROTH)
#define NO_X ~(S_IXUSR | S_IXGRP | S_IXOTH)

/* The title on program start. */
extern char *old_title;
extern bool display_title;
extern bool transparent_exe;

extern bool in_forkee;

extern HANDLE hMainThread;
extern HANDLE hMainProc;
extern HANDLE hProcToken;
extern HANDLE hProcImpToken;

inline void clear_procimptoken ()
{
  if (hProcImpToken)
    {
      HANDLE old_procimp = hProcImpToken;
      hProcImpToken = NULL;
      CloseHandle (old_procimp);
    }
}

extern HANDLE hExeced;
extern HMODULE cygwin_hmodule;

extern bool cygwin_testing;

extern char almost_null[];

#endif /* defined __cplusplus */
