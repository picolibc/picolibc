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
#include <windows.h>
#include <io.h>
#include <sys/fcntl.h>
#include <sys/cygwin.h>
#include <ctype.h>

static char *prog_name;
static char *file_arg;
static char *close_arg;
static int path_flag, unix_flag, windows_flag, absolute_flag;

static struct option long_options[] =
{
  { (char *) "help", no_argument, NULL, 'h' },
  { (char *) "absolute", no_argument, NULL, 'a'},
  { (char *) "option", no_argument, NULL, 'o'},
  { (char *) "path", no_argument, NULL, 'p' },
  { (char *) "close", required_argument, (int *) &close_arg, 'c'},
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
  -a|--absolute		output absolute path\n\
  -c|--close handle	close handle (for use in captured process)\n\
  -f|--file file	read file for path information\n\
  -u|--unix     	print Unix form of filename\n\
  -w|--windows  	print Windows form of filename\n\
  -p|--path     	filename argument is a path\n",
	   prog_name);
  exit (status);
}

static void
doit (char *filename)
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
	(absolute_flag ? cygwin_conv_to_full_posix_path : cygwin_conv_to_posix_path) (filename, buf);
      else
	(absolute_flag ? cygwin_conv_to_full_win32_path : cygwin_conv_to_win32_path) (filename, buf);
    }

  puts (buf);
}

int
main (int argc, char **argv)
{
  int c;
  int options_from_file_flag;
  char *filename;

  prog_name = strrchr (argv[0], '/');
  if (prog_name == NULL)
    prog_name = strrchr (argv[0], '\\');
  if (prog_name == NULL)
    prog_name = argv[0];

  path_flag = 0;
  unix_flag = 0;
  windows_flag = 0;
  options_from_file_flag = 0;
  while ((c = getopt_long (argc, argv, (char *) "hac:f:opuvw", long_options, (int *) NULL))
	 != EOF)
    {
      switch (c)
	{
	case 'a':
	  absolute_flag = 1;
	  break;

	case 'c':
	  CloseHandle ((HANDLE) strtoul (optarg, NULL, 16));
	  break;

	case 'f':
	  file_arg = optarg;
	  break;

	case 'o':
	  options_from_file_flag = 1;
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

  if (options_from_file_flag && !file_arg)
    usage (stderr, 1);

  if (! unix_flag && ! windows_flag && !options_from_file_flag)
    usage (stderr, 1);

  if (!file_arg)
    {
      if (optind != argc - 1)
	usage (stderr, 1);

      filename = argv[optind];
      doit (filename);
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

      setbuf (stdout, NULL);
      while (fgets (buf, sizeof (buf), fp) != NULL)
	{
	  char *s = buf;
	  char *p = strchr (s, '\n');
	  if (p)
	    *p = '\0';
	  if (options_from_file_flag && *s == '-')
	    {
	      char c;
	      for (c = *++s; c && !isspace (c); c = *++s)
		switch (c)
		  {
		  case 'a':
		    absolute_flag = 1;
		    break;
		  case 'w':
		    unix_flag = 0;
		    windows_flag = 1;
		    break;
		  case 'u':
		    windows_flag = 0;
		    unix_flag = 1;
		    break;
		  case 'p':
		    path_flag = 1;
		  }
	      if (*s)
		do
		  s++;
		while (*s && isspace (*s));
	    }
	  if (*s)
	    doit (s);
	}
    }

  exit (0);
}
