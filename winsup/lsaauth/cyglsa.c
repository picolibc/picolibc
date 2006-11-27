/* cyglsa.c: LSA authentication module for Cygwin

   Copyright 2006 Red Hat, Inc.

   Written by Corinna Vinschen <corinna@vinschen.de>

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for details. */

#define WINVER 0x0600
#define _CRT_SECURE_NO_DEPRECATE
#include <ntstatus.h>
#define WIN32_NO_STATUS
#include <windows.h>
#include <wininet.h>
#include <lm.h>
#include <ntsecapi.h>
#include "../cygwin/cyglsa.h"
#include "../cygwin/include/cygwin/version.h"

static PLSA_SECPKG_FUNCS funcs;
static BOOL must_create_logon_sid;

BOOL APIENTRY
DllMain (HINSTANCE inst, DWORD reason, LPVOID res)
{
  switch (reason)
    {
      case DLL_PROCESS_ATTACH:
      case DLL_THREAD_ATTACH:
      case DLL_THREAD_DETACH:
      case DLL_PROCESS_DETACH:
        break;
    }
  return TRUE;
}

static PUNICODE_STRING
uni_alloc (PWCHAR src, DWORD len)
{
  PUNICODE_STRING tgt;

  if (!(tgt = funcs->AllocateLsaHeap (sizeof (UNICODE_STRING))))
    return NULL;
  tgt->Length = len * sizeof (WCHAR);
  tgt->MaximumLength = tgt->Length + sizeof (WCHAR);
  if (!(tgt->Buffer = funcs->AllocateLsaHeap (tgt->MaximumLength)))
    {
      funcs->FreeLsaHeap (tgt);
      return NULL;
    }
  wcscpy (tgt->Buffer, src);
  return tgt;
}

#ifdef DEBUGGING
/* No, I don't want to include stdio.h... */
extern int sprintf (const char *, const char *, ...);

static void
print (HANDLE fh, const char *text, BOOL nl)
{
  DWORD wr;

  WriteFile (fh, text, strlen (text), &wr, NULL);
  if (nl)
    WriteFile (fh, "\n", 1, &wr, NULL);
}

static void
print_sid (HANDLE fh, const char *prefix, int idx, PISID sid)
{
  char buf[256];
  DWORD i;

  print (fh, prefix, FALSE);
  if (idx >= 0)
    {
      sprintf (buf, "[%d] ", idx);
      print (fh, buf, FALSE);
    }
  sprintf (buf, "(0x%08x) ", (INT_PTR) sid);
  print (fh, buf, FALSE);
  if (!sid)
    print (fh, "NULL", TRUE);
  else if (IsBadReadPtr (sid, 8))
    print (fh, "INVALID POINTER", TRUE);
  else if (!IsValidSid ((PSID) sid))
    print (fh, "INVALID SID", TRUE);
  else if (IsBadReadPtr (sid, 8 + sizeof (DWORD) * sid->SubAuthorityCount))
    print (fh, "INVALID POINTER SPACE", TRUE);
  else
    {
      sprintf (buf, "S-%d-%d", sid->Revision, sid->IdentifierAuthority.Value[5]);
      for (i = 0; i < sid->SubAuthorityCount; ++i)
        sprintf (buf + strlen (buf), "-%lu", sid->SubAuthority[i]);
      print (fh, buf, TRUE);
    }
}

static void
print_groups (HANDLE fh, PTOKEN_GROUPS grps)
{
  char buf[256];
  DWORD i;

  sprintf (buf, "Groups: (0x%08x) ", (INT_PTR) grps);
  print (fh, buf, FALSE);
  if (!grps)
    print (fh, "NULL", TRUE);
  else if (IsBadReadPtr (grps, sizeof (DWORD)))
    print (fh, "INVALID POINTER", TRUE);
  else if (IsBadReadPtr (grps, sizeof (DWORD) + sizeof (SID_AND_ATTRIBUTES)
  						* grps->GroupCount))
    print (fh, "INVALID POINTER SPACE", TRUE);
  else
    {
      sprintf (buf, "Count: %lu", grps->GroupCount);
      print (fh, buf, TRUE);
      for (i = 0; i < grps->GroupCount; ++i)
        {
	  sprintf (buf, "(attr: 0x%lx)", grps->Groups[i].Attributes);
	  print_sid (fh, " ", i, (PISID) grps->Groups[i].Sid);
	}
    }
}

