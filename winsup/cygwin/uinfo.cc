/* uinfo.cc: user info (uid, gid, etc...)

   Copyright 1996, 1997, 1998 Cygnus Solutions.

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
#include <sys/cygwin.h>
#include "sync.h"
#include "sigproc.h"
#include "pinfo.h"
#include "fhandler.h"
#include "dtable.h"
#include "cygheap.h"
#include "registry.h"
#include "security.h"

struct passwd *
internal_getlogin (cygheap_user &user)
{
  char username[UNLEN + 1];
  DWORD username_len = UNLEN + 1;
  struct passwd *pw = NULL;

  if (!user.name ())
    if (!GetUserName (username, &username_len))
      user.set_name ("unknown");
    else
      user.set_name (username);

  if (os_being_run == winNT)
    {
      LPWKSTA_USER_INFO_1 wui;
      char buf[MAX_PATH], *env;
      char *un = NULL;

      /* First trying to get logon info from environment */
      if ((env = getenv ("USERNAME")) != NULL)
	un = env;
      if ((env = getenv ("LOGONSERVER")) != NULL)
	user.set_logsrv (env + 2); /* filter leading double backslashes */
      if ((env = getenv ("USERDOMAIN")) != NULL)
	user.set_domain (env);
      /* Trust only if usernames are identical */
      if (un && strcasematch (user.name (), un)
	  && user.domain () && user.logsrv ())
	debug_printf ("Domain: %s, Logon Server: %s",
		      user.domain (), user.logsrv ());
      /* If that failed, try to get that info from NetBIOS */
      else if (!NetWkstaUserGetInfo (NULL, 1, (LPBYTE *)&wui))
	{
	  char buf[512]; /* Bigger than each of the below defines. */

	  sys_wcstombs (buf, wui->wkui1_username, UNLEN + 1);
	  user.set_name (buf);
	  sys_wcstombs (buf, wui->wkui1_logon_server,
	  		INTERNET_MAX_HOST_NAME_LENGTH + 1);
	  user.set_logsrv (buf);
	  sys_wcstombs (buf, wui->wkui1_logon_domain,
			INTERNET_MAX_HOST_NAME_LENGTH + 1);
	  user.set_domain (buf);
	  /* Save values in environment */
	  if (!strcasematch (user.name (), "SYSTEM")
	      && user.domain () && user.logsrv ())
	    {
	      LPUSER_INFO_3 ui = NULL;
	      WCHAR wbuf[INTERNET_MAX_HOST_NAME_LENGTH + 2];

	      strcat (strcpy (buf, "\\\\"), user.logsrv ());
	      setenv ("USERNAME", user.name (), 1);
	      setenv ("LOGONSERVER", buf, 1);
	      setenv ("USERDOMAIN", user.domain (), 1);
	      /* HOMEDRIVE and HOMEPATH are wrong most of the time, too,
		 after changing user context! */
	      sys_mbstowcs (wbuf, buf, INTERNET_MAX_HOST_NAME_LENGTH + 2);
	      if (!NetUserGetInfo (NULL, wui->wkui1_username, 3, (LPBYTE *)&ui)
		  || !NetUserGetInfo (wbuf,wui->wkui1_username,3,(LPBYTE *)&ui))
		{
		  sys_wcstombs (buf, ui->usri3_home_dir, MAX_PATH);
		  if (!buf[0])
		    {
		      sys_wcstombs (buf, ui->usri3_home_dir_drive, MAX_PATH);
		      if (buf[0])
			strcat (buf, "\\");
		      else
			{
			  env = getenv ("SYSTEMDRIVE");
			  if (env && *env)
			    strcat (strcpy (buf, env), "\\");
			  else
			    GetSystemDirectoryA (buf, MAX_PATH);
			}
		    }
		  setenv ("HOMEPATH", buf + 2, 1);
		  buf[2] = '\0';
		  setenv ("HOMEDRIVE", buf, 1);
		  NetApiBufferFree (ui);
		}
	    }
	  debug_printf ("Domain: %s, Logon Server: %s, Windows Username: %s",
			user.domain (), user.logsrv (), user.name ());
	  NetApiBufferFree (wui);
	}
      if (allow_ntsec)
	{
	  HANDLE ptok = user.token; /* Which is INVALID_HANDLE_VALUE if no
				       impersonation took place. */
	  DWORD siz;
	  cygsid tu;
	  int ret = 0;

	  /* Try to get the SID either from already impersonated token
	     or from current process first. To differ that two cases is
	     important, because you can't rely on the user information
	     in a process token of a currently impersonated process. */
	  if (ptok == INVALID_HANDLE_VALUE
	      && !OpenProcessToken (GetCurrentProcess (),
	      			    TOKEN_ADJUST_DEFAULT | TOKEN_QUERY,
				    &ptok))
	    debug_printf ("OpenProcessToken(): %E\n");
	  else if (!GetTokenInformation (ptok, TokenUser, &tu, sizeof tu, &siz))
	    debug_printf ("GetTokenInformation(): %E");
	  else if (!(ret = user.set_sid (tu)))
	    debug_printf ("Couldn't retrieve SID from access token!");
	  /* If that failes, try to get the SID from localhost. This can only
	     be done if a domain is given because there's a chance that a local
	     and a domain user may have the same name. */
	  if (!ret && user.domain ())
	    {
	      /* Concat DOMAIN\USERNAME for the next lookup */
	      strcat (strcat (strcpy (buf, user.domain ()), "\\"), user.name ());
	      if (!(ret = lookup_name (buf, NULL, user.sid ())))
		debug_printf ("Couldn't retrieve SID locally!");
	    }

	  /* If that fails, too, as a last resort try to get the SID from
	     the logon server. */
	  if (!ret && !(ret = lookup_name (user.name (), user.logsrv (),
					  user.sid ())))
	    debug_printf ("Couldn't retrieve SID from '%s'!", user.logsrv ());

	  /* If we have a SID, try to get the corresponding Cygwin user name
	     which can be different from the Windows user name. */
	  cygsid gsid (NULL);
	  if (ret)
	    {
	      cygsid psid;

	      if (!strcasematch (user.name (), "SYSTEM")
		  && user.domain () && user.logsrv ())
		{
		  if (get_registry_hive_path (user.sid (), buf))
		    setenv ("USERPROFILE", buf, 1);
		}
	      for (int pidx = 0; (pw = internal_getpwent (pidx)); ++pidx)
		if (get_pw_sid (psid, pw) && EqualSid (user.sid (), psid))
		  {
		    user.set_name (pw->pw_name);
		    struct group *gr = getgrgid (pw->pw_gid);
		    if (gr)
		      if (!get_gr_sid (gsid.set (), gr))
			  gsid = NULL;
		    break;
		  }
	    }

	  /* If this process is started from a non Cygwin process,
	     set token owner to the same value as token user and
	     primary group to the group which is set as primary group
	     in /etc/passwd. */
	  if (ptok != INVALID_HANDLE_VALUE && myself->ppid == 1)
	    {
	      if (!SetTokenInformation (ptok, TokenOwner, &tu, sizeof tu))
		debug_printf ("SetTokenInformation(TokenOwner): %E");
	      if (gsid && !SetTokenInformation (ptok, TokenPrimaryGroup,
		  			        &gsid, sizeof gsid))
		debug_printf ("SetTokenInformation(TokenPrimaryGroup): %E");
	    }

	  /* Close token only if it's a result from OpenProcessToken(). */
	  if (ptok != INVALID_HANDLE_VALUE
	      && user.token == INVALID_HANDLE_VALUE)
	    CloseHandle (ptok);
	}
    }
  debug_printf ("Cygwins Username: %s", user.name ());
  return pw ?: getpwnam(user.name ());
}

