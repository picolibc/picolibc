/* uinfo.cc: user info (uid, gid, etc...)

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

#include "winsup.h"
#include <iptypes.h>
#include <lm.h>
#include <ntsecapi.h>
#include <wininet.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <wchar.h>
#include <sys/cygwin.h>
#include "cygerrno.h"
#include "pinfo.h"
#include "path.h"
#include "fhandler.h"
#include "dtable.h"
#include "cygheap.h"
#include "shared_info.h"
#include "registry.h"
#include "child_info.h"
#include "environ.h"
#include "tls_pbuf.h"
#include "miscfuncs.h"
#include "ntdll.h"
#include "ldap.h"
#include "cygserver_pwdgrp.h"

/* Initialize the part of cygheap_user that does not depend on files.
   The information is used in shared.cc to create the shared user_info
   region.  Final initialization occurs in uinfo_init */
void
cygheap_user::init ()
{
  WCHAR user_name[UNLEN + 1];
  DWORD user_name_len = UNLEN + 1;

  /* This method is only called if a Cygwin process gets started by a
     native Win32 process.  Try to get the username from the environment,
     first USERNAME (Win32), then USER (POSIX).  If that fails (which is
     very unlikely), it only has an impact if we don't have an entry in
     /etc/passwd for this user either.  In that case the username sticks
     to "unknown".  Since this is called early in initialization, and
     since we don't want to pull in a dependency to any other DLL except
     ntdll and kernel32 at this early stage, don't call GetUserName,
     GetUserNameEx, NetWkstaUserGetInfo, etc. */
  if (GetEnvironmentVariableW (L"USERNAME", user_name, user_name_len)
      || GetEnvironmentVariableW (L"USER", user_name, user_name_len))
    {
      char mb_user_name[user_name_len = sys_wcstombs (NULL, 0, user_name) + 1];
      sys_wcstombs (mb_user_name, user_name_len, user_name);
      set_name (mb_user_name);
    }
  else
    set_name ("unknown");

  NTSTATUS status;
  ULONG size;
  PSECURITY_DESCRIPTOR psd;

  status = NtQueryInformationToken (hProcToken, TokenPrimaryGroup,
				    &groups.pgsid, sizeof (cygsid), &size);
  if (!NT_SUCCESS (status))
    system_printf ("NtQueryInformationToken (TokenPrimaryGroup), %y", status);

  /* Get the SID from current process and store it in effec_cygsid */
  status = NtQueryInformationToken (hProcToken, TokenUser, &effec_cygsid,
				    sizeof (cygsid), &size);
  if (!NT_SUCCESS (status))
    {
      system_printf ("NtQueryInformationToken (TokenUser), %y", status);
      return;
    }

  /* Set token owner to the same value as token user */
  status = NtSetInformationToken (hProcToken, TokenOwner, &effec_cygsid,
				  sizeof (cygsid));
  if (!NT_SUCCESS (status))
    debug_printf ("NtSetInformationToken (TokenOwner), %y", status);

  /* Standard way to build a security descriptor with the usual DACL */
  PSECURITY_ATTRIBUTES sa_buf = (PSECURITY_ATTRIBUTES) alloca (1024);
  psd = (PSECURITY_DESCRIPTOR)
		(sec_user_nih (sa_buf, sid()))->lpSecurityDescriptor;

  BOOLEAN acl_exists, dummy;
  TOKEN_DEFAULT_DACL dacl;

  status = RtlGetDaclSecurityDescriptor (psd, &acl_exists, &dacl.DefaultDacl,
					 &dummy);
  if (NT_SUCCESS (status) && acl_exists && dacl.DefaultDacl)
    {

      /* Set the default DACL and the process DACL */
      status = NtSetInformationToken (hProcToken, TokenDefaultDacl, &dacl,
				      sizeof (dacl));
      if (!NT_SUCCESS (status))
	system_printf ("NtSetInformationToken (TokenDefaultDacl), %y", status);
      if ((status = NtSetSecurityObject (NtCurrentProcess (),
					 DACL_SECURITY_INFORMATION, psd)))
	system_printf ("NtSetSecurityObject, %y", status);
    }
  else
    system_printf("Cannot get dacl, %E");
}

/* Check if sid is an enabled SID in the token group list of the current
   effective token.  Note that we only check for ENABLED groups, not for
   INTEGRITY_ENABLED.  The latter just doesn't make sense in our scenario
   of using the group as primary group.

   This needs careful checking should we use check_token_membership in other
   circumstances. */
bool
check_token_membership (PSID sid)
{
  NTSTATUS status;
  ULONG size;
  tmp_pathbuf tp;
  PTOKEN_GROUPS groups = (PTOKEN_GROUPS) tp.w_get ();

  /* If impersonated, use impersonation token. */
  HANDLE tok = cygheap->user.issetuid () ? cygheap->user.primary_token ()
					 : hProcToken;
  status = NtQueryInformationToken (tok, TokenGroups, groups, 2 * NT_MAX_PATH,
				    &size);
  if (!NT_SUCCESS (status))
    debug_printf ("NtQueryInformationToken (TokenGroups) %y", status);
  else
    {
      for (DWORD pg = 0; pg < groups->GroupCount; ++pg)
	if (RtlEqualSid (sid, groups->Groups[pg].Sid)
	    && (groups->Groups[pg].Attributes & SE_GROUP_ENABLED))
	  return true;
    }
  return false;
}

static void
internal_getlogin (cygheap_user &user)
{
  struct passwd *pwd;
  struct group *pgrp, *grp, *grp2;
  cyg_ldap cldap;

  /* Fetch (and potentially generate) passwd and group entries for the user
     and the primary group in the token. */
  pwd = internal_getpwsid (user.sid (), &cldap);
  pgrp = internal_getgrsid (user.groups.pgsid, &cldap);
  if (!cygheap->pg.nss_cygserver_caching ())
    internal_getgroups (0, NULL, &cldap);
  if (!pwd)
    debug_printf ("user not found in passwd DB");
  else
    {
      cygsid gsid;

      user.set_name (pwd->pw_name);
      myself->uid = pwd->pw_uid;
      myself->gid = pwd->pw_gid;
      /* If the primary group in the passwd DB is different from the primary
	 group in the user token, we have to find the SID of that group and
	 try to override the token primary group. */
      if (!pgrp || myself->gid != pgrp->gr_gid)
	{
	  if (gsid.getfromgr (grp = internal_getgrgid (pwd->pw_gid, &cldap)))
	    {
	      /* We might have a group file with a group entry for the current
		 user's primary group, but the current user has no entry in
		 passwd.  If so, pw_gid is taken from windows and might
		 disagree with gr_gid from the group file.  Overwrite it. */
	      if ((grp2 = internal_getgrsid (gsid, &cldap)) && grp2 != grp)
		myself->gid = pwd->pw_gid = grp2->gr_gid;
	      /* Set primary group to the group in /etc/passwd, *iff*
		 the group in /etc/passwd is in the token *and* enabled. */
	      if (gsid != user.groups.pgsid)
		{
		  NTSTATUS status = STATUS_INVALID_PARAMETER;

		  if (check_token_membership (gsid))
		    {
		      status = NtSetInformationToken (hProcToken,
						      TokenPrimaryGroup,
						      &gsid, sizeof gsid);
		      if (!NT_SUCCESS (status))
			debug_printf ("NtSetInformationToken "
				      "(TokenPrimaryGroup), %y", status);
		    }
		  if (!NT_SUCCESS (status))
		    {
		      /* Revert the primary group setting and override the
			 setting in the passwd entry. */
		      if (pgrp)
			myself->gid = pwd->pw_gid = pgrp->gr_gid;
		    }
		  else
		    user.groups.pgsid = gsid;
		  clear_procimptoken ();
		}
	    }
	  else
	    debug_printf ("group not found in group DB");
	}
    }
  cygheap->user.ontherange (CH_HOME, pwd);
}

void
uinfo_init ()
{
  if (child_proc_info && !cygheap->user.has_impersonation_tokens ())
    return;

  if (!child_proc_info)
    internal_getlogin (cygheap->user); /* Set cygheap->user. */
  /* Conditions must match those in spawn to allow starting child
     processes with ruid != euid and rgid != egid. */
  else if (cygheap->user.issetuid ()
	   && cygheap->user.saved_uid == cygheap->user.real_uid
	   && cygheap->user.saved_gid == cygheap->user.real_gid
	   && !cygheap->user.groups.issetgroups ()
	   && !cygheap->user.setuid_to_restricted)
    {
      cygheap->user.reimpersonate ();
      return;
    }
  else
    cygheap->user.close_impersonation_tokens ();

  cygheap->user.saved_uid = cygheap->user.real_uid = myself->uid;
  cygheap->user.saved_gid = cygheap->user.real_gid = myself->gid;
  cygheap->user.external_token = NO_IMPERSONATION;
  cygheap->user.internal_token = NO_IMPERSONATION;
  cygheap->user.curr_primary_token = NO_IMPERSONATION;
  cygheap->user.curr_imp_token = NO_IMPERSONATION;
  cygheap->user.ext_token_is_restricted = false;
  cygheap->user.curr_token_is_restricted = false;
  cygheap->user.setuid_to_restricted = false;
  cygheap->user.set_saved_sid ();	/* Update the original sid */
  cygheap->user.deimpersonate ();
}

extern "C" int
getlogin_r (char *name, size_t namesize)
{
  const char *login = cygheap->user.name ();
  size_t len = strlen (login) + 1;
  if (len > namesize)
    return ERANGE;
  __try
    {
      strncpy (name, login, len);
    }
  __except (NO_ERROR)
    {
      return EFAULT;
    }
  __endtry
  return 0;
}

extern "C" char *
getlogin (void)
{
  static char username[UNLEN];
  int ret = getlogin_r (username, UNLEN);
  if (ret)
    {
      set_errno (ret);
      return NULL;
    }
  return username;
}

extern "C" uid_t
getuid32 (void)
{
  return cygheap->user.real_uid;
}

#ifdef __i386__
extern "C" __uid16_t
getuid (void)
{
  return cygheap->user.real_uid;
}
#else
EXPORT_ALIAS (getuid32, getuid)
#endif

extern "C" gid_t
getgid32 (void)
{
  return cygheap->user.real_gid;
}

#ifdef __i386__
extern "C" __gid16_t
getgid (void)
{
  return cygheap->user.real_gid;
}
#else
EXPORT_ALIAS (getgid32, getgid)
#endif

extern "C" uid_t
geteuid32 (void)
{
  return myself->uid;
}

#ifdef __i386__
extern "C" uid_t
geteuid (void)
{
  return myself->uid;
}
#else
EXPORT_ALIAS (geteuid32, geteuid)
#endif

extern "C" gid_t
getegid32 (void)
{
  return myself->gid;
}

#ifdef __i386__
extern "C" __gid16_t
getegid (void)
{
  return myself->gid;
}
#else
EXPORT_ALIAS (getegid32, getegid)
#endif

/* Not quite right - cuserid can change, getlogin can't */
extern "C" char *
cuserid (char *src)
{
  if (!src)
    return getlogin ();

  strcpy (src, getlogin ());
  return src;
}