static void
print_privs (HANDLE fh, PTOKEN_PRIVILEGES privs)
{
  char buf[256];
  DWORD i;

  sprintf (buf, "Privileges: (0x%08x) ", (INT_PTR) privs);
  print (fh, buf, FALSE);
  if (!privs)
    print (fh, "NULL", TRUE);
  else if (IsBadReadPtr (privs, sizeof (DWORD)))
    print (fh, "INVALID POINTER", TRUE);
  else if (IsBadReadPtr (privs, sizeof (DWORD) + sizeof (LUID_AND_ATTRIBUTES)
						 * privs->PrivilegeCount))
    print (fh, "INVALID POINTER SPACE", TRUE);
  else
    {
      sprintf (buf, "Count: %lu", privs->PrivilegeCount);
      print (fh, buf, TRUE);
      for (i = 0; i < privs->PrivilegeCount; ++i)
        {
	  sprintf (buf, "Luid: {%ld, %lu} Attributes: 0x%lx",
		   privs->Privileges[i].Luid.HighPart,
		   privs->Privileges[i].Luid.LowPart,
		   privs->Privileges[i].Attributes);
	  print (fh, buf, TRUE);
	}
    }
}

static void
print_dacl (HANDLE fh, PACL dacl)
{
  char buf[256];
  DWORD i;

  sprintf (buf, "DefaultDacl: (0x%08x) ", (INT_PTR) dacl);
  print (fh, buf, FALSE);
  if (!dacl)
    print (fh, "NULL", TRUE);
  else if (IsBadReadPtr (dacl, sizeof (ACL)))
    print (fh, "INVALID POINTER", TRUE);
  else if (IsBadReadPtr (dacl, dacl->AclSize))
    print (fh, "INVALID POINTER SPACE", TRUE);
  else
    {
      sprintf (buf, "Rev: %d, Count: %d", dacl->AclRevision, dacl->AceCount);
      print (fh, buf, TRUE);
      for (i = 0; i < dacl->AceCount; ++i)
        {
	  PACCESS_ALLOWED_ACE ace;

	  if (!GetAce (dacl, i, (PVOID *) &ace))
	    {
	      sprintf (buf, "[%lu] GetAce error %lu", i, GetLastError ());
	      print (fh, buf, TRUE);
	    }
	  else
	    {
	      sprintf (buf, "Type: %x, Flags: %x, Access: %lx, ",
		       ace->Header.AceType, ace->Header.AceFlags, (DWORD) ace->Mask);
	      print_sid (fh, buf, i, (PISID) &ace->SidStart);
	    }
	}
    }
}

