/* security.cc: NT security functions

   Copyright 1997, 1998, 1999, 2000, 2001, 2002 Red Hat, Inc.

   Originaly written by Gunther Ebert, gunther.ebert@ixos-leipzig.de
   Completely rewritten by Corinna Vinschen <corinna@vinschen.de>

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

#include "winsup.h"
#include <grp.h>
#include <pwd.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <limits.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/acl.h>
#include <ctype.h>
#include <winnls.h>
#include <wingdi.h>
#include <winuser.h>
#include <wininet.h>
#include <ntsecapi.h>
#include <subauth.h>
#include <aclapi.h>
#include "cygerrno.h"
#include "security.h"
#include "fhandler.h"
#include "path.h"
#include "dtable.h"
#include "pinfo.h"
#include "cygheap.h"
#include <ntdef.h>
#include "ntdll.h"
#include "lm.h"
#include "pwdgrp.h"

extern BOOL allow_ntea;
BOOL allow_ntsec;
/* allow_smbntsec is handled exclusively in path.cc (path_conv::check).
   It's defined here because of it's strong relationship to allow_ntsec.
   The default is TRUE to reflect the old behaviour. */
BOOL allow_smbntsec;

cygsid *
cygsidlist::alloc_sids (int n)
{
  if (n > 0)
    return (cygsid *) cmalloc (HEAP_STR, n * sizeof (cygsid));
  else
    return NULL;
}

void
cygsidlist::free_sids ()
{
  if (sids)
    cfree (sids);
  sids = NULL;
  count = maxcount = 0;
  type = cygsidlist_empty;
}

extern "C" void
cygwin_set_impersonation_token (const HANDLE hToken)
{
  debug_printf ("set_impersonation_token (%d)", hToken);
  if (cygheap->user.token != hToken)
    {
      cygheap->user.token = hToken;
      cygheap->user.impersonated = FALSE;
    }
}

void
extract_nt_dom_user (const struct passwd *pw, char *domain, char *user)
{
  char *d, *u, *c;

  domain[0] = 0;
  strlcpy (user, pw->pw_name, UNLEN + 1);
  debug_printf ("pw_gecos = %x (%s)", pw->pw_gecos, pw->pw_gecos);

  if ((d = strstr (pw->pw_gecos, "U-")) != NULL &&
      (d == pw->pw_gecos || d[-1] == ','))
    {
      c = strchr (d + 2, ',');
      if ((u = strchr (d + 2, '\\')) == NULL || (c != NULL && u > c))
	u = d + 1;
      else if (u - d <= INTERNET_MAX_HOST_NAME_LENGTH + 2)
	strlcpy (domain, d + 2, u - d - 1);
      if (c == NULL)
	c = u + UNLEN + 1;
      if (c - u <= UNLEN + 1)
	strlcpy (user, u + 1, c - u);
    }
  if (domain[0])
    return;

  cygsid psid;
  DWORD ulen = UNLEN + 1;
  DWORD dlen = INTERNET_MAX_HOST_NAME_LENGTH + 1;
  SID_NAME_USE use;
  if (psid.getfrompw (pw))
    LookupAccountSid (NULL, psid, user, &ulen, domain, &dlen, &use);
}

