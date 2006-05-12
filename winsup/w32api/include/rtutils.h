/*
 * rtutils.h - Routing and Remote Access Services
 *
 * THIS SOFTWARE IS NOT COPYRIGHTED
 *
 * This source code is offered for use in the public domain.  You may use,
 * modify or distribute it freely.
 *
 * This code is distributed in the hope that it will be useful but
 * WITHOUT ANY WARRANTY.  ALL WARRANTIES, EXPRESS OR IMPLIED ARE HEREBY
 * DISCLAIMED.  This includes but is not limited to warranties of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *
 */
#ifndef _RTUTILS_H
#define _RTUTILS_H
#if __GNUC__ >= 3
#pragma GCC system_header
#endif

#ifdef __cplusplus
extern "C" {
#endif

/*--- Tracing Reference */
#if (_WIN32_WINNT >= 0x0500)
DWORD WINAPI TraceDeregisterA(DWORD);
DWORD WINAPI TraceDeregisterW(DWORD);
DWORD WINAPI TraceDeregisterExA(DWORD,DWORD);
DWORD WINAPI TraceDeregisterExW(DWORD,DWORD);
#define TRACE_NO_SYNCH 0x00000004
/*DWORD WINAPI TraceDumpA(DWORD,LPBYTE,DWORD,DWORD,BOOL,LPCSTR);*/
/*DWORD WINAPI TraceDumpW(DWORD,LPBYTE,DWORD,DWORD,BOOL,LPCWSTR);*/
DWORD WINAPI TraceDumpExA(DWORD,DWORD,LPBYTE,DWORD,DWORD,BOOL,LPCSTR);
DWORD WINAPI TraceDumpExW(DWORD,DWORD,LPBYTE,DWORD,DWORD,BOOL,LPCWSTR);
DWORD WINAPI TracePrintfA(DWORD,LPCSTR,...);
DWORD WINAPI TracePrintfW(DWORD,LPCWSTR,...);
DWORD WINAPI TracePrintfExA(DWORD,DWORD,LPCSTR,...);
DWORD WINAPI TracePrintfExW(DWORD,DWORD,LPCWSTR,...);
/*DWORD WINAPI TracePutsA(DWORD,LPCSTR);*/
/*DWORD WINAPI TracePutsW(DWORD,LPCWSTR);*/
DWORD WINAPI TracePutsExA(DWORD,DWORD,LPCSTR);
DWORD WINAPI TracePutsExW(DWORD,DWORD,LPCWSTR);
/*DWORD WINAPI TraceRegisterA(LPCSTR);*/
/*DWORD WINAPI TraceRegisterW(LPCWSTR);*/
DWORD WINAPI TraceRegisterExA(LPCSTR,DWORD);
DWORD WINAPI TraceRegisterExW(LPCWSTR,DWORD);
#define TRACE_USE_FILE 0x00000001
#define TRACE_USE_CONSOLE 0x00000002
#define INVALID_TRACEID 0xFFFFFFFF
/*DWORD WINAPI TraceVprintfA(DWORD,LPCSTR,va_list);*/
/*DWORD WINAPI TraceVprintfW(DWORD,LPCWSTR,va_list);*/
DWORD WINAPI TraceVprintfExA(DWORD,DWORD,LPCSTR,va_list);
DWORD WINAPI TraceVprintfExW(DWORD,DWORD,LPCWSTR,va_list);
#define TRACE_NO_STDINFO 0x00000001
#define TRACE_USE_MASK 0x00000002
#define TRACE_USE_MSEC 0x00000004
#ifdef UNICODE
#define TraceDeregister TraceDeregisterW
#define TraceDeregisterEx TraceDeregisterExW
#define TraceDump TraceDumpW
#define TraceDumpEx TraceDumpExW
#define TracePrintf TracePrintfW
#define TracePrintfEx TracePrintfExW
#define TracePuts TracePutsW
#define TracePutsEx TracePutsExW
#define TraceRegister TraceRegisterW
#define TraceRegisterEx TraceRegisterExW
#define TraceVprintf TraceVprintfW
#define TraceVprintfEx TraceVprintfExW
#else
#define TraceDeregister TraceDeregisterA
#define TraceDeregisterEx TraceDeregisterExA
#define TraceDump TraceDumpA
#define TraceDumpEx TraceDumpExA
#define TracePrintf TracePrintfA
#define TracePrintfEx TracePrintfExA
#define TracePuts TracePutsA
#define TracePutsEx TracePutsExA
#define TraceRegister TraceRegisterA
#define TraceRegisterEx TraceRegisterExA
#define TraceVprintf TraceVprintfA
#define TraceVprintfEx TraceVprintfExA
#endif
#endif /* (_WIN32_WINNT >= 0x0500) */

#ifdef __cplusplus
}
#endif
#endif
