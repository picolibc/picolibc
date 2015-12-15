/* sec_helper.cc: NT security helper functions

   Written by Corinna Vinschen <corinna@vinschen.de>

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

#include "winsup.h"
#include <stdlib.h>
#include <stdarg.h>
#include <cygwin/acl.h>
#include <sys/queue.h>
#include <authz.h>
#include <wchar.h>
#include "cygerrno.h"
#include "security.h"
#include "path.h"
#include "fhandler.h"
#include "dtable.h"
#include "pinfo.h"
#include "cygheap.h"
#include "ntdll.h"
#include "ldap.h"

/* General purpose security attribute objects for global use. */
static NO_COPY_RO SECURITY_DESCRIPTOR null_sdp =
	{ SECURITY_DESCRIPTOR_REVISION, 0, SE_DACL_PRESENT,
	  NULL, NULL, NULL, NULL };
SECURITY_ATTRIBUTES NO_COPY_RO sec_none =
	{ sizeof (SECURITY_ATTRIBUTES), NULL, TRUE };
SECURITY_ATTRIBUTES NO_COPY_RO sec_none_nih =
	{ sizeof (SECURITY_ATTRIBUTES), NULL, FALSE };
SECURITY_ATTRIBUTES NO_COPY_RO sec_all =
	{ sizeof (SECURITY_ATTRIBUTES), &null_sdp, TRUE };
SECURITY_ATTRIBUTES NO_COPY_RO sec_all_nih =
	{ sizeof (SECURITY_ATTRIBUTES), &null_sdp, FALSE };

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
MKSID (well_known_local_service_sid, "S-1-5-19",
       SECURITY_NT_AUTHORITY, 1, SECURITY_LOCAL_SERVICE_RID);
MKSID (well_known_network_service_sid, "S-1-5-20",
       SECURITY_NT_AUTHORITY, 1, SECURITY_NETWORK_SERVICE_RID);
MKSID (well_known_builtin_sid, "S-1-5-32",
       SECURITY_NT_AUTHORITY, 1, SECURITY_BUILTIN_DOMAIN_RID);
MKSID (well_known_admins_sid, "S-1-5-32-544",
       SECURITY_NT_AUTHORITY, 2, SECURITY_BUILTIN_DOMAIN_RID,
				 DOMAIN_ALIAS_RID_ADMINS);
MKSID (well_known_users_sid, "S-1-5-32-545",
       SECURITY_NT_AUTHORITY, 2, SECURITY_BUILTIN_DOMAIN_RID,
				 DOMAIN_ALIAS_RID_USERS);
MKSID (trusted_installer_sid,
       "S-1-5-80-956008885-3418522649-1831038044-1853292631-2271478464",
       SECURITY_NT_AUTHORITY, SECURITY_SERVICE_ID_RID_COUNT,
       SECURITY_SERVICE_ID_BASE_RID, 956008885U, 3418522649U, 1831038044U,
       1853292631U, 2271478464U);
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

