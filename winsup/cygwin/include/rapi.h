/*
	rapi.h - main header file for the RAPI API

	Copyright 1999 Cygnus Solutions.

	This file is part of Cygwin.

	This software is a copyrighted work licensed under the terms of the
	Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
	details.
*/

#ifndef _RAPI_H
#define _RAPI_H

typedef struct IRAPIStream
{
  struct IRAPIStreamVtbl * lpVtbl;
} IRAPIStream;

typedef struct IRAPIStreamVtbl IRAPIStreamVtbl;

typedef enum tagRAPISTREAMFLAG
{
	STREAM_TIMEOUT_READ
} RAPISTREAMFLAG;

struct IRAPIStreamVtbl
{
  HRESULT (__stdcall * SetRapiStat)( IRAPIStream * This, RAPISTREAMFLAG Flag, DWORD dwValue) ;
  HRESULT (__stdcall * GetRapiStat)( IRAPIStream * This, RAPISTREAMFLAG Flag, DWORD *pdwValue) ;
};

// RAPI extension on Windows CE (e.g., MyFunctionFOO) called via CeRapiInvoke should be declared as:
// EXTERN_C RAPIEXT MyFunctionFOO;
typedef  HRESULT (STDAPICALLTYPE RAPIEXT)(
		 DWORD			cbInput,			// [IN]
		 BYTE			*pInput,			// [IN]
		 DWORD			*pcbOutput,			// [OUT]
		 BYTE			**ppOutput,			// [OUT]
		 IRAPIStream	*pIRAPIStream		// [IN]
		 );

typedef struct _RAPIINIT
{
    DWORD cbSize;
    HANDLE heRapiInit;
    HRESULT hrRapiInit;
} RAPIINIT;

STDAPI CeRapiInit();
STDAPI CeRapiInitEx(RAPIINIT*);
STDAPI_(BOOL) CeCreateProcess(LPCWSTR, LPCWSTR, LPSECURITY_ATTRIBUTES, LPSECURITY_ATTRIBUTES,
			      BOOL, DWORD, LPVOID, LPWSTR, LPSTARTUPINFO, LPPROCESS_INFORMATION);
STDAPI CeRapiUninit();

STDAPI_(BOOL) CeWriteFile(HANDLE, LPCVOID, DWORD, LPDWORD, LPOVERLAPPED);
STDAPI_(HANDLE) CeCreateFile(LPCWSTR, DWORD, DWORD, LPSECURITY_ATTRIBUTES, DWORD, DWORD, HANDLE);
STDAPI_(BOOL) CeCreateDirectory(LPCWSTR, LPSECURITY_ATTRIBUTES);
STDAPI_(DWORD) CeGetLastError(void);
STDAPI_(BOOL) CeGetFileTime(HANDLE, LPFILETIME, LPFILETIME, LPFILETIME);
STDAPI_(BOOL) CeCloseHandle(HANDLE);

#endif /* _RAPI_H */
