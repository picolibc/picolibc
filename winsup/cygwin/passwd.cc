/* passwd.cc: getpwnam () and friends

   Copyright 1996, 1997, 1998, 2001, 2002, 2003, 2007 Red Hat, Inc.

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

#include "winsup.h"
#include <stdlib.h>
#include <stdio.h>
#include "cygerrno.h"
#include "security.h"
#include "path.h"
#include "fhandler.h"
#include "dtable.h"
#include "pinfo.h"
#include "cygheap.h"
#include "pwdgrp.h"

/* Read /etc/passwd only once for better performance.  This is done
   on the first call that needs information from it. */

passwd *passwd_buf;
static pwdgrp pr (passwd_buf);

/* Parse /etc/passwd line into passwd structure. */
bool
pwdgrp::parse_passwd ()
{
# define res (*passwd_buf)[curr_lines]
  res.pw_name = next_str (':');
  res.pw_passwd = next_str (':');
  if (!next_num (res.pw_uid))
    return false;
  if (!next_num (res.pw_gid))
    return false;
  res.pw_comment = NULL;
  res.pw_gecos = next_str (':');
  res.pw_dir =  next_str (':');
  res.pw_shell = next_str (':');
  return true;
# undef res
}

/* Read in /etc/passwd and save contents in the password cache.
   This sets pr to loaded or emulated so functions in this file can
   tell that /etc/passwd has been read in or will be emulated. */
void
pwdgrp::read_passwd ()
{
  load ("/etc/passwd");

  char strbuf[128] = "";
  bool searchentry = true;
  struct passwd *pw;
  /* must be static */
  static char NO_COPY pretty_ls[] = "????????:*:-1:-1:";

  add_line (pretty_ls);
  cygsid tu = cygheap->user.sid ();
  tu.string (strbuf);
  if (myself->uid == ILLEGAL_UID)
    searchentry = !internal_getpwsid (tu);
  if (searchentry &&
      (!(pw = internal_getpwnam (cygheap->user.name ())) ||
       (myself->uid != ILLEGAL_UID &&
	myself->uid != (__uid32_t) pw->pw_uid  &&
	!internal_getpwuid (myself->uid))))
    {
      static char linebuf[1024];	// must be static and
					// should not be NO_COPY
      cygheap->user.ontherange (CH_HOME, NULL);
      snprintf (linebuf, sizeof (linebuf), "%s:*:%lu:%lu:,%s:%s:/bin/sh",
		cygheap->user.name (),
		myself->uid == ILLEGAL_UID ? UNKNOWN_UID : myself->uid,
		myself->gid,
		strbuf, getenv ("HOME") ?: "");
      debug_printf ("Completing /etc/passwd: %s", linebuf);
      add_line (linebuf);
    }
}

struct passwd *
internal_getpwsid (cygpsid &sid)
{
  struct passwd *pw;
  char *ptr1, *ptr2, *endptr;
  char sid_string[128] = {0,','};

  pr.refresh (false);

  if (sid.string (sid_string + 2))
    {
      endptr = strchr (sid_string + 2, 0) - 1;
      for (int i = 0; i < pr.curr_lines; i++)
	if ((pw = passwd_buf + i)->pw_dir > pw->pw_gecos + 8)
	  for (ptr1 = endptr, ptr2 = pw->pw_dir - 2;
	       *ptr1 == *ptr2; ptr2--)
	    if (!*--ptr1)
	      return pw;
    }
  return NULL;
}

struct passwd *
internal_getpwuid (__uid32_t uid, bool check)
{
  pr.refresh (check);

  for (int i = 0; i < pr.curr_lines; i++)
    if (uid == (__uid32_t) passwd_buf[i].pw_uid)
      return passwd_buf + i;
  return NULL;
}

struct passwd *
internal_getpwnam (const char *name, bool check)
{
  pr.refresh (check);

  for (int i = 0; i < pr.curr_lines; i++)
    /* on Windows NT user names are case-insensitive */
    if (strcasematch (name, passwd_buf[i].pw_name))
      return passwd_buf + i;
  return NULL;
}


extern "C" struct passwd *
getpwuid32 (__uid32_t uid)
{
  struct passwd *temppw = internal_getpwuid (uid, true);
  pthread_testcancel ();
  return temppw;
}

extern "C" struct passwd *
getpwuid (__uid16_t uid)
{
  return getpwuid32 (uid16touid32 (uid));
}

