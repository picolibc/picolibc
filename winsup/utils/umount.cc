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

static void remove_all_mounts ();
static void remove_all_automounts ();
static void remove_all_user_mounts ();
static void remove_all_system_mounts ();

static const char *progname;

static void
usage (void)
{
  fprintf (stderr, "Usage %s [-s] <posixpath>\n", progname);
  fprintf (stderr, "-s = remove mount point from system-wide registry location\n");
  fprintf (stderr, "\n");
  fprintf (stderr, "--remove-all-mounts = remove all mounts\n");
  fprintf (stderr, "--remove-auto-mounts = remove all automatically mounted mounts\n");
  fprintf (stderr, "--remove-user-mounts = remove all mounts in the current user mount registry area, including auto mounts\n");
  fprintf (stderr, "--remove-system-mounts = Remove all mounts in the system-wide mount registry area\n");
  exit (1);
}

static void
error (char *path)
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

  if (argc == 1)
    usage ();

  for (i = 1; i < argc; ++i)
    {
      if (argv[i][0] != '-')
        break;

      if (strcmp (argv[i], "-s") == 0)
	{
	  flags |= MOUNT_SYSTEM;
	}
      else if (strcmp (argv[i], "--remove-all-mounts") == 0)
	{
	  remove_all_mounts ();
	  exit (0);
	}
      else if (strcmp (argv[i], "--remove-user-mounts") == 0)
	{
	  remove_all_user_mounts ();
	  exit (0);
	}
      else if (strcmp (argv[i], "--remove-system-mounts") == 0)
	{
	  remove_all_system_mounts ();
	  exit (0);
	}
      else if (strcmp (argv[i], "--remove-auto-mounts") == 0)
	{
	  remove_all_automounts ();
	  exit (0);
	}
      else
	usage ();
    }

  if ((i + 1) != argc)
    usage ();

  if (cygwin_umount (argv[i], flags) != 0)
    error (argv[i]);

  return 0;
}

/* remove_all_mounts: Unmount all mounts. */
static void
remove_all_mounts ()
{
  remove_all_user_mounts ();
  remove_all_system_mounts ();
}

/* remove_all_automounts: Unmount all automounts. */
static void
remove_all_automounts ()
{
  FILE *m = setmntent ("/-not-used-", "r");
  struct mntent *p;

  while ((p = getmntent (m)) != NULL)
    {
      /* Remove the mount if it's an automount. */
      if (strcmp (p->mnt_type, "user,auto") == 0)
	{
	  if (cygwin_umount (p->mnt_dir, 0))
	    error (p->mnt_dir);

	  /* We've modified the table so we need to start over. */
	  endmntent (m);
	  m = setmntent ("/-not-used-", "r");
	}
      else if (strcmp (p->mnt_type, "system,auto") == 0)
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