uid_t
cygpsid::get_id (BOOL search_grp, int *type, cyg_ldap *pldap)
{
    /* First try to get SID from group, then passwd */
  uid_t id = ILLEGAL_UID;

  if (search_grp)
    {
      struct group *gr;
      if (cygheap->user.groups.pgsid == psid)
	id = myself->gid;
      else if (sid_id_auth (psid) == 22 && cygheap->pg.nss_grp_db ())
	{
	  /* Samba UNIX group?  Try to map to Cygwin gid.  If there's no
	     mapping in the cache, try to fetch it from the configured
	     RFC 2307 domain (see last comment in cygheap_domain_info::init()
	     for more information) and add it to the mapping cache.
	     If this is a user, not a group, make sure to skip the subsequent
	     internal_getgrsid call, otherwise we end up with a fake group
	     entry for a UNIX user account. */
	  if (sid_sub_auth (psid, 0) == 2)
	    {
	      gid_t gid = sid_sub_auth_rid (psid);
	      gid_t map_gid = cygheap->ugid_cache.get_gid (gid);
	      if (map_gid == ILLEGAL_GID)
		{
		  if (pldap->open (cygheap->dom.get_rfc2307_domain ())
		      == NO_ERROR)
		    map_gid = pldap->remap_gid (gid);
		  if (map_gid == ILLEGAL_GID) 
		    map_gid = MAP_UNIX_TO_CYGWIN_ID (gid);
		  cygheap->ugid_cache.add_gid (gid, map_gid);
		}
	      id = (uid_t) map_gid;
	    }
	}
      else if ((gr = internal_getgrsid (*this, pldap)))
	id = gr->gr_gid;
      if ((gid_t) id != ILLEGAL_GID)
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
      else if (sid_id_auth (psid) == 22 && sid_sub_auth (psid, 0) == 1
	       && cygheap->pg.nss_pwd_db ())
	{
	  /* Samba UNIX user.  See comment above. */
	  uid_t uid = sid_sub_auth_rid (psid);
	  uid_t map_uid = cygheap->ugid_cache.get_uid (uid);
	  if (map_uid == ILLEGAL_UID)
	    {
	      if (pldap->open (cygheap->dom.get_rfc2307_domain ()) == NO_ERROR)
		map_uid = pldap->remap_uid (uid);
	      if (map_uid == ILLEGAL_UID)
		map_uid = MAP_UNIX_TO_CYGWIN_ID (uid);
	      cygheap->ugid_cache.add_uid (uid, map_uid);
	    }
	  id = map_uid;
	}
      else if ((pw = internal_getpwsid (*this, pldap)))
	id = pw->pw_uid;
      if (id != ILLEGAL_UID && type)
	*type = USER;
    }
  return id;
}

PWCHAR
cygpsid::pstring (PWCHAR nsidstr) const
{
  UNICODE_STRING sid;

  if (!psid || !nsidstr)
    return NULL;
  RtlInitEmptyUnicodeString (&sid, nsidstr, 256);
  RtlConvertSidToUnicodeString (&sid, psid, FALSE);
  return nsidstr + sid.Length / sizeof (WCHAR);
}

PWCHAR
cygpsid::string (PWCHAR nsidstr) const
{
  if (pstring (nsidstr))
    return nsidstr;
  return NULL;
}

char *
cygpsid::pstring (char *nsidstr) const
{
  char *t;
  DWORD i;

  if (!psid || !nsidstr)
    return NULL;
  strcpy (nsidstr, "S-1-");
  t = nsidstr + sizeof ("S-1-") - 1;
  t += __small_sprintf (t, "%u", sid_id_auth (psid));
  for (i = 0; i < sid_sub_auth_count (psid); ++i)
    t += __small_sprintf (t, "-%lu", sid_sub_auth (psid, i));
  return t;
}

char *
cygpsid::string (char *nsidstr) const
{
  if (pstring (nsidstr))
    return nsidstr;
  return NULL;
}

PSID
cygsid::get_sid (DWORD s, DWORD cnt, DWORD *r, bool well_known)
{
  DWORD i;
  SID_IDENTIFIER_AUTHORITY sid_auth = { SECURITY_NULL_SID_AUTHORITY };
# define SECURITY_NT_AUTH 5

  /* 2015-10-22: Note that we let slip SIDs with a subauthority count of 0.
     There are systems, which generate the SID S-1-0 as group ownership SID,
     see https://cygwin.com/ml/cygwin/2015-10/msg00141.html. */
  if (s > 255 || cnt > SID_MAX_SUB_AUTHORITIES)
    {
      psid = NO_SID;
      return NULL;
    }
  sid_auth.Value[5] = s;
  set ();
  RtlInitializeSid (psid, &sid_auth, cnt);
  PISID dsid = (PISID) psid;
  for (i = 0; i < cnt; ++i)
    dsid->SubAuthority[i] = r[i];
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
		      || r[0] != SECURITY_NT_NON_UNIQUE);
  return psid;
}