const char *
cygheap_user::ontherange (homebodies what, struct passwd *pw)
{
  char homedrive_env_buf[3];
  char *newhomedrive = NULL;
  char *newhomepath = NULL;
  tmp_pathbuf tp;

  debug_printf ("what %d, pw %p", what, pw);
  if (what == CH_HOME)
    {
      char *p;

      if ((p = getenv ("HOME")))
	debug_printf ("HOME is already in the environment %s", p);
      if (!p)
	{
	  if (pw && pw->pw_dir && *pw->pw_dir)
	    {
	      debug_printf ("Set HOME (from account db) to %s", pw->pw_dir);
	      setenv ("HOME", pw->pw_dir, 1);
	    }
	  else
	    {
	      char home[strlen (name ()) + 8];

	      debug_printf ("Set HOME to default /home/USER");
	      __small_sprintf (home, "/home/%s", name ());
	      setenv ("HOME", home, 1);
	    }
	}
    }

  if (what != CH_HOME && homepath == NULL)
    {
      WCHAR wuser[UNLEN + 1];
      WCHAR wlogsrv[INTERNET_MAX_HOST_NAME_LENGTH + 3];
      PUSER_INFO_3 ui = NULL;
      char *homepath_env_buf = tp.c_get ();
      WCHAR profile[MAX_PATH];
      WCHAR win_id[UNLEN + 1]; /* Large enough for SID */

      homepath_env_buf[0] = homepath_env_buf[1] = '\0';
      /* Use Cygwin home as HOMEDRIVE/HOMEPATH in the first place.  This
	 should cover it completely, in theory.  Still, it might be the
	 wrong choice in the long run, which might demand to set HOMEDRIVE/
	 HOMEPATH to the correct values per Windows.  Keep the entire rest
	 of the code mainly for this reason, so switching is easy. */
      pw = internal_getpwsid (sid ());
      if (pw && pw->pw_dir && *pw->pw_dir)
	cygwin_conv_path (CCP_POSIX_TO_WIN_A, pw->pw_dir, homepath_env_buf,
			  NT_MAX_PATH);
      /* First fallback: Windows path per Windows user DB. */
      else if (logsrv ())
	{
	  sys_mbstowcs (wlogsrv, sizeof (wlogsrv) / sizeof (*wlogsrv),
			logsrv ());
	  sys_mbstowcs (wuser, sizeof wuser / sizeof *wuser, winname ());
	  if (NetUserGetInfo (wlogsrv, wuser, 3, (LPBYTE *) &ui)
	      == NERR_Success)
	    {
	      if (ui->usri3_home_dir_drive && *ui->usri3_home_dir_drive)
		{
		  sys_wcstombs (homepath_env_buf, NT_MAX_PATH,
				ui->usri3_home_dir_drive);
		  strcat (homepath_env_buf, "\\");
		}
	      else if (ui->usri3_home_dir && *ui->usri3_home_dir)
		sys_wcstombs (homepath_env_buf, NT_MAX_PATH,
			      ui->usri3_home_dir);
	    }
	  if (ui)
	    NetApiBufferFree (ui);
	}
      /* Second fallback: Windows profile dir. */
      if (!homepath_env_buf[0]
	  && get_user_profile_directory (get_windows_id (win_id),
					 profile, MAX_PATH))
	sys_wcstombs (homepath_env_buf, NT_MAX_PATH, profile);
      /* Last fallback: Cygwin root dir. */
      if (!homepath_env_buf[0])
	cygwin_conv_path (CCP_POSIX_TO_WIN_A | CCP_ABSOLUTE,
			  "/", homepath_env_buf, NT_MAX_PATH);

      if (homepath_env_buf[1] != ':')
	{
	  newhomedrive = almost_null;
	  newhomepath = homepath_env_buf;
	}
      else
	{
	  homedrive_env_buf[0] = homepath_env_buf[0];
	  homedrive_env_buf[1] = homepath_env_buf[1];
	  homedrive_env_buf[2] = '\0';
	  newhomedrive = homedrive_env_buf;
	  newhomepath = homepath_env_buf + 2;
	}
    }

  if (newhomedrive && newhomedrive != homedrive)
    cfree_and_set (homedrive, (newhomedrive == almost_null)
			      ? almost_null : cstrdup (newhomedrive));

  if (newhomepath && newhomepath != homepath)
    cfree_and_set (homepath, cstrdup (newhomepath));

  switch (what)
    {
    case CH_HOMEDRIVE:
      return homedrive;
    case CH_HOMEPATH:
      return homepath;
    default:
      return homepath;
    }
}

const char *
cygheap_user::test_uid (char *&what, const char *name, size_t namelen)
{
  if (!what && !issetuid ())
    what = getwinenveq (name, namelen, HEAP_STR);
  return what;
}

const char *
cygheap_user::env_logsrv (const char *name, size_t namelen)
{
  if (test_uid (plogsrv, name, namelen))
    return plogsrv;

  const char *mydomain = domain ();
  const char *myname = winname ();
  if (!mydomain || ascii_strcasematch (myname, "SYSTEM"))
    return almost_null;

  WCHAR wdomain[MAX_DOMAIN_NAME_LEN + 1];
  WCHAR wlogsrv[INTERNET_MAX_HOST_NAME_LENGTH + 3];
  sys_mbstowcs (wdomain, MAX_DOMAIN_NAME_LEN + 1, mydomain);
  cfree_and_set (plogsrv, almost_null);
  if (get_logon_server (wdomain, wlogsrv, DS_IS_FLAT_NAME))
    sys_wcstombs_alloc (&plogsrv, HEAP_STR, wlogsrv);
  return plogsrv;
}

const char *
cygheap_user::env_domain (const char *name, size_t namelen)
{
  if (pwinname && test_uid (pdomain, name, namelen))
    return pdomain;

  DWORD ulen = UNLEN + 1;
  WCHAR username[ulen];
  DWORD dlen = MAX_DOMAIN_NAME_LEN + 1;
  WCHAR userdomain[dlen];
  SID_NAME_USE use;

  cfree_and_set (pwinname, almost_null);
  cfree_and_set (pdomain, almost_null);
  if (!LookupAccountSidW (NULL, sid (), username, &ulen,
			  userdomain, &dlen, &use))
    __seterrno ();
  else
    {
      sys_wcstombs_alloc (&pwinname, HEAP_STR, username);
      sys_wcstombs_alloc (&pdomain, HEAP_STR, userdomain);
    }
  return pdomain;
}

const char *
cygheap_user::env_userprofile (const char *name, size_t namelen)
{
  if (test_uid (puserprof, name, namelen))
    return puserprof;

  /* User profile path is never longer than MAX_PATH. */
  WCHAR profile[MAX_PATH];
  WCHAR win_id[UNLEN + 1]; /* Large enough for SID */

  cfree_and_set (puserprof, almost_null);
  if (get_user_profile_directory (get_windows_id (win_id), profile, MAX_PATH))
    sys_wcstombs_alloc (&puserprof, HEAP_STR, profile);

  return puserprof;
}

const char *
cygheap_user::env_homepath (const char *name, size_t namelen)
{
  return ontherange (CH_HOMEPATH);
}

const char *
cygheap_user::env_homedrive (const char *name, size_t namelen)
{
  return ontherange (CH_HOMEDRIVE);
}

const char *
cygheap_user::env_name (const char *name, size_t namelen)
{
  if (!test_uid (pwinname, name, namelen))
    domain ();
  return pwinname;
}

const char *
cygheap_user::env_systemroot (const char *name, size_t namelen)
{
  if (!psystemroot)
    {
      int size = GetSystemWindowsDirectoryW (NULL, 0);
      if (size > 0)
	{
	  WCHAR wsystemroot[size];
	  size = GetSystemWindowsDirectoryW (wsystemroot, size);
	  if (size > 0)
	    sys_wcstombs_alloc (&psystemroot, HEAP_STR, wsystemroot);
	}
      if (size <= 0)
	debug_printf ("GetSystemWindowsDirectoryW(), %E");
    }
  return psystemroot;
}

char *
pwdgrp::next_str (char c)
{
  char *res = lptr;
  lptr = strchrnul (lptr, c);
  if (*lptr)
    *lptr++ = '\0';
  return res;
}

bool
pwdgrp::next_num (unsigned long& n)
{
  char *p = next_str (':');
  char *cp;
  n = strtoul (p, &cp, 10);
  return p != cp && !*cp;
}

char *
pwdgrp::add_line (char *eptr)
{
  if (eptr)
    {
      if (curr_lines >= max_lines)
	{
	  max_lines += 10;
	  pwdgrp_buf = crealloc_abort (pwdgrp_buf,
				       max_lines * pwdgrp_buf_elem_size);
	}
      lptr = eptr;
      if (!(this->*parse) ())
	return NULL;
      curr_lines++;
    }
  return eptr;
}

void
cygheap_pwdgrp::init ()
{
  pwd_cache.cygserver.init_pwd ();
  pwd_cache.file.init_pwd ();
  pwd_cache.win.init_pwd ();
  grp_cache.cygserver.init_grp ();
  grp_cache.file.init_grp ();
  grp_cache.win.init_grp ();
  /* Default settings:

     passwd: files db
     group:  files db
     db_prefix: auto		DISABLED
     db_separator: +		DISABLED
     db_home: cygwin desc
     db_shell: cygwin desc
     db_gecos: cygwin desc
     db_enum: cache builtin
  */
  pwd_src = (NSS_SRC_FILES | NSS_SRC_DB);
  grp_src = (NSS_SRC_FILES | NSS_SRC_DB);
  prefix = NSS_PFX_AUTO;
  separator[0] = L'+';
  enums = (ENUM_CACHE | ENUM_BUILTIN);
  enum_tdoms = NULL;
  caching = true;	/* INTERNAL ONLY */
}

#define NSS_NCMP(s) (!strncmp(c, (s), sizeof(s)-1))
#define NSS_CMP(s) (!strncmp(c, (s), sizeof(s)-1) \
		    && strchr (" \t", c[sizeof(s)-1]))

/* The /etc/nsswitch.conf file is read exactly once by the root process of a
   process tree.  We can't afford methodical changes during the lifetime of a 
   process tree. */
void
cygheap_pwdgrp::nss_init_line (const char *line)
{
  const char *c = line + strspn (line, " \t");
  char *comment = strchr (c, '#');
  if (comment)
    *comment = '\0';
  switch (*c)
    {
    case 'p':
    case 'g':
      {
	uint32_t *src = NULL;
	if (NSS_NCMP ("passwd:"))
	  src = &pwd_src;
	else if (NSS_NCMP ("group:"))
	  src = &grp_src;
	c = strchr (c, ':') + 1;
	if (src)
	  {
	    *src = 0;
	    while (*(c += strspn (c, " \t")))
	      {
		if (NSS_CMP ("files"))
		  {
		    *src |= NSS_SRC_FILES;
		    c += 5;
		  }
		else if (NSS_CMP ("db"))
		  {
		    *src |= NSS_SRC_DB;
		    c += 2;
		  }
		else
		  {
		    c += strcspn (c, " \t");
		    debug_printf ("Invalid nsswitch.conf content: %s", line);
		  }
	      }
	    if (*src == 0)
	      *src = (NSS_SRC_FILES | NSS_SRC_DB);
	  }
      }
      break;
    case 'd':
      if (!NSS_NCMP ("db_"))
	{
	  debug_printf ("Invalid nsswitch.conf content: %s", line);
	  break;
	}
      c += 3;
#if 0 /* Disable setting prefix and separator from nsswitch.conf for now.
	 Remove if nobody complains too loudly. */
      if (NSS_NCMP ("prefix:"))
	{
	  c = strchr (c, ':') + 1;
	  c += strspn (c, " \t");
	  if (NSS_CMP ("auto"))
	    prefix = NSS_AUTO;
	  else if (NSS_CMP ("primary"))
	    prefix = NSS_PRIMARY;
	  else if (NSS_CMP ("always"))
	    prefix = NSS_ALWAYS;
	  else
	    debug_printf ("Invalid nsswitch.conf content: %s", line);
	}
      else if (NSS_NCMP ("separator:"))
	{
	  c = strchr (c, ':') + 1;
	  c += strspn (c, " \t");
	  if ((unsigned char) *c <= 0x7f && *c != ':' && strchr (" \t", c[1]))
	    separator[0] = (unsigned char) *c;
	  else
	    debug_printf ("Invalid nsswitch.conf content: %s", line);
	}
      else
#endif
      if (NSS_NCMP ("enum:"))
	{
	  tmp_pathbuf tp;
	  char *tdoms = tp.c_get ();
	  char *td = tdoms;
	  int new_enums = ENUM_NONE;

	  td[0] = '\0';
	  c = strchr (c, ':') + 1;
	  c += strspn (c, " \t");
	  while (!strchr (" \t", *c))
	    {
	      const char *e = c + strcspn (c, " \t");
	      if (NSS_CMP ("none"))
		new_enums = ENUM_NONE;
	      else if (NSS_CMP ("builtin"))
		new_enums |= ENUM_BUILTIN;
	      else if (NSS_CMP ("cache"))
		new_enums |= ENUM_CACHE;
	      else if (NSS_CMP ("files"))
		new_enums |= ENUM_FILES;
	      else if (NSS_CMP ("local"))
		new_enums |= ENUM_LOCAL;
	      else if (NSS_CMP ("primary"))
		new_enums |= ENUM_PRIMARY;
	      else if (NSS_CMP ("alltrusted"))
		new_enums |= ENUM_TDOMS | ENUM_TDOMS_ALL;
	      else if (NSS_CMP ("all"))
		new_enums |= ENUM_ALL;
	      else
		{
		  td = stpcpy (stpncpy (td, c, e - c), " ");
		  new_enums |= ENUM_TDOMS;
		}
	      c = e;
	      c += strspn (c, " \t");
	    }
	  if ((new_enums & (ENUM_TDOMS | ENUM_TDOMS_ALL)) == ENUM_TDOMS)
	    {
	      if (td > tdoms)
		{
		  PWCHAR spc;
		  sys_mbstowcs_alloc (&enum_tdoms, HEAP_BUF, tdoms);
		  /* Convert string to REG_MULTI_SZ-style. */
		  while ((spc = wcsrchr (enum_tdoms, L' ')))
		    *spc = L'\0';
		}
	      else
	      	new_enums &= ~(ENUM_TDOMS | ENUM_TDOMS_ALL);
	    }
	  enums = new_enums;
	}
      else
	{
	  nss_scheme_t *scheme = NULL;
	  if (NSS_NCMP ("home:"))
	    scheme = home_scheme;
	  else if (NSS_NCMP ("shell:"))
	    scheme = shell_scheme;
	  else if (NSS_NCMP ("gecos:"))
	    scheme = gecos_scheme;
	  if (scheme)
	    {
	      uint16_t idx = 0;

	      scheme[0].method = scheme[1].method = NSS_SCHEME_FALLBACK;
	      c = strchr (c, ':') + 1;
	      c += strspn (c, " \t");
	      while (*c && idx < NSS_SCHEME_MAX)
		{
		  if (NSS_CMP ("windows"))
		    scheme[idx].method = NSS_SCHEME_WINDOWS;
		  else if (NSS_CMP ("cygwin"))
		    scheme[idx].method = NSS_SCHEME_CYGWIN;
		  else if (NSS_CMP ("unix"))
		    scheme[idx].method = NSS_SCHEME_UNIX;
		  else if (NSS_CMP ("desc"))
		    scheme[idx].method = NSS_SCHEME_DESC;
		  else if (NSS_NCMP ("/"))
		    {
		      const char *e = c + strcspn (c, " \t");
		      scheme[idx].method = NSS_SCHEME_PATH;
		      sys_mbstowcs_alloc (&scheme[idx].attrib, HEAP_STR,
					  c, e - c);
		    }
		  else if (NSS_NCMP ("@") && isalnum ((unsigned) *++c))
		    {
		      const char *e = c + strcspn (c, " \t");
		      scheme[idx].method = NSS_SCHEME_FREEATTR;
		      sys_mbstowcs_alloc (&scheme[idx].attrib, HEAP_STR,
					  c, e - c);
		    }
		  else
		    debug_printf ("Invalid nsswitch.conf content: %s", line);
		  c += strcspn (c, " \t");
		  c += strspn (c, " \t");
		  ++idx;
		}
	      /* If nothing has been set, revert to default. */
	      if (scheme[0].method == NSS_SCHEME_FALLBACK)
		{
		  scheme[0].method = NSS_SCHEME_CYGWIN;
		  scheme[1].method = NSS_SCHEME_DESC;
		}
	    }
	}
      break;
    case '\0':
      break;
    default:
      debug_printf ("Invalid nsswitch.conf content: %s", line);
      break;
    }
}

