/* mount.cc

   Copyright 1996, 1997, 1998, 1999, 2000, 2001, 2002, 2003, 2004, 2005, 2006,
   2007, 2008, 2009, 2010, 2011, 2013 Red Hat, Inc.

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
#include <cygwin/version.h>
#include <stdlib.h>
#include <unistd.h>
#include <getopt.h>
#include <dirent.h>
#include "path.h"

#include <errno.h>

#define NT_MAX_PATH 32768

#define EXEC_FLAGS (MOUNT_EXEC | MOUNT_NOTEXEC | MOUNT_CYGWIN_EXEC)

static void mount_entries (void);
static void show_mounts (void);
static void show_cygdrive_info (void);
static void change_cygdrive_prefix (const char *new_prefix, int flags);
static int mount_already_exists (const char *posix_path, int flags);

// static short create_missing_dirs = FALSE;
static bool force = false;

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
  int statres;

  statres = stat (where, &statbuf);

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

  if (statres == -1)
    {
      if (!force)
	fprintf (stderr, "%s: warning - %s does not exist.\n", progname, where);
    }
  else if (!(statbuf.st_mode & S_IFDIR))
    {
      if (!force)
	fprintf (stderr, "%s: warning: %s is not a directory.\n",
		 progname, where);
    }

  if (!force && !(flags & (EXEC_FLAGS | MOUNT_BIND)) && strlen (dev))
    {
      char devtmp[1 + 2 * strlen (dev)];
      strcpy (devtmp, dev);
      char c = strchr (devtmp, '\0')[-1];
      if (c == '/' || c == '\\')
	strcat (devtmp, ".");
      /* Use a curious property of Windows which allows the use of \.. even
	 on non-directory paths. */
      for (const char *p = dev; (p = strpbrk (p, "/\\")); p++)
	strcat (devtmp, "\\..");
      strcat (devtmp, "\\");
      if (GetDriveType (devtmp) == DRIVE_REMOTE)
	{
	  fprintf (stderr,
      "%s: defaulting to 'notexec' mount option for speed since native path\n"
      "%*creferences a remote share.  Use '-f' option to override.\n",
		   progname, (int) strlen(progname) + 2, ' ');
	  flags |= MOUNT_NOTEXEC;
	}
    }

  if (mount (dev, where, flags))
    error (where);
}

static void
from_fstab (bool user)
{
  char path[PATH_MAX];
  char buf[65536];
  mnt_t *m = mount_table + max_mount_entry;

  strcpy (path, "/etc/fstab");
  if (user)
    {
      strcat (path, ".d/");
      strcat (path, getlogin ());
    }
  FILE *fh = fopen (path, "rt");
  if (!fh)
    return;
  while (fgets (buf, 65536, fh))
    {
      char *c = strrchr (buf, '\n');
      if (c)
      	*c = '\0';
      if (from_fstab_line (m, buf, user))
	++m;
    }
  max_mount_entry = m - mount_table;
  fclose (fh);
}

static void
do_mount_from_fstab (const char *where)
{
  force = true;
  /* Read fstab entries. */
  from_fstab (false);
  from_fstab (true);
  /* Loop through fstab entries and see if it matches `where'.  If `where'
     is NULL, all entries match. */
  bool exists = false;
  for (mnt_t *m = mount_table; m - mount_table < max_mount_entry; ++m)
    if (!where || !strcmp (where, m->posix))
      {
	if (m->flags & MOUNT_CYGDRIVE)
	  {
	    /* Get the cygdrive info */
	    char user[MAX_PATH];
	    char system[MAX_PATH];
	    char user_flags[MAX_PATH];
	    char system_flags[MAX_PATH];

	    exists = true;
	    cygwin_internal (CW_GET_CYGDRIVE_INFO, user, system, user_flags,
			     system_flags);
	    if ((*user && strcmp (user, m->posix) != 0)
		|| (*system && strcmp (system, m->posix) != 0))
	      if (mount (NULL, m->posix, m->flags))
		error (m->posix);
	  }
	else
	  {
	    exists = true;
	    /* Compare with existing mount table.  If the entry doesn't exist,
	       mount it. */
	    FILE *mt = setmntent ("/-not-used-", "r");
	    struct mntent *p;

	    while ((p = getmntent (mt)) != NULL)
	      if (!strcmp (m->posix, p->mnt_dir))
		break;
	    if (!p)
	      do_mount (m->native, m->posix, m->flags);
	    endmntent (mt);
	    if (where)
	      break;
	  }
      }
  if (!exists && where)
    fprintf (stderr,
	     "%s: can't find %s in /etc/fstab or in /etc/fstab.d/$USER\n",
	     progname, where);
}

