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
#include <stdlib.h>
#include <getopt.h>

#ifdef errno
#undef errno
#endif
#include <errno.h>

static void show_mounts (void);
static void show_cygdrive_info (void);
static void change_cygdrive_prefix (const char *new_prefix, int flags);
static int mount_already_exists (const char *posix_path, int flags);

// static short create_missing_dirs = FALSE;
static short force = FALSE;

static const char *progname;

static void
error (const char *path)
{
  fprintf (stderr, "%s: %s: %s\n", progname, path,
	   (errno == EMFILE) ? "Too many mount entries" : strerror (errno));
  exit (1);
}

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
    error (where);

  if (statres == -1)
    {
      if (force == FALSE)
	fprintf (stderr, "%s: warning - %s does not exist.\n", progname, where);
    }
  else if (!(statbuf.st_mode & S_IFDIR))
    {
      if (force == FALSE)
	fprintf (stderr, "%s: warning: %s is not a directory.\n", progname, where);
    }

  exit (0);
}

struct option longopts[] =
{
  {"help", no_argument, NULL, 'h' },
  {"binary", no_argument, NULL, 'b'},
  {"force", no_argument, NULL, 'f'},
  {"system", no_argument, NULL, 's'},
  {"text", no_argument, NULL, 't'},
  {"user", no_argument, NULL, 'u'},
  {"executable", no_argument, NULL, 'x'},
  {"change-cygdrive-prefix", no_argument, NULL, 'c'},
  {"cygwin-executable", no_argument, NULL, 'X'},
  {"show-cygdrive-prefix", no_argument, NULL, 'p'},
  {"import-old-mounts", no_argument, NULL, 'i'},
  {NULL, 0, NULL, 0}
};

char opts[] = "hbfstuxXpic";

static void
usage (void)
{
  fprintf (stderr, "Usage: %s [OPTION] [<win32path> <posixpath>]\n\
  -b, --binary                  text files are equivalent to binary files\n\
				(newline = \\n)\n\
  -c, --change-cygdrive-prefix  change the cygdrive path prefix to <posixpath>\n\
  -f, --force                   force mount, don't warn about missing mount\n\
				point directories\n\
  -i, --import-old-mounts copy  old registry mount table mounts into the current\n\
				mount areas\n\
  -p, --show-cygdrive-prefix    show user and/or system cygdrive path prefix\n\
  -s, --system                  add mount point to system-wide registry location\n\
  -t, --text       (default)    text files get \\r\\n line endings\n\
  -u, --user       (default)    add mount point to user registry location\n\
  -x, --executable              treat all files under mount point as executables\n\
  -X, --cygwin-executable       treat all files under mount point as cygwin\n\
				executables\n\
", progname);
  exit (1);
}

int
main (int argc, char **argv)
{
  int i;
  int flags = 0;
  enum do_what
  {
    nada,
    saw_change_cygdrive_prefix,
    saw_import_old_mounts,
    saw_show_cygdrive_prefix
  } do_what = nada;

  progname = argv[0];

  if (argc == 1)
    {
      show_mounts ();
      exit (0);
    }

  while ((i = getopt_long (argc, argv, opts, longopts, NULL)) != EOF)
    switch (i)
      {
      case 'b':
	flags |= MOUNT_BINARY;
	break;
      case 'c':
	if (do_what == nada)
	  do_what = saw_change_cygdrive_prefix;
	else
	  usage ();
	break;
      case 'f':
	force = TRUE;
	break;
      case 'i':
	if (do_what == nada)
	  do_what = saw_import_old_mounts;
	else
	  usage ();
	break;
      case 'p':
	if (do_what == nada)
	  do_what = saw_show_cygdrive_prefix;
	else
	  usage ();
	break;
      case 's':
	flags |= MOUNT_SYSTEM;
	break;
      case 't':
	flags &= ~MOUNT_BINARY;
	break;
      case 'u':
	flags &= ~MOUNT_SYSTEM;
	break;
      case 'X':
	flags |= MOUNT_CYGWIN_EXEC;
	break;
      case 'x':
	flags |= MOUNT_EXEC;
	break;
      default:
	usage ();
      }

  argc--;
  switch (do_what)
    {
    case saw_change_cygdrive_prefix:
      if (optind != argc)
	usage ();
      change_cygdrive_prefix (argv[optind], flags);
      break;
    case saw_import_old_mounts:
      if (optind <= argc)
	usage ();
      else
	cygwin_internal (CW_READ_V1_MOUNT_TABLES);
      break;
    case saw_show_cygdrive_prefix:
      if (optind <= argc)
	usage ();
      show_cygdrive_info ();
      break;
    default:
      if (optind != (argc - 1))
	{
	  fprintf (stderr, "%s: too many arguments\n", progname);
	  usage ();
	}
      if (force || !mount_already_exists (argv[optind + 1], flags))
	do_mount (argv[optind], argv[optind + 1], flags);
      else
	{
	  errno = EBUSY;
	  error (argv[optind + 1]);
	}
    }

  /* NOTREACHED */
  return 0;
}

static void
show_mounts (void)
{
  FILE *m = setmntent ("/-not-used-", "r");
  struct mntent *p;
  const char *format = "%s on %s type %s (%s)\n";

  // printf (format, "Device", "Directory", "Type", "Flags");
  while ((p = getmntent (m)) != NULL)
    printf (format, p->mnt_fsname, p->mnt_dir, p->mnt_type, p->mnt_opts);
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
	  if (p->mnt_type[0] == 'u')
	    {
	      if (!(flags & MOUNT_SYSTEM)) /* both current_user */
		found_matching = 1;
	      else
		fprintf (stderr,
			 "%s: warning: system mount point of '%s' "
			 "will always be masked by user mount.\n",
			 progname, posix_path);
	      break;
	    }
	  else if (p->mnt_type[0] == 's')
	    {
	      if (flags & MOUNT_SYSTEM) /* both system */
		found_matching = 1;
	      else
		fprintf (stderr,
			 "%s: warning: user mount point of '%s' "
			 "masks system mount.\n",
			 progname, posix_path);
	      break;
	    }
	  else
	    {
	      fprintf (stderr, "%s: warning: couldn't determine mount type.\n", progname);
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
    error (new_prefix);

  exit (0);
}

/* show_cygdrive_info: Show the user and/or cygdrive info, i.e., prefix and
   flags.*/
static void
show_cygdrive_info ()
{
  /* Get the cygdrive info */
  char user[MAX_PATH];
  char system[MAX_PATH];
  char user_flags[MAX_PATH];
  char system_flags[MAX_PATH];
  cygwin_internal (CW_GET_CYGDRIVE_INFO, user, system, user_flags,
		   system_flags);

  /* Display the user and system cygdrive path prefix, if necessary
     (ie, not empty) */
  const char *format = "%-18s  %-11s  %s\n";
  printf (format, "Prefix", "Type", "Flags");
  if (strlen (user) > 0)
    printf (format, user, "user", user_flags);
  if (strlen (system) > 0)
    printf (format, system, "system", system_flags);

  exit (0);
}