static void
print_tokinf (PLSA_TOKEN_INFORMATION_V2 ptok, size_t size,
	      PVOID got_start, PVOID gotinf_start, PVOID gotinf_end)
{
  HANDLE fh;
  char buf[256];

  fh = CreateFile ("C:\\cyglsa.dbgout", GENERIC_WRITE,
		   FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
		   NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
  if (fh == INVALID_HANDLE_VALUE)
    return;

  sprintf (buf, "INCOMING: start: 0x%08x infstart: 0x%08x infend: 0x%08x",
	   (INT_PTR) got_start, (INT_PTR) gotinf_start,
	   (INT_PTR) gotinf_end);
  print (fh, buf, TRUE);

  sprintf (buf, "LSA_TOKEN_INFORMATION_V2: 0x%08x - 0x%08x",
	   (INT_PTR) ptok, (INT_PTR) ptok + size);
  print (fh, buf, TRUE);

  /* User SID */
  sprintf (buf, "User: (attr: 0x%lx)", ptok->User.User.Attributes);
  print_sid (fh, "User: ", -1, (PISID) ptok->User.User.Sid);

  /* Groups */
  print_groups (fh, ptok->Groups);

  /* Primary Group SID */
  print_sid (fh, "Primary Group: ", -1, (PISID)ptok->PrimaryGroup.PrimaryGroup);

  /* Privileges */
  print_privs (fh, ptok->Privileges);

  /* Owner */
  print_sid (fh, "Owner: ", -1, (PISID) ptok->Owner.Owner);

  /* Default DACL */
  print_dacl (fh, ptok->DefaultDacl.DefaultDacl);

  CloseHandle (fh);
}
#endif /* DEBUGGING */

NTSTATUS NTAPI
LsaApInitializePackage (ULONG authp_id, PLSA_SECPKG_FUNCS dpt,
			PLSA_STRING dummy1, PLSA_STRING dummy2,
			PLSA_STRING *authp_name)
{
  PLSA_STRING name = NULL;
  DWORD vers, major, minor;

  /* Set global pointer to lsa helper function table. */
  funcs = dpt;

  /* Allocate and set the name of the authentication package.  This is the
     name which has to be used in LsaLookupAuthenticationPackage. */
  if (!(name = funcs->AllocateLsaHeap (sizeof *name)))
    return STATUS_NO_MEMORY;
  if (!(name->Buffer = funcs->AllocateLsaHeap (sizeof (CYG_LSA_PKGNAME))))
    {
      funcs->FreeLsaHeap (name);
      return STATUS_NO_MEMORY;
    }
  name->Length = sizeof (CYG_LSA_PKGNAME) - 1;
  name->MaximumLength = sizeof (CYG_LSA_PKGNAME);
  strcpy (name->Buffer, CYG_LSA_PKGNAME);
  (*authp_name) = name;

  vers = GetVersion ();
  major = LOBYTE (LOWORD (vers));
  minor = HIBYTE (LOWORD (vers));
  /* Check if we're running on Windows 2000 or lower.  If so, we must create
     the logon sid in the group list by ourselves. */
  if (major < 5 || (major == 5 && minor == 0))
    must_create_logon_sid = TRUE;

  return STATUS_SUCCESS;
}

NTSTATUS NTAPI
LsaApLogonUser (PLSA_CLIENT_REQUEST request, SECURITY_LOGON_TYPE logon_type,
		PVOID auth, PVOID client_auth_base, ULONG auth_len,
		PVOID *pbuf, PULONG pbuf_len, PLUID logon_id,
		PNTSTATUS sub_stat, PLSA_TOKEN_INFORMATION_TYPE tok_type,
		PVOID *tok, PLSA_UNICODE_STRING *account,
		PLSA_UNICODE_STRING *authority)
{
  WCHAR user[UNLEN + 1];
  WCHAR domain[INTERNET_MAX_HOST_NAME_LENGTH + 1];
  DWORD checksum, ulen, dlen, i;
  PDWORD csp, csp_end;
  NTSTATUS stat;
  SECPKG_CLIENT_INFO clinf;
  PLSA_TOKEN_INFORMATION_V2 tokinf;

  cyglsa_t *authinf = (cyglsa_t *) auth;

  /* Check if the caller has the SeTcbPrivilege, otherwise refuse service. */
  stat = funcs->GetClientInfo (&clinf);
  if (stat != STATUS_SUCCESS)
    return stat;
  if (!clinf.HasTcbPrivilege)
    return STATUS_ACCESS_DENIED;

  /* Make a couple of validity checks. */
  if (auth_len < sizeof *authinf
      || authinf->magic != CYG_LSA_MAGIC
      || !authinf->username[0]
      || !authinf->domain[0])
    return STATUS_INVALID_PARAMETER;
  checksum = CYGWIN_VERSION_MAGIC (CYGWIN_VERSION_DLL_MAJOR,
				   CYGWIN_VERSION_DLL_MINOR);
  csp = (PDWORD) &authinf->username;
  csp_end = (PDWORD) ((PBYTE) authinf + auth_len);
  while (csp < csp_end)
    checksum += *csp++;
  if (authinf->checksum != checksum)
    return STATUS_INVALID_PARAMETER_3;

  /* Set account to username and authority to domain resp. machine name.
     The name of the logon account name as returned by LookupAccountSid
     is created from here as "authority\account". */
  authinf->username[UNLEN] = '\0';
  ulen = mbstowcs (user, authinf->username, UNLEN + 1);
  authinf->domain[INTERNET_MAX_HOST_NAME_LENGTH] = '\0';
  dlen = mbstowcs (domain, authinf->domain, INTERNET_MAX_HOST_NAME_LENGTH + 1);
  if (account && !(*account = uni_alloc (user, ulen)))
    return STATUS_NO_MEMORY;
  if (authority && !(*authority = uni_alloc (domain, dlen)))
    return STATUS_NO_MEMORY;
  /* Create a fake buffer in pbuf which is free'd again in the client.
     Windows 2000 tends to crash when setting this pointer to NULL. */
  if (pbuf)
    {
      stat = funcs->AllocateClientBuffer (request, 64UL, pbuf);
      if (!LSA_SUCCESS (stat))
        return stat;
    }
  if (pbuf_len)
    *pbuf_len = 64UL;

  /* A PLSA_TOKEN_INFORMATION_V2 is allocated in one piece, so... */
#if defined (__x86_64__) || defined (_M_AMD64)
    {
      /* ...on 64 bit systems we have to convert the incoming 32 bit offsets
	 into 64 bit pointers.  That requires to re-evaluate the size of the
	 outgoing tokinf structure and a somewhat awkward procedure to copy
	 the information over. */
      LONG_PTR base;
      PBYTE tptr;
      DWORD size, newsize;
      PSID src_sid;
      PCYG_TOKEN_GROUPS src_grps;
      PTOKEN_GROUPS grps;
      PTOKEN_PRIVILEGES src_privs;
      PACL src_acl;

      base = (LONG_PTR) &authinf->inf;

      newsize = authinf->inf_size;
      newsize += sizeof (PSID) - sizeof (OFFSET); /* User SID */
      newsize += sizeof (PTOKEN_GROUPS) - sizeof (OFFSET); /* Groups */
      src_grps = (PCYG_TOKEN_GROUPS) (base + authinf->inf.Groups);
      newsize += src_grps->GroupCount		  /* Group SIDs */
      		 * (sizeof (PSID) - sizeof (OFFSET));
      newsize += sizeof (PSID) - sizeof (OFFSET); /* Primary Group SID */
      newsize += sizeof (PSID) - sizeof (OFFSET); /* Owner SID */
      newsize += sizeof (PACL) - sizeof (OFFSET); /* Default DACL */
      if (!(tokinf = funcs->AllocateLsaHeap (newsize)))
	return STATUS_NO_MEMORY;
      tptr = (PBYTE)(tokinf + 1);

      tokinf->ExpirationTime = authinf->inf.ExpirationTime;
      /* User SID */
      src_sid = (PSID) (base + authinf->inf.User.User.Sid);
      size = GetLengthSid (src_sid);
      CopySid (size, (PSID) tptr, src_sid);
      tokinf->User.User.Sid = (PSID) tptr;
      tptr += size;
      tokinf->User.User.Attributes = authinf->inf.User.User.Attributes;
      /* Groups */
      grps = (PTOKEN_GROUPS) tptr;
      tokinf->Groups = grps;
      grps->GroupCount = src_grps->GroupCount;
      tptr += sizeof grps->GroupCount
	      + grps->GroupCount * sizeof (SID_AND_ATTRIBUTES);
      /* Group SIDs */
      for (i = 0; i < src_grps->GroupCount; ++i)
	{
	  src_sid = (PSID) (base + src_grps->Groups[i].Sid);
	  size = GetLengthSid (src_sid);
	  CopySid (size, (PSID) tptr, src_sid);
	  tokinf->Groups->Groups[i].Sid = (PSID) tptr;
	  tptr += size;
	  tokinf->Groups->Groups[i].Attributes = src_grps->Groups[i].Attributes;
	}
      /* Primary Group SID */
      src_sid = (PSID) (base + authinf->inf.PrimaryGroup.PrimaryGroup);
      size = GetLengthSid (src_sid);
      CopySid (size, (PSID) tptr, src_sid);
      tokinf->PrimaryGroup.PrimaryGroup = (PSID) tptr;
      tptr += size;
      /* Privileges */
      src_privs = (PTOKEN_PRIVILEGES) (base + authinf->inf.Privileges);
      size = sizeof src_privs->PrivilegeCount
	     + src_privs->PrivilegeCount * sizeof (LUID_AND_ATTRIBUTES);
      memcpy (tptr, src_privs, size);
      tokinf->Privileges = (PTOKEN_PRIVILEGES) tptr;
      tptr += size;
      /* Owner */
      tokinf->Owner.Owner = NULL;
      /* Default DACL */
      src_acl = (PACL) (base + authinf->inf.DefaultDacl.DefaultDacl);
      size = src_acl->AclSize;
      memcpy (tptr, src_acl, size);
      tokinf->DefaultDacl.DefaultDacl = (PACL) tptr;
    }
#else
    {
      /* ...on 32 bit systems we just allocate tokinf with the same size as
         we get, copy the whole structure and convert offsets into pointers. */

      /* Allocate LUID for usage in the logon SID on Windows 2000.  This is
	 not done in the 64 bit code above for hopefully obvious reasons... */
      LUID logon_sid_id;

      if (must_create_logon_sid && !AllocateLocallyUniqueId (&logon_sid_id))
	return STATUS_INSUFFICIENT_RESOURCES;

      if (!(tokinf = funcs->AllocateLsaHeap (authinf->inf_size)))
	return STATUS_NO_MEMORY;
      memcpy (tokinf, &authinf->inf, authinf->inf_size);

      /* User SID */
      tokinf->User.User.Sid = (PSID)
	      ((PBYTE) tokinf + (LONG_PTR) tokinf->User.User.Sid);
      /* Groups */
      tokinf->Groups = (PTOKEN_GROUPS)
	      ((PBYTE) tokinf + (LONG_PTR) tokinf->Groups);
      /* Group SIDs */
      for (i = 0; i < tokinf->Groups->GroupCount; ++i)
	{
	  tokinf->Groups->Groups[i].Sid = (PSID)
		((PBYTE) tokinf + (LONG_PTR) tokinf->Groups->Groups[i].Sid);
	  if (must_create_logon_sid
	      && tokinf->Groups->Groups[i].Attributes & SE_GROUP_LOGON_ID
	      && *GetSidSubAuthorityCount (tokinf->Groups->Groups[i].Sid) == 3
	      && *GetSidSubAuthority (tokinf->Groups->Groups[i].Sid, 0)
		 == SECURITY_LOGON_IDS_RID)
	    {
	      *GetSidSubAuthority (tokinf->Groups->Groups[i].Sid, 1)
	      = logon_sid_id.HighPart;
	      *GetSidSubAuthority (tokinf->Groups->Groups[i].Sid, 2)
	      = logon_sid_id.LowPart;
	    }
	}

      /* Primary Group SID */
      tokinf->PrimaryGroup.PrimaryGroup = (PSID)
	      ((PBYTE) tokinf + (LONG_PTR) tokinf->PrimaryGroup.PrimaryGroup);
      /* Privileges */
      tokinf->Privileges = (PTOKEN_PRIVILEGES)
	      ((PBYTE) tokinf + (LONG_PTR) tokinf->Privileges);
      /* Owner SID */
      tokinf->Owner.Owner = NULL;
      /* Default DACL */
      tokinf->DefaultDacl.DefaultDacl = (PACL)
	      ((PBYTE) tokinf + (LONG_PTR) tokinf->DefaultDacl.DefaultDacl);

    }
#endif

  *tok = (PVOID) tokinf;
  *tok_type = LsaTokenInformationV2;

#ifdef DEBUGGING
  print_tokinf (tokinf, authinf->inf_size, authinf, &authinf->inf,
		(PVOID)((LONG_PTR) &authinf->inf + authinf->inf_size));
#endif

  /* Create logon session. */
  if (!AllocateLocallyUniqueId (logon_id))
    {
      funcs->FreeLsaHeap (*tok);
      *tok = NULL;
      return STATUS_INSUFFICIENT_RESOURCES;
    }
  stat = funcs->CreateLogonSession (logon_id);
  if (stat != STATUS_SUCCESS)
    {
      funcs->FreeLsaHeap (*tok);
      *tok = NULL;
      return stat;
    }

  return STATUS_SUCCESS;
}

VOID NTAPI
LsaApLogonTerminated(PLUID LogonId)
{
}

NTSTATUS NTAPI
LsaApCallPackage (PLSA_CLIENT_REQUEST request, PVOID authinf,
		  PVOID client_auth_base, ULONG auth_len, PVOID *ret_buf,
		  PULONG ret_buf_len, PNTSTATUS ret_stat) 
{
  return STATUS_NOT_IMPLEMENTED;
}
