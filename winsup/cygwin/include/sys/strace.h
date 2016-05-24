/* sys/strace.h

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

/* This file contains routines for tracing system calls and other internal
   phenomenon.

   When tracing system calls, try to use the same style throughout:

   result = syscall (arg1, arg2, arg3) [optional extra stuff]

   If a system call can block (eg: read, write, wait), print another message
   before hanging so the user will know why the program has stopped.

   Note: __seterrno will also print a trace message.  Have that printed
   *first*.  This will make it easy to always know what __seterrno is
   refering to.  For the same reason, try not to have __seterrno messages
   printed alone.
*/

#ifndef _SYS_STRACE_H
#define _SYS_STRACE_H

#include <stdarg.h>
#include <sys/types.h>

#ifdef __cplusplus

class child_info;
class strace
{
  int vsprntf (char *buf, const char *func, const char *infmt, va_list ap);
  void write (unsigned category, const char *buf, int count);
  unsigned char _active;
public:
  void activate (bool) __attribute__ ((__regparm__ (2)));;
  strace () {}
  int microseconds ();
  int version;
  int lmicrosec;
  bool execing;
  void dll_info () __attribute__ ((__regparm__ (1)));
  void prntf (unsigned, const char *func, const char *, ...) /*__attribute__ ((__regparm__(3)))*/;
  void vprntf (unsigned, const char *func, const char *, va_list ap) /*__attribute__ ((__regparm__(3)))*/;
  void wm (int message, int word, int lon) __attribute__ ((__regparm__(3)));
  void write_childpid (pid_t) __attribute__ ((__regparm__ (3)));
  bool attached () const {return _active == 3;}
  bool active () const {return _active & 1;}
  unsigned char& active_val () {return _active;}
};

extern strace strace;

#endif /* __cplusplus */

#define _STRACE_INTERFACE_ACTIVATE_ADDR  -1
#define _STRACE_INTERFACE_ACTIVATE_ADDR1 -2
#define _STRACE_CHILD_PID -3

/* Bitmasks of tracing messages to print.  */

#define _STRACE_ALL	 0x000001 // so behaviour of strace=1 is unchanged
#define _STRACE_FLUSH	 0x000002 // flush output buffer after every message
#define _STRACE_INHERIT  0x000004 // children inherit mask from parent
#define _STRACE_UHOH	 0x000008 // unusual or weird phenomenon
#define _STRACE_SYSCALL	 0x000010 // system calls
#define _STRACE_STARTUP	 0x000020 // argc/envp printout at startup
#define _STRACE_DEBUG    0x000040 // info to help debugging
#define _STRACE_PARANOID 0x000080 // paranoid info
#define _STRACE_TERMIOS	 0x000100 // info for debugging termios stuff
#define _STRACE_SELECT	 0x000200 // info on ugly select internals
#define _STRACE_WM	 0x000400 // trace windows messages (enable _strace_wm)
#define _STRACE_SIGP	 0x000800 // trace signal and process handling
#define _STRACE_MINIMAL	 0x001000 // very minimal strace output
#define _STRACE_PTHREAD	 0x002000 // pthread calls
#define _STRACE_EXITDUMP 0x004000 // dump strace cache on exit
#define _STRACE_SYSTEM	 0x008000 // cache strace messages
#define _STRACE_NOMUTEX	 0x010000 // don't use mutex for synchronization
#define _STRACE_MALLOC	 0x020000 // trace malloc calls
#define _STRACE_THREAD	 0x040000 // cygthread calls
#define _STRACE_NOTALL	 0x080000 // don't include if _STRACE_ALL
#define _STRACE_SPECIAL	 0x100000 // special case, only for debugging - do not check in

#ifdef __cplusplus
extern "C" {
#endif

void small_printf (const char *, ...);
void strace_printf (unsigned, const char *func, const char *, ...);

#ifdef __cplusplus
}
#endif

#ifdef __cplusplus

#ifdef NOSTRACE
#define strace_printf_wrap(what, fmt, args...)
#define strace_printf_wrap1(what, fmt, args...)
#else
#define strace_printf_wrap(what, fmt, args...) \
   ((void) ({\
	if ((_STRACE_ ## what & _STRACE_SYSTEM) || strace.active ()) \
	  strace.prntf(_STRACE_ ## what, __PRETTY_FUNCTION__, fmt, ## args); \
	0; \
    }))
#define strace_printf_wrap1(what, fmt, args...) \
    ((void) ({\
	if ((_STRACE_ ## what & _STRACE_SYSTEM) || strace.active ()) \
	  strace.prntf((_STRACE_ ## what) | _STRACE_NOTALL, __PRETTY_FUNCTION__, fmt, ## args); \
	0; \
    }))
#define strace_vprintf(what, fmt, arg) \
    ((void) ({\
	if ((_STRACE_ ## what & _STRACE_SYSTEM) || strace.active ()) \
	  strace.vprntf((_STRACE_ ## what), __PRETTY_FUNCTION__, fmt, arg); \
	0; \
    }))
#endif /*NOSTRACE*/

#ifdef DEBUGGING
#define debug_only_printf(fmt, args...) debug_printf (fmt , ## args)
#else
#define debug_only_printf(fmt, args...) do {} while (0)
#endif
#define debug_printf(fmt, args...) strace_printf_wrap(DEBUG, fmt , ## args)
#define malloc_printf(fmt, args...) strace_printf_wrap1(MALLOC, fmt , ## args)
#define minimal_printf(fmt, args...) strace_printf_wrap1(MINIMAL, fmt , ## args)
#define paranoid_printf(fmt, args...) strace_printf_wrap1(PARANOID, fmt , ## args)
#define pthread_printf(fmt, args...) strace_printf_wrap1(PTHREAD, fmt , ## args)
#define select_printf(fmt, args...) strace_printf_wrap(SELECT, fmt , ## args)
#define sigproc_printf(fmt, args...) strace_printf_wrap(SIGP, fmt , ## args)
#define syscall_printf(fmt, args...) strace_printf_wrap(SYSCALL, fmt , ## args)
#define system_printf(fmt, args...) strace_printf_wrap(SYSTEM, fmt , ## args)
#define termios_printf(fmt, args...) strace_printf_wrap(TERMIOS, fmt , ## args)
#define thread_printf(fmt, args...) strace_printf_wrap1(THREAD, fmt , ## args)
#define special_printf(fmt, args...) strace_printf_wrap1(SPECIAL, fmt , ## args)
#define wm_printf(fmt, args...) strace_printf_wrap(WM, fmt , ## args)
#endif /* __cplusplus */
#endif /* _SYS_STRACE_H */