const PSID
cygsid::getfromstr (PCWSTR nsidstr, bool well_known)
{
  PWCHAR lasts;
  DWORD s, cnt = 0;
  DWORD r[SID_MAX_SUB_AUTHORITIES];

  if (nsidstr && !wcsncmp (nsidstr, L"S-1-", 4))
    {
      s = wcstoul (nsidstr + 4, &lasts, 10);
      while (cnt < SID_MAX_SUB_AUTHORITIES && *lasts == '-')
	r[cnt++] = wcstoul (lasts + 1, &lasts, 10);
      if (!*lasts)
	return get_sid (s, cnt, r, well_known);
    }
  return psid = NO_SID;
}

const PSID
cygsid::getfromstr (const char *nsidstr, bool well_known)
{
  char *lasts;
  DWORD s, cnt = 0;
  DWORD r[SID_MAX_SUB_AUTHORITIES];

  if (nsidstr && !strncmp (nsidstr, "S-1-", 4))
    {
      s = strtoul (nsidstr + 4, &lasts, 10);
      while (cnt < SID_MAX_SUB_AUTHORITIES && *lasts == '-')
	r[cnt++] = strtoul (lasts + 1, &lasts, 10);
      if (!*lasts)
	return get_sid (s, cnt, r, well_known);
    }
  return psid = NO_SID;
}

const PSID
cygsid::create (DWORD auth, DWORD subauth_cnt, ...)
{
  va_list ap;
  PSID sid;

  if (subauth_cnt > SID_MAX_SUB_AUTHORITIES)
    return NULL;

  DWORD subauth[subauth_cnt];

  va_start (ap, subauth_cnt);
  for (DWORD i = 0; i < subauth_cnt; ++i)
    subauth[i] = va_arg (ap, DWORD);
  sid = get_sid (auth, subauth_cnt, subauth, false);
  va_end (ap);
  return sid;
}

bool
cygsid::append (DWORD rid)
{
  if (psid == NO_SID)
    return false;
  PISID dsid = (PISID) psid;
  if (dsid->SubAuthorityCount >= SID_MAX_SUB_AUTHORITIES)
    return false;
  dsid->SubAuthority[dsid->SubAuthorityCount++] = rid;
  return true;
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
  { SE_INC_WORKING_SET_NAME,		false },
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
    debug_printf ("%d = set_privilege((token %p) %W, %d)", ret, token,
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
  set_privilege (token, SE_CREATE_GLOBAL_PRIVILEGE, true);
#endif
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
      debug_printf ("RtlCreateAcl: %y", status);
      return false;
    }
  if (sid1)
    {
      status = RtlAddAccessAllowedAce (acl, ACL_REVISION, GENERIC_ALL, sid1);
      if (!NT_SUCCESS (status))
	debug_printf ("RtlAddAccessAllowedAce(sid1) %y", status);
    }
  if (original && (psid = cygheap->user.saved_sid ())
      && psid != sid1 && psid != well_known_system_sid)
    {
      status = RtlAddAccessAllowedAce (acl, ACL_REVISION, GENERIC_ALL, psid);
      if (!NT_SUCCESS (status))
	debug_printf ("RtlAddAccessAllowedAce(original) %y", status);
    }
  if (sid2)
    {
      status = RtlAddAccessAllowedAce (acl, ACL_REVISION, access2, sid2);
      if (!NT_SUCCESS (status))
	debug_printf ("RtlAddAccessAllowedAce(sid2) %y", status);
    }
  if (admins)
    {
      status = RtlAddAccessAllowedAce (acl, ACL_REVISION, GENERIC_ALL,
				       well_known_admins_sid);
      if (!NT_SUCCESS (status))
	debug_printf ("RtlAddAccessAllowedAce(admin) %y", status);
    }
  status = RtlAddAccessAllowedAce (acl, ACL_REVISION, GENERIC_ALL,
				   well_known_system_sid);
  if (!NT_SUCCESS (status))
    debug_printf ("RtlAddAccessAllowedAce(system) %y", status);
  status = RtlFirstFreeAce (acl, &pAce);
  if (NT_SUCCESS (status) && pAce)
    acl->AclSize = (char *) pAce - (char *) acl;
  else
    debug_printf ("RtlFirstFreeAce: %y", status);

  return true;
}

