/* setmetamode.c

   Written by Kazuhiro Fujieda <fujieda@jaist.ac.jp>

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <getopt.h>
#include <sys/ioctl.h>
#include <cygwin/kd.h>
#include <cygwin/version.h>

static void
usage (void)
{
  fprintf (stderr, "Usage: %s [metabit|escprefix]\n"
	   "\n"
	   "Get or set keyboard meta mode\n"
	   "\n"
	   "  Without argument, it shows the current meta key mode.\n"
	   "  metabit|meta|bit     The meta key sets the top bit of the character.\n"
	   "  escprefix|esc|prefix The meta key sends an escape prefix.\n"
	   "\n"
	   "Other options:\n"
	   "\n"
	   "  -h, --help           This text\n"
	   "  -V, --version        Print program version and exit\n\n",
	   program_invocation_short_name);
}

static void
error (void)
{
  fprintf (stderr,
	   "%s: The standard input isn't a console device.\n",
	   program_invocation_short_name);
}

void
print_version ()
{
  printf ("setmetamode (cygwin) %d.%d.%d\n"
	  "Get or set keyboard meta mode\n"
	  "Copyright (C) 2006 - %s Cygwin Authors\n"
	  "This is free software; see the source for copying conditions.  There is NO\n"
	  "warranty; not even for MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.\n",
	  CYGWIN_VERSION_DLL_MAJOR / 1000,
	  CYGWIN_VERSION_DLL_MAJOR % 1000,
	  CYGWIN_VERSION_DLL_MINOR,
	  strrchr (__DATE__, ' ') + 1);
}

struct option longopts[] = {
  {"help", no_argument, NULL, 'h'},
  {"version", no_argument, NULL, 'V'},
  {0, no_argument, NULL, 0}
};
const char *opts = "hV";

int
main (int ac, char *av[])
{
  int param;
  int opt;

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

  while ((opt = getopt_long (ac, av, opts, longopts, NULL)) != -1)
    switch (opt)
      {
      case 'h':
	usage ();
	return 0;
      case 'V':
	print_version ();
	return 0;
      default:
	fprintf (stderr, "Try `%s --help' for more information.\n",
		 program_invocation_short_name);
	return 1;
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
