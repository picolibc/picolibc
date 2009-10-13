/* sec_auth.cc: NT authentication functions

   Copyright 1997, 1998, 1999, 2000, 2001, 2002, 2003, 2004, 2005,
   2006, 2007, 2008, 2009 Red Hat, Inc.

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

#include "winsup.h"
#include <stdlib.h>
#include <wchar.h>
#include <wininet.h>
#include <ntsecapi.h>
#include <dsgetdc.h>
#include "cygerrno.h"
#include "security.h"
#include "path.h"
#include "fhandler.h"
#include "dtable.h"
#include "cygheap.h"
#include "ntdll.h"
#include "tls_pbuf.h"
#include <lm.h>
#include <iptypes.h>
#include "pwdgrp.h"
#include "cyglsa.h"
#include "cygserver_setpwd.h"
#include <cygwin/version.h>

void
set_imp_token (HANDLE token, int type)
{
  debug_printf ("set_imp_token (%d, %d)", token, type);
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

  debug_printf ("pw_gecos %x (%s)", pw->pw_gecos, pw->pw_gecos);

  if (psid.getfrompw (pw)
      && LookupAccountSidW (NULL, psid, user, &ulen, domain, &dlen, &use))
    return;

  char *d, *u, *c;
  domain[0] = L'\0';
  sys_mbstowcs (user, UNLEN + 1, pw->pw_name);
  if ((d = strstr (pw->pw_gecos, "U-")) != NULL &&
      (d == pw->pw_gecos || d[-1] == ','))
    {
      c = strechr (d + 2, ',');
      if ((u = strechr (d + 2, '\\')) >= c)
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
  else if (!SetHandleInformation (hToken,
				  HANDLE_FLAG_INHERIT,
				  HANDLE_FLAG_INHERIT))
    {
      __seterrno ();
      CloseHandle (hToken);
      hToken = INVALID_HANDLE_VALUE;
    }
  cygheap->user.reimpersonate ();
  debug_printf ("%d = logon_user(%s,...)", hToken, pw->pw_name);
  return hToken;
}

static void
str2lsa (LSA_STRING &tgt, const char *srcstr)
{
  tgt.Length = strlen (srcstr);
  tgt.MaximumLength = tgt.Length + 1;
  tgt.Buffer = (PCHAR) srcstr;
}

static void
str2buf2lsa (LSA_STRING &tgt, char *buf, const char *srcstr)
{
  tgt.Length = strlen (srcstr);
  tgt.MaximumLength = tgt.Length + 1;
  tgt.Buffer = (PCHAR) buf;
  memcpy (buf, srcstr, tgt.MaximumLength);
}

HANDLE
open_local_policy (ACCESS_MASK access)
{
  LSA_OBJECT_ATTRIBUTES oa = { 0, 0, 0, 0, 0, 0 };
  HANDLE lsa = INVALID_HANDLE_VALUE;

  NTSTATUS ret = LsaOpenPolicy (NULL, &oa, access, &lsa);
  if (ret != STATUS_SUCCESS)
    {
      __seterrno_from_win_error (LsaNtStatusToWinError (ret));
      /* Some versions of Windows set the lsa handle to NULL when
         LsaOpenPolicy fails. */
      lsa = INVALID_HANDLE_VALUE;
    }
  return lsa;
}

static void
close_local_policy (LSA_HANDLE &lsa)
{
  if (lsa != INVALID_HANDLE_VALUE)
    LsaClose (lsa);
  lsa = INVALID_HANDLE_VALUE;
}

bool
get_logon_server (PWCHAR domain, WCHAR *server, bool rediscovery)
{
  DWORD dret;
  PDOMAIN_CONTROLLER_INFOW pci;
  WCHAR *buf;
  DWORD size = MAX_COMPUTERNAME_LENGTH + 1;

  /* Empty domain is interpreted as local system */
  if ((GetComputerNameW (server + 2, &size)) &&
      (!wcscasecmp (domain, server + 2) || !domain[0]))
    {
      server[0] = server[1] = L'\\';
      return true;
    }

  /* Try to get any available domain controller for this domain */
  dret = DsGetDcNameW (NULL, domain, NULL, NULL,
		       rediscovery ? DS_FORCE_REDISCOVERY : 0, &pci);
  if (dret == ERROR_SUCCESS)
    {
      wcscpy (server, pci->DomainControllerName);
      NetApiBufferFree (pci);
      debug_printf ("DC: rediscovery: %d, server: %W", rediscovery, server);
      return true;
    }
  else if (dret == ERROR_PROC_NOT_FOUND)
    {
      /* NT4 w/o DSClient */
      if (rediscovery)
	dret = NetGetAnyDCName (NULL, domain, (LPBYTE *) &buf);
      else
	dret = NetGetDCName (NULL, domain, (LPBYTE *) &buf);
      if (dret == NERR_Success)
	{
	  wcscpy (server, buf);
	  NetApiBufferFree (buf);
	  debug_printf ("NT: rediscovery: %d, server: %W", rediscovery, server);
	  return true;
	}
    }
  __seterrno_from_win_error (dret);
  return false;
}