PSECURITY_ATTRIBUTES __reg3
__sec_user (PVOID sa_buf, PSID sid1, PSID sid2, DWORD access2, BOOL inherit)
{
  PSECURITY_ATTRIBUTES psa = (PSECURITY_ATTRIBUTES) sa_buf;
  PISECURITY_DESCRIPTOR psd = (PISECURITY_DESCRIPTOR)
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
    debug_printf ("RtlSetDaclSecurityDescriptor %y", status);

  psa->nLength = sizeof (SECURITY_ATTRIBUTES);
  psa->lpSecurityDescriptor = psd;
  psa->bInheritHandle = inherit;
  return psa;
}

/* Helper function to create a file security descriptor which allows
   full access to admins, system, and the sid given as parameter.  See
   try_to_bin for how it's used. */

PSECURITY_DESCRIPTOR
_recycler_sd (void *buf, bool users, bool dir)
{
  NTSTATUS status;
  PISECURITY_DESCRIPTOR psd = (PISECURITY_DESCRIPTOR) buf;

  if (!psd)
    return NULL;
  RtlCreateSecurityDescriptor (psd, SECURITY_DESCRIPTOR_REVISION);
  PACL dacl = (PACL) (psd + 1);
  RtlCreateAcl (dacl, MAX_DACL_LEN (3), ACL_REVISION);
  RtlAddAccessAllowedAceEx (dacl, ACL_REVISION,
			    dir ? SUB_CONTAINERS_AND_OBJECTS_INHERIT
				: NO_INHERITANCE,
			    FILE_ALL_ACCESS, well_known_admins_sid);
  RtlAddAccessAllowedAceEx (dacl, ACL_REVISION,
			    dir ? SUB_CONTAINERS_AND_OBJECTS_INHERIT
				: NO_INHERITANCE,
			    FILE_ALL_ACCESS, well_known_system_sid);
  if (users)
    RtlAddAccessAllowedAceEx (dacl, ACL_REVISION, INHERIT_NO_PROPAGATE,
			      FILE_GENERIC_READ | FILE_GENERIC_EXECUTE
			      | FILE_APPEND_DATA | FILE_WRITE_ATTRIBUTES,
			      well_known_users_sid);
  else
    RtlAddAccessAllowedAceEx (dacl, ACL_REVISION,
			      dir ? SUB_CONTAINERS_AND_OBJECTS_INHERIT
				  : NO_INHERITANCE,
			      FILE_ALL_ACCESS, cygheap->user.sid ());
  LPVOID ace;
  status = RtlFirstFreeAce (dacl, &ace);
  if (!NT_SUCCESS (status))
    {
      debug_printf ("RtlFirstFreeAce: %y", status);
      return NULL;
    }
  dacl->AclSize = (char *) ace - (char *) dacl;
  RtlSetDaclSecurityDescriptor (psd, TRUE, dacl, FALSE);
  /* If the directory DACL is not marked as protected, shell32 thinks
     the recycle dir is corrupted.  As soon as Explorer accesses the
     Recycler, the user will get a GUI dialog "The Recycle Bin on X:\
     is corrupted. Do you want to empty the Recycle Bin for this drive?"
     Of course we want to avoid that. */
  if (dir)
    psd->Control |= SE_DACL_PROTECTED;
  return psd;
}