static char *
fetch_windows_home (cyg_ldap *pldap, PUSER_INFO_3 ui, cygpsid &sid,
		    PCWSTR dnsdomain)
{
  PCWSTR home_from_db = NULL;
  char *home = NULL;

  if (pldap)
    {
      if (pldap->fetch_ad_account (sid, false, dnsdomain))
	{
#if 0
	  /* Disable preferring homeDrive for now.  The drive letter may not
	     be available when it's needed. */
	  home_from_db = pldap->get_string_attribute (L"homeDrive");
	  if (!home_from_db || !*home_from_db)
#endif
	  home_from_db = pldap->get_string_attribute (L"homeDirectory");
	}
    }
  else if (ui)
    {
#if 0
      /* Ditto. */
      if (ui->usri3_home_dir_drive && *ui->usri3_home_dir_drive)
	home_from_db = ui->usri3_home_dir_drive;
      else
#endif
      if (ui->usri3_home_dir && *ui->usri3_home_dir)
	home_from_db = ui->usri3_home_dir;
    }
  if (home_from_db && *home_from_db)
    home = (char *) cygwin_create_path (CCP_WIN_W_TO_POSIX, home_from_db);
  else
    {
      /* The db fields are empty, so we have to evaluate the local profile
	 path, which is the same thing as the default home directory on
	 Windows.  So what we do here is to try to find out if the user
	 already has a profile on this machine.
	 Note that we don't try to generate the profile if it doesn't exist.
	 Think what would happen if we actually have the permissions to do
	 so and call getpwent... in a domain environment.  The problem is,
	 of course, that we can't know the profile path, unless the OS
	 created it.
	 The only reason this could occur is if a user account, which never
	 logged on to the machine before, tries to logon via a Cygwin service
	 like sshd. */
      WCHAR profile[MAX_PATH];
      WCHAR sidstr[128];

      if (get_user_profile_directory (sid.string (sidstr), profile, MAX_PATH))
      	home = (char *) cygwin_create_path (CCP_WIN_W_TO_POSIX, profile);
    }
  return home;
}

/* Local SAM accounts have only a handful attributes available to home users.
   Therefore, allow to fetch additional passwd/group attributes from the
   "Comment" field in XML short style.  For symmetry, this is also allowed
   from the equivalent "description" AD attribute. */
static char *
fetch_from_description (PCWSTR desc, PCWSTR search, size_t len)
{
  PWCHAR s, e;
  char *ret = NULL;

  if ((s = wcsstr (desc, L"<cygwin ")) && (e = wcsstr (s + 8, L"/>")))
    {
      s += 8;
      while (s && s < e)
	{
	  while (*s == L' ')
	    ++s;
	  if (!wcsncmp (s, search, len)) /* Found what we're searching? */
	    {
	      s += len;
	      if ((e = wcschr (s, L'"')))
		{
		  sys_wcstombs_alloc (&ret, HEAP_NOTHEAP, s, e - s);
		  s = e + 1;
		}
	      break;
	    }
	  else /* Skip the current foo="bar" string. */
	    if ((s = wcschr (s, L'"')) && (s = wcschr (s + 1, L'"')))
	      ++s;
	}
    }
  return ret;
}

static char *
fetch_from_path (cyg_ldap *pldap, PUSER_INFO_3 ui, cygpsid &sid, PCWSTR str,
		 PCWSTR dom, PCWSTR dnsdomain, PCWSTR name, bool full_qualified)
{
  tmp_pathbuf tp;
  PWCHAR wpath = tp.w_get ();
  PWCHAR w = wpath;
  PWCHAR we = wpath + NT_MAX_PATH - 1;
  char *home;
  char *ret = NULL;

  while (*str && w < we)
    {
      if (*str != L'%')
      	*w++ = *str++;
      else
      	{
	  switch (*++str)
	    {
	    case L'u':
	      if (full_qualified)
		{
		  w = wcpncpy (w, dom, we - w);
		  if (w < we)
		    *w++ = cygheap->pg.nss_separator ()[0];
		}
	      w = wcpncpy (w, name, we - w);
	      break;
	    case L'U':
	      w = wcpncpy (w, name, we - w);
	      break;
	    case L'D':
	      w = wcpncpy (w, dom, we - w);
	      break;
	    case L'H':
	      home = fetch_windows_home (pldap, ui, sid, dnsdomain);
	      if (home)
		{
		  /* Drop one leading slash to accommodate home being an
		     absolute path.  We don't check for broken usage of
		     %H here, of course. */
		  if (w > wpath && w[-1] == L'/')
		    --w;
		  w += sys_mbstowcs (w, we - w, home) - 1;
		  free (home);
		}
	      break;
	    case L'_':
	      *w++ = L' ';
	      break;
	    default:
	      *w++ = *str;
	      break;
	    }
	  ++str;
	}
    }
  *w = L'\0';
  sys_wcstombs_alloc (&ret, HEAP_NOTHEAP, wpath);
  return ret;
}

char *
cygheap_pwdgrp::get_home (cyg_ldap *pldap, cygpsid &sid, PCWSTR dom,
			  PCWSTR dnsdomain, PCWSTR name, bool full_qualified)
{
  PWCHAR val;
  char *home = NULL;

  for (uint16_t idx = 0; !home && idx < NSS_SCHEME_MAX; ++idx)
    {
      switch (home_scheme[idx].method)
	{
	case NSS_SCHEME_FALLBACK:
	  return NULL;
	case NSS_SCHEME_WINDOWS:
	  if (pldap->fetch_ad_account (sid, false, dnsdomain))
	    home = fetch_windows_home (pldap, NULL, sid, dnsdomain);
	  break;
	case NSS_SCHEME_CYGWIN:
	  if (pldap->fetch_ad_account (sid, false, dnsdomain))
	    {
	      val = pldap->get_string_attribute (L"cygwinHome");
	      if (val && *val)
		sys_wcstombs_alloc (&home, HEAP_NOTHEAP, val);
	    }
	  break;
	case NSS_SCHEME_UNIX:
	  if (pldap->fetch_ad_account (sid, false, dnsdomain))
	    {
	      val = pldap->get_string_attribute (L"unixHomeDirectory");
	      if (val && *val)
		sys_wcstombs_alloc (&home, HEAP_NOTHEAP, val);
	    }
	  break;
	case NSS_SCHEME_DESC:
	  if (pldap->fetch_ad_account (sid, false, dnsdomain))
	    {
	      val = pldap->get_string_attribute (L"description");
	      if (val && *val)
		home = fetch_from_description (val, L"home=\"", 6);
	    }
	  break;
	case NSS_SCHEME_PATH:
	  home = fetch_from_path (pldap, NULL, sid, home_scheme[idx].attrib,
				  dom, dnsdomain, name, full_qualified);
	  break;
	case NSS_SCHEME_FREEATTR:
	  if (pldap->fetch_ad_account (sid, false, dnsdomain))
	    {
	      val = pldap->get_string_attribute (home_scheme[idx].attrib);
	      if (val && *val)
		{
		  if (isdrive (val) || *val == '\\')
		    home = (char *)
			   cygwin_create_path (CCP_WIN_W_TO_POSIX, val);
		  else
		    sys_wcstombs_alloc (&home, HEAP_NOTHEAP, val);
		}
	    }
	  break;
	}
    }
  return home;
}

char *
cygheap_pwdgrp::get_home (PUSER_INFO_3 ui, cygpsid &sid, PCWSTR dom,
			  PCWSTR name, bool full_qualified)
{
  char *home = NULL;

  for (uint16_t idx = 0; !home && idx < NSS_SCHEME_MAX; ++idx)
    {
      switch (home_scheme[idx].method)
	{
	case NSS_SCHEME_FALLBACK:
	  return NULL;
	case NSS_SCHEME_WINDOWS:
	  home = fetch_windows_home (NULL, ui, sid, NULL);
	  break;
	case NSS_SCHEME_CYGWIN:
	case NSS_SCHEME_UNIX:
	case NSS_SCHEME_FREEATTR:
	  break;
	case NSS_SCHEME_DESC:
	  if (ui)
	    home = fetch_from_description (ui->usri3_comment, L"home=\"", 6);
	  break;
	case NSS_SCHEME_PATH:
	  home = fetch_from_path (NULL, ui, sid, home_scheme[idx].attrib,
				  dom, NULL, name, full_qualified);
	  break;
	}
    }
  return home;
}

