/* sec_helper.cc: NT security helper functions

   Copyright 2000, 2001, 2002, 2003, 2004, 2006, 2007, 2008, 2009,
   2010, 2011 Red Hat, Inc.

   Written by Corinna Vinschen <corinna@vinschen.de>

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

#include "winsup.h"
#include <stdlib.h>
#include <sys/acl.h>
#include <wchar.h>
#include "cygerrno.h"
#include "security.h"
#include "path.h"
#include "fhandler.h"
#include "dtable.h"
#include "pinfo.h"
#include "cygheap.h"
#include "pwdgrp.h"
#include "ntdll.h"

/* General purpose security attribute objects for global use. */
SECURITY_ATTRIBUTES NO_COPY sec_none;
SECURITY_ATTRIBUTES NO_COPY sec_none_nih;
SECURITY_ATTRIBUTES NO_COPY sec_all;
SECURITY_ATTRIBUTES NO_COPY sec_all_nih;

MKSID (well_known_null_sid, "S-1-0-0",
       SECURITY_NULL_SID_AUTHORITY, 1, SECURITY_NULL_RID);
MKSID (well_known_world_sid, "S-1-1-0",
       SECURITY_WORLD_SID_AUTHORITY, 1, SECURITY_WORLD_RID);
MKSID (well_known_local_sid, "S-1-2-0",
       SECURITY_LOCAL_SID_AUTHORITY, 1, SECURITY_LOCAL_RID);
MKSID (well_known_console_logon_sid, "S-1-2-1",
       SECURITY_LOCAL_SID_AUTHORITY, 1, 1);
MKSID (well_known_creator_owner_sid, "S-1-3-0",
       SECURITY_CREATOR_SID_AUTHORITY, 1, SECURITY_CREATOR_OWNER_RID);
MKSID (well_known_creator_group_sid, "S-1-3-1",
       SECURITY_CREATOR_SID_AUTHORITY, 1, SECURITY_CREATOR_GROUP_RID);
MKSID (well_known_dialup_sid, "S-1-5-1",
       SECURITY_NT_AUTHORITY, 1, SECURITY_DIALUP_RID);
MKSID (well_known_network_sid, "S-1-5-2",
       SECURITY_NT_AUTHORITY, 1, SECURITY_NETWORK_RID);
MKSID (well_known_batch_sid, "S-1-5-3",
       SECURITY_NT_AUTHORITY, 1, SECURITY_BATCH_RID);
MKSID (well_known_interactive_sid, "S-1-5-4",
       SECURITY_NT_AUTHORITY, 1, SECURITY_INTERACTIVE_RID);
MKSID (well_known_service_sid, "S-1-5-6",
       SECURITY_NT_AUTHORITY, 1, SECURITY_SERVICE_RID);
MKSID (well_known_authenticated_users_sid, "S-1-5-11",
       SECURITY_NT_AUTHORITY, 1, SECURITY_AUTHENTICATED_USER_RID);
MKSID (well_known_this_org_sid, "S-1-5-15",
       SECURITY_NT_AUTHORITY, 1, 15);
MKSID (well_known_system_sid, "S-1-5-18",
       SECURITY_NT_AUTHORITY, 1, SECURITY_LOCAL_SYSTEM_RID);
MKSID (well_known_builtin_sid, "S-1-5-32",
       SECURITY_NT_AUTHORITY, 1, SECURITY_BUILTIN_DOMAIN_RID);
MKSID (well_known_admins_sid, "S-1-5-32-544",
       SECURITY_NT_AUTHORITY, 2, SECURITY_BUILTIN_DOMAIN_RID,
				 DOMAIN_ALIAS_RID_ADMINS);
MKSID (well_known_users_sid, "S-1-5-32-545",
       SECURITY_NT_AUTHORITY, 2, SECURITY_BUILTIN_DOMAIN_RID,
				 DOMAIN_ALIAS_RID_USERS);
