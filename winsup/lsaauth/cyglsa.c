/* cyglsa.c: LSA authentication module for Cygwin

   Written by Corinna Vinschen <corinna@vinschen.de>

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for details. */

#define WINVER 0x0600
#include <ntstatus.h>
#include <windows.h>
#include <wininet.h>
#include <lmcons.h>
#include <iptypes.h>
#include <ntsecapi.h>
#include "../cygwin/cyglsa.h"
#include "../cygwin/include/cygwin/version.h"

static PLSA_SECPKG_FUNCS funcs;

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

/* Declare NTDLL functions here to avoid problems with different
   header file layout in different compiler environments. */
#ifndef NT_SUCCESS
#define NT_SUCCESS(s)	((s) >= 0)
#endif
/* These standard POSIX functions are implemented in NTDLL and exported.
   There's just no header to define them and using wchar.h from mingw
   or Cygwin seems wrong somehow. */
wchar_t *__cdecl wcscpy (wchar_t *, const wchar_t *);
size_t __cdecl wcslen (const wchar_t *);

#ifndef RtlInitEmptyUnicodeString
__inline VOID NTAPI
RtlInitEmptyUnicodeString(PUNICODE_STRING dest, PCWSTR buf, USHORT len)
{
  dest->Length = 0;
  dest->MaximumLength = len;
  dest->Buffer = (PWSTR) buf;
}
#endif

static PUNICODE_STRING
uni_alloc (PWCHAR src, USHORT len)
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

/* No, I don't want to include stdio.h so I take what ntdll offers. */
extern int _vsnprintf (char *, size_t, const char *, va_list);

static HANDLE fh = INVALID_HANDLE_VALUE;

static int
cyglsa_printf (const char *format, ...)
{
  char buf[256];
  DWORD wr;
  int ret;
  va_list ap;

  if (fh == INVALID_HANDLE_VALUE)
    return 0;

  va_start (ap, format);
  ret = _vsnprintf (buf, 256, format, ap);
  va_end (ap);
  if (ret <= 0)
    return ret;
  if (ret > 256)
    ret = 255;
  buf[255] = '\0';
  WriteFile (fh, buf, ret, &wr, NULL);
  return wr;
}

static void
print_sid (const char *prefix, int idx, PISID sid)
{
  DWORD i;

  cyglsa_printf ("%s", prefix);
  if (idx >= 0)
    cyglsa_printf ("[%d] ", idx);
  cyglsa_printf ("(0x%08x) ", (INT_PTR) sid);
  if (!sid)
    cyglsa_printf ("NULL\n");
  else if (IsBadReadPtr (sid, 8))
    cyglsa_printf ("INVALID POINTER\n");
  else if (!IsValidSid ((PSID) sid))
    cyglsa_printf ("INVALID SID\n");
  else if (IsBadReadPtr (sid, 8 + sizeof (DWORD) * sid->SubAuthorityCount))
    cyglsa_printf ("INVALID POINTER SPACE\n");
  else
    {
      cyglsa_printf ("S-%d-%d", sid->Revision, sid->IdentifierAuthority.Value[5]);
      for (i = 0; i < sid->SubAuthorityCount; ++i)
        cyglsa_printf ("-%lu", sid->SubAuthority[i]);
      cyglsa_printf ("\n");
    }
}

static void
print_groups (PTOKEN_GROUPS grps)
{
  DWORD i;

  cyglsa_printf ("Groups: (0x%08x) ", (INT_PTR) grps);
  if (!grps)
    cyglsa_printf ("NULL\n");
  else if (IsBadReadPtr (grps, sizeof (DWORD)))
    cyglsa_printf ("INVALID POINTER\n");
  else if (IsBadReadPtr (grps, sizeof (DWORD) + sizeof (SID_AND_ATTRIBUTES)
  						* grps->GroupCount))
    cyglsa_printf ("INVALID POINTER SPACE\n");
  else
    {
      cyglsa_printf ("Count: %lu\n", grps->GroupCount);
      for (i = 0; i < grps->GroupCount; ++i)
        {
	  cyglsa_printf ("(attr: 0x%lx)", grps->Groups[i].Attributes);
	  print_sid (" ", i, (PISID) grps->Groups[i].Sid);
	}
    }
}

