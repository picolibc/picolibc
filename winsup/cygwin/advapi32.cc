/* advapi32.cc: Win32 replacement functions.

   Copyright 2011 Red Hat, Inc.

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

#include "winsup.h"
#include <winioctl.h>
#include "shared_info.h"
#include "ntdll.h"
#include <wchar.h>

#define DEFAULT_NTSTATUS_TO_BOOL_RETURN \
  if (!NT_SUCCESS (status)) \
    SetLastError (RtlNtStatusToDosError (status)); \
  return NT_SUCCESS (status);

BOOL WINAPI
AccessCheck (PSECURITY_DESCRIPTOR sd, HANDLE tok, DWORD access,
	     PGENERIC_MAPPING mapping, PPRIVILEGE_SET pset, LPDWORD psetlen,
	     LPDWORD granted, LPBOOL allowed)
{
  NTSTATUS status, astatus;

  status = NtAccessCheck (sd, tok, access, mapping, pset, psetlen, granted,
			  &astatus);
  if (NT_SUCCESS (status))
    *allowed = NT_SUCCESS (astatus);
  DEFAULT_NTSTATUS_TO_BOOL_RETURN
}

BOOL WINAPI
PrivilegeCheck (HANDLE tok, PPRIVILEGE_SET pset, LPBOOL res)
{
  NTSTATUS status = NtPrivilegeCheck (tok, pset, (PBOOLEAN) res);
  DEFAULT_NTSTATUS_TO_BOOL_RETURN
}

BOOL WINAPI
EqualSid (PSID sid1, PSID sid2)
{
  return !!RtlEqualSid (sid1, sid2);
}

BOOL WINAPI
CopySid (DWORD len, PSID dest, PSID src)
{
  NTSTATUS status = RtlCopySid (len, dest, src);
  DEFAULT_NTSTATUS_TO_BOOL_RETURN
}

BOOL WINAPI
AddAccessAllowedAce (PACL acl, DWORD revision, DWORD mask, PSID sid)
{
  NTSTATUS status = RtlAddAccessAllowedAce (acl, revision, mask, sid);
  DEFAULT_NTSTATUS_TO_BOOL_RETURN
}

BOOL WINAPI
AddAccessDeniedAce (PACL acl, DWORD revision, DWORD mask, PSID sid)
{
  NTSTATUS status = RtlAddAccessDeniedAce (acl, revision, mask, sid);
  DEFAULT_NTSTATUS_TO_BOOL_RETURN
}

BOOL WINAPI
MakeSelfRelativeSD (PSECURITY_DESCRIPTOR abs_sd, PSECURITY_DESCRIPTOR rel_sd,
		    LPDWORD len)
{
  NTSTATUS status = RtlAbsoluteToSelfRelativeSD (abs_sd, rel_sd, len);
  DEFAULT_NTSTATUS_TO_BOOL_RETURN
}

BOOL WINAPI
SetSecurityDescriptorDacl (PSECURITY_DESCRIPTOR sd, BOOL present, PACL dacl,
			   BOOL def)
{
  NTSTATUS status = RtlSetDaclSecurityDescriptor (sd, (BOOLEAN) !!present, dacl,
						  (BOOLEAN) !!def);
  DEFAULT_NTSTATUS_TO_BOOL_RETURN
}

BOOL WINAPI
SetSecurityDescriptorGroup (PSECURITY_DESCRIPTOR sd, PSID sid, BOOL def)
{
  NTSTATUS status = RtlSetGroupSecurityDescriptor (sd, sid, (BOOLEAN) !!def);
  DEFAULT_NTSTATUS_TO_BOOL_RETURN
}

BOOL WINAPI
SetSecurityDescriptorOwner (PSECURITY_DESCRIPTOR sd, PSID sid, BOOL def)
{
  NTSTATUS status = RtlSetOwnerSecurityDescriptor (sd, sid, (BOOLEAN) !!def);
  DEFAULT_NTSTATUS_TO_BOOL_RETURN
}

BOOL WINAPI
OpenThreadToken (HANDLE thread, DWORD access, BOOL as_self, PHANDLE tok)
{
  NTSTATUS status = NtOpenThreadToken (thread, access, as_self, tok);
  DEFAULT_NTSTATUS_TO_BOOL_RETURN
}

BOOL WINAPI
GetTokenInformation(HANDLE tok, TOKEN_INFORMATION_CLASS infoclass, LPVOID buf,
		    DWORD len, PDWORD retlen)
{
  NTSTATUS status = NtQueryInformationToken (tok, infoclass, buf, len, retlen);
  DEFAULT_NTSTATUS_TO_BOOL_RETURN
}

BOOL WINAPI
SetTokenInformation (HANDLE tok, TOKEN_INFORMATION_CLASS infoclass, PVOID buf,
		     ULONG len)
{
  NTSTATUS status = NtSetInformationToken (tok, infoclass, buf, len);
  DEFAULT_NTSTATUS_TO_BOOL_RETURN
}

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
      sec_attr && sec_attr->bInheritHandle? OBJ_INHERIT : 0,
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

BOOL
ImpersonateNamedPipeClient (HANDLE pipe)
{
  IO_STATUS_BLOCK io;
  NTSTATUS status = NtFsControlFile (pipe, NULL, NULL, NULL, &io,
				     FSCTL_PIPE_IMPERSONATE, NULL, 0, NULL, 0);
  DEFAULT_NTSTATUS_TO_BOOL_RETURN
}