MKSID (fake_logon_sid, "S-1-5-5-0-0",
       SECURITY_NT_AUTHORITY, 3, SECURITY_LOGON_IDS_RID, 0, 0);
MKSID (mandatory_medium_integrity_sid, "S-1-16-8192",
       SECURITY_MANDATORY_LABEL_AUTHORITY, 1, SECURITY_MANDATORY_MEDIUM_RID);
MKSID (mandatory_high_integrity_sid, "S-1-16-12288",
       SECURITY_MANDATORY_LABEL_AUTHORITY, 1, SECURITY_MANDATORY_HIGH_RID);
MKSID (mandatory_system_integrity_sid, "S-1-16-16384",
       SECURITY_MANDATORY_LABEL_AUTHORITY, 1, SECURITY_MANDATORY_SYSTEM_RID);
/* UNIX accounts on a Samba server have the SID prefix "S-1-22-1" */
#define SECURITY_SAMBA_UNIX_AUTHORITY {0,0,0,0,0,22}
MKSID (well_known_samba_unix_user_fake_sid, "S-1-22-1-0",
       SECURITY_SAMBA_UNIX_AUTHORITY, 2, 1, 0);

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

PWCHAR
cygpsid::string (PWCHAR nsidstr) const
{
  UNICODE_STRING sid;

  if (!psid || !nsidstr)
    return NULL;
  RtlInitEmptyUnicodeString (&sid, nsidstr, 256);
  RtlConvertSidToUnicodeString (&sid, psid, FALSE);
  return nsidstr;
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
  t += __small_sprintf (t, "%u", RtlIdentifierAuthoritySid (psid)->Value[5]);
  for (i = 0; i < *RtlSubAuthorityCountSid (psid); ++i)
    t += __small_sprintf (t, "-%lu", *RtlSubAuthoritySid (psid, i));
  return nsidstr;
}

PSID
cygsid::get_sid (DWORD s, DWORD cnt, DWORD *r, bool well_known)
{
  DWORD i;
  SID_IDENTIFIER_AUTHORITY sid_auth = { SECURITY_NULL_SID_AUTHORITY };
# define SECURITY_NT_AUTH 5

  if (s > 255 || cnt < 1 || cnt > 8)
    {
      psid = NO_SID;
      return NULL;
    }
  sid_auth.Value[5] = s;
  set ();
  RtlInitializeSid (psid, &sid_auth, cnt);
  for (i = 0; i < cnt; ++i)
    memcpy ((char *) psid + 8 + sizeof (DWORD) * i, &r[i], sizeof (DWORD));
  /* If the well_known flag isn't set explicitely, we check the SID
     for being a well-known SID ourselves. That's necessary because this
     cygsid is created from a SID string, usually from /etc/passwd or
     /etc/group.  The calling code just doesn't know if the SID is well-known
     or not.  All SIDs are well-known SIDs, except those in the non-unique NT
     authority range. */
  if (well_known)
    well_known_sid = well_known;
  else
    well_known_sid = (s != SECURITY_NT_AUTH
		      || r[0] != SECURITY_NT_NON_UNIQUE_RID);
  return psid;
}

const PSID
cygsid::getfromstr (const char *nsidstr, bool well_known)
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
	return get_sid (s, cnt, r, well_known);
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
  cnt = maxcnt = 0;
  type = cygsidlist_empty;
}

BOOL
cygsidlist::add (const PSID nsi, bool well_known)
{
  if (contains (nsi))
    return TRUE;
  if (cnt >= maxcnt)
    {
      cygsid *tmp = new cygsid [2 * maxcnt];
      if (!tmp)
	return FALSE;
      maxcnt *= 2;
      for (int i = 0; i < cnt; ++i)
	tmp[i] = sids[i];
      delete [] sids;
      sids = tmp;
    }
  if (well_known)
    sids[cnt++] *= nsi;
  else
    sids[cnt++] = nsi;
  return TRUE;
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
  free ();
  if ((psd = (PSECURITY_DESCRIPTOR) ::malloc (nsize)))
    sd_size = nsize;
  return psd;
}

