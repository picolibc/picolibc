/* umount.cc

   Copyright 1996, 1998, 1999 Cygnus Solutions.

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

static void remove_all_mounts ();
static void remove_all_user_mounts ();
static void remove_all_system_mounts ();
static void remove_cygdrive_prefix (int flags);

static const char *progname;

struct option longopts[] =
{
  {"remove-all-mounts", no_argument, NULL, 'A'},
  {"remove-cygdrive-prefix", no_argument, NULL, 'c'},
  {"remove-system-mounts", no_argument, NULL, 'S'},
  {"remove-user-mounts", no_argument, NULL, 'U'},
  {"system", no_argument, NULL, 's'},
  {"user", no_argument, NULL, 'u'},
  {NULL, 0, NULL, 0}
};

char opts[] = "RSUsuc";

static void
usage (void)
{
  fprintf (stderr, "Usage %s [OPTION] [<posixpath>]\n\
  -A, --remove-all-mounts       remove all mounts\n\
  -c, --remove-cygdrive-prefix  remove cygdrive prefix\n\
  -s, --system                  remove system mount\n\
  -S, --remove-system-mounts    remove all system mounts\n\
  -u, --user                    remove user mount\n\
  -U, --remove-user-mounts      remove all user mounts\n\
", progname);
  exit (1);
}

static void
error (const char *path)
{
  fprintf (stderr, "%s: %s: %s\n", progname, path, strerror (errno));
  exit (1);
}

int
main (int argc, char **argv)
{
  int i;
  int flags = 0;
  progname = argv[0];
  enum do_what
  {
    nada,
    saw_remove_all_mounts,
    saw_remove_cygdrive_prefix,
    saw_remove_all_system_mounts,
    saw_remove_all_user_mounts
  } do_what = nada;

  if (argc == 1)
    usage ();

  while ((i = getopt_long (argc, argv, opts, longopts, NULL)) != EOF)
    switch (i)
      {
      case 'A':
	if (do_what != nada)
	  usage ();
	do_what = saw_remove_all_mounts;
	break;
      case 'c':
	if (do_what != nada)
	  usage ();
	do_what = saw_remove_cygdrive_prefix;
	break;
      case 's':
	flags |= MOUNT_SYSTEM;
	break;
      case 'S':
	if (do_what != nada)
	  usage ();
	do_what = saw_remove_all_system_mounts;
	break;
      case 'u':
	flags &= ~MOUNT_SYSTEM;
	break;
      case 'U':
	if (do_what != nada)
	  usage ();
	do_what = saw_remove_all_user_mounts;
	break;
      default:
	usage ();
      }

  switch (do_what)
    {
    case saw_remove_all_mounts:
      if (optind != argc)
	usage ();
      remove_all_mounts ();
      break;
    case saw_remove_cygdrive_prefix:
      if (optind != argc)
	usage ();
      remove_cygdrive_prefix (flags);
      break;
    case saw_remove_all_system_mounts:
      if (optind != argc)
	usage ();
      remove_all_system_mounts ();
      break;
    case saw_remove_all_user_mounts:
      if (optind != argc)
	usage ();
      remove_all_user_mounts ();
      break;
    default:
      if (optind != argc - 1)
	usage ();
      if (cygwin_umount (argv[optind], flags) != 0)
	error (argv[optind]);
    }

  return 0;
}

/* remove_all_mounts: Unmount all mounts. */
static void
remove_all_mounts ()
{
  remove_all_user_mounts ();
  remove_all_system_mounts ();
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
      if (strncmp (p->mnt_type, "user", 4) == 0)
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

/* remove_all_system_mounts: Unmount all system mounts. */
static void
remove_all_system_mounts ()
{
  FILE *m = setmntent ("/-not-used-", "r");
  struct mntent *p;

  while ((p = getmntent (m)) != NULL)
    {
      /* Remove the mount if it's a system mount. */
      if (strncmp (p->mnt_type, "system", 6) == 0)
	{
	  if (cygwin_umount (p->mnt_dir, MOUNT_SYSTEM))
	    error (p->mnt_dir);

	  /* We've modified the table so we need to start over. */
	  endmntent (m);
	  m = setmntent ("/-not-used-", "r");
	}
    }

  endmntent (m);
}

/* remove_cygdrive_prefix: Remove cygdrive user or system path prefix. */
static void
remove_cygdrive_prefix (int flags)
{
  int res = cygwin_umount (NULL, flags | MOUNT_AUTO);
  if (res)
    error ("remove_cygdrive_prefix");
  exit (0);
}