/* Helper function to create an event security descriptor which only allows
   specific access to everyone.  Only the creating process has all access
   rights. */

PSECURITY_DESCRIPTOR
_everyone_sd (void *buf, ACCESS_MASK access)
{
  NTSTATUS status;
  PISECURITY_DESCRIPTOR psd = (PISECURITY_DESCRIPTOR) buf;

  if (psd)
    {
      RtlCreateSecurityDescriptor (psd, SECURITY_DESCRIPTOR_REVISION);
      PACL dacl = (PACL) (psd + 1);
      RtlCreateAcl (dacl, MAX_DACL_LEN (1), ACL_REVISION);
      status = RtlAddAccessAllowedAce (dacl, ACL_REVISION, access,
				       well_known_world_sid);
      if (!NT_SUCCESS (status))
	{
	  debug_printf ("RtlAddAccessAllowedAce: %y", status);
	  return NULL;
	}
      LPVOID ace;
      status = RtlFirstFreeAce (dacl, &ace);
      if (!NT_SUCCESS (status))
	{
	  debug_printf ("RtlFirstFreeAce: %y", status);
	  return NULL;
	}
      dacl->AclSize = (char *) ace - (char *) dacl;
      RtlSetDaclSecurityDescriptor (psd, TRUE, dacl, FALSE);
    }
  return psd;
}

static NO_COPY muto authz_guard;
static LUID authz_dummy_luid = { 0 };

class authz_ctx_cache_entry
{
  SLIST_ENTRY (authz_ctx_cache_entry)	ctx_next;
  cygsid				sid;
  AUTHZ_CLIENT_CONTEXT_HANDLE		ctx_hdl;

  authz_ctx_cache_entry ()
  : sid (NO_SID), ctx_hdl (NULL)
  {
    ctx_next.sle_next = NULL;
  }
  authz_ctx_cache_entry (bool)
  : sid (NO_SID), ctx_hdl (NULL)
  {
    ctx_next.sle_next = NULL;
  }
  void set (PSID psid, AUTHZ_CLIENT_CONTEXT_HANDLE hdl)
  {
    sid = psid;
    ctx_hdl = hdl;
  }
  bool is (PSID psid) const { return RtlEqualSid (sid, psid); }
  AUTHZ_CLIENT_CONTEXT_HANDLE context () const { return ctx_hdl; }

  friend class authz_ctx_cache;
};

class authz_ctx_cache
{
  SLIST_HEAD (, authz_ctx_cache_entry) ctx_list;

  AUTHZ_CLIENT_CONTEXT_HANDLE context (PSID);

  friend class authz_ctx;
};

class authz_ctx
{
  AUTHZ_RESOURCE_MANAGER_HANDLE authz_hdl;
  AUTHZ_CLIENT_CONTEXT_HANDLE user_ctx_hdl;
  authz_ctx_cache ctx_cache;
  operator AUTHZ_RESOURCE_MANAGER_HANDLE ();

  friend class authz_ctx_cache;
public:
  bool get_user_attribute (mode_t *, PSECURITY_DESCRIPTOR, PSID);
};

/* Authz handles are not inheritable. */
static NO_COPY authz_ctx authz;

authz_ctx::operator AUTHZ_RESOURCE_MANAGER_HANDLE ()
{
  if (!authz_hdl)
    {
      /* Create handle to Authz resource manager */
      authz_guard.init ("authz_guard")->acquire ();
      if (!authz_hdl
	  && !AuthzInitializeResourceManager (AUTHZ_RM_FLAG_NO_AUDIT,
					      NULL, NULL, NULL, NULL,
					      &authz_hdl))
	debug_printf ("AuthzInitializeResourceManager, %E");
      authz_guard.release ();
    }
  return authz_hdl;
}