PSECURITY_DESCRIPTOR
security_descriptor::realloc (size_t nsize)
{
  PSECURITY_DESCRIPTOR tmp;

  /* Can't re-use buffer allocated by GetSecurityInfo. */
  if (psd && !sd_size)
    free ();
  if (!(tmp = (PSECURITY_DESCRIPTOR) ::realloc (psd, nsize)))
    return NULL;
  sd_size = nsize;
  return psd = tmp;
}

void
security_descriptor::free ()
{
  if (psd)
    {
      if (!sd_size)
	LocalFree (psd);
      else
	::free (psd);
    }
  psd = NULL;
  sd_size = 0;
}

#undef TEXT
#define TEXT(q) L##q

/* Index must match the corresponding foo_PRIVILEGE value, see security.h. */
static const struct {
  const wchar_t *name;
  bool		 high_integrity; /* UAC: High Mandatory Label required to 
				    be allowed to enable this privilege in
				    the user token. */
} cygpriv[] =
{
  { L"",				false },
  { L"",				false },
  { SE_CREATE_TOKEN_NAME,		true  },
  { SE_ASSIGNPRIMARYTOKEN_NAME,		true  },
  { SE_LOCK_MEMORY_NAME,		false },
  { SE_INCREASE_QUOTA_NAME,		true  },
  { SE_MACHINE_ACCOUNT_NAME,		false },
  { SE_TCB_NAME,			true  },
  { SE_SECURITY_NAME,			true  },
  { SE_TAKE_OWNERSHIP_NAME,		true  },
  { SE_LOAD_DRIVER_NAME,		true  },
  { SE_SYSTEM_PROFILE_NAME,		true  },
  { SE_SYSTEMTIME_NAME,			true  },
  { SE_PROF_SINGLE_PROCESS_NAME,	true  },
  { SE_INC_BASE_PRIORITY_NAME,		true  },
  { SE_CREATE_PAGEFILE_NAME,		true  },
  { SE_CREATE_PERMANENT_NAME,		false },
  { SE_BACKUP_NAME,			true  },
  { SE_RESTORE_NAME,			true  },
  { SE_SHUTDOWN_NAME,			false },
  { SE_DEBUG_NAME,			true  },
  { SE_AUDIT_NAME,			false },
  { SE_SYSTEM_ENVIRONMENT_NAME,		true  },
  { SE_CHANGE_NOTIFY_NAME,		false },
  { SE_REMOTE_SHUTDOWN_NAME,		true  },
  { SE_UNDOCK_NAME,			false },
  { SE_SYNC_AGENT_NAME,			false },
  { SE_ENABLE_DELEGATION_NAME,		false },
  { SE_MANAGE_VOLUME_NAME,		true  },
  { SE_IMPERSONATE_NAME,		true  },
  { SE_CREATE_GLOBAL_NAME,		false },
  { SE_TRUSTED_CREDMAN_ACCESS_NAME,	false },
  { SE_RELABEL_NAME,			true  },
  { SE_INCREASE_WORKING_SET_NAME,	false },
  { SE_TIME_ZONE_NAME,			true  },
  { SE_CREATE_SYMBOLIC_LINK_NAME,	true  }
};

bool
privilege_luid (const PWCHAR pname, LUID &luid, bool &high_integrity)
{
  ULONG idx;
  for (idx = SE_CREATE_TOKEN_PRIVILEGE;
       idx <= SE_MAX_WELL_KNOWN_PRIVILEGE;
       ++idx)
    if (!wcscmp (cygpriv[idx].name, pname))
      {
	luid.HighPart = 0;
	luid.LowPart = idx;
	high_integrity = cygpriv[idx].high_integrity;
	return true;
      }
  return false;
}

static const wchar_t *
privilege_name (const LUID &priv_luid)
{
  if (priv_luid.HighPart || priv_luid.LowPart < SE_CREATE_TOKEN_PRIVILEGE
      || priv_luid.LowPart > SE_MAX_WELL_KNOWN_PRIVILEGE)
    return L"<unknown privilege>";
  return cygpriv[priv_luid.LowPart].name;
}

