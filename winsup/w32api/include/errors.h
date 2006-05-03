#ifndef _ERRORS_H
#define _ERRORS_H
#if __GNUC__ >=3
#pragma GCC system_header
#endif

#ifdef __cplusplus
extern "C" {
#endif

#define MAX_ERROR_TEXT_LEN 160
DWORD WINAPI AMGetErrorTextA(HRESULT,CHAR*,DWORD);
DWORD WINAPI AMGetErrorTextW(HRESULT,WCHAR*,DWORD);
#ifdef UNICODE
#define AMGetErrorText AMGetErrorTextW
#else
#define AMGetErrorText AMGetErrorTextA
#endif

#ifdef __cplusplus
}
#endif
#endif
