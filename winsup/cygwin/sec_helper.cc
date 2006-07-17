/* sec_helper.cc: NT security helper functions

   Copyright 2000, 2001, 2002, 2003, 2004 Red Hat, Inc.

   Written by Corinna Vinschen <corinna@vinschen.de>

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

#include "winsup.h"
#include <grp.h>
#include <pwd.h>
#include <unistd.h>
#include <stdlib.h>
#include <limits.h>
#include <sys/stat.h>
#include <sys/acl.h>
#include <ctype.h>
#include <wingdi.h>
#include <winuser.h>
#include <wininet.h>
#include "cygerrno.h"
#include "security.h"
#include "path.h"
#include "fhandler.h"
#include "dtable.h"
#include "pinfo.h"
#include "cygheap.h"
#include "cygtls.h"
#include "pwdgrp.h"

/* General purpose security attribute objects for global use. */
SECURITY_ATTRIBUTES NO_COPY sec_none;
SECURITY_ATTRIBUTES NO_COPY sec_none_nih;
SECURITY_ATTRIBUTES NO_COPY sec_all;
SECURITY_ATTRIBUTES NO_COPY sec_all_nih;

SID_IDENTIFIER_AUTHORITY NO_COPY sid_auth[] = {
	{SECURITY_NULL_SID_AUTHORITY},
	{SECURITY_WORLD_SID_AUTHORITY},
	{SECURITY_LOCAL_SID_AUTHORITY},
	{SECURITY_CREATOR_SID_AUTHORITY},
	{SECURITY_NON_UNIQUE_AUTHORITY},
	{SECURITY_NT_AUTHORITY}
};

SID (well_known_null_sid, "S-1-0-0",
     SECURITY_NULL_SID_AUTHORITY, 1, SECURITY_NULL_RID);
SID (well_known_world_sid, "S-1-1-0",
     SECURITY_WORLD_SID_AUTHORITY, 1, SECURITY_WORLD_RID);
SID (well_known_local_sid, "S-1-2-0",
     SECURITY_LOCAL_SID_AUTHORITY, 1, SECURITY_LOCAL_RID);
SID (well_known_creator_owner_sid, "S-1-3-0",
     SECURITY_CREATOR_SID_AUTHORITY, 1, SECURITY_CREATOR_OWNER_RID);
SID (well_known_creator_group_sid, "S-1-3-1",
     SECURITY_CREATOR_SID_AUTHORITY, 1, SECURITY_CREATOR_GROUP_RID);
SID (well_known_dialup_sid, "S-1-5-1",
     SECURITY_NT_AUTHORITY, 1, SECURITY_DIALUP_RID);
SID (well_known_network_sid, "S-1-5-2",
     SECURITY_NT_AUTHORITY, 1, SECURITY_NETWORK_RID);
SID (well_known_batch_sid, "S-1-5-3",
     SECURITY_NT_AUTHORITY, 1, SECURITY_BATCH_RID);
SID (well_known_interactive_sid, "S-1-5-4",
     SECURITY_NT_AUTHORITY, 1, SECURITY_INTERACTIVE_RID);
SID (well_known_service_sid, "S-1-5-6",
     SECURITY_NT_AUTHORITY, 1, SECURITY_SERVICE_RID);
SID (well_known_authenticated_users_sid, "S-1-5-11",
     SECURITY_NT_AUTHORITY, 1, SECURITY_AUTHENTICATED_USER_RID);
SID (well_known_system_sid, "S-1-5-18",
     SECURITY_NT_AUTHORITY, 1, SECURITY_LOCAL_SYSTEM_RID);
SID (well_known_admins_sid, "S-1-5-32-544",
     SECURITY_NT_AUTHORITY, 2, SECURITY_BUILTIN_DOMAIN_RID,
			       DOMAIN_ALIAS_RID_ADMINS);

bool
cygpsid::operator== (const char *nsidstr) const
{
  cygsid nsid (nsidstr);
  return psid == nsid;
}

