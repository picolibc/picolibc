/* uinfo.cc: user info (uid, gid, etc...)

   Copyright 1996, 1997, 1998, 1999, 2000, 2001, 2002 Red Hat, Inc.

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

#include "winsup.h"
#include <pwd.h>
#include <unistd.h>
#include <winnls.h>
#include <wininet.h>
#include <utmp.h>
#include <limits.h>
#include <stdlib.h>
#include <lm.h>
#include <errno.h>
#include <sys/cygwin.h>
#include "pinfo.h"
#include "security.h"
#include "fhandler.h"
#include "path.h"
#include "dtable.h"
#include "cygerrno.h"
#include "cygheap.h"
#include "registry.h"
#include "child_info.h"
#include "environ.h"
#include "pwdgrp.h"

void
internal_getlogin (cygheap_user &user)
{
  struct passwd *pw = NULL;
  HANDLE ptok = INVALID_HANDLE_VALUE;

  myself->gid = DEFAULT_GID;
  if (wincap.has_security ())
    {
      DWORD siz;
      cygsid tu;
      DWORD ret = 0;

      /* Try to get the SID either from current process and
	 store it in user.psid */
      if (!OpenProcessToken (hMainProc, TOKEN_ADJUST_DEFAULT | TOKEN_QUERY,
			     &ptok))
	system_printf ("OpenProcessToken(): %E");
      else if (!GetTokenInformation (ptok, TokenUser, &tu, sizeof tu, &siz))
	system_printf ("GetTokenInformation (TokenUser): %E");
      else if (!(ret = user.set_sid (tu)))
	system_printf ("Couldn't retrieve SID from access token!");
      else if (!GetTokenInformation (ptok, TokenPrimaryGroup,
				     &user.groups.pgsid, sizeof tu, &siz))
	system_printf ("GetTokenInformation (TokenPrimaryGroup): %E");
       /* We must set the user name, uid and gid.
	 If we have a SID, try to get the corresponding Cygwin
	 password entry. Set user name which can be different
	 from the Windows user name */
      if (ret)
	{
	  pw = internal_getpwsid (tu);
	  /* Set token owner to the same value as token user */
	  if (!SetTokenInformation (ptok, TokenOwner, &tu, sizeof tu))
	    debug_printf ("SetTokenInformation(TokenOwner): %E");
	 }
    }

  if (!pw && !(pw = internal_getpwnam (user.name ()))
      && !(pw = internal_getpwuid (DEFAULT_UID)))
    debug_printf("user not found in augmented /etc/passwd");
  else
    {
      myself->uid = pw->pw_uid;
      myself->gid = pw->pw_gid;
      user.set_name (pw->pw_name);
      if (wincap.has_security ())
	{
	  cygsid gsid;
	  if (gsid.getfromgr (internal_getgrgid (pw->pw_gid)))
	    {
	      /* Set primary group to the group in /etc/passwd. */
	      user.groups.pgsid = gsid;
	      if (!SetTokenInformation (ptok, TokenPrimaryGroup,
					&gsid, sizeof gsid))
		debug_printf ("SetTokenInformation(TokenPrimaryGroup): %E");
	    }
	  else
	    debug_printf ("gsid not found in augmented /etc/group");
	}
    }
  if (ptok != INVALID_HANDLE_VALUE)
    CloseHandle (ptok);
  (void) cygheap->user.ontherange (CH_HOME, pw);

  return;
}

void
uinfo_init ()
{
  if (!child_proc_info)
    internal_getlogin (cygheap->user); /* Set the cygheap->user. */

  /* Real and effective uid/gid are identical on process start up. */
  cygheap->user.orig_uid = cygheap->user.real_uid = myself->uid;
  cygheap->user.orig_gid = cygheap->user.real_gid = myself->gid;
  cygheap->user.set_orig_sid ();	/* Update the original sid */

  cygheap->user.token = INVALID_HANDLE_VALUE; /* No token present */
}

extern "C" char *
getlogin (void)
{
#ifdef _MT_SAFE
  char *this_username=_reent_winsup ()->_username;
#else
  static char this_username[UNLEN + 1] NO_COPY;
#endif

  return strcpy (this_username, cygheap->user.name ());
}

extern "C" __uid32_t
getuid32 (void)
{
  return cygheap->user.real_uid;
}

extern "C" __uid16_t
getuid (void)
{
  return cygheap->user.real_uid;
}

extern "C" __gid32_t
getgid32 (void)
{
  return cygheap->user.real_gid;
}

extern "C" __gid16_t
getgid (void)
{
  return cygheap->user.real_gid;
}

extern "C" __uid32_t
geteuid32 (void)
{
  return myself->uid;
}

extern "C" __uid16_t
geteuid (void)
{
  return myself->uid;
}

extern "C" __gid32_t
getegid32 (void)
{
  return myself->gid;
}

