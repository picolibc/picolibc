/* sec/auth.cc: NT authentication functions

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

#include "winsup.h"
#include <stdlib.h>
#include <wchar.h>
#include <wininet.h>
#include <ntsecapi.h>
#include "cygerrno.h"
#include "security.h"
#include "path.h"
#include "fhandler.h"
#include "dtable.h"
#include "cygheap.h"
#include "registry.h"
#include "ntdll.h"
#include "tls_pbuf.h"
#include <lm.h>
#include <iptypes.h>
#include <userenv.h>
#define SECURITY_WIN32
#include <secext.h>
#include "cygserver_setpwd.h"
#include <cygwin/version.h>

/* OpenBSD 2.0 and later. */
extern "C"
int
issetugid (void)
{
  return cygheap->user.issetuid () ? 1 : 0;
}

/* The token returned by system functions is a restricted token.  The full
   admin token is linked to it and can be fetched with NtQueryInformationToken.
   This function returns the elevated token if available, the original token
   otherwise.  The token handle is also made inheritable since that's necessary
   anyway. */
static HANDLE
get_full_privileged_inheritable_token (HANDLE token)
{
  TOKEN_LINKED_TOKEN linked;
  ULONG size;

  /* When fetching the linked token without TCB privs, then the linked
     token is not a primary token, only an impersonation token, which is
     not suitable for CreateProcessAsUser.  Converting it to a primary
     token using DuplicateTokenEx does NOT work for the linked token in
     this case.  So we have to switch on TCB privs to get a primary token.
     This is generally performed in the calling functions.  */
  if (NT_SUCCESS (NtQueryInformationToken (token, TokenLinkedToken,
					   (PVOID) &linked, sizeof linked,
					   &size)))
    {
      debug_printf ("Linked Token: %p", linked.LinkedToken);
      if (linked.LinkedToken)
	{
	  TOKEN_TYPE type;

	  /* At this point we don't know if the user actually had TCB
	     privileges.  Check if the linked token is a primary token.
	     If not, just return the original token. */
	  if (NT_SUCCESS (NtQueryInformationToken (linked.LinkedToken,
						   TokenType, (PVOID) &type,
						   sizeof type, &size))
	      && type != TokenPrimary)
	    debug_printf ("Linked Token is not a primary token!");
	  else
	    {
	      CloseHandle (token);
	      token = linked.LinkedToken;
	    }
	}
    }
  if (!SetHandleInformation (token, HANDLE_FLAG_INHERIT, HANDLE_FLAG_INHERIT))
    {
      __seterrno ();
      CloseHandle (token);
      token = NULL;
    }
  return token;
}

void
set_imp_token (HANDLE token, int type)
{
  debug_printf ("set_imp_token (%p, %d)", token, type);
  cygheap->user.external_token = (token == INVALID_HANDLE_VALUE
				  ? NO_IMPERSONATION : token);
  cygheap->user.ext_token_is_restricted = (type == CW_TOKEN_RESTRICTED);
}

extern "C" void
cygwin_set_impersonation_token (const HANDLE hToken)
{
  set_imp_token (hToken, CW_TOKEN_IMPERSONATION);
}

void
extract_nt_dom_user (const struct passwd *pw, PWCHAR domain, PWCHAR user)
{

  cygsid psid;
  DWORD ulen = UNLEN + 1;
  DWORD dlen = MAX_DOMAIN_NAME_LEN + 1;
  SID_NAME_USE use;

  debug_printf ("pw_gecos %p (%s)", pw->pw_gecos, pw->pw_gecos);

  /* The incoming passwd entry is not necessarily a pointer to the
     internal passwd buffers, thus we must not rely on being able to
     cast it to pg_pwd. */
  if (psid.getfrompw_gecos (pw)
      && LookupAccountSidW (NULL, psid, user, &ulen, domain, &dlen, &use))
    return;

  char *d, *u, *c;
  domain[0] = L'\0';
  sys_mbstowcs (user, UNLEN + 1, pw->pw_name);
  if ((d = strstr (pw->pw_gecos, "U-")) != NULL &&
      (d == pw->pw_gecos || d[-1] == ','))
    {
      c = strchrnul (d + 2, ',');
      if ((u = strchrnul (d + 2, '\\')) >= c)
	u = d + 1;
      else if (u - d <= MAX_DOMAIN_NAME_LEN + 2)
	sys_mbstowcs (domain, MAX_DOMAIN_NAME_LEN + 1, d + 2, u - d - 1);
      if (c - u <= UNLEN + 1)
	sys_mbstowcs (user, UNLEN + 1, u + 1, c - u);
    }
}

