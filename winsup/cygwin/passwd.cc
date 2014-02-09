/* passwd.cc: getpwnam () and friends

   Copyright 1996, 1997, 1998, 2000, 2001, 2002, 2003, 2004, 2005, 2007, 2008,
   2009, 2010, 2011, 2012, 2013, 2014 Red Hat, Inc.

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
#include "shared_info.h"

/* Parse /etc/passwd line into passwd structure. */
bool
pwdgrp::parse_passwd ()
{
  pg_pwd &res = passwd ()[curr_lines];
  res.p.pw_name = next_str (':');
  res.p.pw_passwd = next_str (':');
  if (!next_num (res.p.pw_uid))
    return false;
  if (!next_num (res.p.pw_gid))
    return false;
  res.p.pw_comment = NULL;
  res.p.pw_gecos = next_str (':');
  res.p.pw_dir =  next_str (':');
  res.p.pw_shell = next_str (':');
  res.sid.getfrompw (&res.p);
  return true;
}

void
pwdgrp::init_pwd ()
{
  pwdgrp_buf_elem_size = sizeof (pg_pwd);
  parse = &pwdgrp::parse_passwd;
}

pwdgrp *
pwdgrp::prep_tls_pwbuf ()
{
  if (!_my_tls.locals.pwbuf)
    {
      _my_tls.locals.pwbuf = ccalloc_abort (HEAP_BUF, 1,
					    sizeof (pwdgrp) + sizeof (pg_pwd));
      pwdgrp *pw = (pwdgrp *) _my_tls.locals.pwbuf;
      pw->init_pwd ();
      pw->pwdgrp_buf = (void *) (pw + 1);
      pw->max_lines = 1;
    }
  pwdgrp *pw = (pwdgrp *) _my_tls.locals.pwbuf;
  if (pw->curr_lines)
    {
      cfree (pw->passwd ()[0].p.pw_name);
      pw->curr_lines = 0;
    }
  return pw;
}

struct passwd *
pwdgrp::find_user (cygpsid &sid)
{
  for (ULONG i = 0; i < curr_lines; i++)
    if (sid == passwd ()[i].sid)
      return &passwd ()[i].p;
  return NULL;
}

struct passwd *
pwdgrp::find_user (const char *name)
{
  for (ULONG i = 0; i < curr_lines; i++)
    /* on Windows NT user names are case-insensitive */
    if (strcasematch (name, passwd ()[i].p.pw_name))
      return &passwd ()[i].p;
  return NULL;
}

struct passwd *
pwdgrp::find_user (uid_t uid)
{
  for (ULONG i = 0; i < curr_lines; i++)
    if (uid == passwd ()[i].p.pw_uid)
      return &passwd ()[i].p;
  return NULL;
}

struct passwd *
internal_getpwsid (cygpsid &sid)
{
  struct passwd *ret;

  cygheap->pg.nss_init ();
  if (cygheap->pg.nss_pwd_files ())
    {
      cygheap->pg.pwd_cache.file.check_file (false);
      if ((ret = cygheap->pg.pwd_cache.file.find_user (sid)))
	return ret;
      if ((ret = cygheap->pg.pwd_cache.file.add_user_from_file (sid)))
	return ret;
    }
  if (cygheap->pg.nss_pwd_db ())
    {
      if ((ret = cygheap->pg.pwd_cache.win.find_user (sid)))
	return ret;
      return cygheap->pg.pwd_cache.win.add_user_from_windows (sid);
    }
  return NULL;
}

struct passwd *
internal_getpwnam (const char *name)
{
  struct passwd *ret;

  cygheap->pg.nss_init ();
  if (cygheap->pg.nss_pwd_files ())
    {
      cygheap->pg.pwd_cache.file.check_file (false);
      if ((ret = cygheap->pg.pwd_cache.file.find_user (name)))
	return ret;
      if ((ret = cygheap->pg.pwd_cache.file.add_user_from_file (name)))
	return ret;
    }
  if (cygheap->pg.nss_pwd_db ())
    {
      if ((ret = cygheap->pg.pwd_cache.win.find_user (name)))
	return ret;
      return cygheap->pg.pwd_cache.win.add_user_from_windows (name);
    }
  return NULL;
}

struct passwd *
internal_getpwuid (uid_t uid)
{
  struct passwd *ret;

  cygheap->pg.nss_init ();
  if (cygheap->pg.nss_pwd_files ())
    {
      cygheap->pg.pwd_cache.file.check_file (false);
      if ((ret = cygheap->pg.pwd_cache.file.find_user (uid)))
	return ret;
      if ((ret = cygheap->pg.pwd_cache.file.add_user_from_file (uid)))
	return ret;
    }
  if (cygheap->pg.nss_pwd_db ())
    {
      if ((ret = cygheap->pg.pwd_cache.win.find_user (uid)))
	return ret;
      return cygheap->pg.pwd_cache.win.add_user_from_windows (uid);
    }
  else if (uid == ILLEGAL_UID)
    return cygheap->pg.pwd_cache.win.add_user_from_windows (uid);
  return NULL;
}

extern "C" struct passwd *
getpwuid32 (uid_t uid)
{
  struct passwd *temppw = internal_getpwuid (uid);
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

  struct passwd *temppw = internal_getpwuid (uid);
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
  struct passwd *temppw = internal_getpwnam (name);
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

  struct passwd *temppw = internal_getpwnam (nam);
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
  pwdgrp &prf = cygheap->pg.pwd_cache.file;
  if (cygheap->pg.nss_pwd_files ())
    {
      cygheap->pg.pwd_cache.file.check_file (false);
      if (_my_tls.locals.pw_pos < prf.cached_users ())
	return &prf.passwd ()[_my_tls.locals.pw_pos++].p;
    }
  if ((cygheap->pg.nss_pwd_db ()) && cygheap->pg.nss_db_caching ())
    {
      pwdgrp &prw = cygheap->pg.pwd_cache.win;
      if (_my_tls.locals.pw_pos - prf.cached_users () < prw.cached_users ())
	return &prw.passwd ()[_my_tls.locals.pw_pos++ - prf.cached_users ()].p;
    }
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
