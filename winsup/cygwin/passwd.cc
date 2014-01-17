/* passwd.cc: getpwnam () and friends

   Copyright 1996, 1997, 1998, 2000, 2001, 2002, 2003, 2004, 2005, 2007, 2008,
   2009, 2010, 2011, 2012, 2014 Red Hat, Inc.

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
#include "shared_info.h"

/* Read /etc/passwd only once for better performance.  This is done
   on the first call that needs information from it. */

passwd *passwd_buf;
static pwdgrp pr (passwd_buf);

/* Parse /etc/passwd line into passwd structure. */
bool
pwdgrp::parse_passwd ()
{
  passwd &res = (*passwd_buf)[curr_lines];
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
}

/* Read in /etc/passwd and save contents in the password cache.
   This sets pr to loaded or emulated so functions in this file can
   tell that /etc/passwd has been read in or will be emulated. */
void
pwdgrp::read_passwd ()
{
  load (L"\\etc\\passwd");

  char strbuf[128] = "";
  bool searchentry = true;
  struct passwd *pw;
  /* must be static */
  static char NO_COPY pretty_ls[] = "????????:*:-1:-1:::";

  add_line (pretty_ls);
  cygsid tu = cygheap->user.sid ();
  tu.string (strbuf);
  if (!user_shared->cb || myself->uid == ILLEGAL_UID)
    searchentry = !internal_getpwsid (tu);
  if (searchentry
      && (!(pw = internal_getpwnam (cygheap->user.name ()))
	  || !user_shared->cb
	  || (myself->uid != ILLEGAL_UID
	      && myself->uid != pw->pw_uid
	      && !internal_getpwuid (myself->uid))))
    {
      static char linebuf[1024];	// must be static and
					// should not be NO_COPY
      snprintf (linebuf, sizeof (linebuf), "%s:*:%u:%u:,%s:%s:/bin/sh",
		cygheap->user.name (),
		(!user_shared->cb || myself->uid == ILLEGAL_UID)
		? UNKNOWN_UID : myself->uid,
		!user_shared->cb ? UNKNOWN_GID : myself->gid,
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
	{
	  pw = passwd_buf + i;
	  if (pw->pw_dir > pw->pw_gecos + 8)
	    for (ptr1 = endptr, ptr2 = pw->pw_dir - 2;
		 *ptr1 == *ptr2; ptr2--)
	      if (!*--ptr1)
		return pw;
	}
    }
  return NULL;
}

struct passwd *
internal_getpwuid (uid_t uid, bool check)
{
  pr.refresh (check);

  for (int i = 0; i < pr.curr_lines; i++)
    if (uid == passwd_buf[i].pw_uid)
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
getpwuid32 (uid_t uid)
{
  struct passwd *temppw = internal_getpwuid (uid, true);
  pthread_testcancel ();
  return temppw;
}

#ifdef __x86_64__
EXPORT_ALIAS (getpwuid32, getpwuid)
#else
extern "C" struct passwd *
getpwuid (__uid16_t uid)
{
  return getpwuid32 (uid16touid32 (uid));
}
#endif

extern "C" int
getpwuid_r32 (uid_t uid, struct passwd *pwd, char *buffer, size_t bufsize, struct passwd **result)
{
  *result = NULL;

  if (!pwd || !buffer)
    return ERANGE;

  struct passwd *temppw = internal_getpwuid (uid, true);
  pthread_testcancel ();
  if (!temppw)
    return 0;

  /* check needed buffer size. */
  size_t needsize = strlen (temppw->pw_name) + strlen (temppw->pw_passwd)
		    + strlen (temppw->pw_gecos) + strlen (temppw->pw_dir)
		    + strlen (temppw->pw_shell) + 5;
  if (needsize > bufsize)
    return ERANGE;

  /* make a copy of temppw */
  *result = pwd;
  pwd->pw_uid = temppw->pw_uid;
  pwd->pw_gid = temppw->pw_gid;
  buffer = stpcpy (pwd->pw_name = buffer, temppw->pw_name);
  buffer = stpcpy (pwd->pw_passwd = buffer + 1, temppw->pw_passwd);
  buffer = stpcpy (pwd->pw_gecos = buffer + 1, temppw->pw_gecos);
  buffer = stpcpy (pwd->pw_dir = buffer + 1, temppw->pw_dir);
  stpcpy (pwd->pw_shell = buffer + 1, temppw->pw_shell);
  pwd->pw_comment = NULL;
  return 0;
}

#ifdef __x86_64__
EXPORT_ALIAS (getpwuid_r32, getpwuid_r)
#else
extern "C" int
getpwuid_r (__uid16_t uid, struct passwd *pwd, char *buffer, size_t bufsize, struct passwd **result)
{
  return getpwuid_r32 (uid16touid32 (uid), pwd, buffer, bufsize, result);
}
#endif

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
  size_t needsize = strlen (temppw->pw_name) + strlen (temppw->pw_passwd)
		    + strlen (temppw->pw_gecos) + strlen (temppw->pw_dir)
		    + strlen (temppw->pw_shell) + 5;
  if (needsize > bufsize)
    return ERANGE;

  /* make a copy of temppw */
  *result = pwd;
  pwd->pw_uid = temppw->pw_uid;
  pwd->pw_gid = temppw->pw_gid;
  buffer = stpcpy (pwd->pw_name = buffer, temppw->pw_name);
  buffer = stpcpy (pwd->pw_passwd = buffer + 1, temppw->pw_passwd);
  buffer = stpcpy (pwd->pw_gecos = buffer + 1, temppw->pw_gecos);
  buffer = stpcpy (pwd->pw_dir = buffer + 1, temppw->pw_dir);
  stpcpy (pwd->pw_shell = buffer + 1, temppw->pw_shell);
  pwd->pw_comment = NULL;
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

#ifndef __x86_64__
extern "C" struct passwd *
getpwduid (__uid16_t)
{
  return NULL;
}
#endif

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
setpassent (int)
{
  return 0;
}

static void
_getpass_close_fd (void *arg)
{
  if (arg)
    fclose ((FILE *) arg);
}

extern "C" char *
getpass (const char * prompt)
{
  char *pass = _my_tls.locals.pass;
  struct termios ti, newti;
  bool tc_set = false;

  /* Try to use controlling tty in the first place.  Use stdin and stderr
     only as fallback. */
  FILE *in = stdin, *err = stderr;
  FILE *tty = fopen ("/dev/tty", "w+b");
  pthread_cleanup_push  (_getpass_close_fd, tty);
  if (tty)
    {
      /* Set close-on-exec for obvious reasons. */
      fcntl (fileno (tty), F_SETFD, fcntl (fileno (tty), F_GETFD) | FD_CLOEXEC);
      in = err = tty;
    }

  /* Make sure to notice if stdin is closed. */
  if (fileno (in) >= 0)
    {
      flockfile (in);
      /* Change tty attributes if possible. */
      if (!tcgetattr (fileno (in), &ti))
	{
	  newti = ti;
	  newti.c_lflag &= ~(ECHO | ISIG); /* No echo, no signal handling. */
	  if (!tcsetattr (fileno (in), TCSANOW, &newti))
	    tc_set = true;
	}
      fputs (prompt, err);
      fflush (err);
      fgets (pass, _PASSWORD_LEN, in);
      fprintf (err, "\n");
      if (tc_set)
	tcsetattr (fileno (in), TCSANOW, &ti);
      funlockfile (in);
      char *crlf = strpbrk (pass, "\r\n");
      if (crlf)
	*crlf = '\0';
    }
  pthread_cleanup_pop (1);
  return pass;
}
