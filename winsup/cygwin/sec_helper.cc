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

char *
convert_sid_to_string_sid (PSID psid, char *sid_str)
{
  char t[32];
  DWORD i;

  if (!psid || !sid_str)
    return NULL;
  strcpy (sid_str, "S-1-");
  __small_sprintf(t, "%u", GetSidIdentifierAuthority (psid)->Value[5]);
  strcat (sid_str, t);
  for (i = 0; i < *GetSidSubAuthorityCount (psid); ++i)
    {
      __small_sprintf(t, "-%lu", *GetSidSubAuthority (psid, i));
      strcat (sid_str, t);
    }
  return sid_str;
}

PSID
get_sid (PSID psid, DWORD s, DWORD cnt, DWORD *r)
{
  DWORD i;

  if (!psid || s > 5 || cnt < 1 || cnt > 8)
    return NULL;

  InitializeSid(psid, &sid_auth[s], cnt);
  for (i = 0; i < cnt; ++i)
    memcpy ((char *) psid + 8 + sizeof (DWORD) * i, &r[i], sizeof (DWORD));
  return psid;
}

PSID
convert_string_sid_to_sid (PSID psid, const char *sid_str)
{
  char sid_buf[256];
  char *t, *lasts;
  DWORD cnt = 0;
  DWORD s = 0;
  DWORD i, r[8];

  if (!sid_str || strncmp (sid_str, "S-1-", 4))
    return NULL;

  strcpy (sid_buf, sid_str);

  for (t = sid_buf + 4, i = 0;
       cnt < 8 && (t = strtok_r (t, "-", &lasts));
       t = NULL, ++i)
    if (i == 0)
      s = strtoul (t, NULL, 10);
    else
      r[cnt++] = strtoul (t, NULL, 10);

  return get_sid (psid, s, cnt, r);
}

BOOL
get_pw_sid (PSID sid, struct passwd *pw)
{
  char *sp = pw->pw_gecos ? strrchr (pw->pw_gecos, ',') : NULL;

  if (!sp)
    return FALSE;
  return convert_string_sid_to_sid (sid, ++sp) != NULL;
}

BOOL
get_gr_sid (PSID sid, struct group *gr)
{
  return convert_string_sid_to_sid (sid, gr->gr_passwd) != NULL;
}

PSID
get_admin_sid ()
{
  static NO_COPY char admin_sid_buf[MAX_SID_LEN];
  static NO_COPY PSID admin_sid = NULL;

  if (!admin_sid)
    {
      admin_sid = (PSID) admin_sid_buf;
      convert_string_sid_to_sid (admin_sid, "S-1-5-32-544");
    }
  return admin_sid;
}

PSID
get_system_sid ()
{
  static NO_COPY char system_sid_buf[MAX_SID_LEN];
  static NO_COPY PSID system_sid = NULL;

  if (!system_sid)
    {
      system_sid = (PSID) system_sid_buf;
      convert_string_sid_to_sid (system_sid, "S-1-5-18");
    }
  return system_sid;
}

PSID
get_creator_owner_sid ()
{
  static NO_COPY char owner_sid_buf[MAX_SID_LEN];
  static NO_COPY PSID owner_sid = NULL;

  if (!owner_sid)
    {
      owner_sid = (PSID) owner_sid_buf;
      convert_string_sid_to_sid (owner_sid, "S-1-3-0");
    }
  return owner_sid;
}

PSID
get_world_sid ()
{
  static NO_COPY char world_sid_buf[MAX_SID_LEN];
  static NO_COPY PSID world_sid = NULL;

  if (!world_sid)
    {
      world_sid = (PSID) world_sid_buf;
      convert_string_sid_to_sid (world_sid, "S-1-1-0");
    }
  return world_sid;
}