extern "C" HANDLE
cygwin_logon_user (const struct passwd *pw, const char *password)
{
  if (!wincap.has_security ())
    {
      set_errno (ENOSYS);
      return INVALID_HANDLE_VALUE;
    }
  if (!pw)
    {
      set_errno (EINVAL);
      return INVALID_HANDLE_VALUE;
    }

  char nt_domain[INTERNET_MAX_HOST_NAME_LENGTH + 1];
  char nt_user[UNLEN + 1];
  HANDLE hToken;

  extract_nt_dom_user (pw, nt_domain, nt_user);
  debug_printf ("LogonUserA (%s, %s, %s, ...)", nt_user, nt_domain, password);
  if (!LogonUserA (nt_user, *nt_domain ? nt_domain : NULL, (char *) password,
		   LOGON32_LOGON_INTERACTIVE,
		   LOGON32_PROVIDER_DEFAULT,
		   &hToken)
      || !SetHandleInformation (hToken,
				HANDLE_FLAG_INHERIT,
				HANDLE_FLAG_INHERIT))
    {
      __seterrno ();
      return INVALID_HANDLE_VALUE;
    }
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

void
str2buf2uni (UNICODE_STRING &tgt, WCHAR *buf, const char *srcstr)
{
  tgt.Length = strlen (srcstr) * sizeof (WCHAR);
  tgt.MaximumLength = tgt.Length + sizeof (WCHAR);
  tgt.Buffer = (PWCHAR) buf;
  sys_mbstowcs (buf, srcstr, tgt.MaximumLength);
}

#if 0				/* unused */
static void
lsa2wchar (WCHAR *tgt, LSA_UNICODE_STRING &src, int size)
{
  size = (size - 1) * sizeof (WCHAR);
  if (src.Length < size)
    size = src.Length;
  memcpy (tgt, src.Buffer, size);
  size >>= 1;
  tgt[size] = 0;
}
#endif

static void
lsa2str (char *tgt, LSA_UNICODE_STRING &src, int size)
{
  if (src.Length / 2 < size)
    size = src.Length / 2;
  sys_wcstombs (tgt, src.Buffer, size);
  tgt[size] = 0;
}

static LSA_HANDLE
open_local_policy ()
{
  LSA_OBJECT_ATTRIBUTES oa = { 0, 0, 0, 0, 0, 0 };
  LSA_HANDLE lsa = INVALID_HANDLE_VALUE;

  NTSTATUS ret = LsaOpenPolicy (NULL, &oa, POLICY_EXECUTE, &lsa);
  if (ret != STATUS_SUCCESS)
    __seterrno_from_win_error (LsaNtStatusToWinError (ret));
  return lsa;
}

static void
close_local_policy (LSA_HANDLE &lsa)
{
  if (lsa != INVALID_HANDLE_VALUE)
    LsaClose (lsa);
  lsa = INVALID_HANDLE_VALUE;
}

#if 0 /* unused */
static BOOL
get_lsa_srv_inf (LSA_HANDLE lsa, char *logonserver, char *domain)
{
  NET_API_STATUS ret;
  WCHAR *buf;
  char name[INTERNET_MAX_HOST_NAME_LENGTH + 1];
  WCHAR account[INTERNET_MAX_HOST_NAME_LENGTH + 1];
  WCHAR primary[INTERNET_MAX_HOST_NAME_LENGTH + 1];
  PPOLICY_ACCOUNT_DOMAIN_INFO adi;
  PPOLICY_PRIMARY_DOMAIN_INFO pdi;

  if ((ret = LsaQueryInformationPolicy (lsa, PolicyAccountDomainInformation,
					(PVOID *) &adi)) != STATUS_SUCCESS)
    {
      __seterrno_from_win_error (LsaNtStatusToWinError (ret));
      return FALSE;
    }
  lsa2wchar (account, adi->DomainName, INTERNET_MAX_HOST_NAME_LENGTH + 1);
  LsaFreeMemory (adi);
  if ((ret = LsaQueryInformationPolicy (lsa, PolicyPrimaryDomainInformation,
					(PVOID *) &pdi)) != STATUS_SUCCESS)
    {
      __seterrno_from_win_error (LsaNtStatusToWinError (ret));
      return FALSE;
    }
  lsa2wchar (primary, pdi->Name, INTERNET_MAX_HOST_NAME_LENGTH + 1);
  LsaFreeMemory (pdi);
  /* If the SID given in the primary domain info is NULL, the machine is
     not member of a domain.  The name in the primary domain info is the
     name of the workgroup then. */
  if (pdi->Sid &&
      (ret =
       NetGetDCName (NULL, primary, (LPBYTE *) &buf)) == STATUS_SUCCESS)
    {
      sys_wcstombs (name, buf, INTERNET_MAX_HOST_NAME_LENGTH + 1);
      strcpy (logonserver, name);
      if (domain)
	sys_wcstombs (domain, primary, INTERNET_MAX_HOST_NAME_LENGTH + 1);
    }
  else
    {
      sys_wcstombs (name, account, INTERNET_MAX_HOST_NAME_LENGTH + 1);
      strcpy (logonserver, "\\\\");
      strcat (logonserver, name);
      if (domain)
	sys_wcstombs (domain, account, INTERNET_MAX_HOST_NAME_LENGTH + 1);
    }
  if (ret == STATUS_SUCCESS)
    NetApiBufferFree (buf);
  return TRUE;
}
#endif

BOOL
get_logon_server (const char *domain, char *server, WCHAR *wserver)
{
  WCHAR wdomain[INTERNET_MAX_HOST_NAME_LENGTH + 1];
  NET_API_STATUS ret;
  WCHAR *buf;
  DWORD size = INTERNET_MAX_HOST_NAME_LENGTH + 1;

  /* Empty domain is interpreted as local system */
  if ((GetComputerName (server + 2, &size)) &&
      (strcasematch (domain, server + 2) || !domain[0]))
    {
      server[0] = server[1] = '\\';
      if (wserver)
	sys_mbstowcs (wserver, server, INTERNET_MAX_HOST_NAME_LENGTH + 1);
      return TRUE;
    }

  /* Try to get the primary domain controller for the domain */
  sys_mbstowcs (wdomain, domain, INTERNET_MAX_HOST_NAME_LENGTH + 1);
  if ((ret = NetGetDCName (NULL, wdomain, (LPBYTE *) &buf)) == STATUS_SUCCESS)
    {
      sys_wcstombs (server, buf, INTERNET_MAX_HOST_NAME_LENGTH + 1);
      if (wserver)
	for (WCHAR *ptr1 = buf; (*wserver++ = *ptr1++);)
	  ;
      NetApiBufferFree (buf);
      return TRUE;
    }
  __seterrno_from_win_error (ret);
  return FALSE;
}

static BOOL
get_user_groups (WCHAR *wlogonserver, cygsidlist &grp_list, char *user,
		 char *domain)
{
  char dgroup[INTERNET_MAX_HOST_NAME_LENGTH + GNLEN + 2];
  WCHAR wuser[UNLEN + 1];
  sys_mbstowcs (wuser, user, UNLEN + 1);
  LPGROUP_USERS_INFO_0 buf;
  DWORD cnt, tot, len;
  NET_API_STATUS ret;

  /* Look only on logonserver */
  ret = NetUserGetGroups (wlogonserver, wuser, 0, (LPBYTE *) &buf,
			  MAX_PREFERRED_LENGTH, &cnt, &tot);
  if (ret)
    {
      __seterrno_from_win_error (ret);
      /* It's no error when the user name can't be found. */
      return ret == NERR_UserNotFound;
    }

  len = strlen (domain);
  strcpy (dgroup, domain);
  dgroup[len++] = '\\';

  for (DWORD i = 0; i < cnt; ++i)
    {
      cygsid gsid;
      DWORD glen = sizeof (gsid);
      char domain[INTERNET_MAX_HOST_NAME_LENGTH + 1];
      DWORD dlen = sizeof (domain);
      SID_NAME_USE use = SidTypeInvalid;

      sys_wcstombs (dgroup + len, buf[i].grui0_name, GNLEN + 1);
      if (!LookupAccountName (NULL, dgroup, gsid, &glen, domain, &dlen, &use))
	debug_printf ("LookupAccountName(%s): %E", dgroup);
      else if (legal_sid_type (use))
	grp_list += gsid;
      else
	debug_printf ("Global group %s invalid. Domain: %s Use: %d",
		      dgroup, domain, use);
    }

  NetApiBufferFree (buf);
  return TRUE;
}

static BOOL
is_group_member (WCHAR *wgroup, PSID pusersid, cygsidlist &grp_list)
{
  LPLOCALGROUP_MEMBERS_INFO_0 buf;
  DWORD cnt, tot;
  NET_API_STATUS ret;
  BOOL retval = FALSE;

  /* Members can be users or global groups */
  ret = NetLocalGroupGetMembers (NULL, wgroup, 0, (LPBYTE *) &buf,
				 MAX_PREFERRED_LENGTH, &cnt, &tot, NULL);
  if (ret)
    return FALSE;

  for (DWORD bidx = 0; !retval && bidx < cnt; ++bidx)
    if (EqualSid (pusersid, buf[bidx].lgrmi0_sid))
      retval = TRUE;
    else
      for (int glidx = 0; !retval && glidx < grp_list.count; ++glidx)
	if (EqualSid (grp_list.sids[glidx], buf[bidx].lgrmi0_sid))
	  retval = TRUE;

  NetApiBufferFree (buf);
  return retval;
}

static BOOL
get_user_local_groups (cygsidlist &grp_list, PSID pusersid)
{
  LPLOCALGROUP_INFO_0 buf;
  DWORD cnt, tot;
  NET_API_STATUS ret;

  ret = NetLocalGroupEnum (NULL, 0, (LPBYTE *) &buf,
			   MAX_PREFERRED_LENGTH, &cnt, &tot, NULL);
  if (ret)
    {
      __seterrno_from_win_error (ret);
      return FALSE;
    }

  char bgroup[sizeof ("BUILTIN\\") + GNLEN] = "BUILTIN\\";
  char lgroup[INTERNET_MAX_HOST_NAME_LENGTH + GNLEN + 2];
  const DWORD blen = sizeof ("BUILTIN\\") - 1;
  DWORD llen = INTERNET_MAX_HOST_NAME_LENGTH + 1;
  if (!GetComputerNameA (lgroup, &llen))
    {
      __seterrno ();
      return FALSE;
    }
  lgroup[llen++] = '\\';

  for (DWORD i = 0; i < cnt; ++i)
    if (is_group_member (buf[i].lgrpi0_name, pusersid, grp_list))
      {
	cygsid gsid;
	DWORD glen = sizeof (gsid);
	char domain[INTERNET_MAX_HOST_NAME_LENGTH + 1];
	DWORD dlen = sizeof (domain);
	SID_NAME_USE use = SidTypeInvalid;

	sys_wcstombs (bgroup + blen, buf[i].lgrpi0_name, GNLEN + 1);
	if (!LookupAccountName (NULL, bgroup, gsid, &glen, domain, &dlen, &use))
	  {
	    if (GetLastError () != ERROR_NONE_MAPPED)
	      debug_printf ("LookupAccountName(%s): %E", bgroup);
	    strcpy (lgroup + llen, bgroup + blen);
	    if (!LookupAccountName (NULL, lgroup, gsid, &glen,
				    domain, &dlen, &use))
	      debug_printf ("LookupAccountName(%s): %E", lgroup);
	  }
	if (!legal_sid_type (use))
	  debug_printf ("Rejecting local %s. use: %d", bgroup + blen, use);
	else if (!grp_list.contains (gsid))
	  grp_list += gsid;
      }
  NetApiBufferFree (buf);
  return TRUE;
}

static BOOL
sid_in_token_groups (PTOKEN_GROUPS grps, cygsid &sid)
{
  if (!grps)
    return FALSE;
  for (DWORD i = 0; i < grps->GroupCount; ++i)
    if (sid == grps->Groups[i].Sid)
      return TRUE;
  return FALSE;
}

#if 0				/* Unused */
static BOOL
get_user_primary_group (WCHAR *wlogonserver, const char *user,
			PSID pusersid, cygsid &pgrpsid)
{
  LPUSER_INFO_3 buf;
  WCHAR wuser[UNLEN + 1];
  NET_API_STATUS ret;
  BOOL retval = FALSE;
  UCHAR count = 0;

  if (well_known_system_sid == pusersid)
    {
      pgrpsid = well_known_system_sid;
      return TRUE;
    }

  sys_mbstowcs (wuser, user, UNLEN + 1);
  ret = NetUserGetInfo (wlogonserver, wuser, 3, (LPBYTE *) &buf);
  if (ret)
    {
      __seterrno_from_win_error (ret);
      return FALSE;
    }

  pgrpsid = pusersid;
  if (IsValidSid (pgrpsid)
      && (count = *GetSidSubAuthorityCount (pgrpsid)) > 1)
    {
      *GetSidSubAuthority (pgrpsid, count - 1) = buf->usri3_primary_group_id;
      retval = TRUE;
    }
  NetApiBufferFree (buf);
  return retval;
}
#endif

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
      if (gsid.getfromgr (gr) && !grp_list.contains (gsid))
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
      if (sid_in_token_groups (my_grps, well_known_local_sid))
	grp_list += well_known_local_sid;
      if (sid_in_token_groups (my_grps, well_known_dialup_sid))
	grp_list += well_known_dialup_sid;
      if (sid_in_token_groups (my_grps, well_known_network_sid))
	grp_list += well_known_network_sid;
      if (sid_in_token_groups (my_grps, well_known_batch_sid))
	grp_list += well_known_batch_sid;
      if (sid_in_token_groups (my_grps, well_known_interactive_sid))
	grp_list += well_known_interactive_sid;
      if (sid_in_token_groups (my_grps, well_known_service_sid))
	grp_list += well_known_service_sid;
    }
  else
    {
      grp_list += well_known_local_sid;
      grp_list += well_known_interactive_sid;
    }
  if (auth_luid.QuadPart != 999) /* != SYSTEM_LUID */
    {
      char buf[64];
      __small_sprintf (buf, "S-1-5-5-%u-%u", auth_luid.HighPart,
		       auth_luid.LowPart);
      grp_list += buf;
      auth_pos = grp_list.count - 1;
    }
}

