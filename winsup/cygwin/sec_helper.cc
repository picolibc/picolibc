/* sec_helper.cc: NT security helper functions

   Copyright 2000, 2001 Cygnus Solutions.

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
#include <errno.h>
#include <limits.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/acl.h>
#include <ctype.h>
#include <wingdi.h>
#include <winuser.h>
#include <wininet.h>
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

SID_IDENTIFIER_AUTHORITY sid_auth[] = {
        {SECURITY_NULL_SID_AUTHORITY},
        {SECURITY_WORLD_SID_AUTHORITY},
        {SECURITY_LOCAL_SID_AUTHORITY},
        {SECURITY_CREATOR_SID_AUTHORITY},
        {SECURITY_NON_UNIQUE_AUTHORITY},
        {SECURITY_NT_AUTHORITY}
};

cygsid well_known_admin_sid ("S-1-5-32-544");
cygsid well_known_system_sid ("S-1-5-18");
cygsid well_known_creator_owner_sid ("S-1-3-0");
cygsid well_known_world_sid ("S-1-1-0");

char *
cygsid::string (char *nsidstr)
{
  char t[32];
  DWORD i;

  if (!psid || !nsidstr)
    return NULL;
  strcpy (nsidstr, "S-1-");
  __small_sprintf(t, "%u", GetSidIdentifierAuthority (psid)->Value[5]);
  strcat (nsidstr, t);
  for (i = 0; i < *GetSidSubAuthorityCount (psid); ++i)
    {
      __small_sprintf(t, "-%lu", *GetSidSubAuthority (psid, i));
      strcat (nsidstr, t);
    }
  return nsidstr;
}

PSID
cygsid::get_sid (DWORD s, DWORD cnt, DWORD *r)
{
  DWORD i;

  if (s > 5 || cnt < 1 || cnt > 8)
    return NULL;
  set ();
  InitializeSid(psid, &sid_auth[s], cnt);
  for (i = 0; i < cnt; ++i)
    memcpy ((char *) psid + 8 + sizeof (DWORD) * i, &r[i], sizeof (DWORD));
  return psid;
}

const PSID
cygsid::getfromstr (const char *nsidstr)
{
  char sid_buf[256];
  char *t, *lasts;
  DWORD cnt = 0;
  DWORD s = 0;
  DWORD i, r[8];

  if (!nsidstr || strncmp (nsidstr, "S-1-", 4))
    return NULL;

  strcpy (sid_buf, nsidstr);

  for (t = sid_buf + 4, i = 0;
       cnt < 8 && (t = strtok_r (t, "-", &lasts));
       t = NULL, ++i)
    if (i == 0)
      s = strtoul (t, NULL, 10);
    else
      r[cnt++] = strtoul (t, NULL, 10);

  return get_sid (s, cnt, r);
}

BOOL
cygsid::getfrompw (struct passwd *pw)
{
  char *sp = pw->pw_gecos ? strrchr (pw->pw_gecos, ',') : NULL;

  if (!sp)
    return FALSE;
  return (*this = ++sp) != NULL;
}

BOOL
cygsid::getfromgr (struct group *gr)
{
  return (*this = gr->gr_passwd) != NULL;
}

int
cygsid::get_id (BOOL search_grp, int *type)
{
  if (!psid)
    {
      set_errno (EINVAL);
      return -1;
    }
  if (!IsValidSid (psid))
    {
      __seterrno ();
      small_printf ("IsValidSid failed with %E");
      return -1;
    }

  /* First try to get SID from passwd or group entry */
  if (allow_ntsec)
    {
      cygsid sid;
      int id = -1;

      if (!search_grp)
	{
	  struct passwd *pw;
	  for (int pidx = 0; (pw = internal_getpwent (pidx)); ++pidx)
	    {
	      if (sid.getfrompw (pw) && sid == psid)
		{
		  id = pw->pw_uid;
		  break;
		}
	    }
	  if (id >= 0)
	    {
	      if (type)
		*type = USER;
	      return id;
	    }
	}
      if (search_grp || type)
	{
	  struct group *gr;
	  for (int gidx = 0; (gr = internal_getgrent (gidx)); ++gidx)
	    {
	      if (sid.getfromgr (gr) && sid == psid)
		{
		  id = gr->gr_gid;
		  break;
		}
	    }
	  if (id >= 0)
	    {
	      if (type)
		*type = GROUP;
	      return id;
	    }
	}
    }

  /* We use the RID as default UID/GID */
  int id = *GetSidSubAuthority(psid, *GetSidSubAuthorityCount(psid) - 1);

  /*
   * The RID maybe -1 if accountname == computername.
   * In this case we search for the accountname in the passwd and group files.
   * If type is needed, we search in each case.
   */
  if (id == -1 || type)
    {
      char account[UNLEN + 1];
      char domain[INTERNET_MAX_HOST_NAME_LENGTH + 1];
      DWORD acc_len = UNLEN + 1;
      DWORD dom_len = INTERNET_MAX_HOST_NAME_LENGTH + 1;
      SID_NAME_USE acc_type;

      if (!LookupAccountSid (NULL, psid, account, &acc_len,
			     domain, &dom_len, &acc_type))
	{
	  __seterrno ();
	  return -1;
	}

      switch (acc_type)
	{
	  case SidTypeGroup:
	  case SidTypeAlias:
	  case SidTypeWellKnownGroup:
	    if (type)
	      *type = GROUP;
	    if (id == -1)
	      {
		struct group *gr = getgrnam (account);
		if (gr)
		  id = gr->gr_gid;
	      }
	    break;
	  case SidTypeUser:
	    if (type)
	      *type = USER;
	    if (id == -1)
	      {
		struct passwd *pw = getpwnam (account);
		if (pw)
		  id = pw->pw_uid;
	      }
	    break;
	  default:
	    break;
	}
    }
  if (id == -1)
    id = getuid ();
  return id;
}