char *
cygheap_pwdgrp::get_shell (cyg_ldap *pldap, cygpsid &sid, PCWSTR dom,
			   PCWSTR dnsdomain, PCWSTR name, bool full_qualified)
{
  PWCHAR val;
  char *shell = NULL;

  for (uint16_t idx = 0; !shell && idx < NSS_SCHEME_MAX; ++idx)
    {
      switch (shell_scheme[idx].method)
	{
	case NSS_SCHEME_FALLBACK:
	  return NULL;
	case NSS_SCHEME_WINDOWS:
	  break;
	case NSS_SCHEME_CYGWIN:
	  if (pldap->fetch_ad_account (sid, false, dnsdomain))
	    {
	      val = pldap->get_string_attribute (L"cygwinShell");
	      if (val && *val)
		sys_wcstombs_alloc (&shell, HEAP_NOTHEAP, val);
	    }
	  break;
	case NSS_SCHEME_UNIX:
	  if (pldap->fetch_ad_account (sid, false, dnsdomain))
	    {
	      val = pldap->get_string_attribute (L"loginShell");
	      if (val && *val)
		sys_wcstombs_alloc (&shell, HEAP_NOTHEAP, val);
	    }
	  break;
	case NSS_SCHEME_DESC:
	  if (pldap->fetch_ad_account (sid, false, dnsdomain))
	    {
	      val = pldap->get_string_attribute (L"description");
	      if (val && *val)
		shell = fetch_from_description (val, L"shell=\"", 7);
	    }
	  break;
	case NSS_SCHEME_PATH:
	  shell = fetch_from_path (pldap, NULL, sid, shell_scheme[idx].attrib,
	  			   dom, dnsdomain, name, full_qualified);
	  break;
	case NSS_SCHEME_FREEATTR:
	  if (pldap->fetch_ad_account (sid, false, dnsdomain))
	    {
	      val = pldap->get_string_attribute (shell_scheme[idx].attrib);
	      if (val && *val)
		{
		  if (isdrive (val) || *val == '\\')
		    shell = (char *)
			    cygwin_create_path (CCP_WIN_W_TO_POSIX, val);
		  else
		    sys_wcstombs_alloc (&shell, HEAP_NOTHEAP, val);
		}
	    }
	  break;
	}
    }
  return shell;
}

char *
cygheap_pwdgrp::get_shell (PUSER_INFO_3 ui, cygpsid &sid, PCWSTR dom,
			   PCWSTR name, bool full_qualified)
{
  char *shell = NULL;

  for (uint16_t idx = 0; !shell && idx < NSS_SCHEME_MAX; ++idx)
    {
      switch (shell_scheme[idx].method)
	{
	case NSS_SCHEME_FALLBACK:
	  return NULL;
	case NSS_SCHEME_WINDOWS:
	case NSS_SCHEME_CYGWIN:
	case NSS_SCHEME_UNIX:
	case NSS_SCHEME_FREEATTR:
	  break;
	case NSS_SCHEME_DESC:
	  if (ui)
	    shell = fetch_from_description (ui->usri3_comment, L"shell=\"", 7);
	  break;
	case NSS_SCHEME_PATH:
	  shell = fetch_from_path (NULL, ui, sid, shell_scheme[idx].attrib,
				   dom, NULL, name, full_qualified);
	  break;
	}
    }
  return shell;
}

/* Helper function to replace colons with semicolons in pw_gecos field. */
static inline void
colon_to_semicolon (char *str)
{
  char *cp = str;
  while ((cp = strchr (cp, L':')) != NULL)
    *cp++ = L';';
}

char *
cygheap_pwdgrp::get_gecos (cyg_ldap *pldap, cygpsid &sid, PCWSTR dom,
			   PCWSTR dnsdomain, PCWSTR name, bool full_qualified)
{
  PWCHAR val;
  char *gecos = NULL;

  for (uint16_t idx = 0; !gecos && idx < NSS_SCHEME_MAX; ++idx)
    {
      switch (gecos_scheme[idx].method)
	{
	case NSS_SCHEME_FALLBACK:
	  return NULL;
	case NSS_SCHEME_WINDOWS:
	  if (pldap->fetch_ad_account (sid, false, dnsdomain))
	    {
	      val = pldap->get_string_attribute (L"displayName");
	      if (val && *val)
		sys_wcstombs_alloc (&gecos, HEAP_NOTHEAP, val);
	    }
	  break;
	case NSS_SCHEME_CYGWIN:
	  if (pldap->fetch_ad_account (sid, false, dnsdomain))
	    {
	      val = pldap->get_string_attribute (L"cygwinGecos");
	      if (val && *val)
		sys_wcstombs_alloc (&gecos, HEAP_NOTHEAP, val);
	    }
	  break;
	case NSS_SCHEME_UNIX:
	  if (pldap->fetch_ad_account (sid, false, dnsdomain))
	    {
	      val = pldap->get_string_attribute (L"gecos");
	      if (val && *val)
		sys_wcstombs_alloc (&gecos, HEAP_NOTHEAP, val);
	    }
	  break;
	case NSS_SCHEME_DESC:
	  if (pldap->fetch_ad_account (sid, false, dnsdomain))
	    {
	      val = pldap->get_string_attribute (L"description");
	      if (val && *val)
		gecos = fetch_from_description (val, L"gecos=\"", 7);
	    }
	  break;
	case NSS_SCHEME_PATH:
	  gecos = fetch_from_path (pldap, NULL, sid,
				   gecos_scheme[idx].attrib + 1,
				   dom, dnsdomain, name, full_qualified);
	  break;
	case NSS_SCHEME_FREEATTR:
	  if (pldap->fetch_ad_account (sid, false, dnsdomain))
	    {
	      val = pldap->get_string_attribute (gecos_scheme[idx].attrib);
	      if (val && *val)
		sys_wcstombs_alloc (&gecos, HEAP_NOTHEAP, val);
	    }
	  break;
	}
    }
  if (gecos)
    colon_to_semicolon (gecos);
  return gecos;
}

char *
cygheap_pwdgrp::get_gecos (PUSER_INFO_3 ui, cygpsid &sid, PCWSTR dom,
			   PCWSTR name, bool full_qualified)
{
  char *gecos = NULL;

  for (uint16_t idx = 0; !gecos && idx < NSS_SCHEME_MAX; ++idx)
    {
      switch (gecos_scheme[idx].method)
	{
	case NSS_SCHEME_FALLBACK:
	  return NULL;
	case NSS_SCHEME_WINDOWS:
	  if (ui && ui->usri3_full_name && *ui->usri3_full_name)
	    sys_wcstombs_alloc (&gecos, HEAP_NOTHEAP, ui->usri3_full_name);
	  break;
	case NSS_SCHEME_CYGWIN:
	case NSS_SCHEME_UNIX:
	case NSS_SCHEME_FREEATTR:
	  break;
	case NSS_SCHEME_DESC:
	  if (ui)
	    gecos = fetch_from_description (ui->usri3_comment, L"gecos=\"", 7);
	  break;
	case NSS_SCHEME_PATH:
	  gecos = fetch_from_path (NULL, ui, sid, gecos_scheme[idx].attrib + 1,
				   dom, NULL, name, full_qualified);
	  break;
	}
    }
  if (gecos)
    colon_to_semicolon (gecos);
  return gecos;
}

void
cygheap_pwdgrp::_nss_init ()
{
  UNICODE_STRING path;
  OBJECT_ATTRIBUTES attr;
  NT_readline rl;
  tmp_pathbuf tp;
  char *buf = tp.c_get ();

  PCWSTR rel_path = L"\\etc\\nsswitch.conf";
  path.Length = (wcslen (cygheap->installation_root) + wcslen (rel_path))
		* sizeof (WCHAR);
  path.MaximumLength = path.Length + sizeof (WCHAR);
  path.Buffer = (PWCHAR) alloca (path.MaximumLength);
  wcpcpy (wcpcpy (path.Buffer, cygheap->installation_root), rel_path);
  InitializeObjectAttributes (&attr, &path, OBJ_CASE_INSENSITIVE,
			      NULL, NULL);
  if (rl.init (&attr, buf, NT_MAX_PATH))
    while ((buf = rl.gets ()))
      nss_init_line (buf);
  nss_inited = true;
}

/* Override the ParentIndex value of the PDS_DOMAIN_TRUSTSW entry with the
   PosixOffset. */
#define PosixOffset ParentIndex

bool
cygheap_domain_info::init ()
{
  HANDLE lsa;
  NTSTATUS status;
  ULONG ret;
  /* We *have* to copy the information.  Apart from our wish to have the
     stuff in the cygheap, even when not calling LsaFreeMemory on the result,
     the data will be overwritten later.  From what I gather, the information
     is, in fact, stored on the stack. */
  PPOLICY_DNS_DOMAIN_INFO pdom;
  PPOLICY_ACCOUNT_DOMAIN_INFO adom;
  PDS_DOMAIN_TRUSTSW td;
  ULONG tdom_cnt;

  if (adom_name)
    return true;
  lsa = lsa_open_policy (NULL, POLICY_VIEW_LOCAL_INFORMATION);
  if (!lsa)
    {
      system_printf ("lsa_open_policy(NULL) failed");
      return false;
    }
  /* Fetch primary domain information from local LSA. */
  status = LsaQueryInformationPolicy (lsa, PolicyDnsDomainInformation,
				      (PVOID *) &pdom);
  if (status != STATUS_SUCCESS)
    {
      system_printf ("LsaQueryInformationPolicy(Primary) %y", status);
      return false;
    }
  /* Copy primary domain info to cygheap. */
  pdom_name = cwcsdup (pdom->Name.Buffer);
  pdom_dns_name = pdom->DnsDomainName.Length
		  ? cwcsdup (pdom->DnsDomainName.Buffer) : NULL;
  pdom_sid = pdom->Sid;
  LsaFreeMemory (pdom);
  /* Fetch account domain information from local LSA. */
  status = LsaQueryInformationPolicy (lsa, PolicyAccountDomainInformation,
				      (PVOID *) &adom);
  if (status != STATUS_SUCCESS)
    {
      system_printf ("LsaQueryInformationPolicy(Account) %y", status);
      return false;
    }
  /* Copy account domain info to cygheap.  If we're running on a DC the account
     domain is identical to the primary domain.  This leads to confusion when
     trying to compute the uid/gid values.  Therefore we invalidate the account
     domain name if we're running on a DC. */
  adom_sid = adom->DomainSid;
  adom_name = cwcsdup (pdom_sid == adom_sid ? L"@" : adom->DomainName.Buffer);
  LsaFreeMemory (adom);
  lsa_close_policy (lsa);
  if (cygheap->dom.member_machine ())
    {
      ret = DsEnumerateDomainTrustsW (NULL, DS_DOMAIN_DIRECT_INBOUND
					    | DS_DOMAIN_DIRECT_OUTBOUND
					    | DS_DOMAIN_IN_FOREST,
				      &td, &tdom_cnt);
      if (ret != ERROR_SUCCESS)
	{
	  SetLastError (ret);
	  debug_printf ("DsEnumerateDomainTrusts: %E");
	  return true;
	}
      if (tdom_cnt == 0)
	{
	  return true;
	}
      /* Copy trusted domain info to cygheap, setting PosixOffset on the fly. */
      tdom = (PDS_DOMAIN_TRUSTSW)
	cmalloc_abort (HEAP_BUF, tdom_cnt * sizeof (DS_DOMAIN_TRUSTSW));
      memcpy (tdom, td, tdom_cnt * sizeof (DS_DOMAIN_TRUSTSW));
      for (ULONG idx = 0; idx < tdom_cnt; ++idx)
	{
	  /* Copy... */
	  tdom[idx].NetbiosDomainName = cwcsdup (td[idx].NetbiosDomainName);
	  /* DnsDomainName as well as DomainSid can be NULL.  The reason is
	     usually a domain of type TRUST_TYPE_DOWNLEVEL.  This can be an
	     old pre-AD domain, or a Netware domain, etc.  If DnsDomainName
	     is NULL, just set it to NetbiosDomainName.  This simplifies
	     subsequent code which doesn't have to check for a NULL pointer. */
	  tdom[idx].DnsDomainName = td[idx].DnsDomainName
				    ? cwcsdup (td[idx].DnsDomainName)
				    : tdom[idx].NetbiosDomainName;
	  if (td[idx].DomainSid)
	    {
	      ULONG len = RtlLengthSid (td[idx].DomainSid);
	      tdom[idx].DomainSid = cmalloc_abort(HEAP_BUF, len);
	      RtlCopySid (len, tdom[idx].DomainSid, td[idx].DomainSid);
	    }
	  /* ...and set PosixOffset to 0.  This */
	  tdom[idx].PosixOffset = 0;
	}
      NetApiBufferFree (td);
      tdom_count = tdom_cnt;
    }
  /* If we have Microsoft Client for NFS installed, we make use of a name
     mapping server.  This can be either Active Directory to map uids/gids
     directly to Windows SIDs, or an AD LDS or other RFC 2307 compatible
     identity store.  The name of the mapping domain can be fetched from the
     registry key created by the NFS client installation and entered by the
     user via nfsadmin or the "Services For NFS" MMC snap-in.

     Reference:
     http://blogs.technet.com/b/filecab/archive/2012/10/09/nfs-identity-mapping-in-windows-server-2012.aspx
     Note that we neither support UNMP nor local passwd/group file mapping,
     nor UUUA.

     This function returns the mapping server from the aforementioned registry
     key, or, if none is configured, NULL, which will be resolved to the
     primary domain of the machine by the ldap_init function.

     The latter is useful to get an RFC 2307 mapping for Samba UNIX accounts,
     even if no NFS name mapping is configured on the machine.  Fortunately,
     the posixAccount and posixGroup schemas are already available in the
     Active Directory default setup. */
  reg_key reg (HKEY_LOCAL_MACHINE, KEY_READ | KEY_WOW64_64KEY,
	       L"SOFTWARE", L"Microsoft", L"ServicesForNFS", NULL);
  if (!reg.error ())
    {
      DWORD rfc2307 = reg.get_dword (L"Rfc2307", 0);
      if (rfc2307)
	{
	  rfc2307_domain_buf = (PWCHAR) ccalloc_abort (HEAP_STR, 257,
						       sizeof (WCHAR));
	  reg.get_string (L"Rfc2307Domain", rfc2307_domain_buf, 257, L"");
	  if (!rfc2307_domain_buf[0])
	    {
	      cfree (rfc2307_domain_buf);
	      rfc2307_domain_buf = NULL;
	    }
	}
    }
  return true;
}