static bool
get_user_groups (WCHAR *logonserver, cygsidlist &grp_list,
		 PWCHAR user, PWCHAR domain)
{
  WCHAR dgroup[MAX_DOMAIN_NAME_LEN + GNLEN + 2];
  LPGROUP_USERS_INFO_0 buf;
  DWORD cnt, tot, len;
  NET_API_STATUS ret;

  /* Look only on logonserver */
  ret = NetUserGetGroups (logonserver, user, 0, (LPBYTE *) &buf,
			  MAX_PREFERRED_LENGTH, &cnt, &tot);
  if (ret)
    {
      __seterrno_from_win_error (ret);
      /* It's no error when the user name can't be found. */
      return ret == NERR_UserNotFound;
    }

  len = wcslen (domain);
  wcscpy (dgroup, domain);
  dgroup[len++] = L'\\';

  for (DWORD i = 0; i < cnt; ++i)
    {
      cygsid gsid;
      DWORD glen = MAX_SID_LEN;
      WCHAR dom[MAX_DOMAIN_NAME_LEN + 1];
      DWORD dlen = sizeof (dom);
      SID_NAME_USE use = SidTypeInvalid;

      wcscpy (dgroup + len, buf[i].grui0_name);
      if (!LookupAccountNameW (NULL, dgroup, gsid, &glen, dom, &dlen, &use))
	debug_printf ("LookupAccountName(%W), %E", dgroup);
      else if (legal_sid_type (use))
	grp_list += gsid;
      else
	debug_printf ("Global group %W invalid. Use: %d", dgroup, use);
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
  WCHAR builtin_grp[sizeof ("BUILTIN\\") + GNLEN + 2];
  PWCHAR dg_ptr, bg_ptr;
  SID_NAME_USE use;

  dg_ptr = wcpcpy (domlocal_grp, domain);
  *dg_ptr++ = L'\\';
  bg_ptr = wcpcpy (builtin_grp, L"BUILTIN\\");

  for (DWORD i = 0; i < cnt; ++i)
    {
      cygsid gsid;
      DWORD glen = MAX_SID_LEN;
      WCHAR dom[MAX_DOMAIN_NAME_LEN + 1];
      DWORD domlen = sizeof (dom);

      use = SidTypeInvalid;
      wcscpy (dg_ptr, buf[i].lgrpi0_name);
      if (LookupAccountNameW (NULL, domlocal_grp, gsid, &glen,
			      dom, &domlen, &use))
	{
	  if (!legal_sid_type (use))
	    debug_printf ("Rejecting local %W. use: %d", dg_ptr, use);
	  else
	    grp_list += gsid;
	}
      else if (GetLastError () == ERROR_NONE_MAPPED)
	{
	  /* Check if it's a builtin group. */
	  wcscpy (bg_ptr, dg_ptr);
	  if (LookupAccountNameW (NULL, builtin_grp, gsid, &glen,
				  dom, &domlen, &use))
	    {
	      if (!legal_sid_type (use))
		debug_printf ("Rejecting local %W. use: %d", dg_ptr, use);
	      else
		grp_list *= gsid;
	    }
	  else
	    debug_printf ("LookupAccountName(%W), %E", builtin_grp);
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
get_unix_group_sidlist (struct passwd *pw, cygsidlist &grp_list)
{
  struct __group32 *gr;
  cygsid gsid;

  for (int gidx = 0; (gr = internal_getgrent (gidx)); ++gidx)
    {
      if (gr->gr_gid == (__gid32_t) pw->pw_gid)
	goto found;
      else if (gr->gr_mem)
	for (int gi = 0; gr->gr_mem[gi]; ++gi)
	  if (strcasematch (pw->pw_name, gr->gr_mem[gi]))
	    goto found;
      continue;
    found:
      if (gsid.getfromgr (gr))
	grp_list += gsid;

    }
}

static void
get_token_group_sidlist (cygsidlist &grp_list, PTOKEN_GROUPS my_grps,
			 LUID auth_luid, int &auth_pos)
{
  auth_pos = -1;
  if (my_grps)
    {
      grp_list += well_known_local_sid;
      if (sid_in_token_groups (my_grps, well_known_dialup_sid))
	grp_list *= well_known_dialup_sid;
      if (sid_in_token_groups (my_grps, well_known_network_sid))
	grp_list *= well_known_network_sid;
      if (sid_in_token_groups (my_grps, well_known_batch_sid))
	grp_list *= well_known_batch_sid;
      grp_list *= well_known_interactive_sid;
      if (sid_in_token_groups (my_grps, well_known_service_sid))
	grp_list *= well_known_service_sid;
      if (sid_in_token_groups (my_grps, well_known_this_org_sid))
	grp_list *= well_known_this_org_sid;
    }
  else
    {
      grp_list += well_known_local_sid;
      grp_list *= well_known_interactive_sid;
    }
  if (get_ll (auth_luid) != 999LL) /* != SYSTEM_LUID */
    {
      for (DWORD i = 0; i < my_grps->GroupCount; ++i)
	if (my_grps->Groups[i].Attributes & SE_GROUP_LOGON_ID)
	  {
	    grp_list += my_grps->Groups[i].Sid;
	    auth_pos = grp_list.count () - 1;
	    break;
	  }
    }
}

bool
get_server_groups (cygsidlist &grp_list, PSID usersid, struct passwd *pw)
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
      get_unix_group_sidlist (pw, grp_list);
      return true;
    }

  grp_list *= well_known_world_sid;
  grp_list *= well_known_authenticated_users_sid;

  if (!LookupAccountSidW (NULL, usersid, user, &ulen, domain, &dlen, &use))
    {
      __seterrno ();
      return false;
    }
  if (get_logon_server (domain, server, false)
      && !get_user_groups (server, grp_list, user, domain)
      && get_logon_server (domain, server, true))
    get_user_groups (server, grp_list, user, domain);
  get_user_local_groups (server, domain, grp_list, user);
  get_unix_group_sidlist (pw, grp_list);
  return true;
}

static bool
get_initgroups_sidlist (cygsidlist &grp_list,
			PSID usersid, PSID pgrpsid, struct passwd *pw,
			PTOKEN_GROUPS my_grps, LUID auth_luid, int &auth_pos)
{
  grp_list *= well_known_world_sid;
  grp_list *= well_known_authenticated_users_sid;
  if (well_known_system_sid == usersid)
    auth_pos = -1;
  else
    get_token_group_sidlist (grp_list, my_grps, auth_luid, auth_pos);
  if (!get_server_groups (grp_list, usersid, pw))
    return false;

  /* special_pgrp true if pgrpsid is not in normal groups */
  grp_list += pgrpsid;
  return true;
}

static void
get_setgroups_sidlist (cygsidlist &tmp_list, PSID usersid, struct passwd *pw,
		       PTOKEN_GROUPS my_grps, user_groups &groups,
		       LUID auth_luid, int &auth_pos)
{
  tmp_list *= well_known_world_sid;
  tmp_list *= well_known_authenticated_users_sid;
  get_token_group_sidlist (tmp_list, my_grps, auth_luid, auth_pos);
  get_server_groups (tmp_list, usersid, pw);
  for (int gidx = 0; gidx < groups.sgsids.count (); gidx++)
    tmp_list += groups.sgsids.sids[gidx];
  tmp_list += groups.pgsid;
}

static ULONG sys_privs[] = {
  SE_CREATE_TOKEN_PRIVILEGE,
  SE_ASSIGNPRIMARYTOKEN_PRIVILEGE,
  SE_LOCK_MEMORY_PRIVILEGE,
  SE_INCREASE_QUOTA_PRIVILEGE,
  SE_TCB_PRIVILEGE,
  SE_SECURITY_PRIVILEGE,
  SE_TAKE_OWNERSHIP_PRIVILEGE,
  SE_LOAD_DRIVER_PRIVILEGE,
  SE_SYSTEM_PROFILE_PRIVILEGE,		/* Vista ONLY */
  SE_SYSTEMTIME_PRIVILEGE,
  SE_PROF_SINGLE_PROCESS_PRIVILEGE,
  SE_INC_BASE_PRIORITY_PRIVILEGE,
  SE_CREATE_PAGEFILE_PRIVILEGE,
  SE_CREATE_PERMANENT_PRIVILEGE,
  SE_BACKUP_PRIVILEGE,
  SE_RESTORE_PRIVILEGE,
  SE_SHUTDOWN_PRIVILEGE,
  SE_DEBUG_PRIVILEGE,
  SE_AUDIT_PRIVILEGE,
  SE_SYSTEM_ENVIRONMENT_PRIVILEGE,
  SE_CHANGE_NOTIFY_PRIVILEGE,
  SE_UNDOCK_PRIVILEGE,
  SE_MANAGE_VOLUME_PRIVILEGE,
  SE_IMPERSONATE_PRIVILEGE,
  SE_CREATE_GLOBAL_PRIVILEGE,
  SE_INCREASE_WORKING_SET_PRIVILEGE,
  SE_TIME_ZONE_PRIVILEGE,
  SE_CREATE_SYMBOLIC_LINK_PRIVILEGE
};

#define SYSTEM_PRIVILEGES_COUNT (sizeof sys_privs / sizeof *sys_privs)

static PTOKEN_PRIVILEGES
get_system_priv_list (size_t &size)
{
  ULONG max_idx = 0;
  while (max_idx < SYSTEM_PRIVILEGES_COUNT
  	 && sys_privs[max_idx] != wincap.max_sys_priv ())
    ++max_idx;
  if (max_idx >= SYSTEM_PRIVILEGES_COUNT)
    api_fatal ("Coding error: wincap privilege %u doesn't exist in sys_privs",
	       wincap.max_sys_priv ());
  size = sizeof (ULONG) + (max_idx + 1) * sizeof (LUID_AND_ATTRIBUTES);
  PTOKEN_PRIVILEGES privs = (PTOKEN_PRIVILEGES) malloc (size);
  if (!privs)
    {
      debug_printf ("malloc (system_privs) failed.");
      return NULL;
    }
  privs->PrivilegeCount = 0;
  for (ULONG i = 0; i <= max_idx; ++i)
    {
      privs->Privileges[privs->PrivilegeCount].Luid.HighPart = 0L;
      privs->Privileges[privs->PrivilegeCount].Luid.LowPart = sys_privs[i];
      privs->Privileges[privs->PrivilegeCount].Attributes =
	SE_PRIVILEGE_ENABLED | SE_PRIVILEGE_ENABLED_BY_DEFAULT;
      ++privs->PrivilegeCount;
    }
  return privs;
}

static PTOKEN_PRIVILEGES
get_priv_list (LSA_HANDLE lsa, cygsid &usersid, cygsidlist &grp_list,
	       size_t &size)
{
  PLSA_UNICODE_STRING privstrs;
  ULONG cnt;
  PTOKEN_PRIVILEGES privs = NULL;
  NTSTATUS ret;

  if (usersid == well_known_system_sid)
    return get_system_priv_list (size);

  for (int grp = -1; grp < grp_list.count (); ++grp)
    {
      if (grp == -1)
	{
	  if ((ret = LsaEnumerateAccountRights (lsa, usersid, &privstrs,
						&cnt)) != STATUS_SUCCESS)
	    continue;
	}
      else if ((ret = LsaEnumerateAccountRights (lsa, grp_list.sids[grp],
						 &privstrs, &cnt))
	       != STATUS_SUCCESS)
	continue;
      for (ULONG i = 0; i < cnt; ++i)
	{
	  LUID priv;
	  PTOKEN_PRIVILEGES tmp;
	  DWORD tmp_count;

	  if (!privilege_luid (privstrs[i].Buffer, &priv))
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
  DWORD size;
  bool intern = false;

  if (pintern)
    {
      TOKEN_SOURCE ts;
      if (!GetTokenInformation (token, TokenSource,
				&ts, sizeof ts, &size))
	debug_printf ("GetTokenInformation(), %E");
      else
	*pintern = intern = !memcmp (ts.SourceName, "Cygwin.1", 8);
    }
  /* Verify usersid */
  cygsid tok_usersid = NO_SID;
  if (!GetTokenInformation (token, TokenUser,
			    &tok_usersid, sizeof tok_usersid, &size))
    debug_printf ("GetTokenInformation(), %E");
  if (usersid != tok_usersid)
    return false;

  /* For an internal token, if setgroups was not called and if the sd group
     is not well_known_null_sid, it must match pgrpsid */
  if (intern && !groups.issetgroups ())
    {
      const DWORD sd_buf_siz = MAX_SID_LEN + sizeof (SECURITY_DESCRIPTOR);
      PSECURITY_DESCRIPTOR sd_buf = (PSECURITY_DESCRIPTOR) alloca (sd_buf_siz);
      cygpsid gsid (NO_SID);
      if (!GetKernelObjectSecurity (token, GROUP_SECURITY_INFORMATION,
				    sd_buf, sd_buf_siz, &size))
	debug_printf ("GetKernelObjectSecurity(), %E");
      else if (!GetSecurityDescriptorGroup (sd_buf, (PSID *) &gsid,
					    (BOOL *) &size))
	debug_printf ("GetSecurityDescriptorGroup(), %E");
      if (well_known_null_sid != gsid)
	return gsid == groups.pgsid;
    }

  PTOKEN_GROUPS my_grps;
  bool sawpg = false, ret = false;

  if (!GetTokenInformation (token, TokenGroups, NULL, 0, &size) &&
      GetLastError () != ERROR_INSUFFICIENT_BUFFER)
    debug_printf ("GetTokenInformation(token, TokenGroups), %E");
  else if (!(my_grps = (PTOKEN_GROUPS) alloca (size)))
    debug_printf ("alloca (my_grps) failed.");
  else if (!GetTokenInformation (token, TokenGroups, my_grps, size, &size))
    debug_printf ("GetTokenInformation(my_token, TokenGroups), %E");
  else
    {
      if (groups.issetgroups ()) /* setgroups was called */
	{
	  cygsid gsid;
	  struct __group32 *gr;
	  bool saw[groups.sgsids.count ()];
	  memset (saw, 0, sizeof(saw));

	  /* token groups found in /etc/group match the user.gsids ? */
	  for (int gidx = 0; (gr = internal_getgrent (gidx)); ++gidx)
	    if (gsid.getfromgr (gr) && sid_in_token_groups (my_grps, gsid))
	      {
		int pos = groups.sgsids.position (gsid);
		if (pos >= 0)
		  saw[pos] = true;
		else if (groups.pgsid == gsid)
		  sawpg = true;
#if 0
		/* With this `else', verify_token returns false if we find
		   groups in the token, which are not in the group list set
		   with setgroups().  That's rather dangerous.  What we're
		   really interested in is that all groups in the setgroups()
		   list are in the token.  A token created through ADVAPI
		   should be allowed to contain more groups than requested
		   through setgroups(), esecially since Vista and the
		   addition of integrity groups. So we disable this statement
		   for now. */
		else if (gsid != well_known_world_sid
			 && gsid != usersid)
		  goto done;
#endif
	      }
	  /* user.sgsids groups must be in the token, except for builtin groups.
	     These can be different on domain member machines compared to
	     domain controllers, so these builtin groups may be validly missing
	     from a token created through password or lsaauth logon. */
	  for (int gidx = 0; gidx < groups.sgsids.count (); gidx++)
	    if (!saw[gidx]
	    	&& !groups.sgsids.sids[gidx].is_well_known_sid ()
		&& !sid_in_token_groups (my_grps, groups.sgsids.sids[gidx]))
	      goto done;
	}
      /* The primary group must be in the token */
      ret = sawpg
	|| sid_in_token_groups (my_grps, groups.pgsid)
	|| groups.pgsid == usersid;
    }
done:
  return ret;
}

HANDLE
create_token (cygsid &usersid, user_groups &new_groups, struct passwd *pw)
{
  NTSTATUS ret;
  LSA_HANDLE lsa = INVALID_HANDLE_VALUE;

  cygsidlist tmp_gsids (cygsidlist_auto, 12);

  SECURITY_QUALITY_OF_SERVICE sqos =
    { sizeof sqos, SecurityImpersonation, SECURITY_STATIC_TRACKING, FALSE };
  OBJECT_ATTRIBUTES oa = { sizeof oa, 0, 0, 0, 0, &sqos };
  LUID auth_luid = SYSTEM_LUID;
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

  PTOKEN_GROUPS my_tok_gsids = NULL;
  DWORD size;
  size_t psize = 0;

  /* SE_CREATE_TOKEN_NAME privilege needed to call NtCreateToken. */
  push_self_privilege (SE_CREATE_TOKEN_PRIVILEGE, true);

  /* Open policy object. */
  if ((lsa = open_local_policy (POLICY_EXECUTE)) == INVALID_HANDLE_VALUE)
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
      else if (!GetTokenInformation (hProcToken, TokenStatistics,
				     &stats, sizeof stats, &size))
	debug_printf
	  ("GetTokenInformation(hProcToken, TokenStatistics), %E");
      else
	auth_luid = stats.AuthenticationId;

      /* Retrieving current processes group list to be able to inherit
	 some important well known group sids. */
      if (!GetTokenInformation (hProcToken, TokenGroups, NULL, 0, &size)
	  && GetLastError () != ERROR_INSUFFICIENT_BUFFER)
	debug_printf ("GetTokenInformation(hProcToken, TokenGroups), %E");
      else if (!(my_tok_gsids = (PTOKEN_GROUPS) malloc (size)))
	debug_printf ("malloc (my_tok_gsids) failed.");
      else if (!GetTokenInformation (hProcToken, TokenGroups, my_tok_gsids,
				     size, &size))
	{
	  debug_printf ("GetTokenInformation(hProcToken, TokenGroups), %E");
	  free (my_tok_gsids);
	  my_tok_gsids = NULL;
	}
    }

  /* Create list of groups, the user is member in. */
  int auth_pos;
  if (new_groups.issetgroups ())
    get_setgroups_sidlist (tmp_gsids, usersid, pw, my_tok_gsids, new_groups,
			   auth_luid, auth_pos);
  else if (!get_initgroups_sidlist (tmp_gsids, usersid, new_groups.pgsid, pw,
				    my_tok_gsids, auth_luid, auth_pos))
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
  if (auth_pos >= 0)
    new_tok_gsids->Groups[auth_pos].Attributes |= SE_GROUP_LOGON_ID;

  /* On systems supporting Mandatory Integrity Control, add a MIC SID. */
  if (wincap.has_mandatory_integrity_control ())
    {
      new_tok_gsids->Groups[new_tok_gsids->GroupCount].Attributes =
	SE_GROUP_INTEGRITY | SE_GROUP_INTEGRITY_ENABLED;
      if (usersid == well_known_system_sid)
	new_tok_gsids->Groups[new_tok_gsids->GroupCount++].Sid
	  = mandatory_system_integrity_sid;
      else if (tmp_gsids.contains (well_known_admins_sid))
	new_tok_gsids->Groups[new_tok_gsids->GroupCount++].Sid
	  = mandatory_high_integrity_sid;
      else
	new_tok_gsids->Groups[new_tok_gsids->GroupCount++].Sid
	  = mandatory_medium_integrity_sid;
    }

  /* Retrieve list of privileges of that user. */
  if (!(privs = get_priv_list (lsa, usersid, tmp_gsids, psize)))
    goto out;

  /* Let's be heroic... */
  ret = NtCreateToken (&token, TOKEN_ALL_ACCESS, &oa, TokenImpersonation,
		       &auth_luid, &exp, &user, new_tok_gsids, privs, &owner,
		       &pgrp, &dacl, &source);
  if (ret)
    __seterrno_from_nt_status (ret);
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
  if (privs)
    free (privs);
  if (my_tok_gsids)
    free (my_tok_gsids);
  close_local_policy (lsa);

  debug_printf ("%p = create_token ()", primary_token);
  return primary_token;
}

HANDLE
lsaauth (cygsid &usersid, user_groups &new_groups, struct passwd *pw)
{
  cygsidlist tmp_gsids (cygsidlist_auto, 12);
  cygpsid pgrpsid;
  LSA_STRING name;
  HANDLE lsa_hdl = NULL, lsa = INVALID_HANDLE_VALUE;
  LSA_OPERATIONAL_MODE sec_mode;
  NTSTATUS ret, ret2;
  ULONG package_id, size;
  LUID auth_luid = SYSTEM_LUID;
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
  str2lsa (name, "Cygwin");
  SetLastError (0);
  ret = LsaRegisterLogonProcess (&name, &lsa_hdl, &sec_mode);
  if (ret != STATUS_SUCCESS)
    {
      debug_printf ("LsaRegisterLogonProcess: %p", ret);
      __seterrno_from_win_error (LsaNtStatusToWinError (ret));
      goto out;
    }
  else if (GetLastError () == ERROR_PROC_NOT_FOUND)
    {
      debug_printf ("Couldn't load Secur32.dll");
      goto out;
    }
  /* Get handle to our own LSA package. */
  str2lsa (name, CYG_LSA_PKGNAME);
  ret = LsaLookupAuthenticationPackage (lsa_hdl, &name, &package_id);
  if (ret != STATUS_SUCCESS)
    {
      debug_printf ("LsaLookupAuthenticationPackage: %p", ret);
      __seterrno_from_win_error (LsaNtStatusToWinError (ret));
      goto out;
    }

  /* Open policy object. */
  if ((lsa = open_local_policy (POLICY_EXECUTE)) == INVALID_HANDLE_VALUE)
    goto out;

  /* Create origin. */
  str2buf2lsa (origin.str, origin.buf, "Cygwin");
  /* Create token source. */
  memcpy (ts.SourceName, "Cygwin.1", 8);
  ts.SourceIdentifier.HighPart = 0;
  ts.SourceIdentifier.LowPart = 0x0103;

  /* Create list of groups, the user is member in. */
  int auth_pos;
  if (new_groups.issetgroups ())
    get_setgroups_sidlist (tmp_gsids, usersid, pw, NULL, new_groups, auth_luid,
			   auth_pos);
  else if (!get_initgroups_sidlist (tmp_gsids, usersid, new_groups.pgsid, pw,
				    NULL, auth_luid, auth_pos))
    goto out;
  /* The logon SID entry is not generated automatically on Windows 2000
     and earlier for some reason.  So add fake logon sid here, which is
     filled with logon id values in the authentication package. */
  if (wincap.needs_logon_sid_in_sid_list ())
    tmp_gsids += fake_logon_sid;

  tmp_gsids.debug_print ("tmp_gsids");

  /* Evaluate size of TOKEN_GROUPS list */
  non_well_known_cnt =  tmp_gsids.non_well_known_count ();
  gsize = sizeof (DWORD) + non_well_known_cnt * sizeof (SID_AND_ATTRIBUTES);
  tmpidx = -1;
  for (int i = 0; i < non_well_known_cnt; ++i)
    if ((tmpidx = tmp_gsids.next_non_well_known_sid (tmpidx)) >= 0)
      gsize += GetLengthSid (tmp_gsids.sids[tmpidx]);

  /* Retrieve list of privileges of that user. */
  if (!(privs = get_priv_list (lsa, usersid, tmp_gsids, psize)))
    goto out;

  /* Create DefaultDacl. */
  dsize = sizeof (ACL) + 3 * sizeof (ACCESS_ALLOWED_ACE)
	  + GetLengthSid (usersid)
	  + GetLengthSid (well_known_admins_sid)
	  + GetLengthSid (well_known_system_sid);
  dacl = (PACL) alloca (dsize);
  if (!InitializeAcl (dacl, dsize, ACL_REVISION))
    goto out;
  if (!AddAccessAllowedAce (dacl, ACL_REVISION, GENERIC_ALL, usersid))
    goto out;
  if (!AddAccessAllowedAce (dacl, ACL_REVISION, GENERIC_ALL,
			    well_known_admins_sid))
    goto out;
  if (!AddAccessAllowedAce (dacl, ACL_REVISION, GENERIC_ALL,
			    well_known_system_sid))
    goto out;

  /* Evaluate authinf size and allocate authinf. */
  authinf_size = (authinf->data - (PBYTE) authinf);
  authinf_size += GetLengthSid (usersid);	    /* User SID */
  authinf_size += gsize;			    /* Groups + Group SIDs */
  /* When trying to define the admins group as primary group on Vista,
     LsaLogonUser fails with error STATUS_INVALID_OWNER.  As workaround
     we define "Local" as primary group here.  Seteuid32 sets the primary
     group to the group set in /etc/passwd anyway. */
  if (new_groups.pgsid == well_known_admins_sid)
    pgrpsid = well_known_local_sid;
  else
    pgrpsid = new_groups.pgsid;

  authinf_size += GetLengthSid (pgrpsid);	    /* Primary Group SID */

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
  CopySid (GetLengthSid (usersid), (PSID) ((PBYTE) &authinf->inf + offset),
	   usersid);
  offset += GetLengthSid (usersid);
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
      /* Mark logon SID as logon SID :) */
      if (wincap.needs_logon_sid_in_sid_list ()
	  && tmp_gsids.sids[tmpidx] == fake_logon_sid)
	gsids->Groups[i].Attributes += SE_GROUP_LOGON_ID;
      CopySid (GetLengthSid (tmp_gsids.sids[tmpidx]),
	       (PSID) ((PBYTE) &authinf->inf + sids_offset),
	       tmp_gsids.sids[tmpidx]);
      sids_offset += GetLengthSid (tmp_gsids.sids[tmpidx]);
    }
  offset += gsize;
  /* Primary Group SID */
  authinf->inf.PrimaryGroup.PrimaryGroup = offset;
  CopySid (GetLengthSid (pgrpsid), (PSID) ((PBYTE) &authinf->inf + offset),
	   pgrpsid);
  offset += GetLengthSid (pgrpsid);
  /* Privileges */
  authinf->inf.Privileges = offset;
  memcpy ((PBYTE) &authinf->inf + offset, privs, psize);
  offset += psize;
  /* Owner */
  authinf->inf.Owner.Owner = 0;
  /* Default DACL */
  authinf->inf.DefaultDacl.DefaultDacl = offset;
  memcpy ((PBYTE) &authinf->inf + offset, dacl, dsize);

  authinf->checksum = CYGWIN_VERSION_MAGIC (CYGWIN_VERSION_DLL_MAJOR,
					    CYGWIN_VERSION_DLL_MINOR);
  PDWORD csp;
  PDWORD csp_end;
  csp = (PDWORD) &authinf->username;
  csp_end = (PDWORD) ((PBYTE) authinf + authinf_size);
  while (csp < csp_end)
    authinf->checksum += *csp++;

  /* Try to logon... */
  ret = LsaLogonUser (lsa_hdl, (PLSA_STRING) &origin, Interactive, package_id,
		      authinf, authinf_size, NULL, &ts, &profile, &size, &luid,
		      &user_token, &quota, &ret2);
  if (ret != STATUS_SUCCESS)
    {
      debug_printf ("LsaLogonUser: %p", ret);
      __seterrno_from_win_error (LsaNtStatusToWinError (ret));
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

  if (wincap.has_mandatory_integrity_control ())
    {
      TOKEN_LINKED_TOKEN linked;

      if (GetTokenInformation (user_token, TokenLinkedToken,
			       (PVOID) &linked, sizeof linked, &size))
	{
	  debug_printf ("Linked Token: %p", linked.LinkedToken);
	  if (linked.LinkedToken)
	    {
	      CloseHandle (user_token);
	      user_token = linked.LinkedToken;
	    }
	}
    }

  /* The token returned by LsaLogonUser is not inheritable.  Make it so. */
  if (!SetHandleInformation (user_token, HANDLE_FLAG_INHERIT,
			     HANDLE_FLAG_INHERIT))
    system_printf ("SetHandleInformation %E");

out:
  if (privs)
    free (privs);
  close_local_policy (lsa);
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
  HANDLE lsa = INVALID_HANDLE_VALUE;
  HANDLE token = NULL;
  WCHAR sid[256];
  WCHAR domain[MAX_DOMAIN_NAME_LEN + 1];
  WCHAR user[UNLEN + 1];
  WCHAR key_name[MAX_DOMAIN_NAME_LEN + UNLEN + wcslen (SFU_LSA_KEY_SUFFIX) + 2];
  UNICODE_STRING key;
  PUNICODE_STRING data;
  cygsid psid;

  push_self_privilege (SE_TCB_PRIVILEGE, true);

  /* Open policy object. */
  if ((lsa = open_local_policy (POLICY_GET_PRIVATE_INFORMATION))
      == INVALID_HANDLE_VALUE)
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
	{
	  /* No Cygwin key, try Interix key. */
	  if (!*domain)
	    goto out;
	  __small_swprintf (key_name, L"%W_%W%W",
			    domain, user, SFU_LSA_KEY_SUFFIX);
	  RtlInitUnicodeString (&key, key_name);
	  status = LsaRetrievePrivateData (lsa, &key, &data);
	  if (!NT_SUCCESS (status))
	    goto out;
	}
    }

  /* The key is not 0-terminated. */
  PWCHAR passwd;
  passwd = (PWCHAR) alloca (data->Length + sizeof (WCHAR));
  *wcpncpy (passwd, data->Buffer, data->Length / sizeof (WCHAR)) = L'\0';
  LsaFreeMemory (data);
  debug_printf ("Try logon for %W\\%W", domain, user);
  if (!LogonUserW (user, domain, passwd, LOGON32_LOGON_INTERACTIVE,
		   LOGON32_PROVIDER_DEFAULT, &token))
    {
      __seterrno ();
      token = NULL;
    }
  else
    {
      if (wincap.has_mandatory_integrity_control ())
	{
	  TOKEN_LINKED_TOKEN linked;
	  DWORD size;

	  if (GetTokenInformation (token, TokenLinkedToken,
				   (PVOID) &linked, sizeof linked, &size))
	    {
	      debug_printf ("Linked Token: %p", linked.LinkedToken);
	      if (linked.LinkedToken)
		{
		  CloseHandle (token);
		  token = linked.LinkedToken;
		}
	    }
	}
      if (!SetHandleInformation (token, HANDLE_FLAG_INHERIT,
				 HANDLE_FLAG_INHERIT))
	{
	  __seterrno ();
	  CloseHandle (token);
	  token = NULL;
	}
    }

out:
  close_local_policy (lsa);
  pop_self_privilege ();
  return token;
}