__uid32_t
cygpsid::get_id (BOOL search_grp, int *type)
{
    /* First try to get SID from group, then passwd */
  __uid32_t id = ILLEGAL_UID;

  if (search_grp)
    {
      struct __group32 *gr;
      if (cygheap->user.groups.pgsid == psid)
	id = myself->gid;
      else if ((gr = internal_getgrsid (*this)))
	id = gr->gr_gid;
      if (id != ILLEGAL_UID)
	{
	  if (type)
	    *type = GROUP;
	  return id;
	}
    }
  if (!search_grp || type)
    {
      struct passwd *pw;
      if (*this == cygheap->user.sid ())
	id = myself->uid;
      else if ((pw = internal_getpwsid (*this)))
	id = pw->pw_uid;
      if (id != ILLEGAL_UID && type)
	*type = USER;
    }
  return id;
}


char *
cygpsid::string (char *nsidstr) const
{
  char *t;
  DWORD i;

  if (!psid || !nsidstr)
    return NULL;
  strcpy (nsidstr, "S-1-");
  t = nsidstr + sizeof ("S-1-") - 1;
  t += __small_sprintf (t, "%u", GetSidIdentifierAuthority (psid)->Value[5]);
  for (i = 0; i < *GetSidSubAuthorityCount (psid); ++i)
    t += __small_sprintf (t, "-%lu", *GetSidSubAuthority (psid, i));
  return nsidstr;
}

PSID
cygsid::get_sid (DWORD s, DWORD cnt, DWORD *r)
{
  DWORD i;

  if (s > 5 || cnt < 1 || cnt > 8)
    {
      psid = NO_SID;
      return NULL;
    }
  set ();
  InitializeSid (psid, &sid_auth[s], cnt);
  for (i = 0; i < cnt; ++i)
    memcpy ((char *) psid + 8 + sizeof (DWORD) * i, &r[i], sizeof (DWORD));
  return psid;
}

const PSID
cygsid::getfromstr (const char *nsidstr)
{
  char *lasts;
  DWORD s, cnt = 0;
  DWORD r[8];

  if (nsidstr && !strncmp (nsidstr, "S-1-", 4))
    {
      s = strtoul (nsidstr + 4, &lasts, 10);
      while (cnt < 8 && *lasts == '-')
	r[cnt++] = strtoul (lasts + 1, &lasts, 10);
      if (!*lasts)
	return get_sid (s, cnt, r);
    }
  return psid = NO_SID;
}

BOOL
cygsid::getfrompw (const struct passwd *pw)
{
  char *sp = (pw && pw->pw_gecos) ? strrchr (pw->pw_gecos, ',') : NULL;
  return (*this = sp ? sp + 1 : sp) != NULL;
}

BOOL
cygsid::getfromgr (const struct __group32 *gr)
{
  char *sp = (gr && gr->gr_passwd) ? gr->gr_passwd : NULL;
  return (*this = sp) != NULL;
}

bool
get_sids_info (cygpsid owner_sid, cygpsid group_sid, __uid32_t * uidret, __gid32_t * gidret)
{
  struct passwd *pw;
  struct __group32 *gr = NULL;
  bool ret = false;

  owner_sid.debug_print ("get_sids_info: owner SID =");
  group_sid.debug_print ("get_sids_info: group SID =");

  if (group_sid == cygheap->user.groups.pgsid)
    *gidret = myself->gid;
  else if ((gr = internal_getgrsid (group_sid)))
    *gidret = gr->gr_gid;
  else
    *gidret = ILLEGAL_GID;

  if (owner_sid == cygheap->user.sid ())
    {
      *uidret = myself->uid;
      if (*gidret == myself->gid)
	ret = true;
      else
	ret = (internal_getgroups (0, NULL, &group_sid) > 0);
    }
  else if ((pw = internal_getpwsid (owner_sid)))
    {
      *uidret = pw->pw_uid;
      if (gr || (*gidret != ILLEGAL_GID
		 && (gr = internal_getgrgid (*gidret))))
	for (int idx = 0; gr->gr_mem[idx]; ++idx)
	  if ((ret = strcasematch (pw->pw_name, gr->gr_mem[idx])))
	    break;
    }
  else
    *uidret = ILLEGAL_UID;

  return ret;
}

PSECURITY_DESCRIPTOR
security_descriptor::malloc (size_t nsize)
{
  if (psd)
    ::free (psd);
  psd = (PSECURITY_DESCRIPTOR) ::malloc (nsize);
  sd_size = psd ? nsize : 0;
  return psd;
}

