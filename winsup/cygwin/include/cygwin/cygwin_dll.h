/* cygwin_dll.h

   Copyright 1998 Cygnus Solutions

This file is part of Cygwin32.

This software is a copyrighted work licensed under the terms of the
Cygwin32 license.  Please consult the file "CYGWIN_LICENSE" for
details. */

#ifndef __CYGWIN_CYGWIN_DLL_H__
#define __CYGWIN_CYGWIN_DLL_H__

#include <windows.h>

#ifdef __cplusplus
#define CDECL_BEGIN extern "C" {
#define CDECL_END }			      
#else
#define CDECL_BEGIN
#define CDECL_END
#endif

#define DECLARE_CYGWIN_DLL(Entry)					      \
									      \
CDECL_BEGIN								      \
  int WINAPI _cygwin_dll_entry (HANDLE h, DWORD reason, void *ptr);	      \
  int WINAPI _cygwin_noncygwin_dll_entry (HANDLE h, DWORD reason, void *ptr); \
									      \
  int WINAPI Entry (HANDLE h, DWORD reason, void *ptr);		              \
  extern int cygwin_attach_dll ();					      \
  extern void cygwin_detach_dll ();	                      		      \
CDECL_END								      \
									      \
static HANDLE storedHandle;						      \
static DWORD storedReason;						      \
static void* storedPtr;							      \
									      \
static int __dllMain (int a, char **b, char **c)			      \
{									      \
  return Entry (storedHandle, storedReason, storedPtr);		              \
}									      \
									      \
static int dll_index;							      \
									      \
int WINAPI _cygwin_dll_entry (HANDLE h, DWORD reason, void *ptr)	      \
{									      \
  int ret;								      \
  ret = 1;								      \
									      \
  switch (reason)							      \
  {									      \
    case DLL_PROCESS_ATTACH:						      \
    {									      \
      storedHandle = h;							      \
      storedReason = reason;						      \
      storedPtr = ptr;							      \
      dll_index = cygwin_attach_dll (h, &__dllMain);	      		      \
      if (dll_index == -1)						      \
	ret = 0;							      \
    }									      \
    break;								      \
									      \
    case DLL_PROCESS_DETACH:						      \
    {									      \
      ret = Entry (h, reason, ptr);					      \
      if (ret)								      \
      {									      \
	cygwin_detach_dll (dll_index);				      	      \
	dll_index = -1;							      \
      }									      \
    }									      \
    break;								      \
    									      \
    case DLL_THREAD_ATTACH:						      \
    {									      \
      ret = Entry (h, reason, ptr);					      \
    }									      \
    break;								      \
									      \
    case DLL_THREAD_DETACH:						      \
    {									      \
      ret = Entry (h, reason, ptr);					      \
    }									      \
    break;								      \
  }									      \
  return ret;								      \
}									      \
									      \
/* OBSOLETE: This is only provided for source level compatibility. */         \
int WINAPI _cygwin_noncygwin_dll_entry (HANDLE h, DWORD reason, void *ptr)    \
{									      \
  return _cygwin_dll_entry (h, reason, ptr);				      \
}									      \

#endif /* __CYGWIN_CYGWIN_DLL_H__ */