extern "C" HANDLE
cygwin_logon_user (const struct passwd *pw, const char *password)
{
  if (!pw || !password)
    {
      set_errno (EINVAL);
      return INVALID_HANDLE_VALUE;
    }

  WCHAR nt_domain[MAX_DOMAIN_NAME_LEN + 1];
  WCHAR nt_user[UNLEN + 1];
  PWCHAR passwd;
  HANDLE hToken;
  tmp_pathbuf tp;

  extract_nt_dom_user (pw, nt_domain, nt_user);
  debug_printf ("LogonUserW (%W, %W, ...)", nt_user, nt_domain);
  sys_mbstowcs (passwd = tp.w_get (), NT_MAX_PATH, password);
  /* CV 2005-06-08: LogonUser should run under the primary process token,
     otherwise it returns with ERROR_ACCESS_DENIED. */
  cygheap->user.deimpersonate ();
  if (!LogonUserW (nt_user, *nt_domain ? nt_domain : NULL, passwd,
		   LOGON32_LOGON_INTERACTIVE, LOGON32_PROVIDER_DEFAULT,
		   &hToken))
    {
      __seterrno ();
      hToken = INVALID_HANDLE_VALUE;
    }
  else
    {
      HANDLE hPrivToken = NULL;

      /* See the comment in get_full_privileged_inheritable_token for a
      description why we enable TCB privileges here. */
      push_self_privilege (SE_TCB_PRIVILEGE, true);
      hPrivToken = get_full_privileged_inheritable_token (hToken);
      pop_self_privilege ();
      if (!hPrivToken)
	debug_printf ("Can't fetch linked token (%E), use standard token");
      else
	hToken = hPrivToken;
    }
  RtlSecureZeroMemory (passwd, NT_MAX_PATH);
  cygheap->user.reimpersonate ();
  debug_printf ("%R = logon_user(%s,...)", hToken, pw->pw_name);
  return hToken;
}

/* The buffer path points to should be at least MAX_PATH bytes. */
PWCHAR
get_user_profile_directory (PCWSTR sidstr, PWCHAR path, SIZE_T path_len)
{
  if (!sidstr || !path)
    return NULL;

  UNICODE_STRING buf;
  tmp_pathbuf tp;
  tp.u_get (&buf);
  NTSTATUS status;

  RTL_QUERY_REGISTRY_TABLE tab[2] = {
    { NULL, RTL_QUERY_REGISTRY_NOEXPAND | RTL_QUERY_REGISTRY_DIRECT
           | RTL_QUERY_REGISTRY_REQUIRED,
      L"ProfileImagePath", &buf, REG_NONE, NULL, 0 },
    { NULL, 0, NULL, NULL, 0, NULL, 0 }
  };

  WCHAR key[wcslen (sidstr) + 16];
  wcpcpy (wcpcpy (key, L"ProfileList\\"), sidstr);
  status = RtlQueryRegistryValues (RTL_REGISTRY_WINDOWS_NT, key, tab,
                                  NULL, NULL);
  if (!NT_SUCCESS (status) || buf.Length == 0)
    {
      debug_printf ("ProfileImagePath for %W not found, status %y", sidstr,
		    status);
      return NULL;
    }
  ExpandEnvironmentStringsW (buf.Buffer, path, path_len);
  debug_printf ("ProfileImagePath for %W: %W", sidstr, path);
  return path;
}

/* Load user profile if it's not already loaded.  If the user profile doesn't
   exist on the machine try to create it.

   Return a handle to the loaded user registry hive only if it got actually
   loaded here, not if it already existed.  There's no reliable way to know
   when to unload the hive yet, so we're leaking this registry handle for now.
   TODO: Try to find a way to reliably unload the user profile again. */
