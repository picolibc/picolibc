/* mount.cc

   Copyright 1996, 1997, 1998, 1999 Cygnus Solutions.

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

#include <stdio.h>
#include <sys/mount.h>
#include <sys/stat.h>
#include <mntent.h>
#include <windows.h>
#include <sys/cygwin.h>
#include "winsup.h"
#include "external.h"

#ifdef errno
#undef errno
#endif
#include <errno.h>

static void show_mounts (void);
static void change_cygdrive_prefix (const char *new_prefix, int flags);
static int mount_already_exists (const char *posix_path, int flags);

// static short create_missing_dirs = FALSE;
static short force = FALSE;

static const char *progname;

/* FIXME: do_mount should also print a warning message if the dev arg
   is a non-existent Win32 path. */

static void
do_mount (const char *dev, const char *where, int flags)
{
  struct stat statbuf;
  char win32_path[MAX_PATH];
  int statres;

  cygwin_conv_to_win32_path (where, win32_path);

  statres = stat (win32_path, &statbuf);

#if 0
  if (statres == -1)
    {
      /* FIXME: this'll fail if mount dir is missing any parent dirs */
      if (create_missing_dirs == TRUE)
	{
	  if (mkdir (where, 0755) == -1)
	    fprintf (stderr, "Warning: unable to create %s!\n", where);
	  else
	    statres = 0; /* Pretend stat succeeded if we could mkdir. */
	}
    }
#endif

  if (mount (dev, where, flags))
    {
      perror ("mount failed");
      exit (1);
    }

  if (statres == -1)
    {
      if (force == FALSE)
	fprintf (stderr, "%s: warning - %s does not exist.\n", progname, where);
    }
  else if (!(statbuf.st_mode & S_IFDIR))
    {
      if (force == FALSE)
	fprintf (stderr, "%s: warning: %s is not a directory!\n", progname, where);
    }    

  exit (0);
}

static void
usage (void)
{
  fprintf (stderr, "usage %s [-bfstux] <win32path> <posixpath>
-b  text files are equivalent to binary files (newline = \\n)
-f  force mount, don't warn about missing mount point directories
-s  add mount point to system-wide registry location
-t  text files get \\r\\n line endings (default)
-u  add mount point to user registry location (default)
-x  treat all files under mount point as executables

[-bs] --change-cygdrive-prefix <posixpath>
    change the cygdrive path prefix to <posixpath>
--import-old-mounts
    copy old registry mount table mounts into the current mount areas
", progname);
  exit (1);
}

int
main (int argc, const char **argv)
{
  int i;
  int flags = 0;

  progname = argv[0];

  if (argc == 1)
    {
      show_mounts ();
      exit (0);
    }

  for (i = 1; i < argc; ++i)
    {
      if (argv[i][0] != '-')
	 break;

      if (strcmp (argv[i], "--change-cygdrive-prefix") == 0)
	{
	  if ((i + 2) != argc)
	    usage ();

	  change_cygdrive_prefix (argv[i+1], flags);
	}
      else if (strcmp (argv[i], "--import-old-mounts") == 0)
	{
	  if ((i + 1) != argc)
	    usage ();

	  cygwin_internal (CW_READ_V1_MOUNT_TABLES);
	  exit (0);
	}
      else if (strcmp (argv[i], "-b") == 0)
	flags |= MOUNT_BINARY;
      else if (strcmp (argv[i], "-t") == 0)
	flags &= ~MOUNT_BINARY;
      else if  (strcmp (argv[i], "-X") == 0)
	flags |= MOUNT_CYGWIN_EXEC;
#if 0
      else if (strcmp (argv[i], "-x") == 0)
	create_missing_dirs = TRUE;
#endif
      else if (strcmp (argv[i], "-s") == 0)
	flags |= MOUNT_SYSTEM;
      else if (strcmp (argv[i], "-u") == 0)
	flags &= ~MOUNT_SYSTEM;
      else if (strcmp (argv[i], "-x") == 0)
	flags |= MOUNT_EXEC;
      else if (strcmp (argv[i], "-f") == 0)
	force = TRUE;
      else
	usage ();
    }
  
  if ((i + 2) != argc)
    usage ();

  if ((force == FALSE) && (mount_already_exists (argv[i + 1], flags)))
    {
      errno = EBUSY;
      perror ("mount failed");
      exit (1);
    }
  else
    do_mount (argv[i], argv[i + 1], flags);

  /* NOTREACHED */
  return 0;
}

static void
show_mounts (void)
{
  FILE *m = setmntent ("/-not-used-", "r");
  struct mntent *p;
  const char *format = "%-18s  %-18s  %-11s  %s\n";

  printf (format, "Device", "Directory", "Type", "Flags");
  while ((p = getmntent (m)) != NULL)
    {
      printf (format,
	      p->mnt_fsname,
	      p->mnt_dir,
	      p->mnt_type,
	      p->mnt_opts);
    }
  endmntent (m);
}

/* Return 1 if mountpoint from the same registry area is already in
   mount table.  Otherwise return 0. */
static int
mount_already_exists (const char *posix_path, int flags)
{
  int found_matching = 0;

  FILE *m = setmntent ("/-not-used-", "r");
  struct mntent *p;

  while ((p = getmntent (m)) != NULL)
    {
      /* if the paths match, and they're both the same type of mount. */
      if (strcmp (p->mnt_dir, posix_path) == 0)
	{
	  if (p->mnt_type[0] == 'u' && !(flags & MOUNT_SYSTEM)) /* both current_user */
	    {
	      found_matching = 1;
	      break;
	    }
	  else if (p->mnt_type[0] == 's' && (flags & MOUNT_SYSTEM)) /* both system */
	    {
	      found_matching = 1;
	      break;
	    }
	  else
	    {
	      fprintf (stderr, "%s: warning -- couldn't determine mount type.\n", progname);
	      break;
	    }
	}
    }
  endmntent (m);

  return found_matching;
}

/* change_cygdrive_prefix: Change the cygdrive prefix */
static void
change_cygdrive_prefix (const char *new_prefix, int flags)
{
  flags |= MOUNT_AUTO;

  if (mount (NULL, new_prefix, flags))
    {
      perror ("mount failed");
      exit (1);
    }
  
  exit (0);
}
