/* debug.h

   Copyright 1998, 1999, 2000 Cygnus Solutions.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

#ifndef MALLOC_DEBUG
#define MALLOC_CHECK do {} while (0)
#else
#define MALLOC_CHECK ({\
  debug_printf ("checking malloc pool");\
  (void)mallinfo ();\
})
#endif

extern "C" {
#ifndef DEBUGGING0
DWORD __stdcall WFSO (HANDLE, DWORD);
DWORD __stdcall WFMO (DWORD, CONST HANDLE *, BOOL, DWORD);
#else
DWORD __stdcall WFSO (const char *fn, int ln, HANDLE, DWORD);
DWORD __stdcall WFMO (const char *fn, int ln, DWORD, CONST HANDLE *, BOOL, DWORD);
#endif
}

#ifndef DEBUGGING0
#define WaitForSingleObject WFSO
#define WaitForMultipleObject WFMO
#else
#define WaitForSingleObject(a, b) WFSO (__FUNCTION__, __LINE__, a, b)
#define WaitForMultipleObject(a, b, c, d) WFMO (__FUNCTION__, __LINE__, a, b, c, d)
#endif

#if !defined(_DEBUG_H_)
#define _DEBUG_H_

void threadname_init ();
HANDLE __stdcall makethread (LPTHREAD_START_ROUTINE, LPVOID, DWORD, const char *);
const char * __stdcall threadname (DWORD, int lockit = TRUE);
void __stdcall regthread (const char *, DWORD);
int __stdcall iscygthread ();

#ifndef DEBUGGING
# define ForceCloseHandle CloseHandle
# define ForceCloseHandle1(h, n) CloseHandle (h)
# define ForceCloseHandle2(h, n) CloseHandle (h)
# define ProtectHandle(h) do {} while (0)
# define ProtectHandle1(h,n) do {} while (0)
# define ProtectHandle2(h,n) do {} while (0)
# define debug_init() do {} while (0)

#else

# ifdef NO_DEBUG_DEFINES
#   undef NO_DEBUG_DEFINES
# else
#   define CloseHandle(h) \
	close_handle (__PRETTY_FUNCTION__, __LINE__, (h), #h, FALSE)
#   define ForceCloseHandle(h) \
	close_handle (__PRETTY_FUNCTION__, __LINE__, (h), #h, TRUE)
#   define ForceCloseHandle1(h,n) \
	close_handle (__PRETTY_FUNCTION__, __LINE__, (h), #n, TRUE)
#   define ForceCloseHandle2(h,n) \
	close_handle (__PRETTY_FUNCTION__, __LINE__, (h), n, TRUE)
#   define lock_pinfo_for_update(n) lpfu(__PRETTY_FUNCTION__, __LINE__, n)
# endif

# define ProtectHandle(h) add_handle (__PRETTY_FUNCTION__, __LINE__, (h), #h)
# define ProtectHandle1(h,n) add_handle (__PRETTY_FUNCTION__, __LINE__, (h), #n)
# define ProtectHandle2(h,n) add_handle (__PRETTY_FUNCTION__, __LINE__, (h), n)

void debug_init ();
void __stdcall add_handle (const char *, int, HANDLE, const char *);
BOOL __stdcall close_handle (const char *, int, HANDLE, const char *, BOOL);
int __stdcall lpfu (const char *, int, DWORD timeout);

#endif /*DEBUGGING*/
#endif /*_DEBUG_H_*/