void
uinfo_init ()
{
  struct passwd *p;

  /* Initialize to non impersonated values.
     Setting `impersonated' to TRUE seems to be wrong but it
     isn't. Impersonated is thought as "Current User and `token'
     are coincident". See seteuid() for the mechanism behind that. */
  cygwin_set_impersonation_token (INVALID_HANDLE_VALUE);
  cygheap->user.impersonated = TRUE;

  /* If uid is USHRT_MAX, the process is started from a non cygwin
     process or the user context was changed in spawn.cc */
  if (myself->uid == USHRT_MAX)
    if ((p = internal_getlogin (cygheap->user)) != NULL)
      {
	myself->uid = p->pw_uid;
	/* Set primary group only if ntsec is off or the process has been
	   started from a non cygwin process. */
	if (!allow_ntsec || myself->ppid == 1)
	  myself->gid = p->pw_gid;
      }
    else
      {
	myself->uid = DEFAULT_UID;
	myself->gid = DEFAULT_GID;
      }
  /* Real and effective uid/gid are always identical on process start up.
     This is at least true for NT/W2K. */
  cygheap->user.orig_uid = cygheap->user.real_uid = myself->uid;
  cygheap->user.orig_gid = cygheap->user.real_gid = myself->gid;
}

extern "C" char *
getlogin (void)
{
#ifdef _MT_SAFE
  char *this_username=_reent_winsup ()->_username;
#else
  static NO_COPY char this_username[UNLEN + 1];
#endif

  return strcpy (this_username, cygheap->user.name ());
}

extern "C" uid_t
getuid (void)
{
  return cygheap->user.real_uid;
}

extern "C" gid_t
getgid (void)
{
  return cygheap->user.real_gid;
}

extern "C" uid_t
geteuid (void)
{
  return myself->uid;
}

extern "C" gid_t
getegid (void)
{
  return myself->gid;
}

/* Not quite right - cuserid can change, getlogin can't */
extern "C" char *
cuserid (char *src)
{
  if (src)
    {
      strcpy (src, getlogin ());
      return src;
    }
  else
    {
      return getlogin ();
    }
}
