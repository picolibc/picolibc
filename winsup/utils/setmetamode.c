/* setmetamode.c

   Copyright 2006 Red Hat Inc.

   Written by Kazuhiro Fujieda <fujieda@jaist.ac.jp>

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

#include <stdio.h>
#include <string.h>
#include <sys/ioctl.h>
#include <cygwin/kd.h>

static const char version[] = "$Revision$";
static char *prog_name;

static void
usage (void)
{
  fprintf (stderr, "Usage: %s [metabit|escprefix]\n"
	   "  Without argument, it shows the current meta key mode.\n"
	   "  metabit|meta|bit     The meta key sets the top bit of the character.\n"
	   "  escprefix|esc|prefix The meta key sends an escape prefix.\n",
	   prog_name);
}

static void
error (void)
{
  fprintf (stderr,
	   "%s: The standard input isn't a console device.\n",
	   prog_name);
}

int
main (int ac, char *av[])
{
  int param;

  prog_name = strrchr (av[0], '/');
  if (!prog_name)
    prog_name = strrchr (av[0], '\\');
  if (!prog_name)
    prog_name = av[0];
  else
    prog_name++;

  if (ac < 2)
    {
      if (ioctl (0, KDGKBMETA, &param) < 0)
	{
	  error ();
	  return 1;
	}
      if (param == 0x03)
	puts ("metabit");
      else
	puts ("escprefix");
      return 0;
    }
  if (!strcmp ("meta", av[1]) || !strcmp ("bit", av[1])
      || !strcmp ("metabit", av[1]))
    param = 0x03;
  else if (!strcmp ("esc", av[1]) || !strcmp ("prefix", av[1])
	   || !strcmp ("escprefix", av[1]))
    param = 0x04;
  else
    {
      usage ();
      return 1;
    }
  if (ioctl (0, KDSKBMETA, param) < 0)
    {
      error ();
      return 1;
    }
  return 0;
}