static BOOL
get_initgroups_sidlist (cygsidlist &grp_list,
			PSID usersid, PSID pgrpsid, struct passwd *pw,
			PTOKEN_GROUPS my_grps, LUID auth_luid, int &auth_pos,
			BOOL &special_pgrp)
{
  grp_list += well_known_world_sid;
  grp_list += well_known_authenticated_users_sid;
  if (well_known_system_sid == usersid)
    {
      auth_pos = -1;
      grp_list += well_known_admins_sid;
      get_unix_group_sidlist (pw, grp_list);
    }
  else
    {
      char user[UNLEN + 1];
      char domain[INTERNET_MAX_HOST_NAME_LENGTH + 1];
      WCHAR wserver[INTERNET_MAX_HOST_NAME_LENGTH + 3];
      char server[INTERNET_MAX_HOST_NAME_LENGTH + 3];

      get_token_group_sidlist (grp_list, my_grps, auth_luid, auth_pos);
      extract_nt_dom_user (pw, domain, user);
      if (get_logon_server (domain, server, wserver))
	get_user_groups (wserver, grp_list, user, domain);
      get_unix_group_sidlist (pw, grp_list);
      if (!get_user_local_groups (grp_list, usersid))
	return FALSE;
    }
  /* special_pgrp true if pgrpsid is not in normal groups */
  if ((special_pgrp = !grp_list.contains (pgrpsid)))
    grp_list += pgrpsid;
  return TRUE;
}

static void
get_setgroups_sidlist (cygsidlist &tmp_list, PTOKEN_GROUPS my_grps,
		       user_groups &groups, LUID auth_luid, int &auth_pos)
{
  PSID pgpsid = groups.pgsid;
  tmp_list += well_known_world_sid;
  tmp_list += well_known_authenticated_users_sid;
  get_token_group_sidlist (tmp_list, my_grps, auth_luid, auth_pos);
  for (int gidx = 0; gidx < groups.sgsids.count; gidx++)
    tmp_list += groups.sgsids.sids[gidx];
  if (!groups.sgsids.contains (pgpsid))
    tmp_list += pgpsid;
}

static const char *sys_privs[] = {
  SE_TCB_NAME,
  SE_ASSIGNPRIMARYTOKEN_NAME,
  SE_CREATE_TOKEN_NAME,
  SE_CHANGE_NOTIFY_NAME,
  SE_SECURITY_NAME,
  SE_BACKUP_NAME,
  SE_RESTORE_NAME,
  SE_SYSTEMTIME_NAME,
  SE_SHUTDOWN_NAME,
  SE_REMOTE_SHUTDOWN_NAME,
  SE_TAKE_OWNERSHIP_NAME,
  SE_DEBUG_NAME,
  SE_SYSTEM_ENVIRONMENT_NAME,
  SE_SYSTEM_PROFILE_NAME,
  SE_PROF_SINGLE_PROCESS_NAME,
  SE_INC_BASE_PRIORITY_NAME,
  SE_LOAD_DRIVER_NAME,
  SE_CREATE_PAGEFILE_NAME,
  SE_INCREASE_QUOTA_NAME
};

#define SYSTEM_PERMISSION_COUNT (sizeof sys_privs / sizeof (const char *))

PTOKEN_PRIVILEGES
get_system_priv_list (cygsidlist &grp_list)
{
  LUID priv;
  PTOKEN_PRIVILEGES privs = (PTOKEN_PRIVILEGES)
    malloc (sizeof (ULONG) + 20 * sizeof (LUID_AND_ATTRIBUTES));
  if (!privs)
    {
      debug_printf ("malloc (system_privs) failed.");
      return NULL;
    }
  privs->PrivilegeCount = 0;

  for (DWORD i = 0; i < SYSTEM_PERMISSION_COUNT; ++i)
    if (LookupPrivilegeValue (NULL, sys_privs[i], &priv))
      {
	privs->Privileges[privs->PrivilegeCount].Luid = priv;
	privs->Privileges[privs->PrivilegeCount].Attributes =
	  SE_PRIVILEGE_ENABLED | SE_PRIVILEGE_ENABLED_BY_DEFAULT;
	++privs->PrivilegeCount;
      }
  return privs;
}

