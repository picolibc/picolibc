/* pathconv.cc -- convert pathnames between Windows and Unix format
   Copyright 1998, 1999, 2000 Cygnus Solutions.
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
#include <io.h>
#include <sys/fcntl.h>
#include <sys/cygwin.h>

static char *prog_name;
static char *file_arg;

static struct option long_options[] =
{
  { (char *) "help", no_argument, NULL, 'h' },
  { (char *) "path", no_argument, NULL, 'p' },
  { (char *) "unix", no_argument, NULL, 'u' },
  { (char *) "file", required_argument, (int *) &file_arg, 'f'},
  { (char *) "version", no_argument, NULL, 'v' },
  { (char *) "windows", no_argument, NULL, 'w' },
  { 0, no_argument, 0, 0 }
};

static void
usage (FILE *stream, int status)
{
  fprintf (stream, "\
Usage: %s [-p|--path] (-u|--unix)|(-w|--windows) filename\n\
  -f|--file	read file for path information\n\
  -u|--unix     print Unix form of filename\n\
  -w|--windows  print Windows form of filename\n\
  -p|--path     filename argument is a path\n",
	   prog_name);
  exit (status);
}

static void
doit (char *filename, int path_flag, int unix_flag, int windows_flag)
{
  char *buf;
  size_t len;

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
}

int
main (int argc, char **argv)
{
  int path_flag, unix_flag, windows_flag;
  int c;
  char *filename;

  prog_name = strrchr (argv[0], '/');
  if (prog_name == NULL)
    prog_name = strrchr (argv[0], '\\');
  if (prog_name == NULL)
    prog_name = argv[0];

  path_flag = 0;
  unix_flag = 0;
  windows_flag = 0;
  while ((c = getopt_long (argc, argv, (char *) "hf:puvw", long_options, (int *) NULL))
	 != EOF)
    {
      switch (c)
	{
	case 'f':
	  file_arg = optarg;
	  break;

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

  if (! unix_flag && ! windows_flag)
    usage (stderr, 1);

  if (!file_arg)
    {
      if (optind != argc - 1)
	usage (stderr, 1);

      filename = argv[optind];
      doit (filename, path_flag, unix_flag, windows_flag);
    }
  else
    {
      FILE *fp;
      char buf[PATH_MAX * 2 + 1];

      if (argv[optind])
	usage (stderr, 1);

      if (strcmp (file_arg, "-") != 0)
	fp = fopen (file_arg, "rt");
      else
	{
	  fp = stdin;
	  setmode (0, O_TEXT);
	}
      if (fp == NULL)
	{
	  perror ("cygpath");
	  exit (1);
	}

      while (fgets (buf, sizeof (buf), fp) != NULL)
	{
	  char *p = strchr (buf, '\n');
	  if (p)
	    *p = '\0';
	  doit (buf, path_flag, unix_flag, windows_flag);
	}
    }

  exit (0);
}
