/* cygwin.cc: general system debugging tool.

   Copyright 1996, 1998 Cygnus Solutions.

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

/* This program is intended to be a general tool for debugging cygwin.
   Possibilities include
   - dumping various internal data structures
   - poking various values into system tables
   - turning on strace'ing for arbitrary tasks
   */

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <windows.h>
#include "winsup.h"

static char *prog_name;

static void
usage (FILE *stream, int status)
{
  fprintf (stream, "\
Usage: %s \\\n\
       [-s|--strace pid mask]\\\n\
       [-H|--help] [-V|--version]\n\
",
	   prog_name);
  exit (status);
}

static struct option long_options[] =
{
  { "version", no_argument, NULL, 'V' },
  { "help", no_argument, NULL, 'H' },
  { "strace", required_argument, NULL, 's' },
  { 0, no_argument, 0, 0 }
};

struct strace_args
{
  int pid;
  int mask;
  char *fn;
};

/* Turn on strace'ing for the indicated pid.  */

static void
set_strace (strace_args *args)
{
  shared_info *s = cygwin_getshared ();

  pinfo *p = s->p[args->pid];

  if (!p)
    {
      fprintf (stderr, "%s: process %d not found\n", prog_name, args->pid);
      exit (1);
    }

  p->strace_mask = args->mask;
}

int
main (int argc, char *argv[])
{
  int c;
  int seen_flag_p = 0;
  int show_version_p = 0;
  int set_strace_p = 0;
  strace_args strace_args;

  prog_name = strrchr (argv[0], '/');
  if (prog_name == NULL)
    prog_name = strrchr (argv[0], '\\');
  if (prog_name == NULL)
    prog_name = argv[0];

  while ((c = getopt_long (argc, argv, "HVs:", long_options, (int *) 0))
	 != EOF)
    {
      seen_flag_p = 1;

      switch (c)
	{
	case 'H':
	  usage (stdout, 0);
	  break;
	case 'V':
	  show_version_p = 1;
	  break;
	case 's':
	  if (optind + 1 > argc)
	    usage (stderr, 1);
	  strace_args.pid = atoi (optarg);
	  if (optind < argc)
	    strace_args.mask = atoi (argv[optind++]);
	  if (optind < argc)
	    strace_args.fn = argv[optind++];
	  set_strace_p = 1;
	  break;
	default:
	  usage (stderr, 1);
	  break;
	}
    }

  if (show_version_p)
    printf ("CYGWIN version ???\n");

  if (!seen_flag_p || optind != argc)
    usage (stderr, 1);

  if (set_strace_p)
    set_strace (&strace_args);

  return 0;
}
