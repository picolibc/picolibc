/* pathconv.cc -- convert pathnames between Windows and Unix format
   Copyright 1998 Cygnus Solutions.
   Written by Ian Lance Taylor <ian@cygnus.com>.

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <limits.h>
#include <getopt.h>
#include <sys/cygwin.h>

static char *prog_name;

static struct option long_options[] =
{
  { (char *) "help", no_argument, NULL, 'h' },
  { (char *) "path", no_argument, NULL, 'p' },
  { (char *) "unix", no_argument, NULL, 'u' },
  { (char *) "version", no_argument, NULL, 'v' },
  { (char *) "windows", no_argument, NULL, 'w' },
  { 0, no_argument, 0, 0 }
};

static void
usage (FILE *stream, int status)
{
  fprintf (stream, "\
Usage: %s [-p|--path] (-u|--unix)|(-w|--windows) filename\n\
  -u|--unix     print Unix form of filename\n\
  -w|--windows  print Windows form of filename\n\
  -p|--path     filename argument is a path\n",
	   prog_name);
  exit (status);
}

int
main (int argc, char **argv)
{
  int path_flag, unix_flag, windows_flag;
  int c;
  char *filename;
  size_t len;
  char *buf;

  prog_name = strrchr (argv[0], '/');
  if (prog_name == NULL)
    prog_name = strrchr (argv[0], '\\');
  if (prog_name == NULL)
    prog_name = argv[0];

  path_flag = 0;
  unix_flag = 0;
  windows_flag = 0;
  while ((c = getopt_long (argc, argv, (char *) "hpuvw", long_options, (int *) NULL))
	 != EOF)
    {
      switch (c)
	{
	case 'p':
	  path_flag = 1;
	  break;

	case 'u':
	  if (unix_flag || windows_flag)
	    usage (stderr, 1);
	  unix_flag = 1;
	  break;

	case 'w':
	  if (unix_flag || windows_flag)
	    usage (stderr, 1);
	  windows_flag = 1;
	  break;

	case 'h':
	  usage (stdout, 0);
	  break;

	case 'v':
	  printf ("Cygwin pathconv version 1.0\n");
	  printf ("Copyright 1998 Cygnus Solutions\n");
	  exit (0);

	default:
	  usage (stderr, 1);
	  break;
	}
    }

  if (optind != argc - 1)
    usage (stderr, 1);

  if (! unix_flag && ! windows_flag)
    usage (stderr, 1);

  filename = argv[optind];

  if (path_flag)
    {
      if (cygwin_posix_path_list_p (filename)
	  ? unix_flag
	  : windows_flag)
	{
	  /* The path is already in the right format.  */
	  puts (filename);
	  exit (0);
	}
    }

  if (! path_flag)
    len = strlen (filename) + 100;
  else
    {
      if (unix_flag)
	len = cygwin_win32_to_posix_path_list_buf_size (filename);
      else
	len = cygwin_posix_to_win32_path_list_buf_size (filename);
    }

  if (len < PATH_MAX)
    len = PATH_MAX;

  buf = (char *) malloc (len);
  if (buf == NULL)
    {
      fprintf (stderr, "%s: out of memory\n", prog_name);
      exit (1);
    }

  if (path_flag)
    {
      if (unix_flag)
	cygwin_win32_to_posix_path_list (filename, buf);
      else
	cygwin_posix_to_win32_path_list (filename, buf);
    }
  else
    {
      if (unix_flag)
	cygwin_conv_to_posix_path (filename, buf);
      else
	cygwin_conv_to_win32_path (filename, buf);
    }

  puts (buf);

  exit (0);
}