HANDLE
load_user_profile (HANDLE token, struct passwd *pw, cygpsid &usersid)
{
  WCHAR domain[DNLEN + 1];
  WCHAR username[UNLEN + 1];
  WCHAR sid[128];
  WCHAR userpath[MAX_PATH];
  PROFILEINFOW pi;

  /* Initialize */
  if (!cygheap->dom.init ())
    return NULL;

  extract_nt_dom_user (pw, domain, username);
  usersid.string (sid);
  debug_printf ("user: <%W> <%W> <%W>", username, domain, sid);
  /* Check if the local profile dir has already been created. */
  if (!get_user_profile_directory (sid, userpath, MAX_PATH))
   {
     /* No, try to create it. */
     HRESULT res = CreateProfile (sid, username, userpath, MAX_PATH);
     if (res != S_OK)
       {
	 debug_printf ("CreateProfile, HRESULT %x", res);
	 return NULL;
       }
    }
  /* Fill PROFILEINFO */
  memset (&pi, 0, sizeof pi);
  pi.dwSize = sizeof pi;
  pi.dwFlags = PI_NOUI;
  pi.lpUserName = username;
  /* Check if user has a roaming profile and fill in lpProfilePath, if so.
     Call NetUserGetInfo only for local machine accounts, use LDAP otherwise. */
  if (!wcscasecmp (domain, cygheap->dom.account_flat_name ()))
    {
      NET_API_STATUS nas;
      PUSER_INFO_3 ui;

      nas = NetUserGetInfo (NULL, username, 3, (PBYTE *) &ui);
      if (nas != NERR_Success)
	debug_printf ("NetUserGetInfo, %u", nas);
      else
	{
	  if (ui->usri3_profile && *ui->usri3_profile)
	    {
	      wcsncpy (userpath, ui->usri3_profile, MAX_PATH - 1);
	      userpath[MAX_PATH - 1] = L'\0';
	      pi.lpProfilePath = userpath;
	    }
	  NetApiBufferFree (ui);
	}
    }
  else
    {
      cyg_ldap cldap;
      PCWSTR dnsdomain = NULL;

      if (wcscasecmp (domain, cygheap->dom.primary_flat_name ()))
	{
	  PDS_DOMAIN_TRUSTSW td = NULL;

	  for (ULONG idx = 0; (td = cygheap->dom.trusted_domain (idx)); ++idx)
	    if (!wcscasecmp (domain, td->NetbiosDomainName))
	      {
		dnsdomain = td->DnsDomainName;
		break;
	      }
	}
      if (cldap.fetch_ad_account (usersid, false, dnsdomain))
	{
	  PWCHAR val = cldap.get_profile_path ();
	  if (val && *val)
	    {
	      wcsncpy (userpath, val, MAX_PATH - 1);
	      userpath[MAX_PATH - 1] = L'\0';
	      pi.lpProfilePath = userpath;
	    }
	}
    }

  if (!LoadUserProfileW (token, &pi))
    debug_printf ("LoadUserProfileW, %E");
  return pi.hProfile;
}

HANDLE
lsa_open_policy (PWCHAR server, ACCESS_MASK access)
{
  LSA_UNICODE_STRING srvbuf;
  PLSA_UNICODE_STRING srv = NULL;
  static LSA_OBJECT_ATTRIBUTES oa = { 0, 0, 0, 0, 0, 0 };
  HANDLE lsa;

  if (server)
    {
      srv = &srvbuf;
      RtlInitUnicodeString (srv, server);
    }
  NTSTATUS status = LsaOpenPolicy (srv, &oa, access, &lsa);
  if (!NT_SUCCESS (status))
    {
      __seterrno_from_nt_status (status);
      lsa = NULL;
    }
  return lsa;
}

void
lsa_close_policy (HANDLE lsa)
{
  if (lsa)
    LsaClose (lsa);
}

bool
get_logon_server (PCWSTR domain, PWCHAR server, ULONG flags)
{
  DWORD ret;
  PDOMAIN_CONTROLLER_INFOW pci;

  /* Empty domain is interpreted as local system */
  if (cygheap->dom.init ()
      && (!domain[0]
	  || !wcscasecmp (domain, cygheap->dom.account_flat_name ())))
    {
      wcpcpy (wcpcpy (server, L"\\\\"), cygheap->dom.account_flat_name ());
      return true;
    }

  /* Try to get any available domain controller for this domain */
  ret = DsGetDcNameW (NULL, domain, NULL, NULL, flags, &pci);
  if (ret == ERROR_SUCCESS)
    {
      wcscpy (server, pci->DomainControllerName);
      NetApiBufferFree (pci);
      debug_printf ("DC: server: %W", server);
      return true;
    }
  __seterrno_from_win_error (ret);
  return false;
}

