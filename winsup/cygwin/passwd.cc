/* passwd.cc: getpwnam () and friends

   Copyright 1996, 1997, 1998, 2001, 2002 Red Hat, Inc.

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

#include "winsup.h"
#include <stdlib.h>
#include <pwd.h>
#include <stdio.h>
#include <errno.h>
#include "cygerrno.h"
#include "security.h"
#include "fhandler.h"
#include "path.h"
#include "dtable.h"
#include "pinfo.h"
#include "cygheap.h"
#include <sys/termios.h>
#include "pwdgrp.h"

/* Read /etc/passwd only once for better performance.  This is done
   on the first call that needs information from it. */

static struct passwd *passwd_buf;	/* passwd contents in memory */
static int curr_lines = -1;
static int max_lines;

static pwdgrp_check passwd_state;


/* Position in the passwd cache */
#ifdef _MT_SAFE
#define pw_pos  _reent_winsup ()->_pw_pos
#else
static int pw_pos = 0;
#endif

/* Remove a : teminated string from the buffer, and increment the pointer */
static char *
grab_string (char **p)
{
  char *src = *p;
  char *res = src;

  while (*src && *src != ':' && *src != '\n')
    src++;

  if (*src == ':')
    {
      *src = 0;
      src++;
    }
  *p = src;
  return res;
}

/* same, for ints */
static int
grab_int (char **p)
{
  char *src = *p;
  int val = strtol (src, NULL, 10);
  while (*src && *src != ':' && *src != '\n')
    src++;
  if (*src == ':')
    src++;
  *p = src;
  return val;
}

/* Parse /etc/passwd line into passwd structure. */
static int
parse_pwd (struct passwd &res, char *buf)
{
  /* Allocate enough room for the passwd struct and all the strings
     in it in one go */
  size_t len = strlen (buf);
  if (buf[--len] == '\r')
    buf[len] = '\0';
  if (len < 6)
    return 0;

  res.pw_name = grab_string (&buf);
  res.pw_passwd = grab_string (&buf);
  res.pw_uid = grab_int (&buf);
  res.pw_gid = grab_int (&buf);
  res.pw_comment = 0;
  res.pw_gecos = grab_string (&buf);
  res.pw_dir =  grab_string (&buf);
  res.pw_shell = grab_string (&buf);
  return 1;
}

/* Add one line from /etc/passwd into the password cache */
static void
add_pwd_line (char *line)
{
    if (curr_lines >= max_lines)
      {
	max_lines += 10;
	passwd_buf = (struct passwd *) realloc (passwd_buf, max_lines * sizeof (struct passwd));
      }
    if (parse_pwd (passwd_buf[curr_lines], line))
      curr_lines++;
}

class passwd_lock
{
  bool armed;
  static NO_COPY pthread_mutex_t mutex;
 public:
  passwd_lock (bool doit)
  {
    if (doit)
      pthread_mutex_lock (&mutex);
    armed = doit;
  }
  ~passwd_lock ()
  {
    if (armed)
      pthread_mutex_unlock (&mutex);
  }
};

pthread_mutex_t NO_COPY passwd_lock::mutex = (pthread_mutex_t) PTHREAD_MUTEX_INITIALIZER;

/* Cygwin internal */
/* If this ever becomes non-reentrant, update all the getpw*_r functions */
static struct passwd *
search_for (__uid32_t uid, const char *name)
{
  struct passwd *res = 0;

  for (int i = 0; i < curr_lines; i++)
    {
      res = passwd_buf + i;
      /* on Windows NT user names are case-insensitive */
      if (name)
        {
	  if (strcasematch (name, res->pw_name))
	    return res;
	}
      else if (uid == (__uid32_t) res->pw_uid)
	return res;
    }
  return NULL;
}

/* Read in /etc/passwd and save contents in the password cache.
   This sets passwd_state to loaded or emulated so functions in this file can
   tell that /etc/passwd has been read in or will be emulated. */