PDS_DOMAIN_TRUSTSW
cygheap_domain_info::add_domain (PCWSTR domain, PSID sid)
{
  PDS_DOMAIN_TRUSTSW new_tdom;
  cygsid tsid (sid);

  new_tdom = (PDS_DOMAIN_TRUSTSW) crealloc (tdom, (tdom_count + 1)
						  * sizeof (DS_DOMAIN_TRUSTSW));
  if (!new_tdom)
    return NULL;

  tdom = new_tdom;
  new_tdom = &tdom[tdom_count];
  new_tdom->DnsDomainName = new_tdom->NetbiosDomainName = cwcsdup (domain);
  --*RtlSubAuthorityCountSid (tsid);
  ULONG len = RtlLengthSid (tsid);
  new_tdom->DomainSid = cmalloc_abort(HEAP_BUF, len);
  RtlCopySid (len, new_tdom->DomainSid, tsid);
  new_tdom->PosixOffset = 0;
  ++tdom_count;
  return new_tdom;
}

/* Per session, so it changes potentially when switching the user context. */
static cygsid logon_sid ("");

static void
get_logon_sid ()
{
  if (PSID (logon_sid) == NO_SID)
    {
      NTSTATUS status;
      ULONG size;
      tmp_pathbuf tp;
      PTOKEN_GROUPS groups = (PTOKEN_GROUPS) tp.w_get ();

      status = NtQueryInformationToken (hProcToken, TokenGroups, groups,
					2 * NT_MAX_PATH, &size);
      if (!NT_SUCCESS (status))
	debug_printf ("NtQueryInformationToken (TokenGroups) %y", status);
      else
	{
	  for (DWORD pg = 0; pg < groups->GroupCount; ++pg)
	    if (groups->Groups[pg].Attributes & SE_GROUP_LOGON_ID)
	      {
		logon_sid = groups->Groups[pg].Sid;
		break;
	      }
	}
    }
}

/* Fetch special AzureAD group, which is part of the token group list but
   *not* recognized by LookupAccountSid (ERROR_NONE_MAPPED). */
static cygsid azure_grp_sid ("");

static void
get_azure_grp_sid ()
{
  if (PSID (azure_grp_sid) == NO_SID)
    {
      NTSTATUS status;
      ULONG size;
      tmp_pathbuf tp;
      PTOKEN_GROUPS groups = (PTOKEN_GROUPS) tp.w_get ();

      status = NtQueryInformationToken (hProcToken, TokenGroups, groups,
					2 * NT_MAX_PATH, &size);
      if (!NT_SUCCESS (status))
	debug_printf ("NtQueryInformationToken (TokenGroups) %y", status);
      else
	{
	  for (DWORD pg = 0; pg < groups->GroupCount; ++pg)
	    if (sid_id_auth (groups->Groups[pg].Sid) == 12)
	      {
		azure_grp_sid = groups->Groups[pg].Sid;
		break;
	      }
	}
    }
}

void *
pwdgrp::add_account_post_fetch (char *line, bool lock)
{
  void *ret = NULL;

  if (line)
    { 
      if (lock)
	pglock.init ("pglock")->acquire ();
      if (add_line (line))
	ret = ((char *) pwdgrp_buf) + (curr_lines - 1) * pwdgrp_buf_elem_size;
      if (lock)
	pglock.release ();
    }
  return ret;
}

void *
pwdgrp::add_account_from_file (cygpsid &sid)
{
  if (!path.MaximumLength)
    return NULL;
  fetch_user_arg_t arg;
  arg.type = SID_arg;
  arg.sid = &sid;
  char *line = fetch_account_from_file (arg);
  return (struct passwd *) add_account_post_fetch (line, true);
}

void *
pwdgrp::add_account_from_file (const char *name)
{
  if (!path.MaximumLength)
    return NULL;
  fetch_user_arg_t arg;
  arg.type = NAME_arg;
  arg.name = name;
  char *line = fetch_account_from_file (arg);
  return (struct passwd *) add_account_post_fetch (line, true);
}

void *
pwdgrp::add_account_from_file (uint32_t id)
{
  if (!path.MaximumLength)
    return NULL;
  fetch_user_arg_t arg;
  arg.type = ID_arg;
  arg.id = id;
  char *line = fetch_account_from_file (arg);
  return (struct passwd *) add_account_post_fetch (line, true);
}

void *
pwdgrp::add_account_from_windows (cygpsid &sid, cyg_ldap *pldap)
{
  fetch_user_arg_t arg;
  arg.type = SID_arg;
  arg.sid = &sid;
  char *line = fetch_account_from_windows (arg, pldap);
  if (!line)
    return NULL;
  return add_account_post_fetch (line, true);
}

void *
pwdgrp::add_account_from_windows (const char *name, cyg_ldap *pldap)
{
  fetch_user_arg_t arg;
  arg.type = NAME_arg;
  arg.name = name;
  char *line = fetch_account_from_windows (arg, pldap);
  if (!line)
    return NULL;
  return add_account_post_fetch (line, true);
}

void *
pwdgrp::add_account_from_windows (uint32_t id, cyg_ldap *pldap)
{
  fetch_user_arg_t arg;
  arg.type = ID_arg;
  arg.id = id;
  char *line = fetch_account_from_windows (arg, pldap);
  if (!line)
    return NULL;
  return add_account_post_fetch (line, true);
}

/* Called from internal_getgrfull, in turn called from internal_getgroups. */
struct group *
pwdgrp::add_group_from_windows (fetch_acc_t &full_acc, cyg_ldap *pldap)
{
  fetch_user_arg_t arg;
  arg.type = FULL_acc_arg;
  arg.full_acc = &full_acc;
  char *line = fetch_account_from_windows (arg, pldap);
  if (!line)
    return NULL;
  return (struct group *) add_account_post_fetch (line, true);
}

/* Check if file exists and if it has been written to since last checked.
   If file has been changed, invalidate the current cache.
   
   If the file doesn't exist when this function is called the first time,
   by the first Cygwin process in a process tree, the file will never be
   visited again by any process in this process tree.  This is important,
   because we cannot allow a change of UID/GID values for the lifetime
   of a process tree.
   
   If the file gets deleted or unreadable, the file cache will stay in
   place, but we won't try to read new accounts from the file.
   
   The return code indicates to the calling function if the file exists. */
bool
pwdgrp::check_file ()
{
  FILE_BASIC_INFORMATION fbi;
  NTSTATUS status;

  if (!path.Buffer)
    {
      PCWSTR rel_path = is_group () ? L"\\etc\\group" : L"\\etc\\passwd";
      path.Length = (wcslen (cygheap->installation_root) + wcslen (rel_path))
		    * sizeof (WCHAR);
      path.MaximumLength = path.Length + sizeof (WCHAR);
      path.Buffer = (PWCHAR) cmalloc_abort (HEAP_BUF, path.MaximumLength);
      wcpcpy (wcpcpy (path.Buffer, cygheap->installation_root), rel_path);
      InitializeObjectAttributes (&attr, &path, OBJ_CASE_INSENSITIVE,
				  NULL, NULL);
    }
  else if (path.MaximumLength == 0) /* Indicates that the file doesn't exist. */
    return false;
  status = NtQueryAttributesFile (&attr, &fbi);
  if (!NT_SUCCESS (status))
    {
      if (last_modified.QuadPart)
	last_modified.QuadPart = 0LL;
      else
	path.MaximumLength = 0;
      return false;
    }
  if (fbi.LastWriteTime.QuadPart > last_modified.QuadPart)
    {
      last_modified.QuadPart = fbi.LastWriteTime.QuadPart;
      if (curr_lines > 0)
	{
	  pglock.init ("pglock")->acquire ();
	  int curr = curr_lines;
	  curr_lines = 0;
	  for (int i = 0; i < curr; ++i)
	    cfree (is_group () ? this->group ()[i].g.gr_name
			 : this->passwd ()[i].p.pw_name);
	  pglock.release ();
	}
    }
  return true;
}

char *
pwdgrp::fetch_account_from_line (fetch_user_arg_t &arg, const char *line)
{
  char *p, *e;

  switch (arg.type)
    {
    case SID_arg:
      /* Ignore fields, just scan for SID string. */
      if (!(p = strstr (line, arg.name)) || p[arg.len] != ':')
	return NULL;
      break;
    case NAME_arg:
      /* First field is always name. */
      if (!strncasematch (line, arg.name, arg.len) || line[arg.len] != ':')
	return NULL;
      break;
    case ID_arg:
      /* Skip to third field. */
      if (!(p = strchr (line, ':')) || !(p = strchr (p + 1, ':')))
	return NULL;
      if (strtoul (p + 1, &e, 10) != arg.id || !e || *e != ':')
	return NULL;
      break;
    default:
      return NULL;
    }
  return cstrdup (line);
}

char *
pwdgrp::fetch_account_from_file (fetch_user_arg_t &arg)
{
  NT_readline rl;
  tmp_pathbuf tp;
  char *buf = tp.c_get ();
  char str[128];
  char *ret = NULL;

  /* Create search string. */
  switch (arg.type)
    {
    case SID_arg:
      /* Override SID with SID string. */
      arg.sid->string (str);
      arg.name = str;
      /*FALLTHRU*/
    case NAME_arg:
      arg.len = strlen (arg.name);
      break;
    case ID_arg:
      break;
    default:
      return NULL;
    }
  if (rl.init (&attr, buf, NT_MAX_PATH))
    while ((buf = rl.gets ()))
      if ((ret = fetch_account_from_line (arg, buf)))
	return ret;
  return NULL;
}

static ULONG
fetch_posix_offset (PDS_DOMAIN_TRUSTSW td, cyg_ldap *cldap)
{
  uint32_t id_val = UINT32_MAX;

  if (!td->PosixOffset && !(td->Flags & DS_DOMAIN_PRIMARY) && td->DomainSid)
    {
      if (cldap->open (NULL) == NO_ERROR)
	id_val = cldap->fetch_posix_offset_for_domain (td->DnsDomainName);
      if (id_val < PRIMARY_POSIX_OFFSET)
	{
	  /* If the offset is less than the primay domain offset, we're bound
	     to suffer collisions with system and local accounts.  Move offset
	     to a fixed replacement fake offset.  This may result in collisions
	     between other domains all of which were moved to this replacement
	     offset, but we can't fix all problems caused by careless admins. */
	  id_val = UNUSABLE_POSIX_OFFSET;
	}
      else if (id_val == UINT32_MAX)
	{
	  /* We're probably running under a local account, so we're not allowed
	     to fetch any information from AD beyond the most obvious.  Fake a
	     reasonable posix offset as above and hope for the best. */
	  id_val = NOACCESS_POSIX_OFFSET;
	}
      td->PosixOffset = id_val;
    }
  return td->PosixOffset;
}