int
set_privilege (HANDLE token, DWORD privilege, bool enable)
{
  int ret = -1;
  TOKEN_PRIVILEGES new_priv, orig_priv;
  ULONG size;
  NTSTATUS status;

  new_priv.PrivilegeCount = 1;
  new_priv.Privileges[0].Luid.HighPart = 0L;
  new_priv.Privileges[0].Luid.LowPart = privilege;
  new_priv.Privileges[0].Attributes = enable ? SE_PRIVILEGE_ENABLED : 0;

  status = NtAdjustPrivilegesToken (token, FALSE, &new_priv, sizeof orig_priv,
				    &orig_priv, &size);
  if (!NT_SUCCESS (status))
    {
      __seterrno_from_nt_status (status);
      goto out;
    }

  /* If orig_priv.PrivilegeCount is 0, the privilege hasn't been changed. */
  if (!orig_priv.PrivilegeCount)
    ret = enable ? 1 : 0;
  else
    ret = (orig_priv.Privileges[0].Attributes & SE_PRIVILEGE_ENABLED) ? 1 : 0;

out:
  if (ret < 0)
    debug_printf ("%d = set_privilege ((token %x) %W, %d)", ret, token,
		  privilege_name (new_priv.Privileges[0].Luid), enable);
  return ret;
}

/* This is called very early in process initialization.  The code must
   not depend on anything. */
void
set_cygwin_privileges (HANDLE token)
{
  /* Setting these rights at process startup allows processes running under
     user tokens which are in the administrstors group to have root-like
     permissions. */
  /* Allow to access all files, independent of their ACL settings. */
  set_privilege (token, SE_RESTORE_PRIVILEGE, true);
  set_privilege (token, SE_BACKUP_PRIVILEGE, true);
  /* Allow full access to other user's processes. */
  set_privilege (token, SE_DEBUG_PRIVILEGE, true);
#if 0
  /* Allow to create global shared memory.  This isn't required anymore since
     Cygwin 1.7.  It uses its own subdirectories in the global NT namespace
     which isn't affected by the SE_CREATE_GLOBAL_PRIVILEGE restriction. */
  if (wincap.has_create_global_privilege ())
    set_privilege (token, SE_CREATE_GLOBAL_PRIVILEGE, true);
#endif
}

/* Function to return a common SECURITY_DESCRIPTOR that
   allows all access.  */