extern "C" int
getpwuid_r32 (__uid32_t uid, struct passwd *pwd, char *buffer, size_t bufsize, struct passwd **result)
{
  *result = NULL;

  if (!pwd || !buffer)
    return ERANGE;

  struct passwd *temppw = internal_getpwuid (uid, true);
  pthread_testcancel ();
  if (!temppw)
    return 0;

  /* check needed buffer size. */
  size_t needsize = strlen (temppw->pw_name) + strlen (temppw->pw_dir) +
		    strlen (temppw->pw_shell) + strlen (temppw->pw_gecos) +
		    strlen (temppw->pw_passwd) + 5;
  if (needsize > bufsize)
    return ERANGE;

  /* make a copy of temppw */
  *result = pwd;
  pwd->pw_uid = temppw->pw_uid;
  pwd->pw_gid = temppw->pw_gid;
  pwd->pw_name = buffer;
  pwd->pw_dir = pwd->pw_name + strlen (temppw->pw_name) + 1;
  pwd->pw_shell = pwd->pw_dir + strlen (temppw->pw_dir) + 1;
  pwd->pw_gecos = pwd->pw_shell + strlen (temppw->pw_shell) + 1;
  pwd->pw_comment = NULL;
  pwd->pw_passwd = pwd->pw_gecos + strlen (temppw->pw_gecos) + 1;
  strcpy (pwd->pw_name, temppw->pw_name);
  strcpy (pwd->pw_dir, temppw->pw_dir);
  strcpy (pwd->pw_shell, temppw->pw_shell);
  strcpy (pwd->pw_gecos, temppw->pw_gecos);
  strcpy (pwd->pw_passwd, temppw->pw_passwd);
  return 0;
}

extern "C" int
getpwuid_r (__uid16_t uid, struct passwd *pwd, char *buffer, size_t bufsize, struct passwd **result)
{
  return getpwuid_r32 (uid16touid32 (uid), pwd, buffer, bufsize, result);
}

extern "C" struct passwd *
getpwnam (const char *name)
{
  struct passwd *temppw = internal_getpwnam (name, true);
  pthread_testcancel ();
  return temppw;
}


/* the max size buffer we can expect to
 * use is returned via sysconf with _SC_GETPW_R_SIZE_MAX.
 * This may need updating! - Rob Collins April 2001.
 */
extern "C" int
getpwnam_r (const char *nam, struct passwd *pwd, char *buffer, size_t bufsize, struct passwd **result)
{
  *result = NULL;

  if (!pwd || !buffer || !nam)
    return ERANGE;

  struct passwd *temppw = internal_getpwnam (nam, true);
  pthread_testcancel ();

  if (!temppw)
    return 0;

  /* check needed buffer size. */
  size_t needsize = strlen (temppw->pw_name) + strlen (temppw->pw_dir) +
		    strlen (temppw->pw_shell) + strlen (temppw->pw_gecos) +
		    strlen (temppw->pw_passwd) + 5;
  if (needsize > bufsize)
    return ERANGE;

  /* make a copy of temppw */
  *result = pwd;
  pwd->pw_uid = temppw->pw_uid;
  pwd->pw_gid = temppw->pw_gid;
  pwd->pw_name = buffer;
  pwd->pw_dir = pwd->pw_name + strlen (temppw->pw_name) + 1;
  pwd->pw_shell = pwd->pw_dir + strlen (temppw->pw_dir) + 1;
  pwd->pw_gecos = pwd->pw_shell + strlen (temppw->pw_shell) + 1;
  pwd->pw_comment = NULL;
  pwd->pw_passwd = pwd->pw_gecos + strlen (temppw->pw_gecos) + 1;
  strcpy (pwd->pw_name, temppw->pw_name);
  strcpy (pwd->pw_dir, temppw->pw_dir);
  strcpy (pwd->pw_shell, temppw->pw_shell);
  strcpy (pwd->pw_gecos, temppw->pw_gecos);
  strcpy (pwd->pw_passwd, temppw->pw_passwd);
  return 0;
}

extern "C" struct passwd *
getpwent (void)
{
  if (_my_tls.locals.pw_pos == 0)
    pr.refresh (true);
  if (_my_tls.locals.pw_pos < pr.curr_lines)
    return passwd_buf + _my_tls.locals.pw_pos++;

  return NULL;
}

extern "C" struct passwd *
getpwduid (__uid16_t)
{
  return NULL;
}

extern "C" void
setpwent (void)
{
  _my_tls.locals.pw_pos = 0;
}

extern "C" void
endpwent (void)
{
  _my_tls.locals.pw_pos = 0;
}

extern "C" int
setpassent ()
{
  return 0;
}

extern "C" char *
getpass (const char * prompt)
{
  char *pass = _my_tls.locals.pass;
  struct termios ti, newti;

  cygheap_fdget fhstdin (0);

  if (fhstdin < 0)
    pass[0] = '\0';
  else
    {
      fhstdin->tcgetattr (&ti);
      newti = ti;
      newti.c_lflag &= ~ECHO;
      fhstdin->tcsetattr (TCSANOW, &newti);
      fputs (prompt, stderr);
      fgets (pass, _PASSWORD_LEN, stdin);
      fprintf (stderr, "\n");
      for (int i=0; pass[i]; i++)
	if (pass[i] == '\r' || pass[i] == '\n')
	  pass[i] = '\0';
      fhstdin->tcsetattr (TCSANOW, &ti);
    }
  return pass;
}