AUTHZ_CLIENT_CONTEXT_HANDLE
authz_ctx_cache::context (PSID user_sid)
{
  authz_ctx_cache_entry *entry;
  AUTHZ_CLIENT_CONTEXT_HANDLE ctx_hdl = NULL;

  SLIST_FOREACH (entry, &ctx_list, ctx_next)
    {
      if (entry->is (user_sid))
	return entry->context ();
    }
  entry = new authz_ctx_cache_entry (true);
  /* If the user is the current user, prefer to create the context from the
     token, as outlined in MSDN. */
  if (RtlEqualSid (user_sid, cygheap->user.sid ())
      && !AuthzInitializeContextFromToken (0, cygheap->user.issetuid ()
					   ?  cygheap->user.primary_token ()
					   : hProcToken,
					   authz, NULL, authz_dummy_luid,
					   NULL, &ctx_hdl))
    debug_printf ("AuthzInitializeContextFromToken, %E");
  /* In any other case, create the context from the user SID. */
  else if (!AuthzInitializeContextFromSid (0, user_sid, authz, NULL,
					   authz_dummy_luid, NULL, &ctx_hdl))
    debug_printf ("AuthzInitializeContextFromSid, %E");
  else
    {
      entry->set (user_sid, ctx_hdl);
      authz_guard.acquire ();
      SLIST_INSERT_HEAD (&ctx_list, entry, ctx_next);
      authz_guard.release ();
      return entry->context ();
    }
  delete entry;
  return NULL;
}

/* Ask Authz for the effective user permissions of the user with SID user_sid
   on the object with security descriptor psd.  We're caching the handles for
   the Authz resource manager and the user contexts. */
bool
authz_ctx::get_user_attribute (mode_t *attribute, PSECURITY_DESCRIPTOR psd,
			       PSID user_sid)
{
  /* If the owner is the main user of the process token (not some impersonated
     user), cache the user context in the global user_ctx_hdl variable. */
  AUTHZ_CLIENT_CONTEXT_HANDLE ctx_hdl = NULL;
  if (RtlEqualSid (user_sid, cygheap->user.sid ())
      && !cygheap->user.issetuid ())
    {
      if (!user_ctx_hdl)
	{
	  authz_guard.acquire ();
	  if (!AuthzInitializeContextFromToken (0, hProcToken, authz, NULL,
						authz_dummy_luid, NULL,
						&user_ctx_hdl))
	    debug_printf ("AuthzInitializeContextFromToken, %E");
	  authz_guard.release ();
	}
      if (user_ctx_hdl)
	ctx_hdl = user_ctx_hdl;
    }
  if (!ctx_hdl && !(ctx_hdl = ctx_cache.context (user_sid)))
    return false;
  /* All set, check access. */
  ACCESS_MASK access = 0;
  DWORD error = 0;
  AUTHZ_ACCESS_REQUEST req = {
    .DesiredAccess		= MAXIMUM_ALLOWED,
    .PrincipalSelfSid		= NULL,
    .ObjectTypeList		= NULL,
    .ObjectTypeListLength	= 0,
    .OptionalArguments		= NULL
  };
  AUTHZ_ACCESS_REPLY repl = {
    .ResultListLength		= 1,
    .GrantedAccessMask		= &access,
    .SaclEvaluationResults	= NULL,
    .Error			= &error
  };
  if (AuthzAccessCheck (0, ctx_hdl, &req, NULL, psd, NULL, 0, &repl, NULL))
    {
      if (access & FILE_READ_BITS)
	*attribute |= S_IROTH;
      if (access & FILE_WRITE_BITS)
	*attribute |= S_IWOTH;
      if (access & FILE_EXEC_BITS)
	*attribute |= S_IXOTH;
      return true;
    }
  return false;
}

bool
authz_get_user_attribute (mode_t *attribute, PSECURITY_DESCRIPTOR psd,
			  PSID user_sid)
{
  *attribute = 0;
  return authz.get_user_attribute (attribute, psd, user_sid);
}