static struct option longopts[] =
{
  {"all", no_argument, NULL, 'a'},
  {"change-cygdrive-prefix", no_argument, NULL, 'c'},
  {"force", no_argument, NULL, 'f'},
  {"help", no_argument, NULL, 'h' },
  {"mount-entries", no_argument, NULL, 'm'},
  {"options", required_argument, NULL, 'o'},
  {"show-cygdrive-prefix", no_argument, NULL, 'p'},
  {"version", no_argument, NULL, 'V'},
  {NULL, 0, NULL, 0}
};

static char opts[] = "acfhmpVo:";

static void
usage (FILE *where = stderr)
{
  char *options;

  fprintf (where, "Usage: %1$s [OPTION] [<win32path> <posixpath>]\n\
       %1$s -a\n\
       %1$s <posixpath>\n\
\n\
Display information about mounted filesystems, or mount a filesystem\n\
\n\
  -a, --all                     mount all filesystems mentioned in fstab\n\
  -c, --change-cygdrive-prefix  change the cygdrive path prefix to <posixpath>\n\
  -f, --force                   force mount, don't warn about missing mount\n\
				point directories\n\
  -h, --help                    output usage information and exit\n\
  -m, --mount-entries           write fstab entries to replicate mount points\n\
				and cygdrive prefixes\n\
  -o, --options X[,X...]	specify mount options\n\
  -p, --show-cygdrive-prefix    show user and/or system cygdrive path prefix\n\
  -V, --version                 output version information and exit\n\n",
  progname);
  if (!cygwin_internal (CW_LST_MNT_OPTS, &options))
    fprintf (where, "Valid options are: %s\n\n", options);
  exit (where == stderr ? 1 : 0);
}

static void
print_version ()
{
  printf ("mount (cygwin) %d.%d.%d\n"
	  "Mount filesystem utility\n"
	  "Copyright (C) 1996 - %s Red Hat, Inc.\n"
	  "This is free software; see the source for copying conditions.  There is NO\n"
	  "warranty; not even for MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.\n",
	  CYGWIN_VERSION_DLL_MAJOR / 1000,
	  CYGWIN_VERSION_DLL_MAJOR % 1000,
	  CYGWIN_VERSION_DLL_MINOR,
	  strrchr (__DATE__, ' ') + 1);
}

static char *
concat3 (char *a, const char *b, const char *c)
{
  size_t totlen = strlen (a) + strlen (b) + strlen (c) + 1;
  a = (char *) realloc (a, totlen);
  return strcat (strcat (a, b), c);
}