static void
print_privs (PTOKEN_PRIVILEGES privs)
{
  DWORD i;

  cyglsa_printf ("Privileges: (0x%08x) ", (INT_PTR) privs);
  if (!privs)
    cyglsa_printf ("NULL\n");
  else if (IsBadReadPtr (privs, sizeof (DWORD)))
    cyglsa_printf ("INVALID POINTER\n");
  else if (IsBadReadPtr (privs, sizeof (DWORD) + sizeof (LUID_AND_ATTRIBUTES)
						 * privs->PrivilegeCount))
    cyglsa_printf ("INVALID POINTER SPACE\n");
  else
    {
      cyglsa_printf ("Count: %lu\n", privs->PrivilegeCount);
      for (i = 0; i < privs->PrivilegeCount; ++i)
	cyglsa_printf ("Luid: {%ld, %lu} Attributes: 0x%lx\n",
		privs->Privileges[i].Luid.HighPart,
		privs->Privileges[i].Luid.LowPart,
		privs->Privileges[i].Attributes);
    }
}

static void
print_dacl (PACL dacl)
{
  DWORD i;

  cyglsa_printf ("DefaultDacl: (0x%08x) ", (INT_PTR) dacl);
  if (!dacl)
    cyglsa_printf ("NULL\n");
  else if (IsBadReadPtr (dacl, sizeof (ACL)))
    cyglsa_printf ("INVALID POINTER\n");
  else if (IsBadReadPtr (dacl, dacl->AclSize))
    cyglsa_printf ("INVALID POINTER SPACE\n");
  else
    {
      cyglsa_printf ("Rev: %d, Count: %d\n", dacl->AclRevision, dacl->AceCount);
      for (i = 0; i < dacl->AceCount; ++i)
        {
	  PVOID vace;
	  PACCESS_ALLOWED_ACE ace;

	  if (!GetAce (dacl, i, &vace))
	    cyglsa_printf ("[%lu] GetAce error %lu\n", i, GetLastError ());
	  else
	    {
	      ace = (PACCESS_ALLOWED_ACE) vace;
	      cyglsa_printf ("Type: %x, Flags: %x, Access: %lx,",
		      ace->Header.AceType, ace->Header.AceFlags, (DWORD) ace->Mask);
	      print_sid (" ", i, (PISID) &ace->SidStart);
	    }
	}
    }
}

static void
print_tokinf (PLSA_TOKEN_INFORMATION_V2 ptok, size_t size,
	       PVOID got_start, PVOID gotinf_start, PVOID gotinf_end)
{
  if (fh == INVALID_HANDLE_VALUE)
    return;

  cyglsa_printf ("INCOMING: start: 0x%08x infstart: 0x%08x infend: 0x%08x\n",
	  (INT_PTR) got_start, (INT_PTR) gotinf_start,
	  (INT_PTR) gotinf_end);

  cyglsa_printf ("LSA_TOKEN_INFORMATION_V2: 0x%08x - 0x%08x\n",
	  (INT_PTR) ptok, (INT_PTR) ptok + size);

  /* User SID */
  cyglsa_printf ("User: (attr: 0x%lx)", ptok->User.User.Attributes);
  print_sid (" ", -1, (PISID) ptok->User.User.Sid);

  /* Groups */
  print_groups (ptok->Groups);

  /* Primary Group SID */
  print_sid ("Primary Group: ", -1, (PISID)ptok->PrimaryGroup.PrimaryGroup);

  /* Privileges */
  print_privs (ptok->Privileges);

  /* Owner */
  print_sid ("Owner: ", -1, (PISID) ptok->Owner.Owner);

  /* Default DACL */
  print_dacl (ptok->DefaultDacl.DefaultDacl);

  // CloseHandle (fh);
}