int
get_id_from_sid (PSID psid, BOOL search_grp, int *type)
{
  if (!IsValidSid (psid))
    {
      __seterrno ();
      small_printf ("IsValidSid failed with %E");
      return -1;
    }

  /* First try to get SID from passwd or group entry */
  if (allow_ntsec)
    {
      char sidbuf[MAX_SID_LEN];
      PSID sid = (PSID) sidbuf;
      int id = -1;

      if (!search_grp)
	{
	  struct passwd *pw;
	  while ((pw = getpwent ()) != NULL)
	    {
	      if (get_pw_sid (sid, pw) && EqualSid (psid, sid))
		{
		  id = pw->pw_uid;
		  break;
		}
	    }
	  endpwent ();
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
	  while ((gr = getgrent ()) != NULL)
	    {
	      if (get_gr_sid (sid, gr) && EqualSid (psid, sid))
		{
		  id = gr->gr_gid;
		  break;
		}
	    }
	  endgrent ();
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
      char account[MAX_USER_NAME];
      char domain[MAX_COMPUTERNAME_LENGTH+1];
      DWORD acc_len = MAX_USER_NAME;
      DWORD dom_len = MAX_COMPUTERNAME_LENGTH+1;
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

int
get_id_from_sid (PSID psid, BOOL search_grp)
{
  return get_id_from_sid (psid, search_grp, NULL);
}

BOOL
legal_sid_type (SID_NAME_USE type)
{
  return type == SidTypeUser || type == SidTypeGroup
		 || SidTypeAlias || SidTypeWellKnownGroup;
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

BOOL
lookup_name (const char *name, const char *logsrv, PSID ret_sid)
{
  char sidbuf[MAX_SID_LEN];
  PSID sid = (PSID) sidbuf;
  DWORD sidlen;
  char domuser[MAX_COMPUTERNAME_LENGTH+MAX_USER_NAME+1];
  char dom[MAX_COMPUTERNAME_LENGTH+1];
  DWORD domlen;
  SID_NAME_USE acc_type;

  debug_printf ("name  : %s", name ? name : "NULL");

  if (!name)
    return FALSE;

  if (cygheap->user.domain ())
    {
      strcat (strcat (strcpy (domuser, cygheap->user.domain ()), "\\"), name);
      if (LookupAccountName (NULL, domuser,
			     sid, (sidlen = MAX_SID_LEN, &sidlen),
			     dom, (domlen = MAX_COMPUTERNAME_LENGTH, &domlen),
			     &acc_type)
	  && legal_sid_type (acc_type))
	goto got_it;
      if (logsrv && *logsrv
	  && LookupAccountName (logsrv, domuser,
				sid, (sidlen = MAX_SID_LEN, &sidlen),
				dom, (domlen = MAX_COMPUTERNAME_LENGTH,&domlen),
				&acc_type)
	  && legal_sid_type (acc_type))
	goto got_it;
    }
  if (logsrv && *logsrv)
    {
      if (LookupAccountName (logsrv, name,
			     sid, (sidlen = MAX_SID_LEN, &sidlen),
			     dom, (domlen = MAX_COMPUTERNAME_LENGTH, &domlen),
			     &acc_type)
	  && legal_sid_type (acc_type))
	goto got_it;
      if (acc_type == SidTypeDomain)
	{
	  strcat (strcat (strcpy (domuser, dom), "\\"), name);
	  if (LookupAccountName (logsrv, domuser,
				 sid,(sidlen = MAX_SID_LEN, &sidlen),
				 dom,(domlen = MAX_COMPUTERNAME_LENGTH,&domlen),
				 &acc_type))
	    goto got_it;
	}
    }
  if (LookupAccountName (NULL, name,
			 sid, (sidlen = MAX_SID_LEN, &sidlen),
			 dom, (domlen = 100, &domlen),
			 &acc_type)
      && legal_sid_type (acc_type))
    goto got_it;
  if (acc_type == SidTypeDomain)
    {
      strcat (strcat (strcpy (domuser, dom), "\\"), name);
      if (LookupAccountName (NULL, domuser,
			     sid, (sidlen = MAX_SID_LEN, &sidlen),
			     dom, (domlen = MAX_COMPUTERNAME_LENGTH, &domlen),
			     &acc_type))
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