extern "C" __gid16_t
getegid (void)
{
  return myself->gid;
}

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
  LPUSER_INFO_3 ui = NULL;
  WCHAR wuser[UNLEN + 1];
  NET_API_STATUS ret;
  char homepath_env_buf[MAX_PATH + 1];
  char homedrive_env_buf[3];
  char *newhomedrive = NULL;
  char *newhomepath = NULL;


  debug_printf ("what %d, pw %p", what, pw);
  if (what == CH_HOME)
    {
      char *p;
      if (homedrive)
	newhomedrive = homedrive;
      else if ((p = getenv ("HOMEDRIVE")))
	newhomedrive = p;

      if (homepath)
	newhomepath = homepath;
      else if ((p = getenv ("HOMEPATH")))
	newhomepath = p;

      if ((p = getenv ("HOME")))
	debug_printf ("HOME is already in the environment %s", p);
      else
	{
	  if (!pw)
	    pw = internal_getpwnam (name ());
	  if (pw && pw->pw_dir && *pw->pw_dir)
	    {
	      debug_printf ("Set HOME (from /etc/passwd) to %s", pw->pw_dir);
	      setenv ("HOME", pw->pw_dir, 1);
	    }
	  else if (!newhomedrive || !newhomepath)
	    setenv ("HOME", "/", 1);
	  else
	    {
	      char home[MAX_PATH];
	      char buf[MAX_PATH + 1];
	      strcpy (buf, newhomedrive);
	      strcat (buf, newhomepath);
	      cygwin_conv_to_full_posix_path (buf, home);
	      debug_printf ("Set HOME (from HOMEDRIVE/HOMEPATH) to %s", home);
	      setenv ("HOME", home, 1);
	    }
	}
    }

  if (what != CH_HOME && homepath == NULL && newhomepath == NULL)
    {
      if (!pw)
	pw = internal_getpwnam (name ());
      if (pw && pw->pw_dir && *pw->pw_dir)
	cygwin_conv_to_full_win32_path (pw->pw_dir, homepath_env_buf);
      else
	{
	  homepath_env_buf[0] = homepath_env_buf[1] = '\0';
	  if (logsrv ())
	    {
	      WCHAR wlogsrv[INTERNET_MAX_HOST_NAME_LENGTH + 3];
	      sys_mbstowcs (wlogsrv, logsrv (),
			    sizeof (wlogsrv) / sizeof (*wlogsrv));
	     sys_mbstowcs (wuser, winname (), sizeof (wuser) / sizeof (*wuser));
	      if (!(ret = NetUserGetInfo (wlogsrv, wuser, 3,(LPBYTE *)&ui)))
		{
		  sys_wcstombs (homepath_env_buf, ui->usri3_home_dir, MAX_PATH);
		  if (!homepath_env_buf[0])
		    {
		      sys_wcstombs (homepath_env_buf, ui->usri3_home_dir_drive,
				    MAX_PATH);
		      if (homepath_env_buf[0])
			strcat (homepath_env_buf, "\\");
		      else
			cygwin_conv_to_full_win32_path ("/", homepath_env_buf);
		    }
		}
	    }
	  if (ui)
	    NetApiBufferFree (ui);
	}

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
  if (!mydomain || strcasematch (myname, "SYSTEM"))
    return almost_null;

  char logsrv[INTERNET_MAX_HOST_NAME_LENGTH + 3];
  cfree_and_set (plogsrv, almost_null);
  if (get_logon_server (mydomain, logsrv, NULL))
    plogsrv = cstrdup (logsrv);
  return plogsrv;
}

const char *
cygheap_user::env_domain (const char *name, size_t namelen)
{
  if (pwinname && test_uid (pdomain, name, namelen))
    return pdomain;

  char username[UNLEN + 1];
  DWORD ulen = sizeof (username);
  char userdomain[DNLEN + 1];
  DWORD dlen = sizeof (userdomain);
  SID_NAME_USE use;

  cfree_and_set (pwinname, almost_null);
  cfree_and_set (pdomain, almost_null);
  if (!LookupAccountSid (NULL, sid (), username, &ulen,
			 userdomain, &dlen, &use))
    __seterrno ();
  else
    {
      pwinname = cstrdup (username);
      pdomain = cstrdup (userdomain);
    }
  return pdomain;
}

const char *
cygheap_user::env_userprofile (const char *name, size_t namelen)
{
  if (test_uid (puserprof, name, namelen))
    return puserprof;

  char userprofile_env_buf[MAX_PATH + 1];
  cfree_and_set (puserprof, almost_null);
  /* FIXME: Should this just be setting a puserprofile like everything else? */
  const char *myname = winname ();
  if (myname && strcasematch (myname, "SYSTEM")
      && get_registry_hive_path (sid (), userprofile_env_buf))
    puserprof = cstrdup (userprofile_env_buf);

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
    (void) domain ();
  return pwinname;
}
