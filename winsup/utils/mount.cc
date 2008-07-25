/* mount.cc

   Copyright 1996, 1997, 1998, 1999, 2000, 2001, 2002, 2003, 2005,
   2008 Red Hat, Inc.

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
#include <dirent.h>

#ifdef errno
#undef errno
#endif
#include <errno.h>

#define EXEC_FLAGS (MOUNT_EXEC | MOUNT_NOTEXEC | MOUNT_CYGWIN_EXEC)

static void mount_commands (void);
static void show_mounts (void);
static void show_cygdrive_info (void);
static void change_cygdrive_prefix (const char *new_prefix, int flags);
static int mount_already_exists (const char *posix_path, int flags);

// static short create_missing_dirs = FALSE;
static short force = FALSE;

static const char version[] = "$Revision$";
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
	fprintf (stderr, "%s: warning: %s is not a directory.\n", progname, where);
    }

  if (!force && !(flags & EXEC_FLAGS) && strlen (dev))
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
	  fprintf (stderr, "%s: defaulting to '--no-executable' flag for speed since native path\n"
		   "%*creferences a remote share.  Use '-f' option to override.\n", progname,
		   strlen(progname) + 2, ' ');
	  flags |= MOUNT_NOTEXEC;
	}
    }

  if (mount (dev, where, flags))
    error (where);

  exit (0);
}

static struct option longopts[] =
{
  {"change-cygdrive-prefix", no_argument, NULL, 'c'},
  {"force", no_argument, NULL, 'f'},
  {"help", no_argument, NULL, 'h' },
  {"mount-commands", no_argument, NULL, 'm'},
  {"options", required_argument, NULL, 'o'},
  {"show-cygdrive-prefix", no_argument, NULL, 'p'},
  {"version", no_argument, NULL, 'v'},
  {NULL, 0, NULL, 0}
};

static char opts[] = "bcfhmpstuvxEXo:";

static void
usage (FILE *where = stderr)
{
  fprintf (where, "Usage: %s [OPTION] [<win32path> <posixpath>]\n\
Display information about mounted filesystems, or mount a filesystem\n\
\n\
  -c, --change-cygdrive-prefix  change the cygdrive path prefix to <posixpath>\n\
  -f, --force                   force mount, don't warn about missing mount\n\
				point directories\n\
  -h, --help                    output usage information and exit\n\
  -m, --mount-commands          write mount commands to replicate user and\n\
				system mount points and cygdrive prefixes\n\
  -o, --options X[,X...]	specify mount options\n\
  -p, --show-cygdrive-prefix    show user and/or system cygdrive path prefix\n\
  -v, --version                 output version information and exit\n\
  -X, --cygwin-executable       treat all files under mount point as cygwin\n\
				executables\n\
", progname);
  exit (where == stderr ? 1 : 0);
}

struct opt
{
  const char *name;
  unsigned val;
  bool clear;
} oopts[] =
{
  {"user", MOUNT_SYSTEM, true},
  {"system", MOUNT_SYSTEM, false},
  {"binary", MOUNT_BINARY, false},
  {"text", MOUNT_BINARY, true},
  {"exec", MOUNT_EXEC, false},
  {"notexec", MOUNT_NOTEXEC, false},
  {"cygexec", MOUNT_CYGWIN_EXEC, false},
  {"nosuid", 0, 0}
};

static void
print_version ()
{
  const char *v = strchr (version, ':');
  int len;
  if (!v)
    {
      v = "?";
      len = 1;
    }
  else
    {
      v += 2;
      len = strchr (v, ' ') - v;
    }
  printf ("\
%s (cygwin) %.*s\n\
Filesystem Utility\n\
Copyright 1996-2008 Red Hat, Inc.\n\
Compiled on %s\n\
", progname, len, v, __DATE__);
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
    saw_mount_commands
  } do_what = nada;

  progname = strrchr (argv[0], '/');
  if (progname == NULL)
    progname = strrchr (argv[0], '\\');
  if (progname == NULL)
    progname = argv[0];
  else
    progname++;

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
	if (*options)
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
      case 'v':
	print_version ();
	return 0;
	break;
      default:
	usage ();
      }

  while (*options)
    {
      char *p = strchr (options, ',');
      if (p)
	*p++ = '\0';
      else
	p = strchr (options, '\0');

      for (opt *o = oopts; o < (oopts + (sizeof (oopts) / sizeof (oopts[0]))); o++)
	if (strcmp (options, o->name) == 0)
	  {
	    if (o->clear)
	      flags &= ~o->val;
	    else
	      flags |= o->val;
	    goto gotit;
	  }
      fprintf (stderr, "%s: invalid option - '%s'\n", progname, options);
      exit (1);

    gotit:
      options = p;
    }

  if (flags & MOUNT_NOTEXEC && flags & (MOUNT_EXEC | MOUNT_CYGWIN_EXEC))
    {
      fprintf (stderr, "%s: invalid combination of executable options\n", progname);
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
      mount_commands ();
      break;
    default:
      if (optind != (argc - 1))
	{
	  if (optind >= argc)
	    fprintf (stderr, "%s: not enough arguments\n", progname);
	  else
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
mount_commands (void)
{
  FILE *m = setmntent ("/-not-used-", "r");
  struct mntent *p;
  char *c;
  const char *format_mnt = "mount%s \"%s\" \"%s\"\n";
  const char *format_cyg = "mount%s --change-cygdrive-prefix \"%s\"\n";
  char opts[MAX_PATH];
  char user[MAX_PATH];
  char system[MAX_PATH];
  char user_flags[MAX_PATH];
  char system_flags[MAX_PATH];

  // write mount commands for user and system mount points
  while ((p = getmntent (m)) != NULL)
    // Only list non-cygdrives
    if (!strstr (p->mnt_opts, ",noumount"))
      {
	strcpy(opts, " -f");
	if      (p->mnt_opts[0] == 'b')
	  strcat (opts, " -b");
	else if (p->mnt_opts[0] == 't')
	  strcat (opts, " -t");
	if (strstr (p->mnt_opts, ",exec"))
	  strcat (opts, " -x");
	if (strstr (p->mnt_opts, ",noexec"))
	  strcat (opts, " -E");
	if (strstr (p->mnt_opts, ",cygexec"))
	  strcat (opts, " -X");
	while ((c = strchr (p->mnt_fsname, '\\')) != NULL)
	  *c = '/';
	printf (format_mnt, opts, p->mnt_fsname, p->mnt_dir);
      }
  endmntent (m);

  // write mount commands for cygdrive prefixes
  cygwin_internal (CW_GET_CYGDRIVE_INFO, user, system, user_flags,
		   system_flags);

  if (strlen (user) > 0)
    {
      strcpy (opts, " -u");
      if (user_flags[0] == 'b')
	strcat (opts, " -b");
      else if (user_flags[0] == 't')
	strcat (opts, " -t");
      printf (format_cyg, opts, user);
    }

  if (strlen (system) > 0)
    {
      strcpy (opts, " -s");
      if (system_flags[0] == 'b')
	strcat (opts, " -b");
      else if (system_flags[0] == 't')
	strcat (opts, " -t");
      printf (format_cyg, opts, system);
    }

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
    printf (format, system, "system", system_flags);

  exit (0);
}
