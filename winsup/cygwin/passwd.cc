/* passwd.cc: getpwnam () and friends

   Copyright 1996, 1997, 1998 Cygnus Solutions.

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

#include <stdlib.h>
#include <pwd.h>
#include <stdio.h>
#include <errno.h>
#include "winsup.h"

/* Read /etc/passwd only once for better performance.  This is done
   on the first call that needs information from it. */

static struct passwd *passwd_buf = NULL;	/* passwd contents in memory */
static int curr_lines = 0;
static int max_lines = 0;

/* Set to loaded when /etc/passwd has been read in by read_etc_passwd ().
   Set to emulated if passwd is emulated. */
/* Functions in this file need to check the value of passwd_state
   and read in the password file if it isn't set. */
enum pwd_state {
  uninitialized = 0,
  emulated,
  loaded
};
static pwd_state passwd_state = uninitialized;

/* Position in the passwd cache */
#ifdef _MT_SAFE
#define pw_pos  _reent_winsup()->_pw_pos
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
void
parse_pwd (struct passwd &res, char *buf)
{
  /* Allocate enough room for the passwd struct and all the strings
     in it in one go */
  size_t len = strlen (buf);
  char *mybuf = (char *) malloc (len + 1);
  (void) memcpy (mybuf, buf, len + 1);
  if (mybuf[--len] == '\n')
    mybuf[len] = '\0';

  res.pw_name = strlwr(grab_string (&mybuf));
  res.pw_passwd = grab_string (&mybuf);
  res.pw_uid = grab_int (&mybuf);
  res.pw_gid = grab_int (&mybuf);
  res.pw_comment = 0;
  res.pw_gecos = grab_string (&mybuf);
  res.pw_dir =  grab_string (&mybuf);
  res.pw_shell = grab_string (&mybuf);
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
    parse_pwd (passwd_buf[curr_lines++], line);
}

/* Read in /etc/passwd and save contents in the password cache.
   This sets passwd_state to loaded or emulated so functions in this file can
   tell that /etc/passwd has been read in or will be emulated. */
void
read_etc_passwd ()
{
    extern int passwd_sem;
    char linebuf[1024];
    ++passwd_sem;
    FILE *f = fopen ("/etc/passwd", "rt");
    --passwd_sem;

    if (f)
      {
	while (fgets (linebuf, sizeof (linebuf), f) != NULL)
	  {
	    if (strlen (linebuf))
	      add_pwd_line (linebuf);
	  }

	fclose (f);
        passwd_state = loaded;
      }
    else
      {
        debug_printf ("Emulating /etc/passwd");
        char user_name [ MAX_USER_NAME ];
        DWORD user_name_len = MAX_USER_NAME;
        if (! GetUserNameA (user_name, &user_name_len))
          {
            strncpy (user_name, "Administrator", MAX_USER_NAME);
            debug_printf ("Failed to get current user name. %E");
          }
        snprintf (linebuf, sizeof (linebuf), "%s::%u:%u::%s:/bin/sh", user_name,
		  DEFAULT_UID, DEFAULT_GID, getenv ("HOME") ?: "/");
        add_pwd_line (linebuf);
        passwd_state = emulated;
      }
}

/* Cygwin internal */
static struct passwd *
search_for (uid_t uid, const char *name)
{
  struct passwd *res = 0;
  struct passwd *default_pw = 0;

  for (int i = 0; i < curr_lines; i++)
    {
      res = passwd_buf + i;
      if (res->pw_uid == DEFAULT_UID)
        default_pw = res;
      /* on Windows NT user names are case-insensitive */
      if (name)
	{
	  if (strcasematch (name, res->pw_name))
	    return res;
	}
      else if (uid == res->pw_uid)
	return res;
    }

  /* Return default passwd entry if passwd is emulated or it's a
     request for the current user. */
  if (passwd_state != loaded
      || (! name && uid == myself->uid)
      || (  name && strcasematch(name, myself->username)))
    return default_pw;

  return NULL;
}

extern "C" struct passwd *
getpwuid (uid_t uid)
{
  if (passwd_state == uninitialized)
    read_etc_passwd();

  return search_for (uid, 0);
}

extern "C" struct passwd *
getpwnam (const char *name)
{
  if (passwd_state == uninitialized)
    read_etc_passwd();

  return search_for (0, name);
}

extern "C" struct passwd *
getpwent (void)
{
  if (passwd_state == uninitialized)
    read_etc_passwd();

  if (pw_pos < curr_lines)
    return passwd_buf + pw_pos++;

  return NULL;
}

extern "C" struct passwd *
getpwduid (uid_t)
{
  if (passwd_state == uninitialized)
    read_etc_passwd();

  return NULL;
}

extern "C" void
setpwent (void)
{
  if (passwd_state == uninitialized)
    read_etc_passwd();

  pw_pos = 0;
}

extern "C" void
endpwent (void)
{
  if (passwd_state == uninitialized)
    read_etc_passwd();

  pw_pos = 0;
}

extern "C" int
setpassent ()
{
  if (passwd_state == uninitialized)
    read_etc_passwd();

  return 0;
}

extern "C" char *
getpass (const char * prompt)
{
#ifdef _MT_SAFE
  char *pass=_reent_winsup()->_pass;
#else
  static char pass[_PASSWORD_LEN];
#endif
  struct termios ti, newti;

  if (passwd_state == uninitialized)
    read_etc_passwd();

  if (dtable.not_open (0))
    {
      set_errno (EBADF);
      pass[0] = '\0';
    }
  else
    {
      fhandler_base *fhstdin = dtable[0];
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