static inline PSECURITY_DESCRIPTOR
get_null_sd ()
{
  static NO_COPY SECURITY_DESCRIPTOR sd;
  static NO_COPY PSECURITY_DESCRIPTOR null_sdp;

  if (!null_sdp)
    {
      RtlCreateSecurityDescriptor (&sd, SECURITY_DESCRIPTOR_REVISION);
      RtlSetDaclSecurityDescriptor (&sd, TRUE, NULL, FALSE);
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
  NTSTATUS status;
  size_t acl_len = MAX_DACL_LEN (5);
  LPVOID pAce;
  cygpsid psid;

#ifdef DEBUGGING
  if ((unsigned long) acl % 4)
    api_fatal ("Incorrectly aligned incoming ACL buffer!");
#endif
  status = RtlCreateAcl (acl, acl_len, ACL_REVISION);
  if (!NT_SUCCESS (status))
    {
      debug_printf ("RtlCreateAcl: %p", status);
      return false;
    }
  if (sid1)
    {
      status = RtlAddAccessAllowedAce (acl, ACL_REVISION, GENERIC_ALL, sid1);
      if (!NT_SUCCESS (status))
	debug_printf ("RtlAddAccessAllowedAce(sid1) %p", status);
    }
  if (original && (psid = cygheap->user.saved_sid ())
      && psid != sid1 && psid != well_known_system_sid)
    {
      status = RtlAddAccessAllowedAce (acl, ACL_REVISION, GENERIC_ALL, psid);
      if (!NT_SUCCESS (status))
	debug_printf ("RtlAddAccessAllowedAce(original) %p", status);
    }
  if (sid2)
    {
      status = RtlAddAccessAllowedAce (acl, ACL_REVISION, access2, sid2);
      if (!NT_SUCCESS (status))
	debug_printf ("RtlAddAccessAllowedAce(sid2) %p", status);
    }
  if (admins)
    {
      status = RtlAddAccessAllowedAce (acl, ACL_REVISION, GENERIC_ALL,
				       well_known_admins_sid);
      if (!NT_SUCCESS (status))
	debug_printf ("RtlAddAccessAllowedAce(admin) %p", status);
    }
  status = RtlAddAccessAllowedAce (acl, ACL_REVISION, GENERIC_ALL,
				   well_known_system_sid);
  if (!NT_SUCCESS (status))
    debug_printf ("RtlAddAccessAllowedAce(system) %p", status);
  status = RtlFirstFreeAce (acl, &pAce);
  if (NT_SUCCESS (status) && pAce)
    acl->AclSize = (char *) pAce - (char *) acl;
  else
    debug_printf ("RtlFirstFreeAce: %p", status);

  return true;
}

PSECURITY_ATTRIBUTES __stdcall
__sec_user (PVOID sa_buf, PSID sid1, PSID sid2, DWORD access2, BOOL inherit)
{
  PSECURITY_ATTRIBUTES psa = (PSECURITY_ATTRIBUTES) sa_buf;
  PSECURITY_DESCRIPTOR psd = (PSECURITY_DESCRIPTOR)
			     ((char *) sa_buf + sizeof (*psa));
  PACL acl = (PACL) ((char *) sa_buf + sizeof (*psa) + sizeof (*psd));
  NTSTATUS status;

#ifdef DEBUGGING
  if ((unsigned long) sa_buf % 4)
    api_fatal ("Incorrectly aligned incoming SA buffer!");
#endif
  if (!sec_acl (acl, true, true, sid1, sid2, access2))
    return inherit ? &sec_none : &sec_none_nih;

  RtlCreateSecurityDescriptor (psd, SECURITY_DESCRIPTOR_REVISION);
  status = RtlSetDaclSecurityDescriptor (psd, TRUE, acl, FALSE);
  if (!NT_SUCCESS (status))
    debug_printf ("RtlSetDaclSecurityDescriptor %p", status);

  psa->nLength = sizeof (SECURITY_ATTRIBUTES);
  psa->lpSecurityDescriptor = psd;
  psa->bInheritHandle = inherit;
  return psa;
}

/* Helper function to create an event security descriptor which only allows
   specific access to everyone.  Only the creating process has all access
   rights. */

PSECURITY_DESCRIPTOR
_everyone_sd (void *buf, ACCESS_MASK access)
{
  NTSTATUS status;
  PSECURITY_DESCRIPTOR psd = (PSECURITY_DESCRIPTOR) buf;

  if (psd)
    {
      RtlCreateSecurityDescriptor (psd, SECURITY_DESCRIPTOR_REVISION);
      PACL dacl = (PACL) (psd + 1);
      RtlCreateAcl (dacl, MAX_DACL_LEN (1), ACL_REVISION);
      status = RtlAddAccessAllowedAce (dacl, ACL_REVISION, access,
				       well_known_world_sid);
      if (!NT_SUCCESS (status))
	{
	  debug_printf ("RtlAddAccessAllowedAce: %p", status);
	  return NULL;
	}
      LPVOID ace;
      status = RtlFirstFreeAce (dacl, &ace);
      if (!NT_SUCCESS (status))
	{
	  debug_printf ("RtlFirstFreeAce: %p", status);
	  return NULL;
	}
      dacl->AclSize = (char *) ace - (char *) dacl;
      RtlSetDaclSecurityDescriptor (psd, TRUE, dacl, FALSE);
    }
  return psd;
}

