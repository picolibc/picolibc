/* advapi32.cc: Win32 replacement functions.

   Copyright 2011, 2012, 2013 Red Hat, Inc.

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

#include "winsup.h"
#include <winioctl.h>
#include "shared_info.h"
#include "ntdll.h"

#define DEFAULT_NTSTATUS_TO_BOOL_RETURN \
  if (!NT_SUCCESS (status)) \
    SetLastError (RtlNtStatusToDosError (status)); \
  return NT_SUCCESS (status);

/* This file should only contain non-trivial implementations of advapi32
   functions, or advapi32 functions for which the ntdll.dll equivalent
   is not easy to understand.  In all other case, use the ntdll.dll
   equivalent. */

BOOL WINAPI
RevertToSelf ()
{
  HANDLE tok = NULL;
  NTSTATUS status = NtSetInformationThread (NtCurrentThread (),
					    ThreadImpersonationToken,
					    &tok, sizeof tok);
  DEFAULT_NTSTATUS_TO_BOOL_RETURN
}

BOOL WINAPI
DuplicateTokenEx (HANDLE tok, DWORD access, LPSECURITY_ATTRIBUTES sec_attr,
		  SECURITY_IMPERSONATION_LEVEL level, TOKEN_TYPE type,
		  PHANDLE new_tok)
{
  SECURITY_QUALITY_OF_SERVICE sqos =
    { sizeof sqos, level, SECURITY_STATIC_TRACKING, FALSE };
  OBJECT_ATTRIBUTES attr =
    { sizeof attr, NULL, NULL,
      (sec_attr && sec_attr->bInheritHandle) ? OBJ_INHERIT : 0U,
      sec_attr ? sec_attr->lpSecurityDescriptor : NULL, &sqos };
  NTSTATUS status = NtDuplicateToken (tok, access, &attr, FALSE, type, new_tok);
  DEFAULT_NTSTATUS_TO_BOOL_RETURN
}

BOOL WINAPI
ImpersonateLoggedOnUser (HANDLE tok)
{
  NTSTATUS status;
  HANDLE ptok = NULL;
  TOKEN_TYPE type;
  ULONG size;

  status = NtQueryInformationToken (tok, TokenType, &type, sizeof type, &size);
  if (!NT_SUCCESS (status))
    {
      SetLastError (RtlNtStatusToDosError (status));
      return FALSE;
    }
  if (type == TokenPrimary)
    {
      /* If its a primary token it must be converted to an impersonated
	 token. */
      SECURITY_QUALITY_OF_SERVICE sqos =
	{ sizeof sqos, SecurityImpersonation, SECURITY_DYNAMIC_TRACKING, FALSE};
      OBJECT_ATTRIBUTES attr =
	{ sizeof attr, NULL, NULL, 0, NULL, &sqos };

      /* The required rights for the impersonation token according to MSDN. */
      status = NtDuplicateToken (tok, TOKEN_QUERY | TOKEN_IMPERSONATE,
				 &attr, FALSE, TokenImpersonation, &ptok);
      if (!NT_SUCCESS (status))
	{
	  SetLastError (RtlNtStatusToDosError (status));
	  return FALSE;
	}
      tok = ptok;
    }
  status = NtSetInformationThread (NtCurrentThread (), ThreadImpersonationToken,
				   &tok, sizeof tok);
  if (ptok)
    NtClose (ptok);
  DEFAULT_NTSTATUS_TO_BOOL_RETURN
}

BOOL WINAPI
ImpersonateNamedPipeClient (HANDLE pipe)
{
  IO_STATUS_BLOCK io;
  NTSTATUS status = NtFsControlFile (pipe, NULL, NULL, NULL, &io,
				     FSCTL_PIPE_IMPERSONATE, NULL, 0, NULL, 0);
  DEFAULT_NTSTATUS_TO_BOOL_RETURN
}