static inline BOOL
legal_sid_type (SID_NAME_USE type)
{
  return type == SidTypeUser  || type == SidTypeGroup
      || type == SidTypeAlias || type == SidTypeWellKnownGroup;
}

BOOL
is_grp_member (uid_t uid, gid_t gid)
{
  extern int getgroups (int, gid_t *, gid_t, const char *);
  BOOL grp_member = TRUE;

  struct passwd *pw = getpwuid (uid);
  gid_t grps[NGROUPS_MAX];
  int cnt = getgroups (NGROUPS_MAX, grps,
		       pw ? pw->pw_gid : myself->gid,
		       pw ? pw->pw_name : cygheap->user.name ());
  int i;
  for (i = 0; i < cnt; ++i)
    if (grps[i] == gid)
      break;
  grp_member = (i < cnt);
  return grp_member;
}

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
  debug_printf ("LookupAccountName(%s) %E", name);
  __seterrno ();
  return FALSE;

got_it:
  debug_printf ("sid : [%d]", *GetSidSubAuthority((PSID) sid,
			      *GetSidSubAuthorityCount((PSID) sid) - 1));

  if (ret_sid)
    memcpy (ret_sid, sid, sidlen);

  return TRUE;
}

#undef SIDLEN
#undef DOMLEN

int
set_process_privilege (const char *privilege, BOOL enable)
{
  HANDLE hToken = NULL;
  LUID restore_priv;
  TOKEN_PRIVILEGES new_priv;
  int ret = -1;

  if (!OpenProcessToken (hMainProc, TOKEN_ADJUST_PRIVILEGES, &hToken))
    {
      __seterrno ();
      goto out;
    }

  if (!LookupPrivilegeValue (NULL, privilege, &restore_priv))
    {
      __seterrno ();
      goto out;
    }

  new_priv.PrivilegeCount = 1;
  new_priv.Privileges[0].Luid = restore_priv;
  new_priv.Privileges[0].Attributes = enable ? SE_PRIVILEGE_ENABLED : 0;

  if (!AdjustTokenPrivileges (hToken, FALSE, &new_priv, 0, NULL, NULL))
    {
      __seterrno ();
      goto out;
    }

  ret = 0;

out:
  if (hToken)
    CloseHandle (hToken);

  syscall_printf ("%d = set_process_privilege (%s, %d)",ret, privilege, enable);
  return ret;
}