static bool
sid_in_token_groups (PTOKEN_GROUPS grps, cygpsid sid)
{
  if (!grps)
    return false;
  for (DWORD i = 0; i < grps->GroupCount; ++i)
    if (sid == grps->Groups[i].Sid)
      return true;
  return false;
}

bool
get_server_groups (cygsidlist &grp_list, PSID usersid,
		   acct_disabled_chk_t check_account_disabled)
{
  WCHAR user[UNLEN + 1];
  WCHAR domain[MAX_DOMAIN_NAME_LEN + 1];
  DWORD ulen = UNLEN + 1;
  DWORD dlen = MAX_DOMAIN_NAME_LEN + 1;
  SID_NAME_USE use;

  if (well_known_system_sid == usersid)
    {
      grp_list *= well_known_admins_sid;
      return true;
    }

  if (!LookupAccountSidW (NULL, usersid, user, &ulen, domain, &dlen, &use))
    {
      __seterrno ();
      return false;
    }
  /* If the SID does NOT start with S-1-5-21, the domain is some builtin
     domain.  We don't fetch a group list then. */
  if (sid_id_auth (usersid) == 5 /* SECURITY_NT_AUTHORITY */
      && sid_sub_auth (usersid, 0) == SECURITY_NT_NON_UNIQUE)
    {
      tmp_pathbuf tp;
      HANDLE token;
      NTSTATUS status;
      PTOKEN_GROUPS groups;
      ULONG size;

      token = s4uauth (false, domain, user, status);
      if (!token)
	return false;

      groups = (PTOKEN_GROUPS) tp.w_get ();
      status = NtQueryInformationToken (token, TokenGroups, groups,
					2 * NT_MAX_PATH, &size);
      if (NT_SUCCESS (status))
	for (DWORD pg = 0; pg < groups->GroupCount; ++pg)
	  {
	    if (groups->Groups[pg].Attributes & SE_GROUP_USE_FOR_DENY_ONLY)
	      continue;
	    cygpsid grpsid = groups->Groups[pg].Sid;
	    if (sid_id_auth (grpsid) == 5 /* SECURITY_NT_AUTHORITY */
		&& sid_sub_auth (grpsid, 0) == SECURITY_NT_NON_UNIQUE)
	      grp_list += grpsid;
	    else
	      grp_list *= grpsid;
	  }
      NtClose (token);
    }
  return true;
}

/* Accept a token if
   - the requested usersid matches the TokenUser and
   - if setgroups has been called:
	the token groups that are listed in /etc/group match the union of
	the requested primary and supplementary groups in gsids.
   - else the (unknown) implicitly requested supplementary groups and those
	in the token are the groups associated with the usersid. We assume
	they match and verify only the primary groups.
	The requested primary group must appear in the token.
	The primary group in the token is a group associated with the usersid,
	except if the token is internal and the group is in the token SD.  In
	that latter case that group must match the requested primary group.  */
