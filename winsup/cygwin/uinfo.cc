/* uinfo.cc: user info (uid, gid, etc...)

   Copyright 1996, 1997, 1998 Cygnus Solutions.

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

#include <pwd.h>
#include "winsup.h"
#include <utmp.h>
#include <limits.h>
#include <unistd.h>
#include "autoload.h"
#include <stdlib.h>
#include <wchar.h>
#include <lm.h>

/* FIXME: shouldn't violate internal object space -- these two
   should be static inside grp.cc */
void read_etc_group ();
extern int group_in_memory_p;

char *
internal_getlogin (struct pinfo *pi)
{
  if (! pi)
    api_fatal ("pinfo pointer is NULL!\n");

  DWORD username_len = MAX_USER_NAME;
  if (! GetUserName (pi->username, &username_len))
    strcpy (pi->username, "unknown");
  if (os_being_run == winNT)
    {
      LPWKSTA_USER_INFO_1 wui;
      char buf[256], *env;

      /* First trying to get logon info from environment */
      buf[0] = '\0';
      if ((env = getenv ("USERNAME")) != NULL)
        strcpy (buf, env);
      if ((env = getenv ("LOGONSERVER")) != NULL)
        strcpy (pi->logsrv, env + 2); /* filter leading double backslashes */
      if ((env = getenv ("USERDOMAIN")) != NULL)
        strcpy (pi->domain, env);
      /* Trust only if usernames are identical */
      if (!strcasecmp (pi->username, buf) && pi->domain[0] && pi->logsrv[0])
        debug_printf ("Domain: %s, Logon Server: %s", pi->domain, pi->logsrv);
      /* If that failed, try to get that info from NetBIOS */
      else if (!NetWkstaUserGetInfo (NULL, 1, (LPBYTE *)&wui))
        {
          wcstombs (pi->username, wui->wkui1_username,
                    (wcslen (wui->wkui1_username) + 1) * sizeof (WCHAR));
          wcstombs (pi->logsrv, wui->wkui1_logon_server,
                    (wcslen (wui->wkui1_logon_server) + 1) * sizeof (WCHAR));
          wcstombs (pi->domain, wui->wkui1_logon_domain,
                    (wcslen (wui->wkui1_logon_domain) + 1) * sizeof (WCHAR));
          /* Save values in environment */
          if (strcasecmp (pi->username, "SYSTEM")
              && pi->domain[0] && pi->logsrv[0])
            {
              LPUSER_INFO_3 ui = NULL;
              WCHAR wbuf[256];

              strcat (strcpy (buf, "\\\\"), pi->logsrv);
              setenv ("USERNAME", pi->username, 1);
              setenv ("LOGONSERVER", buf, 1);
              setenv ("USERDOMAIN", pi->domain, 1);
              /* HOMEDRIVE and HOMEPATH are wrong most of the time, too,
                 after changing user context! */
              mbstowcs (wbuf, buf, 256);
              if (!NetUserGetInfo (NULL, wui->wkui1_username, 3, (LPBYTE *)&ui)
                  || !NetUserGetInfo (wbuf,wui->wkui1_username,3,(LPBYTE *)&ui))
                {
                  wcstombs (buf, ui->usri3_home_dir,  256);
                  if (!buf[0])
                    {
                      wcstombs (buf, ui->usri3_home_dir_drive, 256);
                      if (buf[0])
                        strcat (buf, "\\");
                    }
                  if (!buf[0])
                    strcat (strcpy (buf, getenv ("SYSTEMDRIVE")), "\\");
                  setenv ("HOMEPATH", buf + 2, 1);
                  buf[2] = '\0';
                  setenv ("HOMEDRIVE", buf, 1);
                  NetApiBufferFree (ui);
                }
            }
          debug_printf ("Domain: %s, Logon Server: %s, Windows Username: %s",
                        pi->domain, pi->logsrv, pi->username);
          NetApiBufferFree (wui);
        }
      if (allow_ntsec)
        {
          /* Try to get the SID from localhost first. This can only
             be done if a domain is given because there's a chance that
             a local and a domain user may have the same name. */
          int ret = 0;

          if (pi->domain[0])
            {
              /* Concat DOMAIN\USERNAME for the next lookup */
              strcat (strcat (strcpy (buf, pi->domain), "\\"), pi->username);
              if (!(ret = lookup_name (buf, NULL, (PSID) pi->sidbuf)))
                debug_printf ("Couldn't retrieve SID locally!");
            }
          if (!ret && !(ret = lookup_name(pi->username, pi->logsrv,
                                          (PSID)pi->sidbuf)))
            debug_printf ("Couldn't retrieve SID from '%s'!", pi->logsrv);
          if (ret)
            {
              struct passwd *pw;
              char psidbuf[40];
              PSID psid = (PSID) psidbuf;

              pi->psid = (PSID) pi->sidbuf;
              if (strcasecmp (pi->username, "SYSTEM")
                  && pi->domain[0] && pi->logsrv[0])
                {
                  if (get_registry_hive_path (pi->psid, buf))
                    setenv ("USERPROFILE", buf, 1);
                }
              while ((pw = getpwent ()) != NULL)
                if (get_pw_sid (psid, pw) && EqualSid (pi->psid, psid))
                  {
                    strcpy (pi->username, pw->pw_name);
                    break;
                  }
              endpwent ();
            }
        }
    }
  debug_printf ("Cygwins Username: %s", pi->username);
  return pi->username;
}

void
uinfo_init ()
{
  char *username;
  struct passwd *p;

  /* If psid is non null, the process is forked or spawned from
     another cygwin process without changing the user context.
     So all user infos in myself as well as the environment are
     (perhaps) valid. */
  username = myself->psid ? myself->username : internal_getlogin (myself);
  if ((p = getpwnam (username)) != NULL)
    {
      /* calling getpwnam assures us that /etc/password has been
         read in, but we can't be sure about /etc/group */

      if (!group_in_memory_p)
        read_etc_group ();

      myself->uid = p->pw_uid;
      myself->gid = p->pw_gid;
    }
  else
    {
      myself->uid = DEFAULT_UID;
      myself->gid = DEFAULT_GID;
    }
  /* Set to non impersonated value. */
  myself->token = INVALID_HANDLE_VALUE;
  myself->impersonated = TRUE;
  myself->orig_uid = myself->real_uid = myself->uid;
  myself->orig_gid = myself->real_gid = myself->gid;
}

extern "C" char *
getlogin (void)
{
#ifdef _MT_SAFE
  char *this_username=_reent_winsup()->_username;
#else
  static NO_COPY char this_username[MAX_USER_NAME];
#endif

  return strcpy (this_username, myself->username);
}

extern "C" uid_t
getuid (void)
{
  return myself->real_uid;
}

extern "C" gid_t
getgid (void)
{
  return myself->real_gid;
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

extern "C" {
LoadDLLinitfunc (netapi32)
{
  HANDLE h;

  if ((h = LoadLibrary ("netapi32.dll")) != NULL)
    netapi32_handle = h;
  else if (! netapi32_handle)
    api_fatal ("could not load netapi32.dll. %d", GetLastError ());
  return 0;
}

static void dummy_autoload (void) __attribute__ ((unused));
static void
dummy_autoload (void)
{
LoadDLLinit (netapi32)
LoadDLLfunc (NetWkstaUserGetInfo, 12, netapi32)
LoadDLLfunc (NetUserGetInfo, 16, netapi32)
LoadDLLfunc (NetApiBufferFree, 4, netapi32)
}
}
