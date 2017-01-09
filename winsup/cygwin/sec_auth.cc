/* sec_auth.cc: NT authentication functions

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
#include <wininet.h>
#include <userenv.h>
#include "cyglsa.h"
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
  HKEY hkey;
  WCHAR userpath[MAX_PATH];
  PROFILEINFOW pi;
  WCHAR server[INTERNET_MAX_HOST_NAME_LENGTH + 3];
  NET_API_STATUS nas = NERR_UserNotFound;
  PUSER_INFO_3 ui;

  extract_nt_dom_user (pw, domain, username);
  usersid.string (sid);
  debug_printf ("user: <%W> <%W>", username, sid);
  /* Check if user hive is already loaded. */
  if (!RegOpenKeyExW (HKEY_USERS, sid, 0, KEY_READ, &hkey))
    {
      debug_printf ("User registry hive for %W already exists", username);
      RegCloseKey (hkey);
      return NULL;
    }
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
  /* Check if user has a roaming profile and fill in lpProfilePath, if so. */
  if (get_logon_server (domain, server, DS_IS_FLAT_NAME))
    {
      nas = NetUserGetInfo (server, username, 3, (PBYTE *) &ui);
      if (NetUserGetInfo (server, username, 3, (PBYTE *) &ui) != NERR_Success)
	debug_printf ("NetUserGetInfo, %u", nas);
      else if (ui->usri3_profile && *ui->usri3_profile)
	pi.lpProfilePath = ui->usri3_profile;
    }

  if (!LoadUserProfileW (token, &pi))
    debug_printf ("LoadUserProfileW, %E");
  /* Free buffer created by NetUserGetInfo */
  if (nas == NERR_Success)
    NetApiBufferFree (ui);
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
get_user_groups (WCHAR *logonserver, cygsidlist &grp_list,
		 PWCHAR user, PWCHAR domain)
{
  WCHAR dgroup[MAX_DOMAIN_NAME_LEN + GNLEN + 2], *grp_p;
  LPGROUP_USERS_INFO_0 buf;
  DWORD cnt, tot;
  NET_API_STATUS ret;

  /* Look only on logonserver */
  ret = NetUserGetGroups (logonserver, user, 0, (LPBYTE *) &buf,
			  MAX_PREFERRED_LENGTH, &cnt, &tot);
  if (ret)
    {
      __seterrno_from_win_error (ret);
      /* It's no error when the user name can't be found.
	 It's also no error if access has been denied.  Yes, sounds weird, but
	 keep in mind that ERROR_ACCESS_DENIED means the current user has no
	 permission to access the AD user information.  However, if we return
	 an error, Cygwin will call DsGetDcName with DS_FORCE_REDISCOVERY set
	 to ask for another server.  This is not only time consuming, it's also
	 useless; the next server will return access denied again. */
      return ret == NERR_UserNotFound || ret == ERROR_ACCESS_DENIED;
    }

  grp_p = wcpncpy (dgroup, domain, MAX_DOMAIN_NAME_LEN);
  *grp_p++ = L'\\';

  for (DWORD i = 0; i < cnt; ++i)
    {
      cygsid gsid;
      DWORD glen = SECURITY_MAX_SID_SIZE;
      WCHAR dom[MAX_DOMAIN_NAME_LEN + 1];
      DWORD dlen = sizeof (dom);
      SID_NAME_USE use = SidTypeInvalid;

      *wcpncpy (grp_p, buf[i].grui0_name, sizeof dgroup / sizeof *dgroup
					 - (grp_p - dgroup) - 1) = L'\0';
      if (!LookupAccountNameW (NULL, dgroup, gsid, &glen, dom, &dlen, &use))
	debug_printf ("LookupAccountName(%W), %E", dgroup);
      else if (well_known_sid_type (use))
	grp_list *= gsid;
      else if (legal_sid_type (use))
	grp_list += gsid;
      else
	debug_printf ("Global group %W invalid. Use: %u", dgroup, use);
    }

  NetApiBufferFree (buf);
  return true;
}

