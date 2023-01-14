/* newgrp.c

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

#define _GNU_SOURCE 1
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <errno.h>
#include <ctype.h>
#include <string.h>
#include <wchar.h>
#include <locale.h>
#include <grp.h>
#include <pwd.h>

#include <sys/cygwin.h>
#include <w32api/windows.h>
#include <w32api/userenv.h>

#define PATH_PREFIX	"PATH=/usr/bin:"

char *
create_env_var (const char *name, const char *val)
{
  char *var, *cp;

  var = (char *) calloc (strlen (name) + strlen (val) + 2, sizeof (char *));
  cp = stpcpy (var, name);
  *cp++ = '=';
  stpcpy (cp, val);
  return var;
}

char **
create_child_env (struct passwd *pw)
{
  char **posix_env, *cp;
  wchar_t *win_env, *wep;
  size_t max_cnt = 0;
  size_t idx = 0;
  HANDLE token;

  /* Fecth Windows default environment of current user */
  if (!OpenProcessToken (GetCurrentProcess (),
			 TOKEN_QUERY | TOKEN_DUPLICATE, &token))
    {
      fprintf (stderr, "%s: creating environment failed with error %u "
		       "(OpenProcessToken)\n",
	       program_invocation_short_name, GetLastError ());
      return NULL;
    }
  if (!CreateEnvironmentBlock ((PVOID *) &win_env, token, FALSE))
    {
      fprintf (stderr, "%s: creating environment failed with error %u "
		       "(CreateEnvironmentBlock)\n",
	       program_invocation_short_name, GetLastError ());
      CloseHandle (token);
      return NULL;
    }
  CloseHandle (token);
  /* Convert to Posix env */
  for (wep = win_env; *wep; wep = wcschr (wep, '\0') + 1)
    ++max_cnt;
  posix_env = (char **) calloc (max_cnt + 6, sizeof (char *));
  if (!posix_env)
    {
      fprintf (stderr, "%s: allocating environment failed: %s\n",
	       program_invocation_short_name, strerror (errno));
      return NULL;
    }
  for (wep = win_env; *wep; ++idx, wep = wcschr (wep, '\0') + 1)
    {
      /* For $PATH we must prepend /usr/bin to the converted POSIX path list */
      if (!wcsncasecmp (wep, L"PATH=", 5))
	{
	  size_t len = cygwin_conv_path_list (CCP_WIN_W_TO_POSIX,
					      wep + 5, NULL, 0);
	  posix_env[idx] = (char *) calloc (sizeof (PATH_PREFIX) + len,
					    sizeof (char *));
	  if (!posix_env[idx])
	    {
	      fprintf (stderr, "%s: allocating environment failed: %s\n",
		       program_invocation_short_name, strerror (errno));
	      return NULL;
	    }
	  cp = stpcpy (posix_env[idx], PATH_PREFIX);
	  cygwin_conv_path_list (CCP_WIN_W_TO_POSIX, wep + 5, cp, len);
	}
      else
	{
	  size_t len = wcstombs (NULL, wep, 0);

	  if (len == (size_t) -1)
	    {
	      fprintf (stderr,
		       "%s: invalid char in environment variable: %ls\n",
		       program_invocation_short_name, wep);
	      return NULL;
	    }
	  posix_env[idx] = (char *) calloc (len + 1, sizeof (char *));
	  if (!posix_env[idx])
	    {
	      fprintf (stderr, "%s: allocating environment failed: %s\n",
		       program_invocation_short_name, strerror (errno));
	      return NULL;
	    }
	  wcstombs (posix_env[idx], wep, len + 1);
	}
    }
  DestroyEnvironmentBlock (win_env);
  /* Add USER, LOGNAME, HOME, LANG, just like sshd */
  posix_env[idx++] = create_env_var ("USER", pw->pw_name);
  posix_env[idx++] = create_env_var ("LOGNAME", pw->pw_name);
  posix_env[idx++] = create_env_var ("HOME", pw->pw_dir);
  cp = getenv("LANG");
  if (cp)
    posix_env[idx] = create_env_var ("LANG", cp);
  cp = getenv("TERM");
  if (cp)
    posix_env[idx] = create_env_var ("TERM", cp);
  return posix_env;
}

int
main (int argc, const char **argv)
{
  const char *cmd, **cmd_av, *fake_av[2];
  struct passwd *pw;
  struct group *gr;
  char **child_env;
  bool new_child_env = false;
  gid_t gid;

  setlocale (LC_ALL, "");

  if (argc < 2 || (argv[1][0] == '-' && argv[1][1]))
    {
      fprintf (stderr, "Usage: %s [-] [group] [command [args...]]\n",
	       program_invocation_short_name);
      return 1;
    }

  /* Check if we have to regenerate a stock environment */
  if (argv[1][0] == '-')
    {
      new_child_env = true;
      --argc;
      ++argv;
    }

  pw = getpwuid (getuid ());

  /* Fetch group */
  if (argv[1] == NULL)
    {
      gid = pw->pw_gid;
    }
  else
    {
      gr = getgrnam (argv[1]);
      if (!gr)
	{
	  fprintf (stderr, "%s: group '%s' does not exist\n",
		   program_invocation_short_name, argv[1]);
	  return 2;
	}
      gid = gr->gr_gid;
      --argc;
      ++argv;
    }

  /* Set primary group */
  if (setgid (gid) != 0)
    {
      fprintf (stderr, "%s: can't switch primary group to '%s'\n",
	       program_invocation_short_name, argv[1]);
      return 2;
    }

  /* Maybe generate stock child environment */
  if (!new_child_env)
    child_env = environ;
  else
    {
      child_env = create_child_env (pw);
      if (!child_env)
	return 3;
      chdir (pw->pw_dir);
    }

  /* Set argc/argv for execvpe */
  --argc;
  ++argv;
  if (argc < 1)
    {
      if (!pw)
	cmd = "/usr/bin/bash";
      else
	cmd = pw->pw_shell;
      fake_av[0] = new_child_env ? "-" : cmd;
      fake_av[1] = NULL;
      cmd_av = fake_av;
    }
  else
    {
      cmd = argv[0];
      cmd_av = argv;
    }

  /* Exec child process */
  execvpe (cmd, (char **) cmd_av, child_env);

  /* Oops */
  fprintf (stderr, "%s: failed to start '%s': %s\n",
	   program_invocation_short_name, cmd, strerror (errno));
  return 4;
}
