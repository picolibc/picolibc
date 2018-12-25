/* lsattr.c

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

int Ropt, aopt, dopt, lopt, nopt;

struct option longopts[] = {
  { "recursive", no_argument, NULL, 'R' },
  { "version", no_argument, NULL, 'V' },
  { "all", no_argument, NULL, 'a' },
  { "directory", no_argument, NULL, 'd' },
  { "help", no_argument, NULL, 'h' },
  { "long", no_argument, NULL, 'l' },
  { "no-headers", no_argument, NULL, 'n' },
  { NULL, no_argument, NULL, 0}
};

const char *opts = "+RVadhln";

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
  { FS_REPARSE_FL,	'r',	"Reparse" },
  { FS_COMPRESSED_FL,	'c',	"Compressed" },
  { FS_OFFLINE_FL,	'o',	"Offline" },
  { FS_NOTINDEXED_FL,	'n',	"Notindexed" },
  { FS_ENCRYPT_FL,	'e',	"Encrypted" },
  { FS_CASESENS_FL,	'C',	"Casesensitive" },
  { 0,			'\0',	NULL },
};

void
print_long (const char *path, uint64_t flags)
{
  int i;
  int first = 1;

  printf("%-28s ", path);
  for (i = 0; supp_flag[i].flagval; ++i)
    if (flags & supp_flag[i].flagval)
      {
	if (!first)
	  fputs (", ", stdout);
	first = 0;
	fputs (supp_flag[i].str, stdout);
      }
  if (first)
    fputs ("---", stdout);
  fputc ('\n', stdout);
}

void
print_short (const char *path, uint64_t flags)
{
  int i;

  for (i = 0; supp_flag[i].flagval; ++i)
    fputc ((flags & supp_flag[i].flagval) ? supp_flag[i].chr : '-', stdout);
  printf(" %s\n", path);
}

int
lsattr (const char *path)
{
  int fd;
  uint64_t flags;

  fd = open (path, O_RDONLY);
  if (fd < 0)
    {
      fprintf (stderr, "%s: %s while trying to open %s\n",
	       program_invocation_short_name, strerror (errno),
	       path);
      return 1;
    }
  if (ioctl (fd, FS_IOC_GETFLAGS, &flags))
    {
      close (fd);
      fprintf (stderr, "%s: %s while trying to fetch flags from %s\n",
	       program_invocation_short_name, strerror (errno),
	       path);
      return 1;
    }
  close (fd);
  if (lopt)
    print_long (path, flags);
  else
    print_short (path, flags);
  return 0;
}

int
lsattr_dir (const char *path)
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

      stpcpy (comp, de->d_name);
      if (lstat (subpath, &st) != 0)
	fprintf (stderr, "%s: %s while trying to stat %s\n",
		 program_invocation_short_name, strerror (errno),
		 subpath);
      else if (de->d_name[0] != '.' || aopt)
	{
	  if (S_ISREG (st.st_mode) || S_ISDIR (st.st_mode))
	    lsattr (subpath);
	  if (S_ISDIR (st.st_mode) && Ropt
	      && strcmp (de->d_name, ".") != 0
		  && strcmp (de->d_name, "..") != 0)
	    {
	      if (!nopt)
		printf ("\n%s:\n", path);
	      lsattr_dir (subpath);
	      if (!nopt)
		fputc ('\n', stdout);
	    }
	}
    }
  free (subpath);
  return 0;
}

static void
print_version ()
{
  printf ("%s (cygwin) %d.%d.%d\n"
	  "Get POSIX ACL information\n"
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

static void
usage (FILE *stream)
{
  fprintf (stream, "Usage: %s [-RVadhln] [file]...\n",
	   program_invocation_short_name);
  if (stream == stderr)
    fprintf (stream, "Try '%s --help' for more information\n",
	     program_invocation_short_name);
  if (stream == stdout)
    fprintf (stream, "\n"
      "List file attributes\n"
      "\n"
      "  -R, --recursive     recursively list attributes of directories and their \n"
      "                      contents\n"
      "  -V, --version       display the program version\n"
      "  -a, --all           list all files in directories, including files that\n"
      "                      start with '.'\n"
      "  -d, --directory     list directories like other files, rather than listing\n"
      "                      their contents.\n"
      "  -l, --long          print options using long names instead of single\n"
      "                      character abbreviations\n"
      "  -n, --no-headers    don't print directory headers when recursing\n"
      "  -h, --help          this help text\n"
      "\n"
      "Supported attributes:\n"
      "\n"
      "  'r', 'Readonly':      file is read-only, directory is system-marked\n"
      "  'h', 'Hidden':        file or directory is hidden\n"
      "  's', 'System':        file or directory that the operating system uses\n"
      "  'a', 'Archive':       file or directory has the archive marker set\n"
      "  't', 'Temporary':     file is being used for temporary storage\n"
      "  'S', 'Sparse':        file is sparse\n"
      "  'r', 'Reparse':       file or directory that has a reparse point\n"
      "  'c', 'Compressed':    file or directory is compressed\n"
      "  'o', 'Offline':       the data of a file is moved to offline storage\n"
      "  'n', 'Notindexed':    file or directory is not to be indexed by the\n"
      "                        content indexing service\n"
      "  'e', 'Encrypted':     file is encrypted\n"
      "  'C', 'Casesensitive': directory is handled case sensitive\n"
      "                        (Windows 10 1803 or later, local NTFS only,\n"
      "                         WSL must be installed)\n");
}

int
main (int argc, char **argv)
{
  int c, ret = 0;

  opterr = 0;
  while ((c = getopt_long (argc, argv, opts, longopts, NULL)) != EOF)
    {
      switch (c)
	{
	case 'R':
	  Ropt = 1;
	  break;
	case 'V':
	  print_version ();
	  return 0;
	case 'a':
	  aopt = 1;
	  break;
	case 'd':
	  dopt = 1;
	  break;
	case 'l':
	  lopt = 1;
	  break;
	case 'n':
	  nopt = 1;
	  break;
	case 'h':
	default:
	  usage (c == 'h' ? stdout : stderr);
	  return 1;
	}
    }
  if (optind > argc - 1)
    lsattr_dir (".");
  else for (; optind < argc; ++optind)
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
      else if (S_ISDIR (st.st_mode) && !dopt)
	{
	  if (lsattr_dir (argv[optind]))
	    ret = 1;
	}
      else if (lsattr (argv[optind]))
      	ret = 1;
    }
  return ret;
}