/* CV 2014-05-08: USER_INFO_24 is not yet defined in Mingw64, but will be in
   the next release.  For the time being, define the structure here with
   another name which won't collide with the upcoming correct definition
   in lmaccess.h. */
struct cyg_USER_INFO_24
{
  BOOL   usri24_internet_identity;
  DWORD  usri24_flags;
  LPWSTR usri24_internet_provider_name;
  LPWSTR usri24_internet_principal_name;
  PSID   usri24_user_sid;
};

char *
pwdgrp::fetch_account_from_windows (fetch_user_arg_t &arg, cyg_ldap *pldap)
{
  /* Used in LookupAccount calls. */
  WCHAR namebuf[UNLEN + 1], *name = namebuf;
  WCHAR dom[DNLEN + 1] = L"";
  cygsid csid;
  DWORD nlen = UNLEN + 1;
  DWORD dlen = DNLEN + 1;
  DWORD slen = SECURITY_MAX_SID_SIZE;
  cygpsid sid (NO_SID);
  SID_NAME_USE acc_type;
  BOOL ret = false;
  /* Cygwin user name style. */
  bool fully_qualified_name = false;
  /* Computed stuff. */
  uid_t uid = ILLEGAL_UID;
  gid_t gid = ILLEGAL_GID;
  bool is_domain_account = true;
  PCWSTR domain = NULL;
  bool is_current_user = false;
  char *shell = NULL;
  char *home = NULL;
  char *gecos = NULL;
  /* Temporary stuff. */
  PWCHAR p;
  WCHAR sidstr[128];
  ULONG posix_offset = 0;
  uint32_t id_val;
  cyg_ldap loc_ldap;
  cyg_ldap *cldap = pldap ?: &loc_ldap;

  /* Initialize */
  if (!cygheap->dom.init ())
    return NULL;

  switch (arg.type)
    {
    case FULL_acc_arg:
      {
	sid = arg.full_acc->sid;
	*wcpncpy (name, arg.full_acc->name->Buffer,
		  arg.full_acc->name->Length / sizeof (WCHAR)) = L'\0';
	*wcpncpy (dom, arg.full_acc->dom->Buffer,
		  arg.full_acc->dom->Length / sizeof (WCHAR)) = L'\0';
	acc_type = arg.full_acc->acc_type;
      	ret = acc_type != SidTypeUnknown;
      }
      break;
    case SID_arg:
      sid = *arg.sid;
      ret = LookupAccountSidW (NULL, sid, name, &nlen, dom, &dlen, &acc_type);
      if (!ret
	  && cygheap->dom.member_machine ()
	  && sid_id_auth (sid) == 5 /* SECURITY_NT_AUTHORITY */
	  && sid_sub_auth (sid, 0) == SECURITY_BUILTIN_DOMAIN_RID)
	{
	  /* LookupAccountSid called on a non-DC cannot resolve aliases which
	     are not defined in the local SAM.  If we encounter an alias which
	     can't be resolved, and if we're a domain member machine, ask a DC.
	     Do *not* use LookupAccountSidW.  It can take ages when called on a
	     DC for some weird reason.  Use LDAP instead. */
	  PWCHAR val;

	  if (cldap->fetch_ad_account (sid, true)
	      && (val = cldap->get_group_name ()))
	    {
	      wcpcpy (name, val);
	      wcpcpy (dom, L"BUILTIN");
	      acc_type = SidTypeAlias;
	      ret = true;
	    }
	}
      if (!ret)
	debug_printf ("LookupAccountSid(%W), %E", sid.string (sidstr));
      break;
    case NAME_arg:
      bool fq_name;

      fq_name = false;
      /* Copy over to wchar for search. */
      sys_mbstowcs (name, UNLEN + 1, arg.name);
      /* If the incoming name has a backslash or at sign, and neither backslash
	 nor at are the domain separator chars, the name is invalid. */
      if ((p = wcspbrk (name, L"\\@")) && *p != cygheap->pg.nss_separator ()[0])
	{
	  debug_printf ("Invalid account name <%s> (backslash/at)", arg.name);
	  return NULL;
	}
      /* Replace domain separator char with backslash and make sure p is NULL
	 or points to the backslash. */
      if ((p = wcschr (name, cygheap->pg.nss_separator ()[0])))
	{
	  fq_name = true;
	  *p = L'\\';
	}
      sid = csid;
      ret = LookupAccountNameW (NULL, name, sid, &slen, dom, &dlen, &acc_type);
      /* If this is a name-only S-1-5-21 account *and* it's a machine account
	 on a domain member machine, then we found the wrong one.  Another
	 weird, but perfectly valid case is, if the group name is identical
	 to the domain name.  Try again with domain name prepended. */
      if (ret
	  && !fq_name
	  && sid_id_auth (sid) == 5 /* SECURITY_NT_AUTHORITY */
	  && sid_sub_auth (sid, 0) == SECURITY_NT_NON_UNIQUE
	  && cygheap->dom.member_machine ()
	  && (wcscasecmp (dom, cygheap->dom.account_flat_name ()) == 0
	      || acc_type == SidTypeDomain))
	{
	  p = wcpcpy (name, cygheap->dom.primary_flat_name ());
	  *p = L'\\';
	  sys_mbstowcs (p + 1, UNLEN + 1, arg.name);
	  slen = SECURITY_MAX_SID_SIZE;
	  dlen = DNLEN + 1;
	  sid = csid;
	  ret = LookupAccountNameW (NULL, name, sid, &slen, dom, &dlen,
				    &acc_type);
	}
      if (!ret)
	{
	  if (!strcmp (arg.name, "no+body"))
	    {
	      /* Special case "nobody" for reproducible construction of a
		 nobody SID for WinFsp and similar services.  We use the
		 value 65534 which is -2 with 16 bit uid/gids. */
	      csid.create (0, 1, 0xfffe);
	      break;
	    }
	  debug_printf ("LookupAccountNameW (%W), %E", name);
	  return NULL;
	}
      /* We can skip the backslash in the rest of this function. */
      if (p)
	name = p + 1;
      /* Last but not least, some validity checks on the name style. */
      if (!fq_name)
	{
	  /* AzureAD user must be prepended by "domain" name. */
	  if (sid_id_auth (sid) == 12)
	    return NULL;
	  /* name_only only if db_prefix is auto. */
	  if (!cygheap->pg.nss_prefix_auto ())
	    {
	      debug_printf ("Invalid account name <%s> (name only/"
			    "db_prefix not auto)", arg.name);
	      return NULL;
	    }
	  /* name_only account is either builtin or primary domain, or
	     account domain on non-domain machines. */
	  if (sid_id_auth (sid) == 5 /* SECURITY_NT_AUTHORITY */
	      && sid_sub_auth (sid, 0) == SECURITY_NT_NON_UNIQUE)
	    {
	      if (cygheap->dom.member_machine ())
		{
		  if (wcscasecmp (dom, cygheap->dom.primary_flat_name ()) != 0)
		    {
		      debug_printf ("Invalid account name <%s> (name only/"
				    "non primary on domain machine)", arg.name);
		      return NULL;
		    }
		}
	      else if (wcscasecmp (dom, cygheap->dom.account_flat_name ()) != 0)
		{
		  debug_printf ("Invalid account name <%s> (name only/"
				"non machine on non-domain machine)", arg.name);
		  return NULL;
		}
	    }
	}
      else
	{
	  /* All is well if db_prefix is always. */
	  if (cygheap->pg.nss_prefix_always ())
	    break;
	  /* AzureAD accounts should be fully qualifed either. */
	  if (sid_id_auth (sid) == 12)
	    break;
	  /* Otherwise, no fully_qualified for builtin accounts, except for
	     NT SERVICE, for which we require the prefix.  Note that there's
	     no equivalent test in the `if (!fq_name)' branch above, because
	     LookupAccountName never returns NT SERVICE accounts if they are
	     not prependend with the domain anyway. */
	  if (sid_id_auth (sid) != 5 /* SECURITY_NT_AUTHORITY */
	      || (sid_sub_auth (sid, 0) != SECURITY_NT_NON_UNIQUE
		  && sid_sub_auth (sid, 0) != SECURITY_SERVICE_ID_BASE_RID))
	    {
	      debug_printf ("Invalid account name <%s> (fully qualified/"
			    "not NON_UNIQUE or NT_SERVICE)", arg.name);
	      return NULL;
	    }
	  /* All is well if db_prefix is primary. */
	  if (cygheap->pg.nss_prefix_primary ())
	    break;
	  /* Domain member and domain == primary domain? */
	  if (cygheap->dom.member_machine ())
	    {
	      if (!wcscasecmp (dom, cygheap->dom.primary_flat_name ()))
		{
		  debug_printf ("Invalid account name <%s> (fully qualified/"
				"primary domain account)", arg.name);
		  return NULL;
		}
	    }
	  /* Not domain member and domain == account domain? */
	  else if (!wcscasecmp (dom, cygheap->dom.account_flat_name ()))
	    {
	      debug_printf ("Invalid account name <%s> (fully qualified/"
			    "local account)", arg.name);
	      return NULL;
	    }
	}
      break;
    case ID_arg:
      /* Construct SID from ID using the SFU rules, just like the code below
         goes the opposite route. */
#ifndef INTERIX_COMPATIBLE
      /* Except for Builtin and Alias groups in the SECURITY_NT_AUTHORITY.
	 We create uid/gid values compatible with the old values generated
	 by mkpasswd/mkgroup. */
      if (arg.id < 0x200)
	csid.create (5, 1, arg.id & 0x1ff);
      else if (arg.id <= 0x3e7)
	csid.create (5, 2, 32, arg.id & 0x3ff);
      else if (arg.id == 0x3e8) /* Special case "Other Organization" */
	csid.create (5, 1, 1000);
      else
#endif
      if (arg.id == 0xffe)
	{
	  /* OtherSession != Logon SID. */
	  get_logon_sid ();
	  /* LookupAccountSidW will fail. */
	  sid = csid = logon_sid;
	  sid_sub_auth_rid (sid) = 0;
	  break;
	}
      else if (arg.id == 0xfff)
	{
	  /* CurrentSession == Logon SID. */
	  get_logon_sid ();
	  /* LookupAccountSidW will fail. */
	  sid = logon_sid;
	  break;
	}
      else if (arg.id == 0x1000)
        {
	  /* AzureAD S-1-12-1-W-X-Y-Z user */
	  csid = cygheap->user.saved_sid ();
	}
      else if (arg.id == 0x1001)
        {
	  /* Special AzureAD group SID */
	  get_azure_grp_sid ();
	  /* LookupAccountSidW will fail. */
	  sid = csid = azure_grp_sid;
	  break;
	}
      else if (arg.id == 0xfffe)
	{
	  /* Special case "nobody" for reproducible construction of a
	     nobody SID for WinFsp and similar services.  We use the
	     value 65534 which is -2 with 16 bit uid/gids. */
	  csid.create (0, 1, 0xfffe);
	  sid = csid;
	  break;
	}
      else if (arg.id < 0x10000)
	{
	  /* Nothing. */
	  debug_printf ("Invalid POSIX id %u", arg.id);
	  return NULL;
	}
      else if (arg.id < 0x20000)
	{
	  /* Well-Known Group */
	  arg.id -= 0x10000;
	  /* SECURITY_APP_PACKAGE_AUTHORITY */
	  if (arg.id >= 0xf20 && arg.id <= 0xf3f)
	    csid.create (15, 2, (arg.id >> 4) & 0xf, arg.id & 0xf);
	  else
	    csid.create (arg.id >> 8, 1, arg.id & 0xff);
	}
      else if (arg.id >= 0x30000 && arg.id < 0x40000)
	{
	  /* Account domain user or group. */
	  csid = cygheap->dom.account_sid ();
	  csid.append (arg.id & 0xffff);
	}
      else if (arg.id < 0x60000)
	{
	  /* Builtin Alias */
	  csid.create (5, 2, arg.id >> 12, arg.id & 0xffff);
	}
      else if (arg.id < 0x70000)
	{
	  /* Mandatory Label. */
	  csid.create (16, 1, arg.id & 0xffff);
	}
      else if (arg.id < 0x80000)
	{
	  /* Identity assertion SIDs. */
	  csid.create (18, 1, arg.id & 0xffff);
	}
      else if (arg.id < PRIMARY_POSIX_OFFSET)
	{
	  /* Nothing. */
	  debug_printf ("Invalid POSIX id %u", arg.id);
	  return NULL;
	}
      else if (arg.id == ILLEGAL_UID)
	{
	  /* Just some fake. */
	  sid = csid.create (99, 1, 0);
	  break;
	}
      else if (arg.id >= UNIX_POSIX_OFFSET)
	{
	  /* UNIX (unknown NFS or Samba) user account. */
	  csid.create (22, 2, is_group () ? 2 : 1,  arg.id & UNIX_POSIX_MASK);
	  /* LookupAccountSidW will fail. */
	  sid = csid;
	  break;
	}
      else
	{
	  /* Some trusted domain? */
	  PDS_DOMAIN_TRUSTSW td = NULL, this_td = NULL;

	  for (ULONG idx = 0; (td = cygheap->dom.trusted_domain (idx)); ++idx)
	    {
	      fetch_posix_offset (td, &loc_ldap);
	      if (td->PosixOffset > posix_offset && td->PosixOffset <= arg.id)
		posix_offset = (this_td = td)->PosixOffset;
	    }
	  if (this_td)
	    {
	      csid = this_td->DomainSid;
	      csid.append (arg.id - posix_offset);
	    }
	  else
	    {
	      /* Primary domain */
	      csid = cygheap->dom.primary_sid ();
	      csid.append (arg.id - PRIMARY_POSIX_OFFSET);
	    }
	  posix_offset = 0;
	}
      sid = csid;
      ret = LookupAccountSidW (NULL, sid, name, &nlen, dom, &dlen, &acc_type);
      if (!ret)
	{
	  debug_printf ("LookupAccountSidW (%W), %E", sid.string (sidstr));
	  return NULL;
	}
      break;
    }
  if (ret)
    {
      /* Builtin account?  SYSTEM, for instance, is returned as SidTypeUser,
	 if a process is running as LocalSystem service.
	 Microsoft Account?  These show up in the user's group list, using the
	 undocumented security authority 11.  Even though this is officially a
	 user account, it only matters as part of the group list, so we convert
	 it to a well-known group here. */
      if (acc_type == SidTypeUser
	  && (sid_sub_auth_count (sid) <= 3 || sid_id_auth (sid) == 11))
	acc_type = SidTypeWellKnownGroup;
      switch (acc_type)
      	{
	case SidTypeUser:
	  if (is_group ())
	    {
	      /* Don't allow users as group.  While this is technically
		 possible, it doesn't make sense in a POSIX scenario.
	 
		 Microsoft Accounts as well as AzureAD accounts have the
		 primary group SID in their user token set to their own
		 user SID.

		 Those we let pass, but no others. */
	      bool its_ok = false;
	      if (sid_id_auth (sid) == 12)
		its_ok = true;
	      else if (wincap.has_microsoft_accounts ())
		{
		  struct cyg_USER_INFO_24 *ui24;
		  if (NetUserGetInfo (NULL, name, 24, (PBYTE *) &ui24)
		      == NERR_Success)
		    {
		      if (ui24->usri24_internet_identity)
			its_ok = true;
		      NetApiBufferFree (ui24);
		    }
		}
	      if (!its_ok)
		return NULL;
	    }
	  /*FALLTHRU*/
	case SidTypeGroup:
	case SidTypeAlias:
	  /* Predefined alias? */
	  if (acc_type == SidTypeAlias
	      && sid_sub_auth (sid, 0) != SECURITY_NT_NON_UNIQUE)
	    {
#ifdef INTERIX_COMPATIBLE
	      posix_offset = 0x30000;
	      uid = 0x1000 * sid_sub_auth (sid, 0)
		    + (sid_sub_auth_rid (sid) & 0xffff);
#else
	      posix_offset = 0;
#endif
	      fully_qualified_name = cygheap->pg.nss_prefix_always ();
	      is_domain_account = false;
	    }
	  /* Account domain account? */
	  else if (!wcscasecmp (dom, cygheap->dom.account_flat_name ()))
	    {
	      posix_offset = 0x30000;
	      if (cygheap->dom.member_machine ()
		  || !cygheap->pg.nss_prefix_auto ())
		fully_qualified_name = true;
	      is_domain_account = false;
	    }
	  /* Domain member machine? */
	  else if (cygheap->dom.member_machine ())
	    {
	      /* Primary domain account? */
	      if (!wcscasecmp (dom, cygheap->dom.primary_flat_name ()))
		{
		  posix_offset = PRIMARY_POSIX_OFFSET;
		  /* In theory domain should have been set to
		     cygheap->dom.primary_dns_name (), but it turns out that
		     not setting the domain here has advantages.  We open the
		     ldap connection to NULL (== some DC of our primary domain)
		     anyway.  So the domain is only used later on.  So, don't
		     set domain here to non-NULL, unless you're sure you have
		     also changed subsequent assumptions that domain is NULL
		     if it's a primary domain account. */
		  if (!cygheap->pg.nss_prefix_auto ())
		    fully_qualified_name = true;
		}
	      else
		{
		  /* No, fetch POSIX offset. */
		  PDS_DOMAIN_TRUSTSW td = NULL;

		  fully_qualified_name = true;
		  for (ULONG idx = 0;
		       (td = cygheap->dom.trusted_domain (idx));
		       ++idx)
		    if (!wcscasecmp (dom, td->NetbiosDomainName))
		      {
			domain = td->DnsDomainName;
			break;
		      }
		  if (!domain)
		    {
		      /* This shouldn't happen, in theory, but it does.  There
			 are cases where the user's logon domain does not show
			 up in the list of trusted domains.  We're desperately
			 trying to workaround that here bu adding an entry for
			 this domain to the trusted domains and ask the DC for
			 a  posix_offset.  There's a good chance this doesn't
			 work either, but at least we tried, and the user can
			 work. */
		      debug_printf ("Unknown domain %W", dom);
		      td = cygheap->dom.add_domain (dom, sid);
		      if (td)
			domain = td->DnsDomainName;
		    }
		  if (domain)
		    posix_offset = fetch_posix_offset (td, &loc_ldap);
		}
	    }
	  /* AzureAD S-1-12-1-W-X-Y-Z user */
	  else if (sid_id_auth (sid) == 12)
	    {
	      uid = gid = 0x1000;
	      fully_qualified_name = true;
	      home = cygheap->pg.get_home ((PUSER_INFO_3) NULL, sid, dom, name,
					   fully_qualified_name);
	      shell = cygheap->pg.get_shell ((PUSER_INFO_3) NULL, sid, dom,
					     name, fully_qualified_name);
	      gecos = cygheap->pg.get_gecos ((PUSER_INFO_3) NULL, sid, dom,
					     name, fully_qualified_name);
	      break;
	    }
	  /* If the domain returned by LookupAccountSid is not our machine
	     name, and if our machine is no domain member, we lose.  We have
	     nobody to ask for the POSIX offset. */
	  else
	    {
	      debug_printf ("Unknown domain %W", dom);
	      return NULL;
	    }
	  /* Generate uid/gid values. */
	  if (uid == ILLEGAL_UID)
	    uid = posix_offset + sid_sub_auth_rid (sid);
	  if (!is_group () && acc_type == SidTypeUser)
	    {
	      /* Default primary group.  If the sid is the current user, fetch
		 the default group from the current user token, otherwise make
		 the educated guess that the user is in group "Domain Users"
		 or "None". */
	      if (sid == cygheap->user.sid ())
		{
		  is_current_user = true;
		  gid = posix_offset
			+ sid_sub_auth_rid (cygheap->user.groups.pgsid);
		}
	      else
		gid = posix_offset + DOMAIN_GROUP_RID_USERS;
	    }

	  if (is_domain_account)
	    {
	      /* Skip this when creating group entries and for non-users. */
	      if (is_group() || acc_type != SidTypeUser)
		break;
	      /* On AD machines, use LDAP to fetch domain account infos. */
	      if (cygheap->dom.primary_dns_name ())
		{
		  /* For the current user we got the primary group from the
		     user token.  For any other user we fetch it from AD. */
		  if (!is_current_user
		      && cldap->fetch_ad_account (sid, false, domain)
		      && (id_val = cldap->get_primary_gid ()) != ILLEGAL_GID)
		    gid = posix_offset + id_val;
		  home = cygheap->pg.get_home (cldap, sid, dom, domain,
					       name, fully_qualified_name);
		  shell = cygheap->pg.get_shell (cldap, sid, dom, domain,
						 name,
						 fully_qualified_name);
		  gecos = cygheap->pg.get_gecos (cldap, sid, dom, domain,
						 name, fully_qualified_name);
		  /* Check and, if necessary, add unix<->windows id mapping
		     on the fly, unless we're called from getpwent. */
		  if (!pldap && cldap->is_open ())
		    {
		      id_val = cldap->get_unix_uid ();
		      if (id_val != ILLEGAL_UID
			  && cygheap->ugid_cache.get_uid (id_val)
			     == ILLEGAL_UID)
			cygheap->ugid_cache.add_uid (id_val, uid);
		    }
		}
	      /* If primary_dns_name() is empty, we're likely running under an
		 NT4 domain, so we can't use LDAP.  For user accounts fall back
		 to NetUserGetInfo.  This isn't overly fast, but keep in mind
		 that NT4 domains are mostly replaced by AD these days. */
	      else
		{
		  WCHAR server[INTERNET_MAX_HOST_NAME_LENGTH + 3];
		  NET_API_STATUS nas;
		  PUSER_INFO_3 ui;
		  
		  if (!get_logon_server (cygheap->dom.primary_flat_name (),
					 server, DS_IS_FLAT_NAME))
		    break;
		  nas = NetUserGetInfo (server, name, 3, (PBYTE *) &ui);
		  if (nas != NERR_Success)
		    {
		      debug_printf ("NetUserGetInfo(%W) %u", name, nas);
		      break;
		    }
		  gid = posix_offset + ui->usri3_primary_group_id;
		  home = cygheap->pg.get_home (ui, sid, dom, name,
					       fully_qualified_name);
		  shell = cygheap->pg.get_shell (ui, sid, dom, name,
						 fully_qualified_name);
		  gecos = cygheap->pg.get_gecos (ui, sid, dom, name,
						 fully_qualified_name);
		}
	    }
	  /* Otherwise check account domain (local SAM).*/
	  else
	    {
	      NET_API_STATUS nas;
	      PUSER_INFO_3 ui;
	      PLOCALGROUP_INFO_1 gi;
	      char *pgrp = NULL;
	      char *uxid = NULL;

	      if (acc_type == SidTypeUser)
		{
		  nas = NetUserGetInfo (NULL, name, 3, (PBYTE *) &ui);
		  if (nas != NERR_Success)
		    {
		      debug_printf ("NetUserGetInfo(%W) %u", name, nas);
		      break;
		    }
		  /* Fetch user attributes. */
		  home = cygheap->pg.get_home (ui, sid, dom, name,
					       fully_qualified_name);
		  shell = cygheap->pg.get_shell (ui, sid, dom, name,
						 fully_qualified_name);
		  gecos = cygheap->pg.get_gecos (ui, sid, dom, name,
						 fully_qualified_name);
		  uxid = fetch_from_description (ui->usri3_comment,
						 L"unix=\"", 6);
		  pgrp = fetch_from_description (ui->usri3_comment,
						 L"group=\"", 7);
		}
	      else /* acc_type == SidTypeAlias */
		{
		  nas = NetLocalGroupGetInfo (NULL, name, 1, (PBYTE *) &gi);
		  if (nas != NERR_Success)
		    {
		      debug_printf ("NetLocalGroupGetInfo(%W) %u", name, nas);
		      break;
		    }
		  /* Fetch unix gid from comment field. */
		  uxid = fetch_from_description (gi->lgrpi1_comment,
						 L"unix=\"", 6);
		}

	      if (acc_type == SidTypeUser)
		NetApiBufferFree (ui);
	      else
		NetApiBufferFree (gi);
	      if (pgrp)
		{
		  /* Set primary group from the "Description" field.  Prepend
		     account domain if this is a domain member machine or the
		     db_prefix setting requires it. */
		  char gname[2 * DNLEN + strlen (pgrp) + 1], *gp = gname;
		  struct group *gr;

		  if (cygheap->dom.member_machine ()
		      || !cygheap->pg.nss_prefix_auto ())
		    {
		      gp = gname
			   + sys_wcstombs (gname, sizeof gname,
					   cygheap->dom.account_flat_name ());
		      *gp++ = cygheap->pg.nss_separator ()[0];
		    }
		  stpcpy (gp, pgrp);
		  if ((gr = internal_getgrnam (gname, cldap)))
		    gid = gr->gr_gid;
		}
	      char *e;
	      if (!pldap && uxid && ((id_val = strtoul (uxid, &e, 10)), !*e))
		{
		  if (acc_type == SidTypeUser)
		    {
		      if (cygheap->ugid_cache.get_uid (id_val) == ILLEGAL_UID)
			cygheap->ugid_cache.add_uid (id_val, uid);
		    }
		  else if (cygheap->ugid_cache.get_gid (id_val) == ILLEGAL_GID)
		    cygheap->ugid_cache.add_gid (id_val, uid);
		}
	      if (pgrp)
		free (pgrp);
	      if (uxid)
		free (uxid);
	    }
	  break;
	case SidTypeWellKnownGroup:
	  fully_qualified_name = (cygheap->pg.nss_prefix_always ()
		  /* NT SERVICE Account */
		  || (sid_id_auth (sid) == 5 /* SECURITY_NT_AUTHORITY */
		      && sid_sub_auth (sid, 0) == SECURITY_SERVICE_ID_BASE_RID)
		  /* Microsoft Account */
		  || sid_id_auth (sid) == 11);
#ifdef INTERIX_COMPATIBLE
	  if (sid_id_auth (sid) == 5 /* SECURITY_NT_AUTHORITY */
	      && sid_sub_auth_count (sid) > 1)
	    {
	      uid = 0x1000 * sid_sub_auth (sid, 0)
		    + (sid_sub_auth_rid (sid) & 0xffff);
	      fully_qualified_name = true;
	    }
	  else
	    uid = 0x10000 + 0x100 * sid_id_auth (sid)
		  + (sid_sub_auth_rid (sid) & 0xff);
#else
	  if (sid_id_auth (sid) == 15 /* SECURITY_APP_PACKAGE_AUTHORITY */)
	    uid = 0x10000 + 0x100 * sid_id_auth (sid)
		  + 0x10 * sid_sub_auth (sid, 0)
		  + (sid_sub_auth_rid (sid) & 0xf);
	  else if (sid_id_auth (sid) != 5 /* SECURITY_NT_AUTHORITY */)
	    uid = 0x10000 + 0x100 * sid_id_auth (sid)
		  + (sid_sub_auth_rid (sid) & 0xff);
	  else if (sid_sub_auth (sid, 0) < SECURITY_PACKAGE_BASE_RID
		   || sid_sub_auth (sid, 0) > SECURITY_MAX_BASE_RID)
	    uid = sid_sub_auth_rid (sid) & 0x7ff;
	  else
	    {
	      uid = 0x1000 * sid_sub_auth (sid, 0)
		    + (sid_sub_auth_rid (sid) & 0xffff);
	    }
#endif
	  /* Special case for "NULL SID", S-1-0-0 and "Everyone", S-1-1-0.
	     Never return "NULL SID" or Everyone as user or group. */
	  if (uid == 0x10000 || uid == 0x10100)
	    return NULL;
	  break;
	case SidTypeLabel:
	  uid = 0x60000 + sid_sub_auth_rid (sid);
	  fully_qualified_name = cygheap->pg.nss_prefix_always ();
	  break;
	default:
	  return NULL;
	}
    } 
  else if (sid_id_auth (sid) == 0 && sid_sub_auth (sid, 0) == 0xfffe)
    {
      /* Special case "nobody" for reproducible construction of a
	 nobody SID for WinFsp and similar services.  We use the
	 value 65534 which is -2 with 16 bit uid/gids. */
      uid = gid = 0xfffe;
      wcpcpy (dom, L"no");
      wcpcpy (name = namebuf, L"body");
      fully_qualified_name = true;
      acc_type = SidTypeUnknown;
    }
  else if (sid_id_auth (sid) == 12 && sid_sub_auth (sid, 0) == 1)
    {
      /* Special AzureAD group SID which can't be resolved by
         LookupAccountSid (ERROR_NONE_MAPPED).  This is only allowed
	 as group entry, not as passwd entry. */
      if (is_passwd ())
	return NULL;
      uid = gid = 0x1001;
      wcpcpy (dom, L"AzureAD");
      wcpcpy (name = namebuf, L"Group");
      fully_qualified_name = true;
      acc_type = SidTypeUnknown;
    }
  else if (sid_id_auth (sid) == 5 /* SECURITY_NT_AUTHORITY */
	   && sid_sub_auth (sid, 0) == SECURITY_LOGON_IDS_RID)
    {
      /* Logon ID. Mine or other? */
      get_logon_sid ();
      if (PSID (logon_sid) == NO_SID)
	return NULL;
      if (RtlEqualSid (sid, logon_sid))
	{
	  uid = 0xfff;
	  wcpcpy (name = namebuf, L"CurrentSession");
	}
      else
	{
	  uid = 0xffe;
	  wcpcpy (name = namebuf, L"OtherSession");
	}
      acc_type = SidTypeUnknown;
    }
  else if (sid_id_auth (sid) == 18)
    {
      /* Authentication assertion SIDs.
      
         Available when using a 2012R2 DC, but not supported by
	 LookupAccountXXX on pre Windows 8/2012 machines */
      uid = 0x11200 + sid_sub_auth_rid (sid);
      wcpcpy (name = namebuf, sid_sub_auth_rid (sid) == 1
	      ? (PWCHAR) L"Authentication authority asserted identity"
	      : (PWCHAR) L"Service asserted identity");
      fully_qualified_name = false;
      acc_type = SidTypeUnknown;
    }
  else if (sid_id_auth (sid) == 22)
    {
      /* Samba UNIX Users/Groups

         This *might* collide with a posix_offset of some trusted domain.
         It's just very unlikely. */
      uid = MAP_UNIX_TO_CYGWIN_ID (sid_sub_auth_rid (sid));
      /* Unfortunately we have no access to the file server from here,
	 so we can't generate correct user names. */
      p = wcpcpy (dom, L"Unix_");
      wcpcpy (p, sid_sub_auth (sid, 0) == 1 ? L"User" : L"Group");
      __small_swprintf (name = namebuf, L"%d", uid & UNIX_POSIX_MASK);
      fully_qualified_name = true;
      acc_type = SidTypeUnknown;
    }
  else
    {
      if (cygheap->dom.member_machine ()
	  && sid_id_auth (sid) == 5 /* SECURITY_NT_AUTHORITY */
	  && sid_sub_auth (sid, 0) == SECURITY_NT_NON_UNIQUE)
	{
	  /* Check if we know the domain.  If so, create a passwd/group
	     entry with domain prefix and RID as username. */
	  PDS_DOMAIN_TRUSTSW td = NULL;

	  sid_sub_auth_count (sid) = sid_sub_auth_count (sid) - 1;
	  if (RtlEqualSid (sid, cygheap->dom.primary_sid ()))
	    {
	      domain = cygheap->dom.primary_flat_name ();
	      posix_offset = PRIMARY_POSIX_OFFSET;
	    }
	  else
	    for (ULONG idx = 0; (td = cygheap->dom.trusted_domain (idx)); ++idx)
	      if (td->DomainSid && RtlEqualSid (sid, td->DomainSid))
		{
		  domain = td->NetbiosDomainName;
		  posix_offset = fetch_posix_offset (td, &loc_ldap);
		  break;
		}
	  sid_sub_auth_count (sid) = sid_sub_auth_count (sid) + 1;
	}
      if (domain)
	{
	  wcscpy (dom, domain);
	  __small_swprintf (name = namebuf, L"%W(%u)",
			    is_group () ? L"Group" : L"User",
			    sid_sub_auth_rid (sid));
	  uid = posix_offset + sid_sub_auth_rid (sid);
	}
      else
	{
	  wcpcpy (dom, L"Unknown");
	  wcpcpy (name = namebuf, is_group () ? L"Group" : L"User");
	}
      fully_qualified_name = true;
      acc_type = SidTypeUnknown;
    }

  tmp_pathbuf tp;
  char *linebuf = tp.c_get ();
  char *line = NULL;

  WCHAR posix_name[UNLEN + 1 + DNLEN + 1];
  p = posix_name;
  if (gid == ILLEGAL_GID)
    gid = uid;
  if (fully_qualified_name)
    p = wcpcpy (wcpcpy (p, dom), cygheap->pg.nss_separator ());
  wcpcpy (p, name);

  if (is_group ())
    __small_sprintf (linebuf, "%W:%s:%u:",
		     posix_name, sid.string ((char *) sidstr), uid);
  /* For non-users, create a passwd entry which doesn't allow interactive
     logon.  Unless it's the SYSTEM account.  This conveniently allows to
     logon interactively as SYSTEM for debugging purposes. */
  else if (acc_type != SidTypeUser && sid != well_known_system_sid)
    __small_sprintf (linebuf, "%W:*:%u:%u:U-%W\\%W,%s:/:/sbin/nologin",
		     posix_name, uid, gid,
		     dom, name,
		     sid.string ((char *) sidstr));
  else
    __small_sprintf (linebuf, "%W:*:%u:%u:%s%sU-%W\\%W,%s:%s%W:%s",
		     posix_name, uid, gid,
		     gecos ?: "", gecos ? "," : "",
		     dom, name,
		     sid.string ((char *) sidstr),
		     home ?: "/home/", home ? L"" : name,
		     shell ?: "/bin/bash");
  if (gecos)
    free (gecos);
  if (home)
    free (home);
  if (shell)
    free (shell);
  line = cstrdup (linebuf);
  debug_printf ("line: <%s>", line);
  return line;
}

