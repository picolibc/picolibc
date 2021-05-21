/* chattr.c

   Written by Corinna Vinschen <vinschen@redhat.com>

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <getopt.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <cygwin/fs.h>
#include <cygwin/version.h>

int Ropt, Vopt, fopt;
uint64_t add, del, set;
int set_used;

struct option longopts[] = {
  { "recursive", no_argument, NULL, 'R' },
  { "verbose", no_argument, NULL, 'V' },
  { "force", no_argument, NULL, 'f' },
  { "help", no_argument, NULL, 'H' },
  { "version", no_argument, NULL, 'v' },
  { NULL, no_argument, NULL, 0}
};

const char *opts = "+RVfHv";

struct
{
  uint64_t	 flagval;
  char		 chr;
  const char	*str;
} supp_flag[] = {
  { FS_READONLY_FL,	'r',	"Readonly" },
  { FS_HIDDEN_FL,	'h',	"Hidden" },
  { FS_SYSTEM_FL,	's',	"System" },
  { FS_ARCHIVE_FL,	'a',	"Archive" },
  { FS_TEMP_FL,		't',	"Temporary" },
  { FS_SPARSE_FL,	'S',	"Sparse" },
  { FS_REPARSE_FL,	'r',	NULL },
  { FS_COMPRESSED_FL,	'c',	"Compressed" },
  { FS_OFFLINE_FL,	'o',	NULL },
  { FS_NOTINDEXED_FL,	'n',	"Notindexed" },
  { FS_ENCRYPT_FL,	'e',	"Encrypted" },
  { FS_CASESENS_FL,	'C',	"Casesensitive" },
  { 0,			'\0',	NULL },
};
const char *supp_list = "rhsatSrconeC";

void
print_flags (uint64_t flags)
{
  int i;

  for (i = 0; supp_flag[i].flagval; ++i)
    fputc ((flags & supp_flag[i].flagval) ? supp_flag[i].chr : '-', stdout);
}

int
get_flags (const char *opt)
{
  const char *p = opt, *sl;
  uint64_t *mode;
  ptrdiff_t idx;

  switch (*p)
    {
    case '+':
      mode = &add;
      break;
    case '-':
      mode = &del;
      break;
    case '=':
      mode = &set;
      set_used = 1;
      break;
    default:
      return 1;
    }
  while (*++p)
    {
      sl = strchr (supp_list, *p);
      if (!sl)
	return 1;
      idx = sl - supp_list;
      if (!supp_flag[idx].str)
	return 1;
      *mode |= supp_flag[idx].flagval;
    }
  return 0;
}

int
sanity_check ()
{
  int ret = -1;
  if (!set_used && !add && !del)
    fprintf (stderr, "%s: Must use at least one of =, + or -\n",
	     program_invocation_short_name);
  else if (set_used && (add | del))
    fprintf (stderr, "%s: = is incompatible with + and -\n",
	     program_invocation_short_name);
  else if ((add & del) != 0)
    fprintf (stderr, "%s: Can't both set and unset same flag.\n",
	     program_invocation_short_name);
  else
    ret = 0;
  return ret;
}

int
chattr (const char *path)
{
  int fd;
  uint64_t flags, newflags;

  fd = open (path, O_RDONLY);
  if (fd < 0)
    {
      fprintf (stderr, "%s: %s while trying to open %s\n",
	       program_invocation_short_name, strerror (errno), path);
      return 1;
    }
  if (ioctl (fd, FS_IOC_GETFLAGS, &flags))
    {
      close (fd);
      fprintf (stderr, "%s: %s while trying to fetch flags from %s\n",
	       program_invocation_short_name, strerror (errno), path);
      return 1;
    }
  if (set_used)
    newflags = set;
  else
    {
      newflags = flags;
      newflags |= add;
      newflags &= ~del;
    }
  if (newflags != flags)
    {
      if (Vopt)
	{
	  printf ("Flags of %s set as ", path);
	  print_flags (newflags);
	  fputc ('\n', stdout);
	}
      if (ioctl (fd, FS_IOC_SETFLAGS, &newflags))
	{
	  close (fd);
	  fprintf (stderr, "%s: %s while trying to set flags on %s\n",
		   program_invocation_short_name, strerror (errno), path);
	  return 1;
	}
    }
  close (fd);
  return 0;
}

int
chattr_dir (const char *path)
{
  DIR *dir;
  struct dirent *de;
  char *subpath = (char *) malloc (strlen (path) + 1 + NAME_MAX + 1);
  char *comp;

  dir = opendir (path);
  if (!dir)
    {
      free (subpath);
      return 1;
    }
  comp = stpcpy (subpath, path);
  if (comp[-1] != '/')
    *comp++ = '/';
  while ((de = readdir (dir)))
    {
      struct stat st;

      if (strcmp (de->d_name, ".") == 0 || strcmp (de->d_name, "..") == 0)
	continue;

      stpcpy (comp, de->d_name);
      if (lstat (subpath, &st) != 0)
	fprintf (stderr, "%s: %s while trying to stat %s\n",
		 program_invocation_short_name, strerror (errno),
		 subpath);
      else
	{
	  if (S_ISREG (st.st_mode) || S_ISDIR (st.st_mode))
	    chattr (subpath);
	  if (S_ISDIR (st.st_mode) && Ropt)
	    chattr_dir (subpath);
	}
    }
  free (subpath);
  return 0;
}

static void
print_version ()
{
  printf ("%s (cygwin) %d.%d.%d\n"
	  "Change file attributes\n"
	  "Copyright (C) 2018 - %s Cygwin Authors\n"
	  "This is free software; see the source for copying conditions.  "
	  "There is NO\n"
	  "warranty; not even for MERCHANTABILITY or FITNESS FOR A "
	  "PARTICULAR PURPOSE.\n",
	  program_invocation_short_name,
	  CYGWIN_VERSION_DLL_MAJOR / 1000,
	  CYGWIN_VERSION_DLL_MAJOR % 1000,
	  CYGWIN_VERSION_DLL_MINOR,
	  strrchr (__DATE__, ' ') + 1);
}

static void __attribute__ ((__noreturn__))
usage (FILE *stream)
{
  fprintf (stream, "Usage: %s [-RVfHv] [+-=mode]... [file]...\n",
	   program_invocation_short_name);
  if (stream == stderr)
    fprintf (stream, "Try '%s --help' for more information\n",
	     program_invocation_short_name);
  if (stream == stdout)
    fprintf (stream, "\n"
      "Change file attributes\n"
      "\n"
      "  -R, --recursive     recursively apply the changes to directories and their\n"
      "                      contents\n"
      "  -V, --verbose       Be verbose during operation\n"
      "  -f, --force         suppress error messages\n"
      "  -H, --help          this help text\n"
      "  -v, --version       display the program version\n"
      "\n"
      "The format of 'mode' is {+-=}[acCehnrsSt]\n"
      "\n"
      "The operator '+' causes the selected attributes to be added to the\n"
      "existing attributes of the files; '-' causes them to be removed; and\n"
      "'=' causes them to be the only attributes that the files have.\n"
      "A single '=' causes all attributes to be removed.\n"
      "\n"
      "Supported attributes:\n"
      "\n"
      "  'r', 'Readonly':      file is read-only\n"
      "  'h', 'Hidden':        file or directory is hidden\n"
      "  's', 'System':        file or directory that the operating system uses\n"
      "  'a', 'Archive':       file or directory has the archive marker set\n"
      "  't', 'Temporary':     file is being used for temporary storage\n"
      "  'S', 'Sparse':        file is sparse\n"
      "  'c', 'Compressed':    file or directory is compressed\n"
      "  'n', 'Notindexed':    file or directory is not to be indexed by the\n"
      "                        content indexing service\n"
      "  'e', 'Encrypted':     file is encrypted\n"
      "  'C', 'Casesensitive': directory is handled case sensitive\n"
      "                        (Windows 10 1803 or later, local NTFS only,\n"
      "                         WSL must be installed)\n");
  exit (stream == stdout ? 0 : 1);
}

int
main (int argc, char **argv)
{
  int c, ret = 0;
  int lastoptind = 1;
  char *opt;

  opterr = 0;
  while ((c = getopt_long (argc, argv, opts, longopts, NULL)) != EOF)
    {
      switch (c)
	{
	case 'R':
	  Ropt = 1;
	  break;
	case 'V':
	  Vopt = 1;
	  break;
	case 'f':
	  fopt = 1;
	  break;
	case 'H':
	  usage (stdout);
	  break;
	case 'v':
	  print_version ();
	  return 0;
	  break;
	default:
	  if (optind > lastoptind)
	    --optind;
	  goto next;
	}
      lastoptind = optind;
    }
next:
  while (optind < argc)
    {
      if (strcmp (argv[optind], "--") == 0)
	{
	  ++optind;
	  break;
	}
      opt = strchr ("+-=", argv[optind][0]);
      if (!opt)
	break;
      if ((*opt != '=' && argv[optind][1] == '\0') || get_flags (argv[optind]))
	usage (stderr);
      ++optind;
    }
  if (sanity_check ())
    return 1;
  if (optind > argc - 1)
    usage (stderr);

  for (; optind < argc; ++optind)
    {
      struct stat st;

      if (lstat (argv[optind], &st) != 0)
	{
	  fprintf (stderr, "%s: %s while trying to stat %s\n",
		   program_invocation_short_name, strerror (errno),
		   argv[optind]);
	  ret = 1;
	}
      else if (!S_ISREG (st.st_mode) && !S_ISDIR (st.st_mode))
	{
	  fprintf (stderr, "%s: %s on %s\n",
		   program_invocation_short_name, strerror (ENOTSUP),
		   argv[optind]);
	  ret = 1;
	}
      else
	{
	  if (chattr (argv[optind]))
	    ret = 1;
	  if (S_ISDIR (st.st_mode) && Ropt && chattr_dir (argv[optind]))
	    ret = 1;
	}
    }
  return ret;
}