bool
verify_token (HANDLE token, cygsid &usersid, user_groups &groups, bool *pintern)
{
  NTSTATUS status;
  ULONG size;
  bool intern = false;
  tmp_pathbuf tp;

  if (pintern)
    {
      TOKEN_SOURCE ts;
      status = NtQueryInformationToken (token, TokenSource, &ts, sizeof ts,
					&size);
      if (!NT_SUCCESS (status))
	debug_printf ("NtQueryInformationToken(), %y", status);
      else
	*pintern = intern = !memcmp (ts.SourceName, "Cygwin.1", 8);
    }
  /* Verify usersid */
  cygsid tok_usersid (NO_SID);
  status = NtQueryInformationToken (token, TokenUser, &tok_usersid,
				    sizeof tok_usersid, &size);
  if (!NT_SUCCESS (status))
    debug_printf ("NtQueryInformationToken(), %y", status);
  if (usersid != tok_usersid)
    return false;

  /* For an internal token, if setgroups was not called and if the sd group
     is not well_known_null_sid, it must match pgrpsid */
  if (intern && !groups.issetgroups ())
    {
      const DWORD sd_buf_siz = SECURITY_MAX_SID_SIZE
			       + sizeof (SECURITY_DESCRIPTOR);
      PSECURITY_DESCRIPTOR sd_buf = (PSECURITY_DESCRIPTOR) alloca (sd_buf_siz);
      cygpsid gsid (NO_SID);
      NTSTATUS status;
      status = NtQuerySecurityObject (token, GROUP_SECURITY_INFORMATION,
				      sd_buf, sd_buf_siz, &size);
      if (!NT_SUCCESS (status))
	debug_printf ("NtQuerySecurityObject(), %y", status);
      else
	{
	  BOOLEAN dummy;
	  status = RtlGetGroupSecurityDescriptor (sd_buf, (PSID *) &gsid,
						  &dummy);
	  if (!NT_SUCCESS (status))
	    debug_printf ("RtlGetGroupSecurityDescriptor(), %y", status);
	}
      if (well_known_null_sid != gsid)
	return gsid == groups.pgsid;
    }

  PTOKEN_GROUPS my_grps = (PTOKEN_GROUPS) tp.w_get ();

  status = NtQueryInformationToken (token, TokenGroups, my_grps,
				    2 * NT_MAX_PATH, &size);
  if (!NT_SUCCESS (status))
    {
      debug_printf ("NtQueryInformationToken(my_token, TokenGroups), %y",
		    status);
      return false;
    }

  bool sawpg = false;

  if (groups.issetgroups ()) /* setgroups was called */
    {
      cygpsid gsid;
      bool saw[groups.sgsids.count ()];

      /* Check that all groups in the setgroups () list are in the token.
	 A token created through ADVAPI should be allowed to contain more
	 groups than requested through setgroups(), especially since the
	 addition of integrity groups. */
      memset (saw, 0, sizeof(saw));
      for (int gidx = 0; gidx < groups.sgsids.count (); gidx++)
	{
	  gsid = groups.sgsids.sids[gidx];
	  if (sid_in_token_groups (my_grps, gsid))
	    {
	      int pos = groups.sgsids.position (gsid);
	      if (pos >= 0)
		saw[pos] = true;
	      else if (groups.pgsid == gsid)
		sawpg = true;
	    }
	}
      /* user.sgsids groups must be in the token, except for builtin groups.
	 These can be different on domain member machines compared to
	 domain controllers, so these builtin groups may be validly missing
	 from a token created through password or lsaauth logon. */
      for (int gidx = 0; gidx < groups.sgsids.count (); gidx++)
	if (!saw[gidx]
	    && !groups.sgsids.sids[gidx].is_well_known_sid ()
	    && !sid_in_token_groups (my_grps, groups.sgsids.sids[gidx]))
	  return false;
    }
  /* The primary group must be in the token */
  return sawpg
	 || sid_in_token_groups (my_grps, groups.pgsid)
	 || groups.pgsid == usersid;
}

const char *
account_restriction (NTSTATUS status)
{
  const char *type;

  switch (status)
    {
    case STATUS_INVALID_LOGON_HOURS:
      type = "Logon outside allowed hours";
      break;
    case STATUS_INVALID_WORKSTATION:
      type = "Logon at this machine not allowed";
      break;
    case STATUS_PASSWORD_EXPIRED:
      type = "Password expired";
      break;
    case STATUS_ACCOUNT_DISABLED:
      type = "Account disabled";
      break;
    default:
      type = "Unknown";
      break;
    }
  return type;
}

#define SFU_LSA_KEY_SUFFIX	L"_microsoft_sfu_utility"