PSECURITY_DESCRIPTOR
security_descriptor::realloc (size_t nsize)
{
  PSECURITY_DESCRIPTOR tmp = (PSECURITY_DESCRIPTOR) ::realloc (psd, nsize);
  if (!tmp)
    return NULL;
  sd_size = nsize;
  return psd = tmp;
}

void
security_descriptor::free ()
{
  if (psd)
    ::free (psd);
  psd = NULL;
  sd_size = 0;
}

#if 0 // unused
#define SIDLEN	(sidlen = MAX_SID_LEN, &sidlen)
#define DOMLEN	(domlen = INTERNET_MAX_HOST_NAME_LENGTH, &domlen)

BOOL
lookup_name (const char *name, const char *logsrv, PSID ret_sid)
{
  cygsid sid;
  DWORD sidlen;
  char domuser[INTERNET_MAX_HOST_NAME_LENGTH + UNLEN + 2];
  char dom[INTERNET_MAX_HOST_NAME_LENGTH + 1];
  DWORD domlen;
  SID_NAME_USE acc_type;

  debug_printf ("name  : %s", name ? name : "NULL");

  if (!name)
    return FALSE;

  if (cygheap->user.domain ())
    {
      strcat (strcat (strcpy (domuser, cygheap->user.domain ()), "\\"), name);
      if (LookupAccountName (NULL, domuser, sid, SIDLEN, dom, DOMLEN, &acc_type)
	  && legal_sid_type (acc_type))
	goto got_it;
      if (logsrv && *logsrv
	  && LookupAccountName (logsrv, domuser, sid, SIDLEN,
				dom, DOMLEN, &acc_type)
	  && legal_sid_type (acc_type))
	goto got_it;
    }
  if (logsrv && *logsrv)
    {
      if (LookupAccountName (logsrv, name, sid, SIDLEN, dom, DOMLEN, &acc_type)
	  && legal_sid_type (acc_type))
	goto got_it;
      if (acc_type == SidTypeDomain)
	{
	  strcat (strcat (strcpy (domuser, dom), "\\"), name);
	  if (LookupAccountName (logsrv, domuser, sid, SIDLEN,
				 dom, DOMLEN, &acc_type))
	    goto got_it;
	}
    }
  if (LookupAccountName (NULL, name, sid, SIDLEN, dom, DOMLEN, &acc_type)
      && legal_sid_type (acc_type))
    goto got_it;
  if (acc_type == SidTypeDomain)
    {
      strcat (strcat (strcpy (domuser, dom), "\\"), name);
      if (LookupAccountName (NULL, domuser, sid, SIDLEN, dom, DOMLEN,&acc_type))
	goto got_it;
    }
  debug_printf ("LookupAccountName (%s) %E", name);
  __seterrno ();
  return FALSE;

got_it:
  debug_printf ("sid : [%d]", *GetSidSubAuthority ((PSID) sid,
			      *GetSidSubAuthorityCount ((PSID) sid) - 1));

  if (ret_sid)
    memcpy (ret_sid, sid, sidlen);

  return TRUE;
}

#undef SIDLEN
#undef DOMLEN
#endif //unused

/* Order must be same as cygperm_idx in winsup.h. */
static const char *cygpriv[] =
{
  SE_CREATE_TOKEN_NAME,
  SE_ASSIGNPRIMARYTOKEN_NAME,
  SE_LOCK_MEMORY_NAME,
  SE_INCREASE_QUOTA_NAME,
  SE_UNSOLICITED_INPUT_NAME,
  SE_MACHINE_ACCOUNT_NAME,
  SE_TCB_NAME,
  SE_SECURITY_NAME,
  SE_TAKE_OWNERSHIP_NAME,
  SE_LOAD_DRIVER_NAME,
  SE_SYSTEM_PROFILE_NAME,
  SE_SYSTEMTIME_NAME,
  SE_PROF_SINGLE_PROCESS_NAME,
  SE_INC_BASE_PRIORITY_NAME,
  SE_CREATE_PAGEFILE_NAME,
  SE_CREATE_PERMANENT_NAME,
  SE_BACKUP_NAME,
  SE_RESTORE_NAME,
  SE_SHUTDOWN_NAME,
  SE_DEBUG_NAME,
  SE_AUDIT_NAME,
  SE_SYSTEM_ENVIRONMENT_NAME,
  SE_CHANGE_NOTIFY_NAME,
  SE_REMOTE_SHUTDOWN_NAME,
  SE_CREATE_GLOBAL_NAME,
  SE_UNDOCK_NAME,
  SE_MANAGE_VOLUME_NAME,
  SE_IMPERSONATE_NAME,
  SE_ENABLE_DELEGATION_NAME,
  SE_SYNC_AGENT_NAME
};