int
main (int argc, char **argv)
{
  int i;
  int flags = MOUNT_BINARY;
  char *options = strdup ("");
  enum do_what
  {
    nada,
    saw_change_cygdrive_prefix,
    saw_show_cygdrive_prefix,
    saw_mount_commands,
    saw_mount_all,
  } do_what = nada;

  progname = program_invocation_short_name;

  if (argc == 1)
    {
      show_mounts ();
      exit (0);
    }

  while ((i = getopt_long (argc, argv, opts, longopts, NULL)) != EOF)
    switch (i)
      {
      case 'a':
	if (do_what == nada)
	  do_what = saw_mount_all;
	else
	  usage ();
	break;
      case 'c':
	if (do_what == nada)
	  do_what = saw_change_cygdrive_prefix;
	else
	  usage ();
	break;
      case 'f':
	force = true;
	break;
      case 'h':
	usage (stdout);
	break;
      case 'm':
	if (do_what == nada)
	  do_what = saw_mount_commands;
	else
	  usage ();
	break;
      case 'o':
	if (do_what == saw_mount_all)
	  usage ();
	else if (*options)
	  options = concat3 (options, ",", optarg);
	else
	  options = strdup (optarg);
	break;
      case 'p':
	if (do_what == nada)
	  do_what = saw_show_cygdrive_prefix;
	else
	  usage ();
	break;
      case 'V':
	print_version ();
	return 0;
	break;
      default:
	fprintf (stderr, "Try `%s --help' for more information.\n", progname);
	return 1;
      }

  if (cygwin_internal (CW_CVT_MNT_OPTS, &options, &flags))
    {
      fprintf (stderr, "%s: invalid option - '%s'\n", progname, options);
      exit (1);
    }

  if (flags & MOUNT_NOTEXEC && flags & (MOUNT_EXEC | MOUNT_CYGWIN_EXEC))
    {
      fprintf (stderr, "%s: invalid combination of executable options\n",
	       progname);
      exit (1);
    }

  cygwin_internal (CW_SET_DOS_FILE_WARNING, false);

  argc--;
  switch (do_what)
    {
    case saw_change_cygdrive_prefix:
      if (optind != argc)
	usage ();
      change_cygdrive_prefix (argv[optind], flags);
      break;
    case saw_show_cygdrive_prefix:
      if (optind <= argc)
	usage ();
      show_cygdrive_info ();
      break;
    case saw_mount_commands:
      if (optind <= argc)
	usage ();
      mount_entries ();
      break;
    case saw_mount_all:
      if (optind <= argc)
	usage ();
      do_mount_from_fstab (NULL);
      break;
    default:
      if (optind == argc)
	do_mount_from_fstab (argv[optind]);
      else if (optind != (argc - 1))
	{
	  fprintf (stderr, "%s: too many arguments\n", progname);
	  usage ();
	}
      else if (force || !mount_already_exists (argv[optind + 1], flags))
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

static char *
convert_spaces (char *tgt, const char *src)
{
  char *tp, *spacep;
  const char *sp;

  tp = tgt;
  for (sp = src; (spacep = strchr (sp, ' ')); sp = spacep + 1)
    {
      tp = stpncpy (tp, sp, spacep - sp);
      tp = stpcpy (tp, "\\040");
    }
  stpcpy (tp, sp);
  return tgt;
}

static void
mount_entries (void)
{
  FILE *m = setmntent ("/-not-used-", "r");
  struct mntent *p;
  const char *format_mnt = "%s %s %s %s 0 0\n";
  const char *format_cyg = "none %s cygdrive %s 0 0\n";

  // write fstab entries for normal mount points
  while ((p = getmntent (m)) != NULL)
    // Only list non-cygdrives and non-automounts
    if (!strstr (p->mnt_opts, ",noumount") && !strstr (p->mnt_opts, ",auto"))
      {
	char fsname[NT_MAX_PATH], dirname[NT_MAX_PATH];
	/* Drop the "bind" option since it can't be reverted. */
	char *c = strstr (p->mnt_opts, ",bind");
	if (c)
	  memmove (c, c + 5, strlen (c + 5) + 1);
	printf (format_mnt, convert_spaces (fsname, p->mnt_fsname),
			    convert_spaces (dirname, p->mnt_dir),
			    p->mnt_type, p->mnt_opts);
      }
  endmntent (m);

  // write fstab entry for cygdrive prefix
  m = setmntent ("/-not-used-", "r");
  while ((p = getmntent (m)) != NULL)
    {
      char *noumount;
      if ((noumount = strstr (p->mnt_opts, ",noumount")))
	{
	  char dirname[NT_MAX_PATH];
	  char opts[strlen (p->mnt_opts) + 1];

	  convert_spaces (dirname, p->mnt_dir);
	  // remove trailing slash
	  char *ls = strrchr (dirname, '/');
	  if (ls)
	    {
	      // last slash == leading slash?  cygdrive prefix == "/"
	      if (ls == dirname)
		++ls;
	      *ls = '\0';
	    }
	  *stpncpy (opts, p->mnt_opts, noumount - p->mnt_opts) = '\0';
	  printf (format_cyg, dirname, opts);
	  break;
	}
    }
  endmntent (m);

  exit(0);
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
			 "masks system mount.\n", progname, posix_path);
	      break;
	    }
	  else
	    {
	      fprintf (stderr, "%s: warning: couldn't determine mount type.\n",
		       progname);
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
  flags |= MOUNT_CYGDRIVE;

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
    printf (format, system, "nouser", system_flags);

  exit (0);
}
