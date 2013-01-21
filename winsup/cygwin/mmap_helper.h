/* mmap_helper.h

   Copyright 2006, 2011 Red Hat, Inc.

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

#ifndef _MMAP_HELPER_H
#define _MMAP_HELPER_H
#define _MMIOWRAP(__ptr, __len, __func) \
({ \
  BOOL __res; \
  for (int __i = 0; __i < 2; __i++) \
    { \
      __res = __func; \
      if (__res || __i > 0) \
	break; \
      DWORD __errcode = GetLastError (); \
      if (__errcode != ERROR_NOACCESS) \
	break; \
      switch (mmap_is_attached_or_noreserve (__ptr, __len)) \
	{ \
	case MMAP_NORESERVE_COMMITED: \
	  continue; \
	case MMAP_RAISE_SIGBUS: \
	  raise(SIGBUS); \
	default: \
	  break; \
	} \
      break; \
    } \
  __res; \
})

#define _MMSOCKWRAP(__ptr, __count, __func) \
({ \
  int __res; \
  for (int __i = 0; __i < 2; __i++) \
    { \
      __res = __func; \
      if (!__res || __i > 0) \
	break; \
      DWORD __errcode = WSAGetLastError (); \
      if (__errcode != WSAEFAULT) \
	break; \
      for (unsigned __j = 0; __j < __count; __j++) \
	switch (mmap_is_attached_or_noreserve (__ptr[__j].buf, __ptr[__j].len)) \
	  { \
	  case MMAP_NORESERVE_COMMITED: \
	    goto keeptrying; \
	  case MMAP_RAISE_SIGBUS: \
	    raise(SIGBUS); \
	  default: \
	    break; \
	  } \
      break; \
    keeptrying: \
      continue; \
    } \
  __res; \
})

extern inline BOOL
mmReadFile (HANDLE hFile, LPVOID lpBuffer, DWORD nNumberOfBytesToRead,
	    LPDWORD lpNumberOfBytesRead, LPOVERLAPPED lpOverlapped)
{
  return _MMIOWRAP (lpBuffer, nNumberOfBytesToRead,
		    (ReadFile (hFile, lpBuffer, nNumberOfBytesToRead,
			       lpNumberOfBytesRead, lpOverlapped)));
}

#ifdef _WINSOCK_H
extern inline int
mmWSARecvFrom (SOCKET s, LPWSABUF lpBuffers, DWORD dwBufferCount,
	    LPDWORD lpNumberOfBytesRecvd, LPDWORD lpFlags,
	    struct sockaddr* lpFrom,
	    LPINT lpFromlen, LPWSAOVERLAPPED lpOverlapped,
	    LPWSAOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine)
{
  return _MMSOCKWRAP (lpBuffers, dwBufferCount,
		      (mmWSARecvFrom(s, lpBuffers, dwBufferCount,
				     lpNumberOfBytesRecvd, lpFlags, lpFrom,
				     lpFromlen, lpOverlapped,
				     lpCompletionRoutine)));
}
#endif /*_WINSOCK_H*/

#endif /*_MMAP_HELPER_H*/