static void
read_etc_passwd ()
{
  static pwdgrp_read pr;

  /* A mutex is ok for speed here - pthreads will use critical sections not
   * mutexes for non-shared mutexes in the future. Also, this function will
   * at most be called once from each thread, after that the passwd_state
   * test will succeed */
  passwd_lock here (cygwin_finished_initializing);

  /* if we got blocked by the mutex, then etc_passwd may have been processed */
  if (passwd_state != uninitialized)
    return;

  if (passwd_state != initializing)
    {
      passwd_state = initializing;
      curr_lines = 0;
      if (pr.open ("/etc/passwd"))
	{
	  char *line;
	  while ((line = pr.gets ()) != NULL)
	    add_pwd_line (line);

	  passwd_state.set_last_modified (pr.get_fhandle (), pr.get_fname ());
	  pr.close ();
	  debug_printf ("Read /etc/passwd, %d lines", curr_lines);
	}

      static char linebuf[1024];
      char strbuf[128] = "";
      BOOL searchentry = TRUE;
      __uid32_t default_uid = DEFAULT_UID;
      struct passwd *pw;

      if (wincap.has_security ())
	{
	  cygsid tu = cygheap->user.sid ();
	  tu.string (strbuf);
	  if (myself->uid == ILLEGAL_UID
	      && (searchentry = !internal_getpwsid (tu)))
	    default_uid = DEFAULT_UID_NT;
	}
      if (searchentry &&
	  (!(pw = search_for (0, cygheap->user.name ())) ||
	   (myself->uid != ILLEGAL_UID &&
	    myself->uid != (__uid32_t) pw->pw_uid  &&
	    !search_for (myself->uid, NULL))))
	{
	  snprintf (linebuf, sizeof (linebuf), "%s:*:%lu:%lu:,%s:%s:/bin/sh",
		    cygheap->user.name (),
		    myself->uid == ILLEGAL_UID ? default_uid : myself->uid,
		    myself->gid,
		    strbuf, getenv ("HOME") ?: "/");
	  debug_printf ("Completing /etc/passwd: %s", linebuf);
	  add_pwd_line (linebuf);
	}
      passwd_state = loaded;
    }
  return;
}

struct passwd *
internal_getpwsid (cygsid &sid)
{
  struct passwd *pw;
  char *ptr1, *ptr2, *endptr;
  char sid_string[128] = {0,','};

  if (curr_lines < 0 && passwd_state  <= initializing)
    read_etc_passwd ();

  if (sid.string (sid_string + 2))
    {
      endptr = strchr (sid_string + 2, 0) - 1;
      for (int i = 0; i < curr_lines; i++)
	if ((pw = passwd_buf + i)->pw_dir > pw->pw_gecos + 8)
	  for (ptr1 = endptr, ptr2 = pw->pw_dir - 2;
	       *ptr1 == *ptr2; ptr2--)
	    if (!*--ptr1)
	      return pw;
    }
  return NULL;
}

extern "C" struct passwd *
getpwuid32 (__uid32_t uid)
{
  if (passwd_state  <= initializing)
    read_etc_passwd ();

  pthread_testcancel ();

  return search_for (uid, 0);
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

  if (passwd_state  <= initializing)
    read_etc_passwd ();

  pthread_testcancel ();

  struct passwd *temppw = search_for (uid, 0);

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
  if (passwd_state  <= initializing)
    read_etc_passwd ();

  pthread_testcancel ();

  return search_for (0, name);
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

  if (passwd_state  <= initializing)
    read_etc_passwd ();

  pthread_testcancel ();

  struct passwd *temppw = search_for (0, nam);

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
  if (passwd_state  <= initializing)
    read_etc_passwd ();

  if (pw_pos < curr_lines)
    return passwd_buf + pw_pos++;

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
  pw_pos = 0;
}

extern "C" void
endpwent (void)
{
  pw_pos = 0;
}

extern "C" int
setpassent ()
{
  return 0;
}

#if 0 /* Unused */
/* Internal function. ONLY USE THIS INTERNALLY, NEVER `getpwent'!!! */
struct passwd *
internal_getpwent (int pos)
{
  if (passwd_state  <= initializing)
    read_etc_passwd ();

  if (pos < curr_lines)
    return passwd_buf + pos;
  return NULL;
}
#endif

extern "C" char *
getpass (const char * prompt)
{
#ifdef _MT_SAFE
  char *pass=_reent_winsup ()->_pass;
#else
  static char pass[_PASSWORD_LEN];
#endif
  struct termios ti, newti;

  if (passwd_state  <= initializing)
    read_etc_passwd ();

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