static bool
get_user_local_groups (PWCHAR logonserver, PWCHAR domain,
		       cygsidlist &grp_list, PWCHAR user)
{
  LPLOCALGROUP_INFO_0 buf;
  DWORD cnt, tot;
  NET_API_STATUS ret;

  ret = NetUserGetLocalGroups (logonserver, user, 0, LG_INCLUDE_INDIRECT,
			       (LPBYTE *) &buf, MAX_PREFERRED_LENGTH,
			       &cnt, &tot);
  if (ret)
    {
      __seterrno_from_win_error (ret);
      return false;
    }

  WCHAR domlocal_grp[MAX_DOMAIN_NAME_LEN + GNLEN + 2];
  WCHAR builtin_grp[2 * GNLEN + 2];
  PWCHAR dg_ptr, bg_ptr = NULL;
  SID_NAME_USE use;

  dg_ptr = wcpcpy (domlocal_grp, domain);
  *dg_ptr++ = L'\\';

  for (DWORD i = 0; i < cnt; ++i)
    {
      cygsid gsid;
      DWORD glen = SECURITY_MAX_SID_SIZE;
      WCHAR dom[MAX_DOMAIN_NAME_LEN + 1];
      DWORD domlen = MAX_DOMAIN_NAME_LEN + 1;

      use = SidTypeInvalid;
      wcscpy (dg_ptr, buf[i].lgrpi0_name);
      if (LookupAccountNameW (NULL, domlocal_grp, gsid, &glen,
			      dom, &domlen, &use))
	{
	  if (well_known_sid_type (use))
	    grp_list *= gsid;
	  else if (legal_sid_type (use))
	    grp_list += gsid;
	  else
	    debug_printf ("Rejecting local %W. use: %u", dg_ptr, use);
	}
      else if (GetLastError () == ERROR_NONE_MAPPED)
	{
	  /* Check if it's a builtin group. */
	  if (!bg_ptr)
	    {
	      /* Retrieve name of builtin group from system since it's
		 localized. */
	      glen = 2 * GNLEN + 2;
	      if (!LookupAccountSidW (NULL, well_known_builtin_sid,
				      builtin_grp, &glen, domain, &domlen, &use))
		debug_printf ("LookupAccountSid(BUILTIN), %E");
	      else
		{
		  bg_ptr = builtin_grp + wcslen (builtin_grp);
		  bg_ptr = wcpcpy (builtin_grp, L"\\");
		}
	    }
	  if (bg_ptr)
	    {
	      wcscpy (bg_ptr, dg_ptr);
	      glen = SECURITY_MAX_SID_SIZE;
	      domlen = MAX_DOMAIN_NAME_LEN + 1;
	      if (LookupAccountNameW (NULL, builtin_grp, gsid, &glen,
				      dom, &domlen, &use))
		{
		  if (!legal_sid_type (use))
		    debug_printf ("Rejecting local %W. use: %u", dg_ptr, use);
		  else
		    grp_list *= gsid;
		}
	      else
		debug_printf ("LookupAccountName(%W), %E", builtin_grp);
	    }
	}
      else
	debug_printf ("LookupAccountName(%W), %E", domlocal_grp);
    }
  NetApiBufferFree (buf);
  return true;
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

static void
get_token_group_sidlist (cygsidlist &grp_list, PTOKEN_GROUPS my_grps)
{
  if (my_grps)
    {
      grp_list += well_known_local_sid;
      if (wincap.has_console_logon_sid ())
	grp_list += well_known_console_logon_sid;
      if (sid_in_token_groups (my_grps, well_known_dialup_sid))
	grp_list *= well_known_dialup_sid;
      if (sid_in_token_groups (my_grps, well_known_network_sid))
	grp_list *= well_known_network_sid;
      if (sid_in_token_groups (my_grps, well_known_batch_sid))
	grp_list *= well_known_batch_sid;
      grp_list *= well_known_interactive_sid;
#if 0
      /* Don't add the SERVICE group when switching the user context.
	 That's much too dangerous, since the service group adds the
	 SE_IMPERSONATE_NAME privilege to the user.  After all, the
	 process started with this token is not the service process
	 anymore anyway. */
      if (sid_in_token_groups (my_grps, well_known_service_sid))
	grp_list *= well_known_service_sid;
#endif
      if (sid_in_token_groups (my_grps, well_known_this_org_sid))
	grp_list *= well_known_this_org_sid;
      grp_list *= well_known_users_sid;
    }
  else
    {
      grp_list += well_known_local_sid;
      grp_list *= well_known_interactive_sid;
      grp_list *= well_known_users_sid;
    }
}

bool
get_server_groups (cygsidlist &grp_list, PSID usersid)
{
  WCHAR user[UNLEN + 1];
  WCHAR domain[MAX_DOMAIN_NAME_LEN + 1];
  WCHAR server[INTERNET_MAX_HOST_NAME_LENGTH + 3];
  DWORD ulen = UNLEN + 1;
  DWORD dlen = MAX_DOMAIN_NAME_LEN + 1;
  SID_NAME_USE use;

  if (well_known_system_sid == usersid)
    {
      grp_list *= well_known_admins_sid;
      return true;
    }

  grp_list *= well_known_world_sid;
  grp_list *= well_known_authenticated_users_sid;

  if (!LookupAccountSidW (NULL, usersid, user, &ulen, domain, &dlen, &use))
    {
      __seterrno ();
      return false;
    }
  /* If the SID does NOT start with S-1-5-21, the domain is some builtin
     domain.  The search for a logon server and fetching group accounts
     is moot. */
  if (sid_id_auth (usersid) == 5 /* SECURITY_NT_AUTHORITY */
      && sid_sub_auth (usersid, 0) == SECURITY_NT_NON_UNIQUE
      && get_logon_server (domain, server, DS_IS_FLAT_NAME))
    {
      get_user_groups (server, grp_list, user, domain);
      get_user_local_groups (server, domain, grp_list, user);
    }
  return true;
}

static bool
get_initgroups_sidlist (cygsidlist &grp_list, PSID usersid, PSID pgrpsid,
			PTOKEN_GROUPS my_grps)
{
  grp_list *= well_known_world_sid;
  grp_list *= well_known_authenticated_users_sid;
  if (well_known_system_sid != usersid)
    get_token_group_sidlist (grp_list, my_grps);
  if (!get_server_groups (grp_list, usersid))
    return false;

  /* special_pgrp true if pgrpsid is not in normal groups */
  grp_list += pgrpsid;
  return true;
}

static void
get_setgroups_sidlist (cygsidlist &tmp_list, PSID usersid,
		       PTOKEN_GROUPS my_grps, user_groups &groups)
{
  tmp_list *= well_known_world_sid;
  tmp_list *= well_known_authenticated_users_sid;
  get_token_group_sidlist (tmp_list, my_grps);
  get_server_groups (tmp_list, usersid);
  for (int gidx = 0; gidx < groups.sgsids.count (); gidx++)
    tmp_list += groups.sgsids.sids[gidx];
  tmp_list += groups.pgsid;
}

/* Fixed size TOKEN_PRIVILEGES list to reflect privileges given to the
   SYSTEM account by default. */
const struct
{
  DWORD PrivilegeCount;
  LUID_AND_ATTRIBUTES Privileges[28];
} sys_privs =
{
  28,
  {
    { { SE_CREATE_TOKEN_PRIVILEGE, 0 },
	SE_PRIVILEGE_ENABLED | SE_PRIVILEGE_ENABLED_BY_DEFAULT },
    { { SE_ASSIGNPRIMARYTOKEN_PRIVILEGE, 0 },
	SE_PRIVILEGE_ENABLED | SE_PRIVILEGE_ENABLED_BY_DEFAULT },
    { { SE_LOCK_MEMORY_PRIVILEGE, 0 },
	SE_PRIVILEGE_ENABLED | SE_PRIVILEGE_ENABLED_BY_DEFAULT },
    { { SE_INCREASE_QUOTA_PRIVILEGE, 0 },
	SE_PRIVILEGE_ENABLED | SE_PRIVILEGE_ENABLED_BY_DEFAULT },
    { { SE_TCB_PRIVILEGE, 0 },
	SE_PRIVILEGE_ENABLED | SE_PRIVILEGE_ENABLED_BY_DEFAULT },
    { { SE_SECURITY_PRIVILEGE, 0 },
	SE_PRIVILEGE_ENABLED | SE_PRIVILEGE_ENABLED_BY_DEFAULT },
    { { SE_TAKE_OWNERSHIP_PRIVILEGE, 0 },
	SE_PRIVILEGE_ENABLED | SE_PRIVILEGE_ENABLED_BY_DEFAULT },
    { { SE_LOAD_DRIVER_PRIVILEGE, 0 },
	SE_PRIVILEGE_ENABLED | SE_PRIVILEGE_ENABLED_BY_DEFAULT },
    { { SE_SYSTEM_PROFILE_PRIVILEGE, 0 },
	SE_PRIVILEGE_ENABLED | SE_PRIVILEGE_ENABLED_BY_DEFAULT },
    { { SE_SYSTEMTIME_PRIVILEGE, 0 },
	SE_PRIVILEGE_ENABLED | SE_PRIVILEGE_ENABLED_BY_DEFAULT },
    { { SE_PROF_SINGLE_PROCESS_PRIVILEGE, 0 },
	SE_PRIVILEGE_ENABLED | SE_PRIVILEGE_ENABLED_BY_DEFAULT },
    { { SE_INC_BASE_PRIORITY_PRIVILEGE, 0 },
	SE_PRIVILEGE_ENABLED | SE_PRIVILEGE_ENABLED_BY_DEFAULT },
    { { SE_CREATE_PAGEFILE_PRIVILEGE, 0 },
	SE_PRIVILEGE_ENABLED | SE_PRIVILEGE_ENABLED_BY_DEFAULT },
    { { SE_CREATE_PERMANENT_PRIVILEGE, 0 },
	SE_PRIVILEGE_ENABLED | SE_PRIVILEGE_ENABLED_BY_DEFAULT },
    { { SE_BACKUP_PRIVILEGE, 0 },
	SE_PRIVILEGE_ENABLED | SE_PRIVILEGE_ENABLED_BY_DEFAULT },
    { { SE_RESTORE_PRIVILEGE, 0 },
	SE_PRIVILEGE_ENABLED | SE_PRIVILEGE_ENABLED_BY_DEFAULT },
    { { SE_SHUTDOWN_PRIVILEGE, 0 },
	SE_PRIVILEGE_ENABLED | SE_PRIVILEGE_ENABLED_BY_DEFAULT },
    { { SE_DEBUG_PRIVILEGE, 0 },
	SE_PRIVILEGE_ENABLED | SE_PRIVILEGE_ENABLED_BY_DEFAULT },
    { { SE_AUDIT_PRIVILEGE, 0 },
	SE_PRIVILEGE_ENABLED | SE_PRIVILEGE_ENABLED_BY_DEFAULT },
    { { SE_SYSTEM_ENVIRONMENT_PRIVILEGE, 0 },
	SE_PRIVILEGE_ENABLED | SE_PRIVILEGE_ENABLED_BY_DEFAULT },
    { { SE_CHANGE_NOTIFY_PRIVILEGE, 0 },
	SE_PRIVILEGE_ENABLED | SE_PRIVILEGE_ENABLED_BY_DEFAULT },
    { { SE_UNDOCK_PRIVILEGE, 0 },
	SE_PRIVILEGE_ENABLED | SE_PRIVILEGE_ENABLED_BY_DEFAULT },
    { { SE_MANAGE_VOLUME_PRIVILEGE, 0 },
	SE_PRIVILEGE_ENABLED | SE_PRIVILEGE_ENABLED_BY_DEFAULT },
    { { SE_IMPERSONATE_PRIVILEGE, 0 },
	SE_PRIVILEGE_ENABLED | SE_PRIVILEGE_ENABLED_BY_DEFAULT },
    { { SE_CREATE_GLOBAL_PRIVILEGE, 0 },
	SE_PRIVILEGE_ENABLED | SE_PRIVILEGE_ENABLED_BY_DEFAULT },
    { { SE_INCREASE_WORKING_SET_PRIVILEGE, 0 },
	SE_PRIVILEGE_ENABLED | SE_PRIVILEGE_ENABLED_BY_DEFAULT },
    { { SE_TIME_ZONE_PRIVILEGE, 0 },
	SE_PRIVILEGE_ENABLED | SE_PRIVILEGE_ENABLED_BY_DEFAULT },
    { { SE_CREATE_SYMBOLIC_LINK_PRIVILEGE, 0 },
	SE_PRIVILEGE_ENABLED | SE_PRIVILEGE_ENABLED_BY_DEFAULT }
  }
};

static PTOKEN_PRIVILEGES
get_priv_list (LSA_HANDLE lsa, cygsid &usersid, cygsidlist &grp_list,
	       size_t &size, cygpsid *mandatory_integrity_sid)
{
  PLSA_UNICODE_STRING privstrs;
  ULONG cnt;
  PTOKEN_PRIVILEGES privs = NULL;

  if (usersid == well_known_system_sid)
    {
      if (mandatory_integrity_sid)
	*mandatory_integrity_sid = mandatory_system_integrity_sid;
      return (PTOKEN_PRIVILEGES) &sys_privs;
    }

  if (mandatory_integrity_sid)
    *mandatory_integrity_sid = mandatory_medium_integrity_sid;

  for (int grp = -1; grp < grp_list.count (); ++grp)
    {
      if (grp == -1)
	{
	  if (LsaEnumerateAccountRights (lsa, usersid, &privstrs, &cnt)
	      != STATUS_SUCCESS)
	    continue;
	}
      else if (LsaEnumerateAccountRights (lsa, grp_list.sids[grp],
					  &privstrs, &cnt) != STATUS_SUCCESS)
	continue;
      for (ULONG i = 0; i < cnt; ++i)
	{
	  LUID priv;
	  PTOKEN_PRIVILEGES tmp;
	  DWORD tmp_count;
	  bool high_integrity;

	  if (!privilege_luid (privstrs[i].Buffer, priv, high_integrity))
	    continue;

	  if (privs)
	    {
	      DWORD pcnt = privs->PrivilegeCount;
	      LUID_AND_ATTRIBUTES *p = privs->Privileges;
	      for (; pcnt > 0; --pcnt, ++p)
		if (priv.HighPart == p->Luid.HighPart
		    && priv.LowPart == p->Luid.LowPart)
		  goto next_account_right;
	    }

	  tmp_count = privs ? privs->PrivilegeCount : 0;
	  size = sizeof (DWORD)
		 + (tmp_count + 1) * sizeof (LUID_AND_ATTRIBUTES);
	  tmp = (PTOKEN_PRIVILEGES) realloc (privs, size);
	  if (!tmp)
	    {
	      if (privs)
		free (privs);
	      LsaFreeMemory (privstrs);
	      debug_printf ("realloc (privs) failed.");
	      return NULL;
	    }
	  tmp->PrivilegeCount = tmp_count;
	  privs = tmp;
	  privs->Privileges[privs->PrivilegeCount].Luid = priv;
	  privs->Privileges[privs->PrivilegeCount].Attributes =
	    SE_PRIVILEGE_ENABLED | SE_PRIVILEGE_ENABLED_BY_DEFAULT;
	  ++privs->PrivilegeCount;
	  if (mandatory_integrity_sid && high_integrity)
	    *mandatory_integrity_sid = mandatory_high_integrity_sid;

	next_account_right:
	  ;
	}
      LsaFreeMemory (privstrs);
    }
  return privs;
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
	except if the token is internal and the group is in the token SD
	(see create_token). In that latter case that group must match the
	requested primary group.  */
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

HANDLE
create_token (cygsid &usersid, user_groups &new_groups)
{
  NTSTATUS status;
  LSA_HANDLE lsa = NULL;

  cygsidlist tmp_gsids (cygsidlist_auto, 12);

  SECURITY_QUALITY_OF_SERVICE sqos =
    { sizeof sqos, SecurityImpersonation, SECURITY_STATIC_TRACKING, FALSE };
  OBJECT_ATTRIBUTES oa = { sizeof oa, 0, 0, 0, 0, &sqos };
  /* Up to Windows 7, when using a authwentication LUID other than "Anonymous",
     Windows whoami prints the wrong username, the one from the login session,
     not the one from the actual user token of the process.  This is apparently
     fixed in Windows 8.  However, starting with Windows 8, access rights of
     the anonymous logon session is further restricted.  Therefore we create
     the new user token with the authentication id of the local service
     account.  Hopefully that's sufficient. */
  const LUID auth_luid_7 = ANONYMOUS_LOGON_LUID;
  const LUID auth_luid_8 = LOCALSERVICE_LUID;
  LUID auth_luid = wincap.has_broken_whoami () ? auth_luid_7 : auth_luid_8;
  LARGE_INTEGER exp = { QuadPart:INT64_MAX };

  TOKEN_USER user;
  PTOKEN_GROUPS new_tok_gsids = NULL;
  PTOKEN_PRIVILEGES privs = NULL;
  TOKEN_OWNER owner;
  TOKEN_PRIMARY_GROUP pgrp;
  TOKEN_DEFAULT_DACL dacl = {};
  TOKEN_SOURCE source;
  TOKEN_STATISTICS stats;
  memcpy (source.SourceName, "Cygwin.1", 8);
  source.SourceIdentifier.HighPart = 0;
  source.SourceIdentifier.LowPart = 0x0101;

  HANDLE token = INVALID_HANDLE_VALUE;
  HANDLE primary_token = INVALID_HANDLE_VALUE;

  tmp_pathbuf tp;
  PTOKEN_GROUPS my_tok_gsids = NULL;
  cygpsid mandatory_integrity_sid;
  ULONG size;
  size_t psize = 0;

  /* SE_CREATE_TOKEN_NAME privilege needed to call NtCreateToken. */
  push_self_privilege (SE_CREATE_TOKEN_PRIVILEGE, true);

  /* Open policy object. */
  if (!(lsa = lsa_open_policy (NULL, POLICY_EXECUTE)))
    goto out;

  /* User, owner, primary group. */
  user.User.Sid = usersid;
  user.User.Attributes = 0;
  owner.Owner = usersid;

  /* Retrieve authentication id and group list from own process. */
  if (hProcToken)
    {
      /* Switching user context to SYSTEM doesn't inherit the authentication
	 id of the user account running current process. */
      if (usersid == well_known_system_sid)
	/* nothing to do */;
      else
	{
	  status = NtQueryInformationToken (hProcToken, TokenStatistics,
					    &stats, sizeof stats, &size);
	  if (!NT_SUCCESS (status))
	    debug_printf ("NtQueryInformationToken(hProcToken, "
			  "TokenStatistics), %y", status);
	}

      /* Retrieving current processes group list to be able to inherit
	 some important well known group sids. */
      my_tok_gsids = (PTOKEN_GROUPS) tp.w_get ();
      status = NtQueryInformationToken (hProcToken, TokenGroups, my_tok_gsids,
					2 * NT_MAX_PATH, &size);
      if (!NT_SUCCESS (status))
	{
	  debug_printf ("NtQueryInformationToken(hProcToken, TokenGroups), "
			"%y", status);
	  my_tok_gsids = NULL;
	}
    }

  /* Create list of groups, the user is member in. */
  if (new_groups.issetgroups ())
    get_setgroups_sidlist (tmp_gsids, usersid, my_tok_gsids, new_groups);
  else if (!get_initgroups_sidlist (tmp_gsids, usersid, new_groups.pgsid,
				    my_tok_gsids))
    goto out;

  /* Primary group. */
  pgrp.PrimaryGroup = new_groups.pgsid;

  /* Create a TOKEN_GROUPS list from the above retrieved list of sids. */
  new_tok_gsids = (PTOKEN_GROUPS)
		  alloca (sizeof (DWORD) + (tmp_gsids.count () + 1)
					   * sizeof (SID_AND_ATTRIBUTES));
  new_tok_gsids->GroupCount = tmp_gsids.count ();
  for (DWORD i = 0; i < new_tok_gsids->GroupCount; ++i)
    {
      new_tok_gsids->Groups[i].Sid = tmp_gsids.sids[i];
      new_tok_gsids->Groups[i].Attributes = SE_GROUP_MANDATORY
					    | SE_GROUP_ENABLED_BY_DEFAULT
					    | SE_GROUP_ENABLED;
    }

  /* Retrieve list of privileges of that user.  Based on the usersid and
     the returned privileges, get_priv_list sets the mandatory_integrity_sid
     pointer to the correct MIC SID for UAC. */
  if (!(privs = get_priv_list (lsa, usersid, tmp_gsids, psize,
			       &mandatory_integrity_sid)))
    goto out;

  new_tok_gsids->Groups[new_tok_gsids->GroupCount].Attributes =
    SE_GROUP_INTEGRITY | SE_GROUP_INTEGRITY_ENABLED;
  new_tok_gsids->Groups[new_tok_gsids->GroupCount++].Sid
    = mandatory_integrity_sid;

  /* Let's be heroic... */
  status = NtCreateToken (&token, TOKEN_ALL_ACCESS, &oa, TokenImpersonation,
			  &auth_luid, &exp, &user, new_tok_gsids, privs, &owner,
			  &pgrp, &dacl, &source);
  if (status)
    __seterrno_from_nt_status (status);
  else
    {
      /* Convert to primary token. */
      if (!DuplicateTokenEx (token, MAXIMUM_ALLOWED, &sec_none,
			     SecurityImpersonation, TokenPrimary,
			     &primary_token))
	{
	  __seterrno ();
	  debug_printf ("DuplicateTokenEx %E");
	}
    }

out:
  pop_self_privilege ();
  if (token != INVALID_HANDLE_VALUE)
    CloseHandle (token);
  if (privs && privs != (PTOKEN_PRIVILEGES) &sys_privs)
    free (privs);
  lsa_close_policy (lsa);

  debug_printf ("%p = create_token ()", primary_token);
  return primary_token;
}

HANDLE
lsaauth (cygsid &usersid, user_groups &new_groups)
{
  cygsidlist tmp_gsids (cygsidlist_auto, 12);
  cygpsid pgrpsid;
  LSA_STRING name;
  HANDLE lsa_hdl = NULL, lsa = NULL;
  LSA_OPERATIONAL_MODE sec_mode;
  NTSTATUS status, sub_status;
  ULONG package_id, size;
  struct {
    LSA_STRING str;
    CHAR buf[16];
  } origin;
  DWORD ulen = UNLEN + 1;
  DWORD dlen = MAX_DOMAIN_NAME_LEN + 1;
  SID_NAME_USE use;
  cyglsa_t *authinf = NULL;
  ULONG authinf_size;
  TOKEN_SOURCE ts;
  PCYG_TOKEN_GROUPS gsids = NULL;
  PTOKEN_PRIVILEGES privs = NULL;
  PACL dacl = NULL;
  PVOID profile = NULL;
  LUID luid;
  QUOTA_LIMITS quota;
  size_t psize = 0, gsize = 0, dsize = 0;
  OFFSET offset, sids_offset;
  int tmpidx, non_well_known_cnt;

  HANDLE user_token = NULL;

  push_self_privilege (SE_TCB_PRIVILEGE, true);

  /* Register as logon process. */
  RtlInitAnsiString (&name, "Cygwin");
  SetLastError (0);
  status = LsaRegisterLogonProcess (&name, &lsa_hdl, &sec_mode);
  if (status != STATUS_SUCCESS)
    {
      debug_printf ("LsaRegisterLogonProcess: %y", status);
      __seterrno_from_nt_status (status);
      goto out;
    }
  else if (GetLastError () == ERROR_PROC_NOT_FOUND)
    {
      debug_printf ("Couldn't load Secur32.dll");
      goto out;
    }
  /* Get handle to our own LSA package. */
  RtlInitAnsiString (&name, CYG_LSA_PKGNAME);
  status = LsaLookupAuthenticationPackage (lsa_hdl, &name, &package_id);
  if (status != STATUS_SUCCESS)
    {
      debug_printf ("LsaLookupAuthenticationPackage: %y", status);
      __seterrno_from_nt_status (status);
      goto out;
    }

  /* Open policy object. */
  if (!(lsa = lsa_open_policy (NULL, POLICY_EXECUTE)))
    goto out;

  /* Create origin. */
  stpcpy (origin.buf, "Cygwin");
  RtlInitAnsiString (&origin.str, origin.buf);
  /* Create token source. */
  memcpy (ts.SourceName, "Cygwin.1", 8);
  ts.SourceIdentifier.HighPart = 0;
  ts.SourceIdentifier.LowPart = 0x0103;

  /* Create list of groups, the user is member in. */
  if (new_groups.issetgroups ())
    get_setgroups_sidlist (tmp_gsids, usersid, NULL, new_groups);
  else if (!get_initgroups_sidlist (tmp_gsids, usersid, new_groups.pgsid,
				    NULL))
    goto out;

  tmp_gsids.debug_print ("tmp_gsids");

  /* Evaluate size of TOKEN_GROUPS list */
  non_well_known_cnt =  tmp_gsids.non_well_known_count ();
  gsize = sizeof (DWORD) + non_well_known_cnt * sizeof (SID_AND_ATTRIBUTES);
  tmpidx = -1;
  for (int i = 0; i < non_well_known_cnt; ++i)
    if ((tmpidx = tmp_gsids.next_non_well_known_sid (tmpidx)) >= 0)
      gsize += RtlLengthSid (tmp_gsids.sids[tmpidx]);

  /* Retrieve list of privileges of that user.  The MIC SID is created by
     the LSA here. */
  if (!(privs = get_priv_list (lsa, usersid, tmp_gsids, psize, NULL)))
    goto out;

  /* Create DefaultDacl. */
  dsize = sizeof (ACL) + 3 * sizeof (ACCESS_ALLOWED_ACE)
	  + RtlLengthSid (usersid)
	  + RtlLengthSid (well_known_admins_sid)
	  + RtlLengthSid (well_known_system_sid);
  dacl = (PACL) alloca (dsize);
  if (!NT_SUCCESS (RtlCreateAcl (dacl, dsize, ACL_REVISION)))
    goto out;
  if (!NT_SUCCESS (RtlAddAccessAllowedAce (dacl, ACL_REVISION, GENERIC_ALL,
					   usersid)))
    goto out;
  if (!NT_SUCCESS (RtlAddAccessAllowedAce (dacl, ACL_REVISION, GENERIC_ALL,
					   well_known_admins_sid)))
    goto out;
  if (!NT_SUCCESS (RtlAddAccessAllowedAce (dacl, ACL_REVISION, GENERIC_ALL,
					   well_known_system_sid)))
    goto out;

  /* Evaluate authinf size and allocate authinf. */
  authinf_size = (authinf->data - (PBYTE) authinf);
  authinf_size += RtlLengthSid (usersid);	    /* User SID */
  authinf_size += gsize;			    /* Groups + Group SIDs */
  /* When trying to define the admins group as primary group on Vista,
     LsaLogonUser fails with error STATUS_INVALID_OWNER.  As workaround
     we define "Local" as primary group here.  Seteuid32 sets the primary
     group to the group set in /etc/passwd anyway. */
  if (new_groups.pgsid == well_known_admins_sid)
    pgrpsid = well_known_local_sid;
  else
    pgrpsid = new_groups.pgsid;

  authinf_size += RtlLengthSid (pgrpsid);	    /* Primary Group SID */

  authinf_size += psize;			    /* Privileges */
  authinf_size += 0;				    /* Owner SID */
  authinf_size += dsize;			    /* Default DACL */

  authinf = (cyglsa_t *) alloca (authinf_size);
  authinf->inf_size = authinf_size - ((PBYTE) &authinf->inf - (PBYTE) authinf);

  authinf->magic = CYG_LSA_MAGIC;

  if (!LookupAccountSidW (NULL, usersid, authinf->username, &ulen,
			  authinf->domain, &dlen, &use))
    {
      __seterrno ();
      goto out;
    }

  /* Store stuff in authinf with offset relative to start of "inf" member,
     instead of using pointers. */
  offset = authinf->data - (PBYTE) &authinf->inf;

  authinf->inf.ExpirationTime.LowPart = 0xffffffffL;
  authinf->inf.ExpirationTime.HighPart = 0x7fffffffL;
  /* User SID */
  authinf->inf.User.User.Sid = offset;
  authinf->inf.User.User.Attributes = 0;
  RtlCopySid (RtlLengthSid (usersid), (PSID) ((PBYTE) &authinf->inf + offset),
	      usersid);
  offset += RtlLengthSid (usersid);
  /* Groups */
  authinf->inf.Groups = offset;
  gsids = (PCYG_TOKEN_GROUPS) ((PBYTE) &authinf->inf + offset);
  sids_offset = offset + sizeof (ULONG) + non_well_known_cnt
					  * sizeof (SID_AND_ATTRIBUTES);
  gsids->GroupCount = non_well_known_cnt;
  /* Group SIDs */
  tmpidx = -1;
  for (int i = 0; i < non_well_known_cnt; ++i)
    {
      if ((tmpidx = tmp_gsids.next_non_well_known_sid (tmpidx)) < 0)
	break;
      gsids->Groups[i].Sid = sids_offset;
      gsids->Groups[i].Attributes = SE_GROUP_MANDATORY
				    | SE_GROUP_ENABLED_BY_DEFAULT
				    | SE_GROUP_ENABLED;
      RtlCopySid (RtlLengthSid (tmp_gsids.sids[tmpidx]),
		  (PSID) ((PBYTE) &authinf->inf + sids_offset),
		  tmp_gsids.sids[tmpidx]);
      sids_offset += RtlLengthSid (tmp_gsids.sids[tmpidx]);
    }
  offset += gsize;
  /* Primary Group SID */
  authinf->inf.PrimaryGroup.PrimaryGroup = offset;
  RtlCopySid (RtlLengthSid (pgrpsid), (PSID) ((PBYTE) &authinf->inf + offset),
	      pgrpsid);
  offset += RtlLengthSid (pgrpsid);
  /* Privileges */
  authinf->inf.Privileges = offset;
  memcpy ((PBYTE) &authinf->inf + offset, privs, psize);
  offset += psize;
  /* Owner */
  authinf->inf.Owner.Owner = 0;
  /* Default DACL */
  authinf->inf.DefaultDacl.DefaultDacl = offset;
  memcpy ((PBYTE) &authinf->inf + offset, dacl, dsize);

  authinf->checksum = CYG_LSA_MAGIC;
  PDWORD csp;
  PDWORD csp_end;
  csp = (PDWORD) &authinf->username;
  csp_end = (PDWORD) ((PBYTE) authinf + authinf_size);
  while (csp < csp_end)
    authinf->checksum += *csp++;

  /* Try to logon... */
  status = LsaLogonUser (lsa_hdl, (PLSA_STRING) &origin, Interactive,
			 package_id, authinf, authinf_size, NULL, &ts,
			 &profile, &size, &luid, &user_token, &quota,
			 &sub_status);
  if (status != STATUS_SUCCESS)
    {
      debug_printf ("LsaLogonUser: %y (sub-status %y)", status, sub_status);
      __seterrno_from_nt_status (status);
      goto out;
    }
  if (profile)
    {
#ifdef JUST_ANOTHER_NONWORKING_SOLUTION
      /* See ../lsaauth/cyglsa.c. */
      cygprf_t *prf = (cygprf_t *) profile;
      if (prf->magic_pre == MAGIC_PRE && prf->magic_post == MAGIC_POST
	  && prf->token)
	{
	  CloseHandle (user_token);
	  user_token = prf->token;
	  system_printf ("Got token through profile: %p", user_token);
	}
#endif /* JUST_ANOTHER_NONWORKING_SOLUTION */
      LsaFreeReturnBuffer (profile);
    }
  user_token = get_full_privileged_inheritable_token (user_token);

out:
  if (privs && privs != (PTOKEN_PRIVILEGES) &sys_privs)
    free (privs);
  lsa_close_policy (lsa);
  if (lsa_hdl)
    LsaDeregisterLogonProcess (lsa_hdl);
  pop_self_privilege ();

  debug_printf ("%p = lsaauth ()", user_token);
  return user_token;
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
