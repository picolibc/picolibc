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
  DWORD username_len = MAX_USER_NAME;
  LPWKSTA_USER_INFO_1 ui = NULL;

  if (! pi)
    api_fatal ("pinfo pointer is NULL!\n");

  if (os_being_run == winNT)
    {
      int ret = NetWkstaUserGetInfo (NULL, 1, (LPBYTE *)&ui);
      if (! ret)
        {
          wcstombs (pi->domain,
                    ui->wkui1_logon_domain,
                    (wcslen (ui->wkui1_logon_domain) + 1) * sizeof (WCHAR));
          debug_printf ("Domain: %s", pi->domain);
          wcstombs (pi->logsrv,
                    ui->wkui1_logon_server,
                    (wcslen (ui->wkui1_logon_server) + 1) * sizeof (WCHAR));
          if (! *pi->logsrv)
            {
              LPWSTR logon_srv = NULL;

              if (!NetGetDCName (NULL,
                                 ui->wkui1_logon_domain,
                                 (LPBYTE *)&logon_srv))
                wcstombs (pi->logsrv,
                          logon_srv, // filter leading double backslashes
                          (wcslen (logon_srv) + 1) * sizeof (WCHAR));
              if (logon_srv)
                NetApiBufferFree (logon_srv);
              debug_printf ("AnyDC Server: %s", pi->logsrv);
            }
          else
            debug_printf ("Logon Server: %s", pi->logsrv);
          wcstombs (pi->username,
                    ui->wkui1_username,
                    (wcslen (ui->wkui1_username) + 1) * sizeof (WCHAR));
          debug_printf ("Windows Username: %s", pi->username);
          NetApiBufferFree (ui);
        }
      else
        {
          debug_printf ("%d = NetWkstaUserGetInfo ()\n", ret);
          if (! GetUserName (pi->username, &username_len))
            strcpy (pi->username, "unknown");
        }
        if (!lookup_name (pi->username, pi->logsrv, pi->psid))
          {
            debug_printf ("myself->psid = NULL");
            pi->psid = NULL;
          }
        else if (allow_ntsec)
          {
            extern BOOL get_pw_sid (PSID, struct passwd*);
            struct passwd *pw;
            char psidbuf[40];
            PSID psid = (PSID) psidbuf;

            while ((pw = getpwent ()) != NULL)
              if (get_pw_sid (psid, pw) && EqualSid (pi->psid, psid))
                {
                  strcpy (pi->username, pw->pw_name);
                  break;
                }
            endpwent ();
          }
    }
  else
    {
      debug_printf ("myself->psid = NULL");
      pi->psid = NULL;
      if (! GetUserName (pi->username, &username_len))
        strcpy (pi->username, "unknown");
    }
  debug_printf ("Cygwins Username: %s\n", pi->username);
  return pi->username;
}

void
uinfo_init ()
{
  struct passwd *p;

  myself->psid = (PSID) myself->sidbuf;
  if ((p = getpwnam (internal_getlogin (myself))) != NULL)
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
  return myself->uid;
}

extern "C" gid_t
getgid (void)
{
  return myself->gid;
}

extern "C" uid_t
geteuid (void)
{
  return getuid ();
}

extern "C" gid_t
getegid (void)
{
  return getgid ();
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

LoadDLLinitfunc (netapi32)
{
  HANDLE h;

  if ((h = LoadLibrary ("netapi32.dll")) != NULL)
    netapi32_handle = h;
  else if (! netapi32_handle)
    api_fatal ("could not load netapi32.dll. %d", GetLastError ());
  return 0;
}
LoadDLLinit (netapi32)
LoadDLLfunc (NetWkstaUserGetInfo, 12, netapi32)
LoadDLLfunc (NetGetDCName, 12, netapi32)
LoadDLLfunc (NetApiBufferFree, 4, netapi32)

