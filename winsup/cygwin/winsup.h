/* winsup.h: main Cygwin header file.

   Copyright 1996, 1997, 1998, 1999, 2000 Red Hat, Inc.

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#define __INSIDE_CYGWIN__

#ifdef __cplusplus

#define alloca(x) __builtin_alloca (x)
#define strlen __builtin_strlen
#define strcpy __builtin_strcpy
#define memcpy __builtin_memcpy
#define memcmp __builtin_memcmp
#ifdef HAVE_BUILTIN_MEMSET
# define memset __builtin_memset
#endif

#if !defined(__STDC_VERSION__) || __STDC_VERSION__ >= 199900L
#define NEW_MACRO_VARARGS
#endif

#include <sys/types.h>
#include <sys/strace.h>

#undef strchr
#define strchr cygwin_strchr
extern "C" inline __stdcall char * strchr(const char * s, int c)
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

#define cfree newlib_cfree_dont_use

#define WIN32_LEAN_AND_MEAN 1
#define _WINGDI_H
#define _WINUSER_H
#define _WINNLS_H
#define _WINVER_H
#define _WINNETWK_H
#define _WINSVC_H
#include <windows.h>
#include <wincrypt.h>
#undef _WINGDI_H
#undef _WINUSER_H
#undef _WINNLS_H
#undef _WINVER_H
#undef _WINNETWK_H
#undef _WINSVC_H

/* The one function we use from winuser.h most of the time */
extern "C" DWORD WINAPI GetLastError (void);

/* Used for runtime OS check/decisions. */
enum os_type {winNT = 1, win95, win98, win32s, unknown};
extern os_type os_being_run;

/* Used to check if Cygwin DLL is dynamically loaded. */
extern int dynamically_loaded;

#define sys_wcstombs(tgt,src,len) \
		    WideCharToMultiByte(CP_ACP,0,(src),-1,(tgt),(len),NULL,NULL)
#define sys_mbstowcs(tgt,src,len) \
		    MultiByteToWideChar(CP_ACP,0,(src),-1,(tgt),(len))

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

#include "debug.h"

/* Events/mutexes */
extern HANDLE title_mutex;

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

#ifdef NEW_MACRO_VARARGS
# define api_fatal(...) __api_fatal ("%P: *** " __VA_ARGS__)
#else
# define api_fatal(fmt, args...) __api_fatal ("%P: *** " fmt,## args)
#endif

#undef issep
#define issep(ch) (strchr (" \t\n\r", (ch)) != NULL)

#define isdirsep SLASH_P
#define isabspath(p) \
  (isdirsep (*(p)) || (isalpha (*(p)) && (p)[1] == ':' && (!(p)[2] || isdirsep ((p)[2]))))

/******************** Initialization/Termination **********************/

class per_process;
/* cygwin .dll initialization */
void dll_crt0 (per_process *);
extern "C" void __stdcall _dll_crt0 ();

/* dynamically loaded dll initialization */
extern "C" int dll_dllcrt0 (HMODULE, per_process *);

/* dynamically loaded dll initialization for non-cygwin apps */
extern "C" int dll_noncygwin_dllcrt0 (HMODULE, per_process *);

/* exit the program */
extern "C" void __stdcall do_exit (int) __attribute__ ((noreturn));

/* UID/GID */
void uinfo_init (void);

/* various events */
void events_init (void);
void events_terminate (void);

void __stdcall close_all_files (void);

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

extern int cygwin_finished_initializing;

/**************************** Miscellaneous ******************************/

void __stdcall set_std_handle (int);
int __stdcall writable_directory (const char *file);
int __stdcall stat_dev (DWORD, int, unsigned long, struct stat *);
extern BOOL allow_ntsec;

unsigned long __stdcall hash_path_name (unsigned long hash, const char *name);
void __stdcall nofinalslash (const char *src, char *dst);
extern "C" char *__stdcall rootdir (char *full_path);

/* String manipulation */
char *__stdcall strccpy (char *s1, const char **s2, char c);
int __stdcall strcasematch (const char *s1, const char *s2);
int __stdcall strncasematch (const char *s1, const char *s2, size_t n);
char *__stdcall strcasestr (const char *searchee, const char *lookfor);

/* Time related */
void __stdcall totimeval (struct timeval *dst, FILETIME * src, int sub, int flag);
long __stdcall to_time_t (FILETIME * ptr);

void __stdcall set_console_title (char *);
void set_console_handler ();

void set_winsock_errno ();

/* Printf type functions */
extern "C" void __api_fatal (const char *, ...) __attribute__ ((noreturn));
extern "C" int __small_sprintf (char *dst, const char *fmt, ...);
extern "C" int __small_vsprintf (char *dst, const char *fmt, va_list ap);

/**************************** Exports ******************************/

extern "C" {
int cygwin_select (int , fd_set *, fd_set *, fd_set *,
		   struct timeval *to);
int cygwin_gethostname (char *__name, size_t __len);

int kill_pgrp (pid_t, int);
int _kill (int, int);
int _raise (int sig);

extern DWORD binmode;
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

#define STD_RBITS (S_IRUSR | S_IRGRP | S_IROTH)
#define STD_WBITS (S_IWUSR)
#define STD_XBITS (S_IXUSR | S_IXGRP | S_IXOTH)

#define O_NOSYMLINK 0x080000
#define O_DIROPEN   0x100000

/* The title on program start. */
extern char *old_title;
extern BOOL display_title;

extern HANDLE hMainThread;
extern HANDLE hMainProc;

#endif /* defined __cplusplus */
