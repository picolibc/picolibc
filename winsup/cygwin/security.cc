/* security.cc: NT security functions

   Copyright 1997, 1998, 1999, 2000, 2001 Cygnus Solutions.

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
#include "cygerrno.h"
#include "perprocess.h"
#include "fhandler.h"
#include "path.h"
#include "dtable.h"
#include "sync.h"
#include "sigproc.h"
#include "pinfo.h"
#include "cygheap.h"
#include "security.h"
#include <ntdef.h>
#include "ntdll.h"
#include "lm.h"


extern BOOL allow_ntea;
BOOL allow_ntsec = FALSE;
/* allow_smbntsec is handled exclusively in path.cc (path_conv::check).
   It's defined here because of it's strong relationship to allow_ntsec.
   The default is TRUE to reflect the old behaviour. */
BOOL allow_smbntsec = TRUE;

extern "C"
void
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
  cygsid psid;
  DWORD ulen = UNLEN + 1;
  DWORD dlen = INTERNET_MAX_HOST_NAME_LENGTH + 1;
  SID_NAME_USE use;
  char buf[INTERNET_MAX_HOST_NAME_LENGTH + UNLEN + 2];
  char *c;

  strcpy (domain, "");
  strcpy (buf, pw->pw_name);
  debug_printf ("pw_gecos = %x (%s)", pw->pw_gecos, pw->pw_gecos);

  if (psid.getfrompw (pw) &&
      LookupAccountSid (NULL, psid, user, &ulen, domain, &dlen, &use))
    return;

  if (pw->pw_gecos)
    {
      if ((c = strstr (pw->pw_gecos, "U-")) != NULL &&
          (c == pw->pw_gecos || c[-1] == ','))
	{
	  buf[0] = '\0';
	  strncat (buf, c + 2, INTERNET_MAX_HOST_NAME_LENGTH + UNLEN + 1);
	  if ((c = strchr (buf, ',')) != NULL)
	    *c = '\0';
	}
    }
  if ((c = strchr (buf, '\\')) != NULL)
    {
      *c++ = '\0';
      strcpy (domain, buf);
      strcpy (user, c);
    }
  else
    {
      strcpy (domain, "");
      strcpy (user, buf);
    }
}

extern "C"
HANDLE
cygwin_logon_user (const struct passwd *pw, const char *password)
{
  if (os_being_run != winNT)
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
  memcpy(buf, srcstr, tgt.MaximumLength);
}

static void
str2buf2uni (UNICODE_STRING &tgt, WCHAR *buf, const char *srcstr)
{
  tgt.Length = strlen (srcstr) * sizeof (WCHAR);
  tgt.MaximumLength = tgt.Length + sizeof(WCHAR);
  tgt.Buffer = (PWCHAR) buf;
  sys_mbstowcs (buf, srcstr, tgt.MaximumLength);
}

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

static LSA_HANDLE
open_local_policy ()
{
  LSA_OBJECT_ATTRIBUTES oa = { 0, 0, 0, 0, 0, 0 };
  LSA_HANDLE lsa = INVALID_HANDLE_VALUE;

  NTSTATUS ret = LsaOpenPolicy(NULL, &oa, POLICY_ALL_ACCESS, &lsa);
  if (ret != STATUS_SUCCESS)
    set_errno (LsaNtStatusToWinError (ret));
  return lsa;
}

static void
close_local_policy (LSA_HANDLE &lsa)
{
  if (lsa != INVALID_HANDLE_VALUE)
    LsaClose (lsa);
  lsa = INVALID_HANDLE_VALUE;
}

static BOOL
get_lsa_srv_inf (LSA_HANDLE lsa, char *logonserver, char *domain)
{
  NET_API_STATUS ret;
  LPSERVER_INFO_101 buf;
  DWORD cnt, tot;
  char name[INTERNET_MAX_HOST_NAME_LENGTH + 1];
  WCHAR account[INTERNET_MAX_HOST_NAME_LENGTH + 1];
  WCHAR primary[INTERNET_MAX_HOST_NAME_LENGTH + 1];
  PPOLICY_ACCOUNT_DOMAIN_INFO adi;
  PPOLICY_PRIMARY_DOMAIN_INFO pdi;

  if ((ret = LsaQueryInformationPolicy (lsa, PolicyAccountDomainInformation,
                                        (PVOID *) &adi)) != STATUS_SUCCESS)
    {
      set_errno (LsaNtStatusToWinError(ret));
      return FALSE;
    }
  lsa2wchar (account, adi->DomainName, INTERNET_MAX_HOST_NAME_LENGTH + 1);
  LsaFreeMemory (adi);
  if ((ret = LsaQueryInformationPolicy (lsa, PolicyPrimaryDomainInformation,
                                        (PVOID *) &pdi)) != STATUS_SUCCESS)
    {
      set_errno (LsaNtStatusToWinError(ret));
      return FALSE;
    }
  lsa2wchar (primary, pdi->Name, INTERNET_MAX_HOST_NAME_LENGTH + 1);
  LsaFreeMemory (pdi);
  if ((ret = NetServerEnum (NULL, 101, (LPBYTE *) &buf, MAX_PREFERRED_LENGTH,
                            &cnt, &tot, SV_TYPE_DOMAIN_CTRL, primary, NULL))
      == STATUS_SUCCESS && cnt > 0)
    {
      sys_wcstombs (name, buf[0].sv101_name, INTERNET_MAX_HOST_NAME_LENGTH + 1);
      if (domain)
        sys_wcstombs (domain, primary, INTERNET_MAX_HOST_NAME_LENGTH + 1);
    }
  else
    {
      sys_wcstombs (name, account, INTERNET_MAX_HOST_NAME_LENGTH + 1);
      if (domain)
        sys_wcstombs (domain, account, INTERNET_MAX_HOST_NAME_LENGTH + 1);
    }
  if (ret == STATUS_SUCCESS)
    NetApiBufferFree (buf);
  strcpy (logonserver, "\\\\");
  strcat (logonserver, name);
  return TRUE;
}