client_request_pwdgrp::client_request_pwdgrp (fetch_user_arg_t &arg, bool group)
  : client_request (CYGSERVER_REQUEST_PWDGRP, &_parameters, sizeof (_parameters))
{
  size_t len = 0;
  char *p;

  _parameters.in.group = group;
  _parameters.in.type = arg.type;
  switch (arg.type)
    {
    case SID_arg:
      RtlCopySid (sizeof (DBGSID), (PSID) &_parameters.in.arg.sid, *arg.sid);
      len = RtlLengthSid (*arg.sid);
      break;
    case NAME_arg:
      p = stpcpy (_parameters.in.arg.name, arg.name);
      len = p - _parameters.in.arg.name + 1;
      break;
    case ID_arg:
      _parameters.in.arg.id = arg.id;
      len = sizeof (uint32_t);
      break;
    default:
      api_fatal ("Fetching account info from cygserver with wrong arg.type "
		 "%d", arg.type);
    }
  msglen (__builtin_offsetof (struct _pwdgrp_param_t::_pwdgrp_in_t, arg) + len);
}

char *
pwdgrp::fetch_account_from_cygserver (fetch_user_arg_t &arg)
{
  client_request_pwdgrp request (arg, is_group ());
  if (request.make_request () == -1 || request.error_code ())
    {
      /* Cygserver not running?  Don't try again.  This will automatically
	 avoid an endless loop in cygserver itself. */
      if (request.error_code () == ENOSYS)
	cygheap->pg.nss_disable_cygserver_caching ();
      return NULL;
    }
  if (!request.line ())
    return NULL;
  return cstrdup (request.line ());
}

void *
pwdgrp::add_account_from_cygserver (cygpsid &sid)
{
  /* No, Everyone is no group in terms of POSIX. */
  if (sid_id_auth (sid) == 1 /* SECURITY_WORLD_SID_AUTHORITY */
      && sid_sub_auth (sid, 0) == SECURITY_WORLD_RID)
    return NULL;
  fetch_user_arg_t arg;
  arg.type = SID_arg;
  arg.sid = &sid;
  char *line = fetch_account_from_cygserver (arg);
  return add_account_post_fetch (line, true);
}

void *
pwdgrp::add_account_from_cygserver (const char *name)
{
  fetch_user_arg_t arg;
  arg.type = NAME_arg;
  arg.name = name;
  char *line = fetch_account_from_cygserver (arg);
  return add_account_post_fetch (line, true);
}

void *
pwdgrp::add_account_from_cygserver (uint32_t id)
{
  fetch_user_arg_t arg;
  arg.type = ID_arg;
  arg.id = id;
  char *line = fetch_account_from_cygserver (arg);
  return add_account_post_fetch (line, true);
}
