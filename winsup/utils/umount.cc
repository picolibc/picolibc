/* umount.cc

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

#include <stdio.h>
#include <string.h>
#include <sys/mount.h>
#include <mntent.h>
#include <stdlib.h>
#include <errno.h>
#include <getopt.h>
#include <cygwin/version.h>

static void remove_all_user_mounts ();

static const char *progname;

struct option longopts[] =
{
  {"help", no_argument, NULL, 'h' },
  {"remove-user-mounts", no_argument, NULL, 'U'},
  {"version", no_argument, NULL, 'V'},
  {NULL, 0, NULL, 0}
};

char opts[] = "hUV";

static void
usage (FILE *where = stderr)
{
  fprintf (where, "\
Usage: %s [OPTION] [<posixpath>]\n\
\n\
Unmount filesystems\n\
\n\
  -h, --help                    output usage information and exit\n\
  -U, --remove-user-mounts      remove all user mounts\n\
  -V, --version                 output version information and exit\n\
\n", progname);
  exit (where == stderr ? 1 : 0);
}

static void
error (const char *path)
{
  fprintf (stderr, "%s: %s: %s\n", progname, path, strerror (errno));
  exit (1);
}

static void
print_version ()
{
  printf ("umount (cygwin) %d.%d.%d\n"
	  "Unmount filesystem utility\n"
	  "Copyright (C) 1996 - %s Cygwin Authors\n"
	  "This is free software; see the source for copying conditions.  There is NO\n"
	  "warranty; not even for MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.\n",
	  CYGWIN_VERSION_DLL_MAJOR / 1000,
	  CYGWIN_VERSION_DLL_MAJOR % 1000,
	  CYGWIN_VERSION_DLL_MINOR,
	  strrchr (__DATE__, ' ') + 1);
}

int
main (int argc, char **argv)
{
  int i;
  int flags = 0;
  int default_flag = MOUNT_SYSTEM;
  enum do_what
  {
    nada,
    saw_remove_all_user_mounts
  } do_what = nada;

  progname = program_invocation_short_name;

  if (argc == 1)
    usage ();

  while ((i = getopt_long (argc, argv, opts, longopts, NULL)) != EOF)
    switch (i)
      {
      case 'h':
	usage (stdout);
      case 'U':
	if (do_what != nada)
	  usage ();
	do_what = saw_remove_all_user_mounts;
	break;
      case 'V':
	print_version ();
	exit (0);
      default:
	fprintf (stderr, "Try `%s --help' for more information.\n", progname);
	exit (1);
      }

  switch (do_what)
    {
    case saw_remove_all_user_mounts:
      if (optind != argc)
	usage ();
      remove_all_user_mounts ();
      break;
    default:
      if (optind != argc - 1)
	usage ();
      if (cygwin_umount (argv[optind], flags | default_flag) != 0)
	error (argv[optind]);
    }

  return 0;
}

/* remove_all_user_mounts: Unmount all user mounts. */
static void
remove_all_user_mounts ()
{
  FILE *m = setmntent ("/-not-used-", "r");
  struct mntent *p;

  while ((p = getmntent (m)) != NULL)
    {
      /* Remove the mount if it's a user mount. */
      if (strncmp (p->mnt_type, "user", 4) == 0 &&
	  strstr (p->mnt_opts, "noumount") == NULL)
	{
	  if (cygwin_umount (p->mnt_dir, 0))
	    error (p->mnt_dir);

	  /* We've modified the table so we need to start over. */
	  endmntent (m);
	  m = setmntent ("/-not-used-", "r");
	}
    }

  endmntent (m);
}