static BOOL
get_logon_server (LSA_HANDLE lsa, char *logonserver)
{
  return get_lsa_srv_inf (lsa, logonserver, NULL);
}

BOOL
get_logon_server_and_user_domain (char *logonserver, char *userdomain)
{
  BOOL ret = FALSE;
  LSA_HANDLE lsa = open_local_policy ();
  if (lsa)
    {
      ret = get_lsa_srv_inf (lsa, logonserver, userdomain);
      close_local_policy (lsa);
    }
  return ret;
}

static BOOL
get_user_groups (WCHAR *wlogonserver, cygsidlist &grp_list, char *user)
{
  WCHAR wuser[UNLEN + 1]; 
  sys_mbstowcs (wuser, user, UNLEN + 1);
  LPGROUP_USERS_INFO_0 buf; 
  DWORD cnt, tot;
  NET_API_STATUS ret;

  if ((ret = NetUserGetGroups (wlogonserver, wuser, 0, (LPBYTE *) &buf,
			       MAX_PREFERRED_LENGTH, &cnt, &tot)))
    {
      debug_printf ("%d = NetUserGetGroups ()", ret);
      set_errno (ret);
      /* It's no error when the user name can't be found. */
      return ret == NERR_UserNotFound;
    }

  for (DWORD i = 0; i < cnt; ++i)
    {
      cygsid gsid;
      char group[UNLEN + 1];
      char domain[INTERNET_MAX_HOST_NAME_LENGTH + 1];
      DWORD glen = UNLEN + 1;
      DWORD dlen = INTERNET_MAX_HOST_NAME_LENGTH + 1;
      SID_NAME_USE use = SidTypeInvalid;

      sys_wcstombs (group, buf[i].grui0_name, UNLEN + 1);
      if (!LookupAccountName (NULL, group, gsid, &glen, domain, &dlen, &use))
        debug_printf ("LookupAccountName(%s): %lu\n", group, GetLastError ());
      if (!legal_sid_type (use))
        {
          strcat (strcpy (group, domain), "\\");
          sys_wcstombs (group + strlen (group), buf[i].grui0_name,
                        UNLEN + 1 - strlen (group));
          glen = UNLEN + 1;
          dlen = INTERNET_MAX_HOST_NAME_LENGTH + 1;
          if (!LookupAccountName(NULL, group, gsid, &glen, domain, &dlen, &use))
            debug_printf ("LookupAccountName(%s): %lu\n", group,GetLastError());
        }
      if (legal_sid_type (use))
        grp_list += gsid;
    }

  NetApiBufferFree (buf);
  return TRUE;
}

static BOOL
is_group_member (WCHAR *wlogonserver, WCHAR *wgroup,
                 cygsid &usersid, cygsidlist &grp_list)
{
  LPLOCALGROUP_MEMBERS_INFO_0 buf;
  DWORD cnt, tot;
  BOOL ret = FALSE;

  if (NetLocalGroupGetMembers (wlogonserver, wgroup, 0, (LPBYTE *) &buf,
                               MAX_PREFERRED_LENGTH, &cnt, &tot, NULL))
    return FALSE;

  for (DWORD bidx = 0; !ret && bidx < cnt; ++bidx)
    if (EqualSid (usersid, buf[bidx].lgrmi0_sid))
      ret = TRUE;
    else
      for (int glidx = 0; !ret && glidx < grp_list.count; ++glidx)
	if (EqualSid (grp_list.sids[glidx], buf[bidx].lgrmi0_sid))
	  ret = TRUE;

  NetApiBufferFree (buf);
  return ret;
}