HANDLE
lsaprivkeyauth (struct passwd *pw)
{
  NTSTATUS status;
  HANDLE lsa = NULL;
  HANDLE token = NULL;
  WCHAR sid[256];
  WCHAR domain[MAX_DOMAIN_NAME_LEN + 1];
  WCHAR user[UNLEN + 1];
  WCHAR key_name[MAX_DOMAIN_NAME_LEN + UNLEN + wcslen (SFU_LSA_KEY_SUFFIX) + 2];
  UNICODE_STRING key;
  PUNICODE_STRING data = NULL;
  cygsid psid;
  BOOL ret;

  push_self_privilege (SE_TCB_PRIVILEGE, true);

  /* Open policy object. */
  if (!(lsa = lsa_open_policy (NULL, POLICY_GET_PRIVATE_INFORMATION)))
    goto out;

  /* Needed for Interix key and LogonUser. */
  extract_nt_dom_user (pw, domain, user);

  /* First test for a Cygwin entry. */
  if (psid.getfrompw (pw) && psid.string (sid))
    {
      wcpcpy (wcpcpy (key_name, CYGWIN_LSA_KEY_PREFIX), sid);
      RtlInitUnicodeString (&key, key_name);
      status = LsaRetrievePrivateData (lsa, &key, &data);
      if (!NT_SUCCESS (status))
	data = NULL;
    }
  /* No Cygwin key, try Interix key. */
  if (!data && *domain)
    {
      __small_swprintf (key_name, L"%W_%W%W",
			domain, user, SFU_LSA_KEY_SUFFIX);
      RtlInitUnicodeString (&key, key_name);
      status = LsaRetrievePrivateData (lsa, &key, &data);
      if (!NT_SUCCESS (status))
	data = NULL;
    }
  /* Found an entry?  Try to logon. */
  if (data)
    {
      /* The key is not 0-terminated. */
      PWCHAR passwd;
      size_t pwdsize = data->Length + sizeof (WCHAR);

      passwd = (PWCHAR) alloca (pwdsize);
      *wcpncpy (passwd, data->Buffer, data->Length / sizeof (WCHAR)) = L'\0';
      /* Weird:  LsaFreeMemory invalidates the content of the UNICODE_STRING
	 structure, but it does not invalidate the Buffer content. */
      RtlSecureZeroMemory (data->Buffer, data->Length);
      LsaFreeMemory (data);
      debug_printf ("Try logon for %W\\%W", domain, user);
      ret = LogonUserW (user, domain, passwd, LOGON32_LOGON_INTERACTIVE,
			LOGON32_PROVIDER_DEFAULT, &token);
      RtlSecureZeroMemory (passwd, pwdsize);
      if (!ret)
	{
	  __seterrno ();
	  token = NULL;
	}
      else
	token = get_full_privileged_inheritable_token (token);
    }
  lsa_close_policy (lsa);

out:
  pop_self_privilege ();
  return token;
}

/* The following code is inspired by the generate_s4u_user_token
   and lookup_principal_name functions from
   https://github.com/PowerShell/openssh-portable

   Thanks guys!  For courtesy here's the original copyright disclaimer: */

/*
* Author: Manoj Ampalam <manoj.ampalam@microsoft.com>
*   Utilities to generate user tokens
*
* Author: Bryan Berns <berns@uwalumni.com>
*   Updated s4u, logon, and profile loading routines to use
*   normalized login names.
*
* Copyright (c) 2015 Microsoft Corp.
* All rights reserved
*
* Microsoft openssh win32 port
*
* Redistribution and use in source and binary forms, with or without
* modification, are permitted provided that the following conditions
* are met:
*
* 1. Redistributions of source code must retain the above copyright
* notice, this list of conditions and the following disclaimer.
* 2. Redistributions in binary form must reproduce the above copyright
* notice, this list of conditions and the following disclaimer in the
* documentation and/or other materials provided with the distribution.
*
* THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
* IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
* OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
* IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
* INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
* NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
* DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
* THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
* (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
* THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

/* In w32api prior to 10.0.0, MsV1_0S4ULogon and MSV1_0_S4U_LOGON are only
   defined in ddk/ntifs.h, which we can't include. */
#if (__MINGW64_VERSION_MAJOR < 10)

#define MsV1_0S4ULogon ((MSV1_0_LOGON_SUBMIT_TYPE) 12)

typedef struct _MSV1_0_S4U_LOGON
{
  MSV1_0_LOGON_SUBMIT_TYPE MessageType;
  ULONG Flags;
  UNICODE_STRING UserPrincipalName;
  UNICODE_STRING DomainName;
} MSV1_0_S4U_LOGON, *PMSV1_0_S4U_LOGON;

/* Missing in Mingw-w64 */
#define KERB_S4U_LOGON_FLAG_IDENTIFY 0x08

