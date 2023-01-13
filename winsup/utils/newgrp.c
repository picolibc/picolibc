/* newgrp.c

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <ctype.h>
#include <string.h>
#include <grp.h>
#include <pwd.h>

int
main (int argc, const char **argv)
{
  struct group *gr;
  gid_t gid;
  const char *cmd;
  const char **cmd_av;
  const char *fake_av[2];

  /* TODO: Implement '-' option */
  /* TODO: Add command description to documentation */

  if (argc < 2 || argv[1][0] == '-')
    {
      fprintf (stderr,
	       "Usage: %1$s group [command [args...]]\n"
	       "\n"
	       "%1$s changes the current primary group for a command.\n"
	       "The primary group must be member of the supplementary group\n"
	       "list of the user.\n"
	       "The command and its arguments are specified on the command\n"
	       "line.  Default is the user's standard shell.\n",
	       program_invocation_short_name);
      return 1;
    }
  if (isdigit ((int) argv[1][0]))
    {
      char *e = NULL;

      gid = strtol (argv[1], &e, 10);
      if (e && *e != '\0')
	{
	  fprintf (stderr, "%s: invalid gid `%s'\n",
		   program_invocation_short_name, argv[1]);
	  return 2;
	}
      gr = getgrgid (gid);
      if (!gr)
	{
	  fprintf (stderr, "%s: unknown group gid `%u'\n",
		   program_invocation_short_name, gid);
	  return 2;
	}
    }
  else
    {
      gr = getgrnam (argv[1]);
      if (!gr)
	{
	  fprintf (stderr, "%s: unknown group name `%s'\n",
		   program_invocation_short_name, argv[1]);
	  return 2;
	}
      gid = gr->gr_gid;
    }
  if (setgid (gid) != 0)
    {
      fprintf (stderr, "%s: can't switch primary group to `%s'\n",
	       program_invocation_short_name, argv[1]);
      return 2;
    }
  argc -= 2;
  argv += 2;
  if (argc < 1)
    {
      struct passwd *pw = getpwuid (getuid ());
      if (!pw)
	cmd = "/usr/bin/bash";
      else
	cmd = pw->pw_shell;
      fake_av[0] = cmd;
      fake_av[1] = NULL;
      cmd_av = fake_av;
    }
  else
    {
      cmd = argv[0];
      cmd_av = argv;
    }
  execvp (cmd, (char **) cmd_av);
  fprintf (stderr, "%s: failed to start `%s': %s\n",
	   program_invocation_short_name, cmd, strerror (errno));
  return 3;
}