const LUID *
privilege_luid (cygpriv_idx idx)
{
  if (idx < 0 || idx >= SE_NUM_PRIVS)
    return NULL;
  if (!cygheap->luid[idx].LowPart && !cygheap->luid[idx].HighPart
      && !LookupPrivilegeValue (NULL, cygpriv[idx], &cygheap->luid[idx]))
    {
      __seterrno ();
      return NULL;
    }
  return &cygheap->luid[idx];
}

const LUID *
privilege_luid_by_name (const char *pname)
{
  int idx;

  if (!pname)
    return NULL;
  for (idx = 0; idx < SE_NUM_PRIVS; ++idx)
    if (!strcmp (pname, cygpriv[idx]))
      return privilege_luid ((cygpriv_idx) idx);
  return NULL;
}

const char *
privilege_name (cygpriv_idx idx)
{
  if (idx < 0 || idx >= SE_NUM_PRIVS)
    return "<unknown privilege>";
  return cygpriv[idx];
}

int
set_privilege (HANDLE token, cygpriv_idx privilege, bool enable)
{
  int ret = -1;
  const LUID *priv_luid;
  TOKEN_PRIVILEGES new_priv, orig_priv;
  DWORD size;

  if (!(priv_luid = privilege_luid (privilege)))
    {
      __seterrno ();
      goto out;
    }

  new_priv.PrivilegeCount = 1;
  new_priv.Privileges[0].Luid = *priv_luid;
  new_priv.Privileges[0].Attributes = enable ? SE_PRIVILEGE_ENABLED : 0;

  if (!AdjustTokenPrivileges (token, FALSE, &new_priv,
			      sizeof orig_priv, &orig_priv, &size))
    {
      __seterrno ();
      goto out;
    }
  /* AdjustTokenPrivileges returns TRUE even if the privilege could not
     be enabled. GetLastError () returns an correct error code, though. */
  if (enable && GetLastError () == ERROR_NOT_ALL_ASSIGNED)
    {
      __seterrno ();
      goto out;
    }

  /* If orig_priv.PrivilegeCount is 0, the privilege hasn't been changed. */
  if (!orig_priv.PrivilegeCount)
    ret = enable ? 1 : 0;
  else
    ret = (orig_priv.Privileges[0].Attributes & SE_PRIVILEGE_ENABLED) ? 1 : 0;

out:
  syscall_printf ("%d = set_privilege ((token %x) %s, %d)",
		  ret, token, privilege_name (privilege), enable);
  return ret;
}

void
set_cygwin_privileges (HANDLE token)
{
  set_privilege (token, SE_RESTORE_PRIV, true);
  set_privilege (token, SE_BACKUP_PRIV, true);
  set_privilege (token, SE_CHANGE_NOTIFY_PRIV, !allow_traverse);
}

/* Function to return a common SECURITY_DESCRIPTOR that
   allows all access.  */


SECURITY_DESCRIPTOR *__stdcall
get_null_sd ()
{
  static NO_COPY SECURITY_DESCRIPTOR sd;
  static NO_COPY SECURITY_DESCRIPTOR *null_sdp;

  if (!null_sdp)
    {
      InitializeSecurityDescriptor (&sd, SECURITY_DESCRIPTOR_REVISION);
      SetSecurityDescriptorDacl (&sd, TRUE, 0, FALSE);
      null_sdp = &sd;
    }
  return null_sdp;
}

/* Initialize global security attributes.
   Called from dcrt0.cc (_dll_crt0).  */

