/* woutsup.h: for Cygwin code compiled outside the DLL (i.e. cygserver).

   Copyright 2002 Red Hat, Inc.

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#ifdef __INSIDE_CYGWIN__
#error "woutsup.h is not for code being compiled inside the dll"
#endif

#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0500
#endif

#if _WIN32_WINNT < 0x0500
#undef _WIN32_WINNT
#define _WIN32_WINNT 0x0500
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
#undef _WINGDI_H
#undef _WINUSER_H
#undef _WINNLS_H
#undef _WINVER_H
#undef _WINNETWK_H
#undef _WINSVC_H

#include "wincap.h"

/* The one function we use from winuser.h most of the time */
extern "C" DWORD WINAPI GetLastError (void);

extern int cygserver_running;

#if !defined(__STDC_VERSION__) || __STDC_VERSION__ >= 199900L
#define NEW_MACRO_VARARGS
#endif

/*
 * A reproduction of the <sys/strace.h> macros.  This allows code that
 * runs both inside and outside the Cygwin DLL to use the same macros
 * for logging messages.
 */

extern "C" void __cygserver__printf (const char *, const char *, ...);

#ifdef NEW_MACRO_VARARGS

#define system_printf(...)					\
  do								\
    {								\
      __cygserver__printf (__PRETTY_FUNCTION__, __VA_ARGS__);	\
    } while (false)

#define __noop_printf(...) do {;} while (false)

#else /* !NEW_MACRO_VARARGS */

#define system_printf(args...)					\
  do								\
    {								\
      __cygserver__printf (__PRETTY_FUNCTION__, ## args);	\
    } while (false)

#define __noop_printf(args...) do {;} while (false)

#endif /* !NEW_MACRO_VARARGS */

#ifdef DEBUGGING
#define debug_printf system_printf
#define paranoid_printf system_printf
#define select_printf system_printf
#define sigproc_printf system_printf
#define syscall_printf system_printf
#define termios_printf system_printf
#define wm_printf system_printf
#define minimal_printf system_printf
#define malloc_printf system_printf
#define thread_printf system_printf
#else
#define debug_printf __noop_printf
#define paranoid_printf __noop_printf
#define select_printf __noop_printf
#define sigproc_printf __noop_printf
#define syscall_printf __noop_printf
#define termios_printf __noop_printf
#define wm_printf __noop_printf
#define minimal_printf __noop_printf
#define malloc_printf __noop_printf
#define thread_printf __noop_printf
#endif

#include "safe_memory.h"