static BOOL
get_user_local_groups (WCHAR *wlogonserver, const char *logonserver,
		       cygsidlist &grp_list, cygsid &usersid)
{
  LPLOCALGROUP_INFO_0 buf;
  DWORD cnt, tot;
  NET_API_STATUS ret;

  if ((ret = NetLocalGroupEnum (wlogonserver, 0, (LPBYTE *) &buf,
			        MAX_PREFERRED_LENGTH, &cnt, &tot, NULL)))
    {
      debug_printf ("%d = NetLocalGroupEnum ()", ret);
      set_errno (ret);
      return FALSE;
    }

  for (DWORD i = 0; i < cnt; ++i)
    if (is_group_member (wlogonserver, buf[i].lgrpi0_name, usersid, grp_list))
      {
	cygsid gsid;
	char group[UNLEN + 1];
	char domain[INTERNET_MAX_HOST_NAME_LENGTH + 1];
	DWORD glen = UNLEN + 1;
	DWORD dlen = INTERNET_MAX_HOST_NAME_LENGTH + 1;
	SID_NAME_USE use = SidTypeInvalid;

	sys_wcstombs (group, buf[i].lgrpi0_name, UNLEN + 1);
	if (!LookupAccountName (NULL, group, gsid, &glen, domain, &dlen, &use))
	  {
	    glen = UNLEN + 1;
	    dlen = INTERNET_MAX_HOST_NAME_LENGTH + 1;
	    if (!LookupAccountName (logonserver + 2, group,
				    gsid, &glen, domain, &dlen, &use))
	      debug_printf ("LookupAccountName(%s): %lu\n", group,
							    GetLastError ());
	  }
	else if (!legal_sid_type (use))
	  {
	    strcat (strcpy (group, domain), "\\");
	    sys_wcstombs (group + strlen (group), buf[i].lgrpi0_name,
		          UNLEN + 1 - strlen (group));
	    glen = UNLEN + 1;
	    dlen = INTERNET_MAX_HOST_NAME_LENGTH + 1;
	    if (!LookupAccountName (NULL, group, gsid, &glen,
				    domain, &dlen, &use))
	      debug_printf ("LookupAccountName(%s): %lu\n", group,
							    GetLastError ());
	  }
	if (legal_sid_type (use))
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

static BOOL
get_user_primary_group (WCHAR *wlogonserver, const char *user,
			cygsid &usersid, cygsid &pgrpsid)
{
  LPUSER_INFO_3 buf;
  WCHAR wuser[UNLEN + 1];
  BOOL ret = FALSE;
  UCHAR count;

  if (usersid == well_known_system_sid)
    {
      pgrpsid = well_known_system_sid;
      return TRUE;
    }

  sys_mbstowcs (wuser, user, UNLEN + 1);
  if (NetUserGetInfo (wlogonserver, wuser, 3, (LPBYTE *) &buf))
    return FALSE;
  pgrpsid = usersid;
  if (IsValidSid (pgrpsid) && (count = *GetSidSubAuthorityCount (pgrpsid)) > 1)
    {
      *GetSidSubAuthority (pgrpsid, count - 1) = buf->usri3_primary_group_id;
      ret = TRUE;
    }
  NetApiBufferFree (buf);
  return ret;
}

static BOOL
get_group_sidlist (const char *logonserver, cygsidlist &grp_list,
		   cygsid &usersid, cygsid &pgrpsid,
		   PTOKEN_GROUPS my_grps, LUID auth_luid, int &auth_pos)
{
  WCHAR wserver[INTERNET_MAX_HOST_NAME_LENGTH + 1];
  char user[INTERNET_MAX_HOST_NAME_LENGTH + 1];
  char domain[INTERNET_MAX_HOST_NAME_LENGTH + 1];
  DWORD ulen = INTERNET_MAX_HOST_NAME_LENGTH + 1;
  DWORD dlen = INTERNET_MAX_HOST_NAME_LENGTH + 1;
  SID_NAME_USE use;
  
  auth_pos = -1;
  sys_mbstowcs (wserver, logonserver, INTERNET_MAX_HOST_NAME_LENGTH + 1);
  if (!LookupAccountSid (NULL, usersid, user, &ulen, domain, &dlen, &use))
    {
      debug_printf ("LookupAccountSid () %E");
      __seterrno ();
      return FALSE;
    }
  grp_list += well_known_world_sid;
  if (usersid == well_known_system_sid)
    {
      grp_list += well_known_system_sid;
      grp_list += well_known_admin_sid;
    }
  else
    {
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
	  grp_list += well_known_authenticated_users_sid;
	}
      else
	{
	  grp_list += well_known_local_sid;
	  grp_list += well_known_interactive_sid;
	  grp_list += well_known_authenticated_users_sid;
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
  if (!pgrpsid)
    get_user_primary_group (wserver, user, usersid, pgrpsid);
  if (!get_user_groups (wserver, grp_list, user) ||
      !get_user_local_groups (wserver, logonserver, grp_list, usersid))
    return FALSE;
  if (!grp_list.contains (pgrpsid))
    grp_list += pgrpsid;
  return TRUE;
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
  PTOKEN_PRIVILEGES privs = (PTOKEN_PRIVILEGES) malloc (sizeof (ULONG) +
					    20 * sizeof (LUID_AND_ATTRIBUTES));
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
          if ((ret = LsaEnumerateAccountRights (lsa, usersid, &privstrs, &cnt))
              != STATUS_SUCCESS)
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

          sys_wcstombs (buf, privstrs[i].Buffer,
	  		INTERNET_MAX_HOST_NAME_LENGTH + 1);
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

#define token_acl_size (sizeof (ACL) + \
			2 * (sizeof (ACCESS_ALLOWED_ACE) + MAX_SID_LEN))

static BOOL
get_dacl (PACL acl, cygsid usersid, cygsidlist &grp_list)
{
  if (!InitializeAcl(acl, token_acl_size, ACL_REVISION))
    {
      __seterrno ();
      return FALSE;
    } 
  if (grp_list.contains (well_known_admin_sid))
    {
      if (!AddAccessAllowedAce(acl, ACL_REVISION, GENERIC_ALL,
			       well_known_admin_sid))
        {
	  __seterrno ();
          return FALSE;
        }
    }
  else if (!AddAccessAllowedAce(acl, ACL_REVISION, GENERIC_ALL, usersid))
    {
      __seterrno ();
      return FALSE;
    }
  if (!AddAccessAllowedAce(acl, ACL_REVISION, GENERIC_ALL,
			   well_known_system_sid))
    {
      __seterrno ();
      return FALSE;
    }
  return TRUE;
}

HANDLE
create_token (cygsid &usersid, cygsid &pgrpsid)
{
  NTSTATUS ret;
  LSA_HANDLE lsa = NULL;
  char logonserver[INTERNET_MAX_HOST_NAME_LENGTH + 1];
  int old_priv_state;

  cygsidlist grpsids;

  SECURITY_QUALITY_OF_SERVICE sqos =
    { sizeof sqos, SecurityImpersonation, SECURITY_STATIC_TRACKING, FALSE };
  OBJECT_ATTRIBUTES oa =
    { sizeof oa, 0, 0, 0, 0, &sqos };
  SECURITY_ATTRIBUTES sa = { sizeof sa, NULL, TRUE };
  LUID auth_luid = SYSTEM_LUID;
  LARGE_INTEGER exp = { 0x7fffffffffffffffLL } ;

  TOKEN_USER user;
  PTOKEN_GROUPS grps = NULL;
  PTOKEN_PRIVILEGES privs = NULL;
  TOKEN_OWNER owner;
  TOKEN_PRIMARY_GROUP pgrp;
  char acl_buf[token_acl_size];
  TOKEN_DEFAULT_DACL dacl;
  TOKEN_SOURCE source;
  TOKEN_STATISTICS stats;
  memcpy(source.SourceName, "Cygwin.1", 8);
  source.SourceIdentifier.HighPart = 0;
  source.SourceIdentifier.LowPart = 0x0101;

  HANDLE token;
  HANDLE primary_token = INVALID_HANDLE_VALUE;

  HANDLE my_token = INVALID_HANDLE_VALUE;
  PTOKEN_GROUPS my_grps = NULL;
  DWORD size;

  /* SE_CREATE_TOKEN_NAME privilege needed to call NtCreateToken. */
  if ((old_priv_state = set_process_privilege (SE_CREATE_TOKEN_NAME)) < 0)
    goto out;
   
  /* Open policy object. */
  if ((lsa = open_local_policy ()) == INVALID_HANDLE_VALUE)
    goto out;

  /* Get logon server. */
  if (!get_logon_server (lsa, logonserver))
    goto out;

  /* User, owner, primary group. */
  user.User.Sid = usersid;
  user.User.Attributes = 0;
  owner.Owner = usersid;

  /* Retrieve authentication id and group list from own process. */
  if (!OpenProcessToken (GetCurrentProcess (), TOKEN_QUERY, &my_token))
    debug_printf ("OpenProcessToken(my_token): %E\n");
  else
    {
      /* Switching user context to SYSTEM doesn't inherit the authentication
         id of the user account running current process. */
      if (usersid != well_known_system_sid)
	if (!GetTokenInformation (my_token, TokenStatistics,
				  &stats, sizeof stats, &size))
	  debug_printf ("GetTokenInformation(my_token, TokenStatistics): %E\n");
	else
	  auth_luid = stats.AuthenticationId;

      /* Retrieving current processes group list to be able to inherit
         some important well known group sids. */
      if (!GetTokenInformation (my_token, TokenGroups, NULL, 0, &size) &&
          GetLastError () != ERROR_INSUFFICIENT_BUFFER)
	debug_printf ("GetTokenInformation(my_token, TokenGroups): %E\n");
      else if (!(my_grps = (PTOKEN_GROUPS) malloc (size)))
	debug_printf ("malloc (my_grps) failed.");
      else if (!GetTokenInformation (my_token, TokenGroups, my_grps,
				     size, &size))
	{
	  debug_printf ("GetTokenInformation(my_token, TokenGroups): %E\n");
	  free (my_grps);
	  my_grps = NULL;
	}
    }

  /* Create list of groups, the user is member in. */
  int auth_pos;
  if (!get_group_sidlist (logonserver, grpsids, usersid, pgrpsid,
			  my_grps, auth_luid, auth_pos))
    goto out;

  /* Primary group. */
  pgrp.PrimaryGroup = pgrpsid;

  /* Create a TOKEN_GROUPS list from the above retrieved list of sids. */
  char grps_buf[sizeof (ULONG) + grpsids.count * sizeof (SID_AND_ATTRIBUTES)];
  grps = (PTOKEN_GROUPS) grps_buf;
  grps->GroupCount = grpsids.count;
  for (DWORD i = 0; i < grps->GroupCount; ++i)
    {
      grps->Groups[i].Sid = grpsids.sids[i];
      grps->Groups[i].Attributes = SE_GROUP_MANDATORY |
                                   SE_GROUP_ENABLED_BY_DEFAULT |
                                   SE_GROUP_ENABLED;
      if (auth_pos >= 0 && i == (DWORD) auth_pos)
        grps->Groups[i].Attributes |= SE_GROUP_LOGON_ID;
    }

  /* Retrieve list of privileges of that user. */
  if (!(privs = get_priv_list (lsa, usersid, grpsids)))
    goto out;

  /* Create default dacl. */
  if (!get_dacl ((PACL) acl_buf, usersid, grpsids))
    goto out;
  dacl.DefaultDacl = (PACL) acl_buf;

  /* Let's be heroic... */
  ret = NtCreateToken (&token, TOKEN_ALL_ACCESS, &oa, TokenImpersonation,
		       &auth_luid, &exp, &user, grps, privs, &owner, &pgrp,
		       &dacl, &source);
  if (ret)
    set_errno (RtlNtStatusToDosError (ret));
  else if (GetLastError () == ERROR_PROC_NOT_FOUND)
    {
      __seterrno ();
      debug_printf ("Loading NtCreateToken failed.");
    }

  /* Convert to primary token. */
  if (!DuplicateTokenEx (token, TOKEN_ALL_ACCESS, &sa,
  			 SecurityImpersonation, TokenPrimary,
			 &primary_token))
    __seterrno ();

out:
  if (old_priv_state >= 0)
    set_process_privilege (SE_CREATE_TOKEN_NAME, old_priv_state);
  if (token != INVALID_HANDLE_VALUE)
    CloseHandle (token);
  if (privs)
    free (privs);
  if (my_grps)
    free (my_grps);
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

  if ((old_tcb_state = set_process_privilege(SE_TCB_NAME)) < 0)
    return INVALID_HANDLE_VALUE;

  /* Register as logon process. */
  str2lsa (name, "Cygwin");
  SetLastError (0);
  ret = LsaRegisterLogonProcess(&name, &lsa_hdl, &sec_mode);
  if (ret != STATUS_SUCCESS)
    {
      debug_printf ("LsaRegisterLogonProcess: %d", ret);
      set_errno (LsaNtStatusToWinError(ret));
      goto out;
    }
  else if (GetLastError () == ERROR_PROC_NOT_FOUND)
    {
      debug_printf ("Couldn't load Secur32.dll");
      goto out;
    }
  /* Get handle to MSV1_0 package. */
  str2lsa (name, MSV1_0_PACKAGE_NAME);
  ret = LsaLookupAuthenticationPackage(lsa_hdl, &name, &package_id);
  if (ret != STATUS_SUCCESS)
    {
      debug_printf ("LsaLookupAuthenticationPackage: %d", ret);
      set_errno (LsaNtStatusToWinError(ret));
      LsaDeregisterLogonProcess(lsa_hdl);
      goto out;
    }
  /* Create origin. */
  str2buf2lsa (origin.str, origin.buf, "Cygwin");
  /* Create token source. */
  memcpy(ts.SourceName, "Cygwin.1", 8);
  ts.SourceIdentifier.HighPart = 0;
  ts.SourceIdentifier.LowPart = 0x0100;
  /* Get user information. */
  extract_nt_dom_user (pw, nt_domain, nt_user);
  /* Fill subauth with values. */
  subbuf.auth.MessageType = MsV1_0NetworkLogon;
  str2buf2uni(subbuf.auth.LogonDomainName, subbuf.dombuf, nt_domain);
  str2buf2uni(subbuf.auth.UserName, subbuf.usrbuf, nt_user);
  str2buf2uni(subbuf.auth.Workstation, subbuf.wkstbuf, "");
  memcpy(subbuf.auth.ChallengeToClient, "12345678", MSV1_0_CHALLENGE_LENGTH);
  str2buf2lsa(subbuf.auth.CaseSensitiveChallengeResponse, subbuf.authinf1, "");
  str2buf2lsa(subbuf.auth.CaseInsensitiveChallengeResponse, subbuf.authinf2,"");
  subbuf.auth.ParameterControl = 0 | (subauth_id << 24);
  /* Try to logon... */
  ret = LsaLogonUser(lsa_hdl, (PLSA_STRING) &origin, Network,
  		     package_id, &subbuf, sizeof subbuf,
		     NULL, &ts, (PVOID *)&profile, &size,
		     &luid, &user_token, &quota, &ret2);
  if (ret != STATUS_SUCCESS)
    {
      debug_printf ("LsaLogonUser: %d", ret);
      set_errno (LsaNtStatusToWinError(ret));
      LsaDeregisterLogonProcess(lsa_hdl);
      goto out;
    }
  LsaFreeReturnBuffer(profile);
  /* Convert to primary token. */
  if (!DuplicateTokenEx (user_token, TOKEN_ALL_ACCESS, &sa,
  			 SecurityImpersonation, TokenPrimary,
			 &primary_token))
    __seterrno ();

out:
  set_process_privilege(SE_TCB_NAME, old_tcb_state);
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
read_sd(const char *file, PSECURITY_DESCRIPTOR sd_buf, LPDWORD sd_size)
{
  /* Check parameters */
  if (!sd_size)
    {
      set_errno (EINVAL);
      return -1;
    }

  debug_printf("file = %s", file);

  DWORD len = 0;
  const char *pfile = file;
  char fbuf [PATH_MAX];
  if (current_codepage == oem_cp)
    {
      DWORD fname_len = min (sizeof (fbuf) - 1, strlen (file));
      bzero (fbuf, sizeof (fbuf));
      OemToCharBuff(file, fbuf, fname_len);
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
  debug_printf("file = %s: len=%d", file, len);
  if (len > *sd_size)
    {
      *sd_size = len;
      return 0;
    }
  return 1;
}

LONG
write_sd(const char *file, PSECURITY_DESCRIPTOR sd_buf, DWORD sd_size)
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

static int
get_nt_attribute (const char *file, int *attribute,
		  uid_t *uidret, gid_t *gidret)
{
  if (os_being_run != winNT)
    return 0;

  syscall_printf ("file: %s", file);

  /* Yeah, sounds too much, but I've seen SDs of 2100 bytes!*/
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

  uid_t uid = cygsid(owner_sid).get_uid ();
  gid_t gid = cygsid(group_sid).get_gid ();
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

  ACCESS_ALLOWED_ACE *ace;
  int allow = 0;
  int deny = 0;
  int *flags, *anti;

  for (DWORD i = 0; i < acl->AceCount; ++i)
    {
      if (!GetAce (acl, i, (PVOID *) &ace))
	continue;
      if (ace->Header.AceFlags & INHERIT_ONLY_ACE)
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
      if (owner_sid && ace_sid == owner_sid)
	{
	  if (ace->Mask & FILE_READ_DATA)
	    *flags |= S_IRUSR;
	  if (ace->Mask & FILE_WRITE_DATA)
	    *flags |= S_IWUSR;
	  if (ace->Mask & FILE_EXECUTE)
	    *flags |= S_IXUSR;
	}
      else if (group_sid && ace_sid == group_sid)
	{
	  if (ace->Mask & FILE_READ_DATA)
	    *flags |= S_IRGRP
		      | ((grp_member && !(*anti & S_IRUSR)) ? S_IRUSR : 0);
	  if (ace->Mask & FILE_WRITE_DATA)
	    *flags |= S_IWGRP
		      | ((grp_member && !(*anti & S_IWUSR)) ? S_IWUSR : 0);
	  if (ace->Mask & FILE_EXECUTE)
	    *flags |= S_IXGRP
		      | ((grp_member && !(*anti & S_IXUSR)) ? S_IXUSR : 0);
	}
      else if (ace_sid == well_known_world_sid)
	{
	  if (ace->Mask & FILE_READ_DATA)
	    *flags |= S_IROTH
		      | ((!(*anti & S_IRGRP)) ? S_IRGRP : 0)
		      | ((!(*anti & S_IRUSR)) ? S_IRUSR : 0);
	  if (ace->Mask & FILE_WRITE_DATA)
	    *flags |= S_IWOTH
		      | ((!(*anti & S_IWGRP)) ? S_IWGRP : 0)
		      | ((!(*anti & S_IWUSR)) ? S_IWUSR : 0);
	  if (ace->Mask & FILE_EXECUTE)
	    {
	      *flags |= S_IXOTH
			| ((!(*anti & S_IXGRP)) ? S_IXGRP : 0)
			| ((!(*anti & S_IXUSR)) ? S_IXUSR : 0);
	      /* Sticky bit for directories according to linux rules. */
	      if (!(ace->Mask & FILE_DELETE_CHILD)
		  && S_ISDIR(*attribute)
		  && !(*anti & S_ISVTX))
		*flags |= S_ISVTX;
	    }
	}
    }
  *attribute &= ~(S_IRWXU|S_IRWXG|S_IRWXO|S_ISVTX);
  *attribute |= allow;
  *attribute &= ~deny;
  syscall_printf ("file: %s %x, uid %d, gid %d", file, *attribute, uid, gid);
  return 0;
}

int
get_file_attribute (int use_ntsec, const char *file,
		    int *attribute, uid_t *uidret, gid_t *gidret)
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
    *uidret = getuid ();
  if (gidret)
    *gidret = getgid ();

  if (!attribute)
    return 0;

  int oatt = *attribute;
  res = NTReadEA (file, ".UNIXATTR", (char *) attribute, sizeof (*attribute));
  *attribute |= oatt;

  /* symlinks are everything for everyone!*/
  if ((*attribute & S_IFLNK) == S_IFLNK)
    *attribute |= S_IRWXU | S_IRWXG | S_IRWXO;

  return res > 0 ? 0 : -1;
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
  if (GetAce(acl, offset, (PVOID *) &ace))
    ace->Header.AceFlags |= inherit;
  len_add += sizeof (ACCESS_DENIED_ACE) - sizeof (DWORD)
	     + GetLengthSid (sid);
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
  if (GetAce(acl, offset, (PVOID *) &ace))
    ace->Header.AceFlags |= inherit;
  len_add += sizeof (ACCESS_DENIED_ACE) - sizeof (DWORD)
	     + GetLengthSid (sid);
  return TRUE;
}

PSECURITY_DESCRIPTOR
alloc_sd (uid_t uid, gid_t gid, const char *logsrv, int attribute,
	  PSECURITY_DESCRIPTOR sd_ret, DWORD *sd_size_ret)
{
  BOOL dummy;

  if (os_being_run != winNT)
    return NULL;

  if (!sd_ret || !sd_size_ret)
    {
      set_errno (EINVAL);
      return NULL;
    }

  /* Get SID and name of new owner. */
  char owner[UNLEN + 1];
  cygsid owner_sid;
  struct passwd *pw = getpwuid (uid);
  strcpy (owner, pw ? pw->pw_name : getlogin ());
  if ((!pw || !owner_sid.getfrompw (pw))
      && !lookup_name (owner, logsrv, owner_sid))
    return NULL;
  debug_printf ("owner: %s [%d]", owner,
		*GetSidSubAuthority(owner_sid,
		*GetSidSubAuthorityCount(owner_sid) - 1));

  /* Get SID and name of new group. */
  cygsid group_sid (NO_SID);
  struct group *grp = getgrgid (gid);
  if (grp)
    {
      if ((!grp || !group_sid.getfromgr (grp))
	  && !lookup_name (grp->gr_name, logsrv, group_sid))
	return NULL;
    }
  else
    debug_printf ("no group");

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
  static int win2KorHigher = -1;
  if (win2KorHigher == -1)
    {
      DWORD version = GetVersion ();
      win2KorHigher = (version & 0x80000000) || (version & 0xff) < 5 ? 0 : 1;
    }
  if (win2KorHigher > 0)
    SetSecurityDescriptorControl (&sd, SE_DACL_PROTECTED, SE_DACL_PROTECTED);

  /* Create owner for local security descriptor. */
  if (!SetSecurityDescriptorOwner(&sd, owner_sid, FALSE))
    {
      __seterrno ();
      return NULL;
    }

  /* Create group for local security descriptor. */
  if (group_sid && !SetSecurityDescriptorGroup(&sd, group_sid, FALSE))
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

  /*
   * VTX bit may only be set if executable for `other' is set.
   * For correct handling under WinNT, FILE_DELETE_CHILD has to
   * be (un)set in each ACE.
   */
  if (!(attribute & S_IXOTH))
    attribute &= ~S_ISVTX;
  if (!(attribute & S_IFDIR))
    attribute |= S_ISVTX;

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
  if (!(attribute & S_ISVTX))
    owner_allow |= FILE_DELETE_CHILD;

  /* Construct allow attribute for group. */
  DWORD group_allow = STANDARD_RIGHTS_READ
		      | FILE_READ_ATTRIBUTES | FILE_READ_EA;
  if (attribute & S_IRGRP)
    group_allow |= FILE_GENERIC_READ;
  if (attribute & S_IWGRP)
    group_allow |= STANDARD_RIGHTS_WRITE | FILE_GENERIC_WRITE | DELETE;
  if (attribute & S_IXGRP)
    group_allow |= FILE_GENERIC_EXECUTE;
  if (!(attribute & S_ISVTX))
    group_allow |= FILE_DELETE_CHILD;

  /* Construct allow attribute for everyone. */
  DWORD other_allow = STANDARD_RIGHTS_READ
		      | FILE_READ_ATTRIBUTES | FILE_READ_EA;
  if (attribute & S_IROTH)
    other_allow |= FILE_GENERIC_READ;
  if (attribute & S_IWOTH)
    other_allow |= STANDARD_RIGHTS_WRITE | FILE_GENERIC_WRITE | DELETE;
  if (attribute & S_IXOTH)
    other_allow |= FILE_GENERIC_EXECUTE;
  if (!(attribute & S_ISVTX))
    other_allow |= FILE_DELETE_CHILD;

  /* Construct deny attributes for owner and group. */
  DWORD owner_deny = 0;
  if (is_grp_member (uid, gid))
    owner_deny = ~owner_allow & (group_allow | other_allow);
  else
    owner_deny = ~owner_allow & other_allow;
  owner_deny &= ~(STANDARD_RIGHTS_READ
		  | FILE_READ_ATTRIBUTES | FILE_READ_EA
		  | FILE_WRITE_ATTRIBUTES | FILE_WRITE_EA);
  DWORD group_deny = ~group_allow & other_allow;
  group_deny &= ~(STANDARD_RIGHTS_READ | FILE_READ_ATTRIBUTES | FILE_READ_EA);

  /* Construct appropriate inherit attribute. */
  DWORD inherit = (attribute & S_IFDIR) ? INHERIT_ALL : DONT_INHERIT;

  /* Set deny ACE for owner. */
  if (owner_deny
      && !add_access_denied_ace (acl, ace_off++, owner_deny,
				  owner_sid, acl_len, inherit))
      return NULL;
  /* Set allow ACE for owner. */
  if (!add_access_allowed_ace (acl, ace_off++, owner_allow,
				owner_sid, acl_len, inherit))
    return NULL;
  /* Set deny ACE for group. */
  if (group_deny
      && !add_access_denied_ace (acl, ace_off++, group_deny,
				  group_sid, acl_len, inherit))
      return NULL;
  /* Set allow ACE for group. */
  if (!add_access_allowed_ace (acl, ace_off++, group_allow,
				group_sid, acl_len, inherit))
    return NULL;

  /* Set allow ACE for everyone. */
  if (!add_access_allowed_ace (acl, ace_off++, other_allow,
				well_known_world_sid, acl_len, inherit))
    return NULL;

  /* Get owner and group from current security descriptor. */
  PSID cur_owner_sid = NULL;
  PSID cur_group_sid = NULL;
  if (!GetSecurityDescriptorOwner (sd_ret, &cur_owner_sid, &dummy))
    debug_printf ("GetSecurityDescriptorOwner %E");
  if (!GetSecurityDescriptorGroup (sd_ret, &cur_group_sid, &dummy))
    debug_printf ("GetSecurityDescriptorGroup %E");

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
	  if ((cur_owner_sid && ace_sid == cur_owner_sid)
	      || (owner_sid && ace_sid == owner_sid)
	      || (cur_group_sid && ace_sid == cur_group_sid)
	      || (group_sid && ace_sid == group_sid)
	      || (ace_sid == well_known_world_sid))
	    continue;
	  /*
	   * Add unrelated ACCESS_DENIED_ACE to the beginning but
	   * behind the owner_deny, ACCESS_ALLOWED_ACE to the end.
	   */
	  if (!AddAce(acl, ACL_REVISION,
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

static int
set_nt_attribute (const char *file, uid_t uid, gid_t gid,
		  const char *logsrv, int attribute)
{
  if (os_being_run != winNT)
    return 0;

  DWORD sd_size = 4096;
  char sd_buf[4096];
  PSECURITY_DESCRIPTOR psd = (PSECURITY_DESCRIPTOR) sd_buf;

  int ret;
  if ((ret = read_sd (file, psd, &sd_size)) <= 0)
    {
      debug_printf ("read_sd %E");
      return ret;
    }

  sd_size = 4096;
  if (!(psd = alloc_sd (uid, gid, logsrv, attribute, psd, &sd_size)))
    return -1;

  return write_sd (file, psd, sd_size);
}

int
set_file_attribute (int use_ntsec, const char *file,
		    uid_t uid, gid_t gid,
		    int attribute, const char *logsrv)
{
  /* symlinks are anything for everyone!*/
  if ((attribute & S_IFLNK) == S_IFLNK)
    attribute |= S_IRWXU | S_IRWXG | S_IRWXO;

  if (!use_ntsec || !allow_ntsec)
    {
      if (!NTWriteEA (file, ".UNIXATTR", (char *) &attribute,
		      sizeof (attribute)))
	{
	  __seterrno ();
	  return -1;
	}
      return 0;
    }

  int ret = set_nt_attribute (file, uid, gid, logsrv, attribute);
  syscall_printf ("%d = set_file_attribute (%s, %d, %d, %p)",
		  ret, file, uid, gid, attribute);
  return ret;
}

int
set_file_attribute (int use_ntsec, const char *file, int attribute)
{
  return set_file_attribute (use_ntsec, file,
			     myself->uid, myself->gid,
			     attribute, cygheap->user.logsrv ());
}