void
init_global_security ()
{
  sec_none.nLength = sec_none_nih.nLength =
  sec_all.nLength = sec_all_nih.nLength = sizeof (SECURITY_ATTRIBUTES);
  sec_none.bInheritHandle = sec_all.bInheritHandle = TRUE;
  sec_none_nih.bInheritHandle = sec_all_nih.bInheritHandle = FALSE;
  sec_none.lpSecurityDescriptor = sec_none_nih.lpSecurityDescriptor = NULL;
  sec_all.lpSecurityDescriptor = sec_all_nih.lpSecurityDescriptor =
    get_null_sd ();
}

bool
sec_acl (PACL acl, bool original, bool admins, PSID sid1, PSID sid2, DWORD access2)
{
  size_t acl_len = MAX_DACL_LEN(5);
  LPVOID pAce;
  cygpsid psid;

#ifdef DEBUGGING
  if ((unsigned long) acl % 4)
    api_fatal ("Incorrectly aligned incoming ACL buffer!");
#endif
  if (!InitializeAcl (acl, acl_len, ACL_REVISION))
    {
      debug_printf ("InitializeAcl %E");
      return false;
    }
  if (sid1)
    if (!AddAccessAllowedAce (acl, ACL_REVISION,
			      GENERIC_ALL, sid1))
      debug_printf ("AddAccessAllowedAce(sid1) %E");
  if (original && (psid = cygheap->user.saved_sid ())
      && psid != sid1 && psid != well_known_system_sid)
    if (!AddAccessAllowedAce (acl, ACL_REVISION,
			      GENERIC_ALL, psid))
      debug_printf ("AddAccessAllowedAce(original) %E");
  if (sid2)
    if (!AddAccessAllowedAce (acl, ACL_REVISION,
			      access2, sid2))
      debug_printf ("AddAccessAllowedAce(sid2) %E");
  if (admins)
    if (!AddAccessAllowedAce (acl, ACL_REVISION,
			      GENERIC_ALL, well_known_admins_sid))
      debug_printf ("AddAccessAllowedAce(admin) %E");
  if (!AddAccessAllowedAce (acl, ACL_REVISION,
			    GENERIC_ALL, well_known_system_sid))
    debug_printf ("AddAccessAllowedAce(system) %E");
  FindFirstFreeAce (acl, &pAce);
  if (pAce)
    acl->AclSize = (char *) pAce - (char *) acl;
  else
    debug_printf ("FindFirstFreeAce %E");

  return true;
}

PSECURITY_ATTRIBUTES __stdcall
__sec_user (PVOID sa_buf, PSID sid1, PSID sid2, DWORD access2, BOOL inherit)
{
  PSECURITY_ATTRIBUTES psa = (PSECURITY_ATTRIBUTES) sa_buf;
  PSECURITY_DESCRIPTOR psd = (PSECURITY_DESCRIPTOR)
			     ((char *) sa_buf + sizeof (*psa));
  PACL acl = (PACL) ((char *) sa_buf + sizeof (*psa) + sizeof (*psd));

#ifdef DEBUGGING
  if ((unsigned long) sa_buf % 4)
    api_fatal ("Incorrectly aligned incoming SA buffer!");
#endif
  if (!wincap.has_security ()
      || !sec_acl (acl, true, true, sid1, sid2, access2))
    return inherit ? &sec_none : &sec_none_nih;

  if (!InitializeSecurityDescriptor (psd, SECURITY_DESCRIPTOR_REVISION))
    debug_printf ("InitializeSecurityDescriptor %E");

/*
 * Setting the owner lets the created security attribute not work
 * on NT4 SP3 Server. Don't know why, but the function still does
 * what it should do also if the owner isn't set.
*/
#if 0
  if (!SetSecurityDescriptorOwner (psd, sid, FALSE))
    debug_printf ("SetSecurityDescriptorOwner %E");
#endif

  if (!SetSecurityDescriptorDacl (psd, TRUE, acl, FALSE))
    debug_printf ("SetSecurityDescriptorDacl %E");

  psa->nLength = sizeof (SECURITY_ATTRIBUTES);
  psa->lpSecurityDescriptor = psd;
  psa->bInheritHandle = inherit;
  return psa;
}