#endif

/* If logon is true we need an impersonation token.  Otherwise we just
   need an identification token, e. g. to fetch the group list. */
HANDLE
s4uauth (bool logon, PCWSTR domain, PCWSTR user, NTSTATUS &ret_status)
{
  LSA_STRING name;
  HANDLE lsa_hdl = NULL;
  LSA_OPERATIONAL_MODE sec_mode;
  NTSTATUS status, sub_status;
  bool kerberos_auth;
  ULONG package_id, size;
  struct {
    LSA_STRING str;
    CHAR buf[16];
  } origin;

  tmp_pathbuf tp;
  PVOID authinf = NULL;
  ULONG authinf_size;
  TOKEN_SOURCE ts;
  PKERB_INTERACTIVE_PROFILE profile = NULL;
  LUID luid;
  QUOTA_LIMITS quota;
  HANDLE token = NULL;

  /* Initialize */
  if (!cygheap->dom.init ())
    return NULL;

  push_self_privilege (SE_TCB_PRIVILEGE, true);

  if (logon)
    {
      /* Register as logon process. */
      debug_printf ("Impersonation requested");
      RtlInitAnsiString (&name, "Cygwin");
      status = LsaRegisterLogonProcess (&name, &lsa_hdl, &sec_mode);
    }
  else
    {
      /* Connect untrusted to just create a identification token */
      debug_printf ("Identification requested");
      status = LsaConnectUntrusted (&lsa_hdl);
    }
  if (status != STATUS_SUCCESS)
    {
      debug_printf ("%s: %y", logon ? "LsaRegisterLogonProcess"
				    : "LsaConnectUntrusted", status);
      /* If the privilege is not held, set the proper error code. */
      if (status == STATUS_PORT_CONNECTION_REFUSED)
	status = STATUS_PRIVILEGE_NOT_HELD;
      __seterrno_from_nt_status (status);
      goto out;
    }

  /* Check if this is a domain user.  If so, use Kerberos. */
  kerberos_auth = cygheap->dom.member_machine ()
	      && wcscasecmp (domain, cygheap->dom.account_flat_name ());
  debug_printf ("kerb %d, domain member %d, user domain <%W>, machine <%W>",
		kerberos_auth, cygheap->dom.member_machine (), domain,
		cygheap->dom.account_flat_name ());

  /* Connect to authentication package. */
  RtlInitAnsiString (&name, kerberos_auth ? MICROSOFT_KERBEROS_NAME_A
					  : MSV1_0_PACKAGE_NAME);
  status = LsaLookupAuthenticationPackage (lsa_hdl, &name, &package_id);
  if (status != STATUS_SUCCESS)
    {
      debug_printf ("LsaLookupAuthenticationPackage: %y", status);
      __seterrno_from_nt_status (status);
      goto out;
    }

  /* Create origin. */
  stpcpy (origin.buf, "Cygwin");
  RtlInitAnsiString (&origin.str, origin.buf);

  /* Create token source. */
  memcpy (ts.SourceName, "Cygwin.1", 8);
  ts.SourceIdentifier.HighPart = 0;
  ts.SourceIdentifier.LowPart = kerberos_auth ? 0x0105 : 0x0106;

  if (kerberos_auth)
    {
      PWCHAR sam_name = tp.w_get ();
      PWCHAR upn_name = tp.w_get ();
      size = NT_MAX_PATH;
      KERB_S4U_LOGON *s4u_logon;
      USHORT name_len;

      wcpcpy (wcpcpy (wcpcpy (sam_name, domain), L"\\"), user);
      if (TranslateNameW (sam_name, NameSamCompatible, NameUserPrincipal,
			  upn_name, &size) == 0)
	{
	  PWCHAR translated_name = tp.w_get ();
	  PWCHAR p;

	  debug_printf ("TranslateNameW(%W, NameUserPrincipal) %E", sam_name);
	  size = NT_MAX_PATH;
	  if (TranslateNameW (sam_name, NameSamCompatible, NameCanonical,
			      translated_name, &size) == 0)
	    {
	      debug_printf ("TranslateNameW(%W, NameCanonical) %E", sam_name);
	      goto out;
	    }
	  p = wcschr (translated_name, L'/');
	  if (p)
	    *p = '\0';
	  wcpcpy (wcpcpy (wcpcpy (upn_name, user), L"@"), translated_name);
	}

      name_len = wcslen (upn_name) * sizeof (WCHAR);
      authinf_size = sizeof (KERB_S4U_LOGON) + name_len;
      authinf = tp.c_get ();
      RtlSecureZeroMemory (authinf, authinf_size);
      s4u_logon = (KERB_S4U_LOGON *) authinf;
      s4u_logon->MessageType = KerbS4ULogon;
      s4u_logon->Flags = logon ? 0 : KERB_S4U_LOGON_FLAG_IDENTIFY;
      /* Append user to login info */
      RtlInitEmptyUnicodeString (&s4u_logon->ClientUpn,
				 (PWCHAR) (s4u_logon + 1),
				 name_len);
      RtlAppendUnicodeToString (&s4u_logon->ClientUpn, upn_name);
      debug_printf ("KerbS4ULogon: ClientUpn: <%S>", &s4u_logon->ClientUpn);
    }
  else
    {
      MSV1_0_S4U_LOGON *s4u_logon;
      USHORT user_len, domain_len;

      user_len = wcslen (user) * sizeof (WCHAR);
      domain_len = wcslen (domain) * sizeof (WCHAR);	/* Local machine */
      authinf_size = sizeof (MSV1_0_S4U_LOGON) + user_len + domain_len;
      if (!authinf)
	authinf = tp.c_get ();
      RtlSecureZeroMemory (authinf, authinf_size);
      s4u_logon = (MSV1_0_S4U_LOGON *) authinf;
      s4u_logon->MessageType = MsV1_0S4ULogon;
      s4u_logon->Flags = 0;
      /* Append user and domain to login info */
      RtlInitEmptyUnicodeString (&s4u_logon->UserPrincipalName,
				 (PWCHAR) (s4u_logon + 1),
				 user_len);
      RtlInitEmptyUnicodeString (&s4u_logon->DomainName,
				 (PWCHAR) ((PBYTE) (s4u_logon + 1) + user_len),
				 domain_len);
      RtlAppendUnicodeToString (&s4u_logon->UserPrincipalName, user);
      RtlAppendUnicodeToString (&s4u_logon->DomainName, domain);
      debug_printf ("MsV1_0S4ULogon: DomainName: <%S> UserPrincipalName: <%S>",
		    &s4u_logon->DomainName, &s4u_logon->UserPrincipalName);
    }

  /* Try to logon. */
  status = LsaLogonUser (lsa_hdl, (PLSA_STRING) &origin, Network, package_id,
			 authinf, authinf_size, NULL, &ts, (PVOID *) &profile,
			 &size, &luid, &token, &quota, &sub_status);
  switch (status)
    {
    case STATUS_SUCCESS:
      break;
    case STATUS_ACCOUNT_RESTRICTION:
      debug_printf ("%s S4U LsaLogonUser failed: %y (%s)",
		    kerberos_auth ? "Kerberos" : "MsV1_0", status,
		    account_restriction (sub_status));
      break;
    default:
      debug_printf ("%s S4U LsaLogonUser failed: %y",
		    kerberos_auth ? "Kerberos" : "MsV1_0", status);
      break;
    }

out:
  if (lsa_hdl)
    LsaDeregisterLogonProcess (lsa_hdl);
  if (profile)
    LsaFreeReturnBuffer (profile);

  if (token && logon)
    {
      /* Convert to primary token.  CreateProcessAsUser takes impersonation
	 tokens since Windows 7 but MSDN still claims a primary token is
	 required.  Better safe than sorry. */
      HANDLE tmp_token;

      if (DuplicateTokenEx (token, MAXIMUM_ALLOWED, &sec_none,
			    SecurityImpersonation, TokenPrimary, &tmp_token))
	{
	  CloseHandle (token);
	  token = tmp_token;
	}
      else
	{
	  __seterrno ();
	  debug_printf ("DuplicateTokenEx %E");
	  /* Make sure not to allow create_token. */
	  status = STATUS_INVALID_HANDLE;
	  CloseHandle (token);
	  token = NULL;
	}
    }

  pop_self_privilege ();
  ret_status = status;
  return token;
}