PTOKEN_PRIVILEGES
get_priv_list (LSA_HANDLE lsa, cygsid &usersid, cygsidlist &grp_list)
{
  PLSA_UNICODE_STRING privstrs;
  ULONG cnt;
  PTOKEN_PRIVILEGES privs = NULL;
  NTSTATUS ret;
  char buf[INTERNET_MAX_HOST_NAME_LENGTH + 1];

  if (usersid == well_known_system_sid)
    return get_system_priv_list (grp_list);

  for (int grp = -1; grp < grp_list.count; ++grp)
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

	  lsa2str (buf, privstrs[i], sizeof (buf) - 1);
	  if (!LookupPrivilegeValue (NULL, buf, &priv))
	    continue;

	  for (DWORD p = 0; privs && p < privs->PrivilegeCount; ++p)
	    if (!memcmp (&priv, &privs->Privileges[p].Luid, sizeof (LUID)))
	      goto next_account_right;

	  tmp_count = privs ? privs->PrivilegeCount : 0;
	  tmp = (PTOKEN_PRIVILEGES)
	    realloc (privs, sizeof (ULONG) +
		     (tmp_count + 1) * sizeof (LUID_AND_ATTRIBUTES));
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
BOOL
verify_token (HANDLE token, cygsid &usersid, user_groups &groups, BOOL *pintern)
{
  DWORD size;
  BOOL intern = FALSE;

  if (pintern)
    {
      TOKEN_SOURCE ts;
      if (!GetTokenInformation (cygheap->user.token, TokenSource,
				&ts, sizeof ts, &size))
	debug_printf ("GetTokenInformation(): %E");
      else
	*pintern = intern = !memcmp (ts.SourceName, "Cygwin.1", 8);
    }
  /* Verify usersid */
  cygsid tok_usersid = NO_SID;
  if (!GetTokenInformation (token, TokenUser,
			    &tok_usersid, sizeof tok_usersid, &size))
    debug_printf ("GetTokenInformation(): %E");
  if (usersid != tok_usersid)
    return FALSE;

  /* For an internal token, if setgroups was not called and if the sd group
     is not well_known_null_sid, it must match pgrpsid */
  if (intern && !groups.issetgroups ())
    {
      char sd_buf[MAX_SID_LEN + sizeof (SECURITY_DESCRIPTOR)];
      PSID gsid = NO_SID;
      if (!GetKernelObjectSecurity (token, GROUP_SECURITY_INFORMATION,
				    (PSECURITY_DESCRIPTOR) sd_buf,
				    sizeof sd_buf, &size))
	debug_printf ("GetKernelObjectSecurity(): %E");
      else if (!GetSecurityDescriptorGroup ((PSECURITY_DESCRIPTOR) sd_buf,
					    &gsid, (BOOL *) &size))
	debug_printf ("GetSecurityDescriptorGroup(): %E");
      if (well_known_null_sid != gsid)
	return gsid == groups.pgsid;
    }

  PTOKEN_GROUPS my_grps = NULL;
  BOOL ret = FALSE;
  char saw_buf[NGROUPS_MAX] = {};
  char *saw = saw_buf, sawpg = FALSE;

  if (!GetTokenInformation (token, TokenGroups, NULL, 0, &size) &&
      GetLastError () != ERROR_INSUFFICIENT_BUFFER)
    debug_printf ("GetTokenInformation(token, TokenGroups): %E");
  else if (!(my_grps = (PTOKEN_GROUPS) malloc (size)))
    debug_printf ("malloc (my_grps) failed.");
  else if (!GetTokenInformation (token, TokenGroups, my_grps, size, &size))
    debug_printf ("GetTokenInformation(my_token, TokenGroups): %E");
  else if (!groups.issetgroups ()) /* setgroups was never called */
    {
      ret = sid_in_token_groups (my_grps, groups.pgsid);
      if (ret == FALSE)
	ret = (groups.pgsid == tok_usersid);
    }
  else /* setgroups was called */
    {
      struct __group32 *gr;
      cygsid gsid;
      if (groups.sgsids.count > (int) sizeof (saw_buf) &&
	  !(saw = (char *) calloc (groups.sgsids.count, sizeof (char))))
	goto done;

      /* token groups found in /etc/group match the user.gsids ? */
      for (int gidx = 0; (gr = internal_getgrent (gidx)); ++gidx)
	if (gsid.getfromgr (gr) && sid_in_token_groups (my_grps, gsid))
	  {
	    int pos = groups.sgsids.position (gsid);
	    if (pos >= 0)
	      saw[pos] = TRUE;
	    else if (groups.pgsid == gsid)
	      sawpg = TRUE;
	   else if (gsid != well_known_world_sid &&
		    gsid != usersid)
	      goto done;
	  }
      for (int gidx = 0; gidx < groups.sgsids.count; gidx++)
	if (!saw[gidx])
	  goto done;
      if (sawpg ||
	  groups.sgsids.contains (groups.pgsid) ||
	  groups.pgsid == usersid)
	ret = TRUE;
    }
done:
  if (my_grps)
    free (my_grps);
  if (saw != saw_buf)
    free (saw);
  return ret;
}

HANDLE
create_token (cygsid &usersid, user_groups &new_groups, struct passwd *pw)
{
  NTSTATUS ret;
  LSA_HANDLE lsa = INVALID_HANDLE_VALUE;
  int old_priv_state;

  cygsidlist tmp_gsids (cygsidlist_auto, 12);

  SECURITY_QUALITY_OF_SERVICE sqos =
    { sizeof sqos, SecurityImpersonation, SECURITY_STATIC_TRACKING, FALSE };
  OBJECT_ATTRIBUTES oa = { sizeof oa, 0, 0, 0, 0, &sqos };
  PSECURITY_ATTRIBUTES psa;
  BOOL special_pgrp = FALSE;
  char sa_buf[1024];
  LUID auth_luid = SYSTEM_LUID;
  LARGE_INTEGER exp = { QuadPart:0x7fffffffffffffffLL };

  TOKEN_USER user;
  PTOKEN_GROUPS new_tok_gsids = NULL;
  PTOKEN_PRIVILEGES privs = NULL;
  TOKEN_OWNER owner;
  TOKEN_PRIMARY_GROUP pgrp;
  char acl_buf[MAX_DACL_LEN (5)];
  TOKEN_DEFAULT_DACL dacl;
  TOKEN_SOURCE source;
  TOKEN_STATISTICS stats;
  memcpy (source.SourceName, "Cygwin.1", 8);
  source.SourceIdentifier.HighPart = 0;
  source.SourceIdentifier.LowPart = 0x0101;

  HANDLE token = INVALID_HANDLE_VALUE;
  HANDLE primary_token = INVALID_HANDLE_VALUE;

  HANDLE my_token = INVALID_HANDLE_VALUE;
  PTOKEN_GROUPS my_tok_gsids = NULL;
  DWORD size;

  /* SE_CREATE_TOKEN_NAME privilege needed to call NtCreateToken. */
  if ((old_priv_state = set_process_privilege (SE_CREATE_TOKEN_NAME)) < 0)
    goto out;

  /* Open policy object. */
  if ((lsa = open_local_policy ()) == INVALID_HANDLE_VALUE)
    goto out;

  /* User, owner, primary group. */
  user.User.Sid = usersid;
  user.User.Attributes = 0;
  owner.Owner = usersid;

  /* Retrieve authentication id and group list from own process. */
  if (!OpenProcessToken (hMainProc, TOKEN_QUERY, &my_token))
    debug_printf ("OpenProcessToken(my_token): %E");
  else
    {
      /* Switching user context to SYSTEM doesn't inherit the authentication
	 id of the user account running current process. */
      if (usersid != well_known_system_sid)
	if (!GetTokenInformation (my_token, TokenStatistics,
				  &stats, sizeof stats, &size))
	  debug_printf
	    ("GetTokenInformation(my_token, TokenStatistics): %E");
	else
	  auth_luid = stats.AuthenticationId;

      /* Retrieving current processes group list to be able to inherit
	 some important well known group sids. */
      if (!GetTokenInformation (my_token, TokenGroups, NULL, 0, &size) &&
	  GetLastError () != ERROR_INSUFFICIENT_BUFFER)
	debug_printf ("GetTokenInformation(my_token, TokenGroups): %E");
      else if (!(my_tok_gsids = (PTOKEN_GROUPS) malloc (size)))
	debug_printf ("malloc (my_tok_gsids) failed.");
      else if (!GetTokenInformation (my_token, TokenGroups, my_tok_gsids,
				     size, &size))
	{
	  debug_printf ("GetTokenInformation(my_token, TokenGroups): %E");
	  free (my_tok_gsids);
	  my_tok_gsids = NULL;
	}
      CloseHandle (my_token);
    }

  /* Create list of groups, the user is member in. */
  int auth_pos;
  if (new_groups.issetgroups ())
    get_setgroups_sidlist (tmp_gsids, my_tok_gsids, new_groups, auth_luid,
			   auth_pos);
  else if (!get_initgroups_sidlist (tmp_gsids, usersid, new_groups.pgsid, pw,
				    my_tok_gsids, auth_luid, auth_pos,
				    special_pgrp))
    goto out;

  /* Primary group. */
  pgrp.PrimaryGroup = new_groups.pgsid;

  /* Create a TOKEN_GROUPS list from the above retrieved list of sids. */
  char grps_buf[sizeof (ULONG) + tmp_gsids.count * sizeof (SID_AND_ATTRIBUTES)];
  new_tok_gsids = (PTOKEN_GROUPS) grps_buf;
  new_tok_gsids->GroupCount = tmp_gsids.count;
  for (DWORD i = 0; i < new_tok_gsids->GroupCount; ++i)
    {
      new_tok_gsids->Groups[i].Sid = tmp_gsids.sids[i];
      new_tok_gsids->Groups[i].Attributes = SE_GROUP_MANDATORY |
	SE_GROUP_ENABLED_BY_DEFAULT | SE_GROUP_ENABLED;
    }
  if (auth_pos >= 0)
    new_tok_gsids->Groups[auth_pos].Attributes |= SE_GROUP_LOGON_ID;

  /* Retrieve list of privileges of that user. */
  if (!(privs = get_priv_list (lsa, usersid, tmp_gsids)))
    goto out;

  /* Create default dacl. */
  if (!sec_acl ((PACL) acl_buf, FALSE,
		tmp_gsids.contains (well_known_admins_sid) ?
		well_known_admins_sid : usersid))
    goto out;
  dacl.DefaultDacl = (PACL) acl_buf;

  /* Let's be heroic... */
  ret = NtCreateToken (&token, TOKEN_ALL_ACCESS, &oa, TokenImpersonation,
		       &auth_luid, &exp, &user, new_tok_gsids, privs, &owner,
		       &pgrp, &dacl, &source);
  if (ret)
    __seterrno_from_win_error (RtlNtStatusToDosError (ret));
  else if (GetLastError () == ERROR_PROC_NOT_FOUND)
    {
      __seterrno ();
      debug_printf ("Loading NtCreateToken failed.");
    }
  else
    {
      /* Set security descriptor and primary group */
      psa = __sec_user (sa_buf, usersid, TRUE);
      if (psa->lpSecurityDescriptor &&
	  !SetSecurityDescriptorGroup ((PSECURITY_DESCRIPTOR)
				       psa->lpSecurityDescriptor,
				       special_pgrp ? new_groups.pgsid
						    : well_known_null_sid,
				       FALSE))
	debug_printf ("SetSecurityDescriptorGroup %E");
      /* Convert to primary token. */
      if (!DuplicateTokenEx (token, MAXIMUM_ALLOWED, psa, SecurityImpersonation,
			     TokenPrimary, &primary_token))
	{
	  __seterrno ();
	  debug_printf ("DuplicateTokenEx %E");
	}
    }

out:
  if (old_priv_state >= 0)
    set_process_privilege (SE_CREATE_TOKEN_NAME, old_priv_state);
  if (token != INVALID_HANDLE_VALUE)
    CloseHandle (token);
  if (privs)
    free (privs);
  if (my_tok_gsids)
    free (my_tok_gsids);
  close_local_policy (lsa);

  debug_printf ("%d = create_token ()", primary_token);
  return primary_token;
}

int subauth_id = 255;

HANDLE
subauth (struct passwd *pw)
{
  LSA_STRING name;
  HANDLE lsa_hdl;
  LSA_OPERATIONAL_MODE sec_mode;
  NTSTATUS ret, ret2;
  ULONG package_id, size;
  struct {
    LSA_STRING str;
    CHAR buf[16];
  } origin;
  struct {
    MSV1_0_LM20_LOGON auth;
    WCHAR dombuf[INTERNET_MAX_HOST_NAME_LENGTH + 1];
    WCHAR usrbuf[UNLEN + 1];
    WCHAR wkstbuf[1];
    CHAR authinf1[1];
    CHAR authinf2[1];
  } subbuf;
  TOKEN_SOURCE ts;
  PMSV1_0_LM20_LOGON_PROFILE profile;
  LUID luid;
  QUOTA_LIMITS quota;
  char nt_domain[INTERNET_MAX_HOST_NAME_LENGTH + 1];
  char nt_user[UNLEN + 1];
  SECURITY_ATTRIBUTES sa = { sizeof sa, NULL, TRUE };
  HANDLE user_token = INVALID_HANDLE_VALUE;
  HANDLE primary_token = INVALID_HANDLE_VALUE;
  int old_tcb_state;

  if ((old_tcb_state = set_process_privilege (SE_TCB_NAME)) < 0)
    return INVALID_HANDLE_VALUE;

  /* Register as logon process. */
  str2lsa (name, "Cygwin");
  SetLastError (0);
  ret = LsaRegisterLogonProcess (&name, &lsa_hdl, &sec_mode);
  if (ret != STATUS_SUCCESS)
    {
      debug_printf ("LsaRegisterLogonProcess: %d", ret);
      __seterrno_from_win_error (LsaNtStatusToWinError (ret));
      goto out;
    }
  else if (GetLastError () == ERROR_PROC_NOT_FOUND)
    {
      debug_printf ("Couldn't load Secur32.dll");
      goto out;
    }
  /* Get handle to MSV1_0 package. */
  str2lsa (name, MSV1_0_PACKAGE_NAME);
  ret = LsaLookupAuthenticationPackage (lsa_hdl, &name, &package_id);
  if (ret != STATUS_SUCCESS)
    {
      debug_printf ("LsaLookupAuthenticationPackage: %d", ret);
      __seterrno_from_win_error (LsaNtStatusToWinError (ret));
      LsaDeregisterLogonProcess (lsa_hdl);
      goto out;
    }
  /* Create origin. */
  str2buf2lsa (origin.str, origin.buf, "Cygwin");
  /* Create token source. */
  memcpy (ts.SourceName, "Cygwin.1", 8);
  ts.SourceIdentifier.HighPart = 0;
  ts.SourceIdentifier.LowPart = 0x0100;
  /* Get user information. */
  extract_nt_dom_user (pw, nt_domain, nt_user);
  /* Fill subauth with values. */
  subbuf.auth.MessageType = MsV1_0NetworkLogon;
  str2buf2uni (subbuf.auth.LogonDomainName, subbuf.dombuf, nt_domain);
  str2buf2uni (subbuf.auth.UserName, subbuf.usrbuf, nt_user);
  str2buf2uni (subbuf.auth.Workstation, subbuf.wkstbuf, "");
  memcpy (subbuf.auth.ChallengeToClient, "12345678", MSV1_0_CHALLENGE_LENGTH);
  str2buf2lsa (subbuf.auth.CaseSensitiveChallengeResponse, subbuf.authinf1, "");
  str2buf2lsa (subbuf.auth.CaseInsensitiveChallengeResponse,subbuf.authinf2,"");
  subbuf.auth.ParameterControl = 0 | (subauth_id << 24);
  /* Try to logon... */
  ret = LsaLogonUser (lsa_hdl, (PLSA_STRING) &origin, Network,
		      package_id, &subbuf, sizeof subbuf,
		      NULL, &ts, (PVOID *) &profile, &size,
		      &luid, &user_token, &quota, &ret2);
  if (ret != STATUS_SUCCESS)
    {
      debug_printf ("LsaLogonUser: %d", ret);
      __seterrno_from_win_error (LsaNtStatusToWinError (ret));
      LsaDeregisterLogonProcess (lsa_hdl);
      goto out;
    }
  LsaFreeReturnBuffer (profile);
  /* Convert to primary token. */
  if (!DuplicateTokenEx (user_token, TOKEN_ALL_ACCESS, &sa,
			 SecurityImpersonation, TokenPrimary, &primary_token))
    __seterrno ();

out:
  set_process_privilege (SE_TCB_NAME, old_tcb_state);
  if (user_token != INVALID_HANDLE_VALUE)
    CloseHandle (user_token);
  return primary_token;
}

/* read_sd reads a security descriptor from a file.
   In case of error, -1 is returned and errno is set.
   If sd_buf is too small, 0 is returned and sd_size
   is set to the needed buffer size.
   On success, 1 is returned.

   GetFileSecurity() is used instead of BackupRead()
   to avoid access denied errors if the caller has
   not the permission to open that file for read.

   Originally the function should return the size
   of the SD on success. Unfortunately NT returns
   0 in `len' on success, while W2K returns the
   correct size!
*/

LONG
read_sd (const char *file, PSECURITY_DESCRIPTOR sd_buf, LPDWORD sd_size)
{
  /* Check parameters */
  if (!sd_size)
    {
      set_errno (EINVAL);
      return -1;
    }

  debug_printf ("file = %s", file);

  DWORD len = 0;
  const char *pfile = file;
  char fbuf[PATH_MAX];
  if (current_codepage == oem_cp)
    {
      DWORD fname_len = min (sizeof (fbuf) - 1, strlen (file));
      bzero (fbuf, sizeof (fbuf));
      OemToCharBuff (file, fbuf, fname_len);
      pfile = fbuf;
    }

  if (!GetFileSecurity (pfile,
			OWNER_SECURITY_INFORMATION
			| GROUP_SECURITY_INFORMATION
			| DACL_SECURITY_INFORMATION,
			sd_buf, *sd_size, &len))
    {
      __seterrno ();
      return -1;
    }
  debug_printf ("file = %s: len=%d", file, len);
  if (len > *sd_size)
    {
      *sd_size = len;
      return 0;
    }
  return 1;
}

LONG
write_sd (const char *file, PSECURITY_DESCRIPTOR sd_buf, DWORD sd_size)
{
  /* Check parameters */
  if (!sd_buf || !sd_size)
    {
      set_errno (EINVAL);
      return -1;
    }

  /* No need to be thread save. */
  static BOOL first_time = TRUE;
  if (first_time)
    {
      set_process_privilege (SE_RESTORE_NAME);
      first_time = FALSE;
    }

  HANDLE fh;
  fh = CreateFile (file,
		   WRITE_OWNER | WRITE_DAC,
		   FILE_SHARE_READ | FILE_SHARE_WRITE,
		   &sec_none_nih,
		   OPEN_EXISTING,
		   FILE_ATTRIBUTE_NORMAL | FILE_FLAG_BACKUP_SEMANTICS,
		   NULL);

  if (fh == INVALID_HANDLE_VALUE)
    {
      __seterrno ();
      return -1;
    }

  LPVOID context = NULL;
  DWORD bytes_written = 0;
  WIN32_STREAM_ID header;

  memset (&header, 0, sizeof (header));
  /* write new security info header */
  header.dwStreamId = BACKUP_SECURITY_DATA;
  header.dwStreamAttributes = STREAM_CONTAINS_SECURITY;
  header.Size.HighPart = 0;
  header.Size.LowPart = sd_size;
  header.dwStreamNameSize = 0;
  if (!BackupWrite (fh, (LPBYTE) &header,
		    3 * sizeof (DWORD) + sizeof (LARGE_INTEGER),
		    &bytes_written, FALSE, TRUE, &context))
    {
      __seterrno ();
      CloseHandle (fh);
      return -1;
    }

  /* write new security descriptor */
  if (!BackupWrite (fh, (LPBYTE) sd_buf,
		    header.Size.LowPart + header.dwStreamNameSize,
		    &bytes_written, FALSE, TRUE, &context))
    {
      /* Samba returns ERROR_NOT_SUPPORTED.
	 FAT returns ERROR_INVALID_SECURITY_DESCR.
	 This shouldn't return as error, but better be ignored. */
      DWORD ret = GetLastError ();
      if (ret != ERROR_NOT_SUPPORTED && ret != ERROR_INVALID_SECURITY_DESCR)
	{
	  __seterrno ();
	  BackupWrite (fh, NULL, 0, &bytes_written, TRUE, TRUE, &context);
	  CloseHandle (fh);
	  return -1;
	}
    }

  /* terminate the restore process */
  BackupWrite (fh, NULL, 0, &bytes_written, TRUE, TRUE, &context);
  CloseHandle (fh);
  return 0;
}

static void
get_attribute_from_acl (int * attribute, PACL acl, PSID owner_sid,
			PSID group_sid, BOOL grp_member)
{
  ACCESS_ALLOWED_ACE *ace;
  int allow = 0;
  int deny = 0;
  int *flags, *anti;

  for (DWORD i = 0; i < acl->AceCount; ++i)
    {
      if (!GetAce (acl, i, (PVOID *) &ace))
	continue;
      if (ace->Header.AceFlags & INHERIT_ONLY)
	continue;
      switch (ace->Header.AceType)
	{
	case ACCESS_ALLOWED_ACE_TYPE:
	  flags = &allow;
	  anti = &deny;
	  break;
	case ACCESS_DENIED_ACE_TYPE:
	  flags = &deny;
	  anti = &allow;
	  break;
	default:
	  continue;
	}

      cygsid ace_sid ((PSID) &ace->SidStart);
      if (ace_sid == well_known_world_sid)
	{
	  if (ace->Mask & FILE_READ_DATA)
	    *flags |= ((!(*anti & S_IROTH)) ? S_IROTH : 0)
		      | ((!(*anti & S_IRGRP)) ? S_IRGRP : 0)
		      | ((!(*anti & S_IRUSR)) ? S_IRUSR : 0);
	  if (ace->Mask & FILE_WRITE_DATA)
	    *flags |= ((!(*anti & S_IWOTH)) ? S_IWOTH : 0)
		      | ((!(*anti & S_IWGRP)) ? S_IWGRP : 0)
		      | ((!(*anti & S_IWUSR)) ? S_IWUSR : 0);
	  if (ace->Mask & FILE_EXECUTE)
	    *flags |= ((!(*anti & S_IXOTH)) ? S_IXOTH : 0)
	              | ((!(*anti & S_IXGRP)) ? S_IXGRP : 0)
	              | ((!(*anti & S_IXUSR)) ? S_IXUSR : 0);
	  if ((*attribute & S_IFDIR) &&
	      (ace->Mask & (FILE_WRITE_DATA | FILE_EXECUTE | FILE_DELETE_CHILD))
	      == (FILE_WRITE_DATA | FILE_EXECUTE))
	    *flags |= S_ISVTX;
	}
      else if (ace_sid == well_known_null_sid)
	{
	  /* Read SUID, SGID and VTX bits from NULL ACE. */
	  if (ace->Mask & FILE_READ_DATA)
	    *flags |= S_ISVTX;
	  if (ace->Mask & FILE_WRITE_DATA)
	    *flags |= S_ISGID;
	  if (ace->Mask & FILE_APPEND_DATA)
	    *flags |= S_ISUID;
	}
      else if (ace_sid == owner_sid)
	{
	  if (ace->Mask & FILE_READ_DATA)
	    *flags |= ((!(*anti & S_IRUSR)) ? S_IRUSR : 0);
	  if (ace->Mask & FILE_WRITE_DATA)
	    *flags |= ((!(*anti & S_IWUSR)) ? S_IWUSR : 0);
	  if (ace->Mask & FILE_EXECUTE)
	    *flags |= ((!(*anti & S_IXUSR)) ? S_IXUSR : 0);
	}
      else if (ace_sid == group_sid)
	{
	  if (ace->Mask & FILE_READ_DATA)
	    *flags |= ((!(*anti & S_IRGRP)) ? S_IRGRP : 0)
		      | ((grp_member && !(*anti & S_IRUSR)) ? S_IRUSR : 0);
	  if (ace->Mask & FILE_WRITE_DATA)
	    *flags |= ((!(*anti & S_IWGRP)) ? S_IWGRP : 0)
		      | ((grp_member && !(*anti & S_IWUSR)) ? S_IWUSR : 0);
	  if (ace->Mask & FILE_EXECUTE)
	    *flags |= ((!(*anti & S_IXGRP)) ? S_IXGRP : 0)
		      | ((grp_member && !(*anti & S_IXUSR)) ? S_IXUSR : 0);
	}
    }
  *attribute &= ~(S_IRWXU | S_IRWXG | S_IRWXO | S_ISVTX | S_ISGID | S_ISUID);
  if (owner_sid && group_sid && EqualSid (owner_sid, group_sid)
      /* FIXME: temporary exception for /var/empty */
      && well_known_system_sid != group_sid)
    {
      allow &= ~(S_IRGRP | S_IWGRP | S_IXGRP);
      allow |= (((allow & S_IRUSR) ? S_IRGRP : 0)
		| ((allow & S_IWUSR) ? S_IWGRP : 0)
		| ((allow & S_IXUSR) ? S_IXGRP : 0));
    }
  *attribute |= allow;
  return;
}

static int
get_nt_attribute (const char *file, int *attribute,
		  __uid32_t *uidret, __gid32_t *gidret)
{
  if (!wincap.has_security ())
    return 0;

  syscall_printf ("file: %s", file);

  /* Yeah, sounds too much, but I've seen SDs of 2100 bytes! */
  DWORD sd_size = 4096;
  char sd_buf[4096];
  PSECURITY_DESCRIPTOR psd = (PSECURITY_DESCRIPTOR) sd_buf;

  int ret;
  if ((ret = read_sd (file, psd, &sd_size)) <= 0)
    {
      debug_printf ("read_sd %E");
      return ret;
    }

  PSID owner_sid;
  PSID group_sid;
  BOOL dummy;

  if (!GetSecurityDescriptorOwner (psd, &owner_sid, &dummy))
    debug_printf ("GetSecurityDescriptorOwner %E");
  if (!GetSecurityDescriptorGroup (psd, &group_sid, &dummy))
    debug_printf ("GetSecurityDescriptorGroup %E");

  PACL acl;
  BOOL acl_exists;

  if (!GetSecurityDescriptorDacl (psd, &acl_exists, &acl, &dummy))
    {
      __seterrno ();
      debug_printf ("GetSecurityDescriptorDacl %E");
      return -1;
    }

  __uid32_t uid = cygsid (owner_sid).get_uid ();
  __gid32_t gid = cygsid (group_sid).get_gid ();
  if (uidret)
    *uidret = uid;
  if (gidret)
    *gidret = gid;

  if (!attribute)
    {
      syscall_printf ("file: %s uid %d, gid %d", file, uid, gid);
      return 0;
    }

  BOOL grp_member = is_grp_member (uid, gid);

  if (!acl_exists || !acl)
    {
      *attribute |= S_IRWXU | S_IRWXG | S_IRWXO;
      syscall_printf ("file: %s No ACL = %x, uid %d, gid %d",
		      file, *attribute, uid, gid);
      return 0;
    }
  get_attribute_from_acl (attribute, acl, owner_sid, group_sid, grp_member);

  syscall_printf ("file: %s %x, uid %d, gid %d", file, *attribute, uid, gid);
  return 0;
}

int
get_file_attribute (int use_ntsec, const char *file,
		    int *attribute, __uid32_t *uidret, __gid32_t *gidret)
{
  int res;

  if (use_ntsec && allow_ntsec)
    {
      res = get_nt_attribute (file, attribute, uidret, gidret);
      if (attribute && (*attribute & S_IFLNK) == S_IFLNK)
	*attribute |= S_IRWXU | S_IRWXG | S_IRWXO;
      return res;
    }

  if (uidret)
    *uidret = getuid32 ();
  if (gidret)
    *gidret = getgid32 ();

  if (!attribute)
    return 0;

  if (allow_ntea)
    {
      int oatt = *attribute;
      res = NTReadEA (file, ".UNIXATTR", (char *)attribute, sizeof (*attribute));
      *attribute |= oatt;
    }
  else
    res = 0;

  /* symlinks are everything for everyone! */
  if ((*attribute & S_IFLNK) == S_IFLNK)
    *attribute |= S_IRWXU | S_IRWXG | S_IRWXO;

  return res > 0 ? 0 : -1;
}

static int
get_nt_object_attribute (HANDLE handle, SE_OBJECT_TYPE object_type,
			 int *attribute, __uid32_t *uidret, __gid32_t *gidret)
{
  if (!wincap.has_security ())
    return 0;

  PSECURITY_DESCRIPTOR psd = NULL;
  PSID owner_sid;
  PSID group_sid;
  PACL acl;

  if (ERROR_SUCCESS != GetSecurityInfo (handle, object_type,
					DACL_SECURITY_INFORMATION |
					GROUP_SECURITY_INFORMATION |
					OWNER_SECURITY_INFORMATION,
					&owner_sid, &group_sid,
					&acl, NULL, &psd))
    {
      __seterrno ();
      debug_printf ("GetSecurityInfo %E");
      return -1;
    }

  __uid32_t uid = cygsid (owner_sid).get_uid ();
  __gid32_t gid = cygsid (group_sid).get_gid ();
  if (uidret)
    *uidret = uid;
  if (gidret)
    *gidret = gid;

  if (!attribute)
    {
      syscall_printf ("uid %d, gid %d", uid, gid);
      LocalFree (psd);
      return 0;
    }

  BOOL grp_member = is_grp_member (uid, gid);

  if (!acl)
    {
      *attribute |= S_IRWXU | S_IRWXG | S_IRWXO;
      syscall_printf ("No ACL = %x, uid %d, gid %d", *attribute, uid, gid);
      LocalFree (psd);
      return 0;
    }

  get_attribute_from_acl (attribute, acl, owner_sid, group_sid, grp_member);

  LocalFree (psd);

  syscall_printf ("%x, uid %d, gid %d", *attribute, uid, gid);
  return 0;
}

int
get_object_attribute (HANDLE handle, SE_OBJECT_TYPE object_type,
		      int *attribute, __uid32_t *uidret, __gid32_t *gidret)
{
  if (allow_ntsec)
    {
      int res = get_nt_object_attribute (handle, object_type, attribute,
					 uidret, gidret);
      if (attribute && (*attribute & S_IFLNK) == S_IFLNK)
	*attribute |= S_IRWXU | S_IRWXG | S_IRWXO;
      return res;
    }

  if (uidret)
    *uidret = getuid32 ();
  if (gidret)
    *gidret = getgid32 ();

  if (!attribute)
    return 0;

  /* symlinks are everything for everyone! */
  if ((*attribute & S_IFLNK) == S_IFLNK)
    *attribute |= S_IRWXU | S_IRWXG | S_IRWXO;

  return 0;
}

BOOL
add_access_allowed_ace (PACL acl, int offset, DWORD attributes,
			PSID sid, size_t &len_add, DWORD inherit)
{
  if (!AddAccessAllowedAce (acl, ACL_REVISION, attributes, sid))
    {
      __seterrno ();
      return FALSE;
    }
  ACCESS_ALLOWED_ACE *ace;
  if (inherit && GetAce (acl, offset, (PVOID *) &ace))
    ace->Header.AceFlags |= inherit;
  len_add += sizeof (ACCESS_ALLOWED_ACE) - sizeof (DWORD) + GetLengthSid (sid);
  return TRUE;
}

BOOL
add_access_denied_ace (PACL acl, int offset, DWORD attributes,
		       PSID sid, size_t &len_add, DWORD inherit)
{
  if (!AddAccessDeniedAce (acl, ACL_REVISION, attributes, sid))
    {
      __seterrno ();
      return FALSE;
    }
  ACCESS_DENIED_ACE *ace;
  if (inherit && GetAce (acl, offset, (PVOID *) &ace))
    ace->Header.AceFlags |= inherit;
  len_add += sizeof (ACCESS_DENIED_ACE) - sizeof (DWORD) + GetLengthSid (sid);
  return TRUE;
}

PSECURITY_DESCRIPTOR
alloc_sd (__uid32_t uid, __gid32_t gid, int attribute,
	  PSECURITY_DESCRIPTOR sd_ret, DWORD *sd_size_ret)
{
  BOOL dummy;

  debug_printf("uid %d, gid %d, attribute %x", uid, gid, attribute);
  if (!sd_ret || !sd_size_ret)
    {
      set_errno (EINVAL);
      return NULL;
    }

  /* Get owner and group from current security descriptor. */
  PSID cur_owner_sid = NULL;
  PSID cur_group_sid = NULL;
  if (!GetSecurityDescriptorOwner (sd_ret, &cur_owner_sid, &dummy))
    debug_printf ("GetSecurityDescriptorOwner %E");
  if (!GetSecurityDescriptorGroup (sd_ret, &cur_group_sid, &dummy))
    debug_printf ("GetSecurityDescriptorGroup %E");

  /* Get SID of owner. */
  cygsid owner_sid (NO_SID);
  /* Check for current user first */
  if (uid == myself->uid)
    owner_sid = cygheap->user.sid ();
  else if (uid == ILLEGAL_UID)
    owner_sid = cur_owner_sid;
  else if (!owner_sid.getfrompw (internal_getpwuid (uid)))
    {
      set_errno (EINVAL);
      return NULL;
    }
  owner_sid.debug_print ("alloc_sd: owner SID =");

  /* Get SID of new group. */
  cygsid group_sid (NO_SID);
  /* Check for current user first */
  if (gid == myself->gid)
    group_sid = cygheap->user.groups.pgsid;
  else if (gid == ILLEGAL_GID)
    group_sid = cur_group_sid;
  else if (!group_sid.getfromgr (internal_getgrgid (gid)))
    {
      set_errno (EINVAL);
      return NULL;
    }
  group_sid.debug_print ("alloc_sd: group SID =");

  /* Initialize local security descriptor. */
  SECURITY_DESCRIPTOR sd;
  PSECURITY_DESCRIPTOR psd = NULL;
  if (!InitializeSecurityDescriptor (&sd, SECURITY_DESCRIPTOR_REVISION))
    {
      __seterrno ();
      return NULL;
    }

  /*
   * We set the SE_DACL_PROTECTED flag here to prevent the DACL from being
   * modified by inheritable ACEs.
   * This flag as well as the SetSecurityDescriptorControl call are available
   * only since Win2K.
   */
  if (wincap.has_security_descriptor_control ())
    SetSecurityDescriptorControl (&sd, SE_DACL_PROTECTED, SE_DACL_PROTECTED);

  /* Create owner for local security descriptor. */
  if (!SetSecurityDescriptorOwner (&sd, owner_sid, FALSE))
    {
      __seterrno ();
      return NULL;
    }

  /* Create group for local security descriptor. */
  if (!SetSecurityDescriptorGroup (&sd, group_sid, FALSE))
    {
      __seterrno ();
      return NULL;
    }

  /* Initialize local access control list. */
  char acl_buf[3072];
  PACL acl = (PACL) acl_buf;
  if (!InitializeAcl (acl, 3072, ACL_REVISION))
    {
      __seterrno ();
      return NULL;
    }

  /* From here fill ACL. */
  size_t acl_len = sizeof (ACL);
  int ace_off = 0;

  /* Construct allow attribute for owner. */
  DWORD owner_allow = (STANDARD_RIGHTS_ALL & ~DELETE)
		      | FILE_WRITE_ATTRIBUTES | FILE_WRITE_EA;
  if (attribute & S_IRUSR)
    owner_allow |= FILE_GENERIC_READ;
  if (attribute & S_IWUSR)
    owner_allow |= FILE_GENERIC_WRITE | DELETE;
  if (attribute & S_IXUSR)
    owner_allow |= FILE_GENERIC_EXECUTE;
  if ((attribute & (S_IFDIR | S_IWUSR | S_IXUSR))
      == (S_IFDIR | S_IWUSR | S_IXUSR))
    owner_allow |= FILE_DELETE_CHILD;

  /* Construct allow attribute for group. */
  DWORD group_allow = STANDARD_RIGHTS_READ
		      | FILE_READ_ATTRIBUTES | FILE_READ_EA;
  if (attribute & S_IRGRP)
    group_allow |= FILE_GENERIC_READ;
  if (attribute & S_IWGRP)
    group_allow |= STANDARD_RIGHTS_WRITE | FILE_GENERIC_WRITE;
  if (attribute & S_IXGRP)
    group_allow |= FILE_GENERIC_EXECUTE;
  if ((attribute & (S_IFDIR | S_IWGRP | S_IXGRP))
      == (S_IFDIR | S_IWGRP | S_IXGRP) && !(attribute & S_ISVTX))
    group_allow |= FILE_DELETE_CHILD;

  /* Construct allow attribute for everyone. */
  DWORD other_allow = STANDARD_RIGHTS_READ
		      | FILE_READ_ATTRIBUTES | FILE_READ_EA;
  if (attribute & S_IROTH)
    other_allow |= FILE_GENERIC_READ;
  if (attribute & S_IWOTH)
    other_allow |= STANDARD_RIGHTS_WRITE | FILE_GENERIC_WRITE;
  if (attribute & S_IXOTH)
    other_allow |= FILE_GENERIC_EXECUTE;
  if ((attribute & (S_IFDIR | S_IWOTH | S_IXOTH))
      == (S_IFDIR | S_IWOTH | S_IXOTH)
      && !(attribute & S_ISVTX))
    other_allow |= FILE_DELETE_CHILD;

  /* Construct SUID, SGID and VTX bits in NULL ACE. */
  DWORD null_allow = 0L;
  if (attribute & (S_ISUID | S_ISGID | S_ISVTX))
    {
      if (attribute & S_ISUID)
	null_allow |= FILE_APPEND_DATA;
      if (attribute & S_ISGID)
	null_allow |= FILE_WRITE_DATA;
      if (attribute & S_ISVTX)
	null_allow |= FILE_READ_DATA;
    }

  /* Add owner and group permissions if SIDs are equal
     and construct deny attributes for group and owner. */
  DWORD group_deny;
  if (owner_sid == group_sid)
    {
      owner_allow |= group_allow;
      group_allow = group_deny = 0L;
    }
  else
    {
      group_deny = ~group_allow & other_allow;
      group_deny &= ~(STANDARD_RIGHTS_READ
		      | FILE_READ_ATTRIBUTES | FILE_READ_EA);
    }
  DWORD owner_deny = ~owner_allow & (group_allow | other_allow);
  owner_deny &= ~(STANDARD_RIGHTS_READ
		  | FILE_READ_ATTRIBUTES | FILE_READ_EA
		  | FILE_WRITE_ATTRIBUTES | FILE_WRITE_EA);

  /* Construct appropriate inherit attribute. */
  DWORD inherit = (attribute & S_IFDIR) ? SUB_CONTAINERS_AND_OBJECTS_INHERIT
					: NO_INHERITANCE;

  /* Set deny ACE for owner. */
  if (owner_deny
      && !add_access_denied_ace (acl, ace_off++, owner_deny,
				 owner_sid, acl_len, inherit))
    return NULL;
  /* Set deny ACE for group here to respect the canonical order,
     if this does not impact owner */
  if (group_deny && !(owner_allow & group_deny))
    {
      if (!add_access_denied_ace (acl, ace_off++, group_deny,
				 group_sid, acl_len, inherit))
	return NULL;
      group_deny = 0;
    }
  /* Set allow ACE for owner. */
  if (!add_access_allowed_ace (acl, ace_off++, owner_allow,
			       owner_sid, acl_len, inherit))
    return NULL;
  /* Set deny ACE for group, if still needed. */
  if (group_deny
      && !add_access_denied_ace (acl, ace_off++, group_deny,
				 group_sid, acl_len, inherit))
    return NULL;
  /* Set allow ACE for group. */
  if (group_allow
      && !add_access_allowed_ace (acl, ace_off++, group_allow,
				  group_sid, acl_len, inherit))
    return NULL;

  /* Set allow ACE for everyone. */
  if (!add_access_allowed_ace (acl, ace_off++, other_allow,
			       well_known_world_sid, acl_len, inherit))
    return NULL;
  /* Set null ACE for special bits. */
  if (null_allow
      && !add_access_allowed_ace (acl, ace_off++, null_allow,
				  well_known_null_sid, acl_len, NO_INHERITANCE))
    return NULL;

  /* Fill ACL with unrelated ACEs from current security descriptor. */
  PACL oacl;
  BOOL acl_exists;
  ACCESS_ALLOWED_ACE *ace;
  if (GetSecurityDescriptorDacl (sd_ret, &acl_exists, &oacl, &dummy)
      && acl_exists && oacl)
    for (DWORD i = 0; i < oacl->AceCount; ++i)
      if (GetAce (oacl, i, (PVOID *) &ace))
	{
	  cygsid ace_sid ((PSID) &ace->SidStart);
	  /* Check for related ACEs. */
	  if ((ace_sid == cur_owner_sid)
	      || (ace_sid == owner_sid)
	      || (ace_sid == cur_group_sid)
	      || (ace_sid == group_sid)
	      || (ace_sid == well_known_world_sid)
	      || (ace_sid == well_known_null_sid))
	    continue;
	  /*
	   * Add unrelated ACCESS_DENIED_ACE to the beginning but
	   * behind the owner_deny, ACCESS_ALLOWED_ACE to the end.
	   */
	  if (!AddAce (acl, ACL_REVISION,
		       ace->Header.AceType == ACCESS_DENIED_ACE_TYPE ?
		       (owner_deny ? 1 : 0) : MAXDWORD,
		       (LPVOID) ace, ace->Header.AceSize))
	    {
	      __seterrno ();
	      return NULL;
	    }
	  acl_len += ace->Header.AceSize;
	}

  /* Set AclSize to computed value. */
  acl->AclSize = acl_len;
  debug_printf ("ACL-Size: %d", acl_len);

  /* Create DACL for local security descriptor. */
  if (!SetSecurityDescriptorDacl (&sd, TRUE, acl, FALSE))
    {
      __seterrno ();
      return NULL;
    }

  /* Make self relative security descriptor. */
  *sd_size_ret = 0;
  MakeSelfRelativeSD (&sd, sd_ret, sd_size_ret);
  if (*sd_size_ret <= 0)
    {
      __seterrno ();
      return NULL;
    }
  if (!MakeSelfRelativeSD (&sd, sd_ret, sd_size_ret))
    {
      __seterrno ();
      return NULL;
    }
  psd = sd_ret;
  debug_printf ("Created SD-Size: %d", *sd_size_ret);

  return psd;
}

void
set_security_attribute (int attribute, PSECURITY_ATTRIBUTES psa,
			void *sd_buf, DWORD sd_buf_size)
{
  /* symlinks are anything for everyone! */
  if ((attribute & S_IFLNK) == S_IFLNK)
    attribute |= S_IRWXU | S_IRWXG | S_IRWXO;

  psa->lpSecurityDescriptor = sd_buf;
  InitializeSecurityDescriptor ((PSECURITY_DESCRIPTOR) sd_buf,
				SECURITY_DESCRIPTOR_REVISION);
  psa->lpSecurityDescriptor = alloc_sd (geteuid32 (), getegid32 (), attribute,
					(PSECURITY_DESCRIPTOR) sd_buf,
					&sd_buf_size);
}

static int
set_nt_attribute (const char *file, __uid32_t uid, __gid32_t gid,
		  int attribute)
{
  if (!wincap.has_security ())
    return 0;

  DWORD sd_size = 4096;
  char sd_buf[4096];
  PSECURITY_DESCRIPTOR psd = (PSECURITY_DESCRIPTOR) sd_buf;

  int ret;
  if ((ret = read_sd (file, psd, &sd_size)) <= 0)
    {
      debug_printf ("read_sd %E");
      return -1;
    }

  sd_size = 4096;
  if (!(psd = alloc_sd (uid, gid, attribute, psd, &sd_size)))
    return -1;

  return write_sd (file, psd, sd_size);
}

int
set_file_attribute (int use_ntsec, const char *file,
		    __uid32_t uid, __gid32_t gid, int attribute)
{
  int ret = 0;

  if (use_ntsec && allow_ntsec)
    ret = set_nt_attribute (file, uid, gid, attribute);
  else if (allow_ntea && !NTWriteEA (file, ".UNIXATTR", (char *) &attribute,
				     sizeof (attribute)))
    {
      __seterrno ();
      ret = -1;
    }
  syscall_printf ("%d = set_file_attribute (%s, %d, %d, %p)",
		  ret, file, uid, gid, attribute);
  return ret;
}

int
set_file_attribute (int use_ntsec, const char *file, int attribute)
{
  return set_file_attribute (use_ntsec, file,
			     myself->uid, myself->gid, attribute);
}