NTSTATUS NTAPI
LsaApInitializePackage (ULONG authp_id, PLSA_SECPKG_FUNCS dpt,
			PLSA_STRING dummy1, PLSA_STRING dummy2,
			PLSA_STRING *authp_name)
{
  PLSA_STRING name = NULL;

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

#ifdef DEBUGGING
  fh = CreateFile ("C:\\cyglsa.dbgout", GENERIC_WRITE,
		   FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
		   NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
  cyglsa_printf ("Initialized\n");
#endif /* DEBUGGING */

  return STATUS_SUCCESS;
}

NTSTATUS NTAPI
LsaApLogonUserEx (PLSA_CLIENT_REQUEST request, SECURITY_LOGON_TYPE logon_type,
		  PVOID auth, PVOID client_auth_base, ULONG auth_len,
		  PVOID *pbuf, PULONG pbuf_len, PLUID logon_id,
		  PNTSTATUS sub_stat, PLSA_TOKEN_INFORMATION_TYPE tok_type,
		  PVOID *tok, PUNICODE_STRING *account,
		  PUNICODE_STRING *authority, PUNICODE_STRING *machine)
{
  DWORD checksum, i;
  PDWORD csp, csp_end;
  NTSTATUS stat;
  SECPKG_CLIENT_INFO clinf;
  PLSA_TOKEN_INFORMATION_V2 tokinf;

  cyglsa_t *authinf = (cyglsa_t *) auth;

  /* Check if the caller has the SeTcbPrivilege, otherwise refuse service. */
  stat = funcs->GetClientInfo (&clinf);
  if (stat != STATUS_SUCCESS)
    {
      cyglsa_printf ("GetClientInfo failed: 0x%08lx\n", stat);
      return stat;
    }
  if (!clinf.HasTcbPrivilege)
    {
      cyglsa_printf ("Client has no TCB privilege.  Access denied.\n");
      return STATUS_ACCESS_DENIED;
    }

  /* Make a couple of validity checks. */
  if (auth_len < sizeof *authinf
      || authinf->magic != CYG_LSA_MAGIC
      || !authinf->username[0]
      || !authinf->domain[0])
    {
      cyglsa_printf ("Invalid authentication parameter.\n");
      return STATUS_INVALID_PARAMETER;
    }
  checksum = CYG_LSA_MAGIC;
  csp = (PDWORD) &authinf->username;
  csp_end = (PDWORD) ((PBYTE) authinf + auth_len);
  while (csp < csp_end)
    checksum += *csp++;
  if (authinf->checksum != checksum)
    {
      cyglsa_printf ("Invalid checksum.\n");
      return STATUS_INVALID_PARAMETER_3;
    }

  /* Set account to username and authority to domain resp. machine name.
     The name of the logon account name as returned by LookupAccountSid
     is created from here as "authority\account". */
  authinf->username[UNLEN] = '\0';
  authinf->domain[MAX_DOMAIN_NAME_LEN] = '\0';
  if (account && !(*account = uni_alloc (authinf->username,
  					 wcslen (authinf->username))))
    {
      cyglsa_printf ("No memory trying to create account.\n");
      return STATUS_NO_MEMORY;
    }
  if (authority && !(*authority = uni_alloc (authinf->domain,
					     wcslen (authinf->domain))))
    {
      cyglsa_printf ("No memory trying to create authority.\n");
      return STATUS_NO_MEMORY;
    }
  if (machine)
    {
      WCHAR mach[MAX_COMPUTERNAME_LENGTH + 1];
      DWORD msize = MAX_COMPUTERNAME_LENGTH + 1;
      if (!GetComputerNameW (mach, &msize))
        wcscpy (mach, L"UNKNOWN");
      if (!(*machine = uni_alloc (mach, wcslen (mach))))
	{
	  cyglsa_printf ("No memory trying to create machine.\n");
	  return STATUS_NO_MEMORY;
	}
    }
  /* Create a fake buffer in pbuf which is free'd again in the client.
     Windows 2000 tends to crash when setting this pointer to NULL. */
  if (pbuf)
    {
#ifdef JUST_ANOTHER_NONWORKING_SOLUTION
      cygprf_t prf;
      WCHAR sam_username[MAX_DOMAIN_NAME_LEN + UNLEN + 2];
      SECURITY_STRING sam_user, prefix;
      PUCHAR user_auth;
      ULONG user_auth_size;
      WCHAR flatname[UNLEN + 1];
      UNICODE_STRING flatnm;
      TOKEN_SOURCE ts;
      HANDLE token;
#endif /* JUST_ANOTHER_NONWORKING_SOLUTION */

      stat = funcs->AllocateClientBuffer (request, 64UL, pbuf);
      if (!LSA_SUCCESS (stat))
	{
	  cyglsa_printf ("AllocateClientBuffer failed: 0x%08lx\n", stat);
	  return stat;
	}
#ifdef JUST_ANOTHER_NONWORKING_SOLUTION
      prf.magic_pre = MAGIC_PRE;
      prf.token = NULL;
      prf.magic_post = MAGIC_POST;

#if 0
      /* That's how it was supposed to work according to MSDN... */
      wcscpy (sam_username, authinf->domain);
      wcscat (sam_username, L"\\");
      wcscat (sam_username, authinf->username);
#else
      /* That's the only solution which worked, and then it only worked
         for machine local accounts.  No domain authentication possible.
	 STATUS_NO_SUCH_USER galore! */
      wcscpy (sam_username, authinf->username);
#endif
      RtlInitUnicodeString (&sam_user, sam_username);
      RtlInitUnicodeString (&prefix, L"");
      RtlInitEmptyUnicodeString (&flatnm, flatname,
				 (UNLEN + 1) * sizeof (WCHAR));

      stat = funcs->GetAuthDataForUser (&sam_user, SecNameSamCompatible,
					NULL, &user_auth,
					&user_auth_size, &flatnm);
      if (!NT_SUCCESS (stat))
	{
	  char sam_u[MAX_DOMAIN_NAME_LEN + UNLEN + 2];
	  wcstombs (sam_u, sam_user.Buffer, sizeof (sam_u));
	  cyglsa_printf ("GetAuthDataForUser (%u,%u,%s) failed: 0x%08lx\n",
		  sam_user.Length, sam_user.MaximumLength, sam_u, stat);
	  return stat;
	}

      memcpy (ts.SourceName, "Cygwin.1", 8);
      ts.SourceIdentifier.HighPart = 0;
      ts.SourceIdentifier.LowPart = 0x0104;
      RtlInitEmptyUnicodeString (&flatnm, flatname,
				 (UNLEN + 1) * sizeof (WCHAR));
      stat = funcs->ConvertAuthDataToToken (user_auth, user_auth_size,
					    SecurityDelegation, &ts,
					    Interactive, *authority,
					    &token, logon_id, &flatnm,
					    sub_stat);
      if (!NT_SUCCESS (stat))
	{
	  cyglsa_printf ("ConvertAuthDataToToken failed: 0x%08lx\n", stat);
	  return stat;
	}

      stat = funcs->DuplicateHandle (token, &prf.token);
      if (!NT_SUCCESS (stat))
	{
	  cyglsa_printf ("DuplicateHandle failed: 0x%08lx\n", stat);
	  return stat;
	}
      
      stat = funcs->CopyToClientBuffer (request, sizeof prf, *pbuf, &prf);
      if (!NT_SUCCESS (stat))
	{
	  cyglsa_printf ("CopyToClientBuffer failed: 0x%08lx\n", stat);
	  return stat;
	}
      funcs->FreeLsaHeap (user_auth);
#endif /* JUST_ANOTHER_NONWORKING_SOLUTION */
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
      newsize += sizeof (TOKEN_USER) - sizeof (CYG_TOKEN_USER); /* User SID */
      newsize += sizeof (PTOKEN_GROUPS) - sizeof (OFFSET); /* Groups */
      src_grps = (PCYG_TOKEN_GROUPS) (base + authinf->inf.Groups);
      newsize += src_grps->GroupCount		  /* Group SIDs */
      		 * (sizeof (SID_AND_ATTRIBUTES)
		    - sizeof (CYG_SID_AND_ATTRIBUTES));
      newsize += sizeof (PSID) - sizeof (OFFSET); /* Primary Group SID */
      newsize += sizeof (PTOKEN_PRIVILEGES) - sizeof (OFFSET); /* Privileges */
      newsize += 0; /* Owner SID */
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
	tokinf->Groups->Groups[i].Sid = (PSID)
	      ((PBYTE) tokinf + (LONG_PTR) tokinf->Groups->Groups[i].Sid);
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

  print_tokinf (tokinf, authinf->inf_size, authinf, &authinf->inf,
		(PVOID)((LONG_PTR) &authinf->inf + authinf->inf_size));

  /* Create logon session. */
  if (!AllocateLocallyUniqueId (logon_id))
    {
      funcs->FreeLsaHeap (*tok);
      *tok = NULL;
      cyglsa_printf ("AllocateLocallyUniqueId failed: Win32 error %lu\n",
		     GetLastError ());
      return STATUS_INSUFFICIENT_RESOURCES;
    }
  stat = funcs->CreateLogonSession (logon_id);
  if (stat != STATUS_SUCCESS)
    {
      funcs->FreeLsaHeap (*tok);
      *tok = NULL;
      cyglsa_printf ("CreateLogonSession failed: 0x%08lx\n", stat);
      return stat;
    }

  cyglsa_printf ("BINGO!!!\n", stat);
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
