/* sys/strace.h */

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

#ifdef __cplusplus
extern "C" {
#endif

#define _STRACE_INTERFACE_ACTIVATE_ADDR -1

/* Bitmasks of tracing messages to print.  */

#define _STRACE_ALL	 0x00001 // so behaviour of strace=1 is unchanged
#define _STRACE_FLUSH	 0x00002 // flush output buffer after every message
#define _STRACE_INHERIT  0x00004 // children inherit mask from parent
#define _STRACE_UHOH	 0x00008 // unusual or weird phenomenon
#define _STRACE_SYSCALL	 0x00010 // system calls
#define _STRACE_STARTUP	 0x00020 // argc/envp printout at startup
#define _STRACE_DEBUG    0x00040 // info to help debugging
#define _STRACE_PARANOID 0x00080 // paranoid info
#define _STRACE_TERMIOS	 0x00100 // info for debugging termios stuff
#define _STRACE_SELECT	 0x00200 // info on ugly select internals
#define _STRACE_WM	 0x00400 // trace windows messages (enable _strace_wm)
#define _STRACE_SIGP	 0x00800 // trace signal and process handling
#define _STRACE_MINIMAL	 0x01000 // very minimal strace output
#define _STRACE_EXITDUMP 0x04000 // dump strace cache on exit
#define _STRACE_CACHE	 0x08000 // cache strace messages
#define _STRACE_NOMUTEX	 0x10000 // don't use mutex for synchronization
#define _STRACE_MALLOC	 0x20000 // trace malloc calls
#define _STRACE_THREAD	 0x40000 // thread-locking calls
#define _STRACE_NOTALL	 0x80000 // don't include if _STRACE_ALL

void small_printf (const char *, ...);

#ifdef NOSTRACE
#define strace_printf(category, fmt...) 0
#define strace_printf_wrap(category, fmt...) 0
#define strace_printf_wrap1(category, fmt...) 0
#define strace_wm(category, msg...) 0
#else
/* Output message to strace log */
void strace_printf (unsigned, const char *, ...);
void __system_printf (const char *, ...);

#define system_printf(fmt, args...) \
  __system_printf("%F: " fmt, __PRETTY_FUNCTION__ , ## args)

void _strace_wm (int __message, int __word, int __lon);

#define strace_printf_wrap(what, fmt, args...) \
   ((void) ({\
	if (strace_active) \
	  strace_printf(_STRACE_ ## what, "%F: " fmt, __PRETTY_FUNCTION__ , ## args); \
	0; \
    }))
#define strace_printf_wrap1(what, fmt, args...) \
    ((void) ({\
	if (strace_active) \
	  strace_printf((_STRACE_ ## what) | _STRACE_NOTALL, "%F: " fmt, __PRETTY_FUNCTION__ , ## args); \
	0; \
    }))
#endif /*NOSTRACE*/

#define debug_printf(fmt, args...) strace_printf_wrap(DEBUG, fmt , ## args)
#define syscall_printf(fmt, args...) strace_printf_wrap(SYSCALL, fmt , ## args)
#define paranoid_printf(fmt, args...) strace_printf_wrap(PARANOID, fmt , ## args)
#define termios_printf(fmt, args...) strace_printf_wrap(TERMIOS, fmt , ## args)
#define select_printf(fmt, args...) strace_printf_wrap(SELECT, fmt , ## args)
#define wm_printf(fmt, args...) strace_printf_wrap(WM, fmt , ## args)
#define sigproc_printf(fmt, args...) strace_printf_wrap(SIGP, fmt , ## args)
#define minimal_printf(fmt, args...) strace_printf_wrap1(MINIMAL, fmt , ## args)
#define malloc_printf(fmt, args...) strace_printf_wrap1(MALLOC, fmt , ## args)
#define thread_printf(fmt, args...) strace_printf_wrap1(THREAD, fmt , ## args)

#ifdef __cplusplus
}
#endif

#endif /* _SYS_STRACE_H */
