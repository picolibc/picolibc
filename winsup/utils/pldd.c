/* pldd.c

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

#define WIN32_LEAN_AND_MEAN
#define UNICODE
#include <errno.h>
#include <error.h>
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/cygwin.h>
#include <cygwin/version.h>
#include <windows.h>
#include <psapi.h>

struct option longopts[] =
{
  {"help", no_argument, NULL, '?'},
  {"version", no_argument, NULL, 'V'},
  {"usage", no_argument, NULL, 0},
  {0, no_argument, NULL, 0}
};
const char *opts = "?V";

__attribute__((noreturn))
static void
print_help (void)
{
  printf ("Usage: pldd [OPTION...] PID\n\n"
	  "List dynamic shared objects loaded into a process.\n\n"
	  "  -?, --help                 Give this help list\n"
	  "      --usage                Give a short usage message\n"
	  "  -V, --version              Print program version\n");
  exit (EXIT_SUCCESS);
}

__attribute__((noreturn))
static void
print_usage (void)
{
  printf ("Usage: pldd [-?V] [--help] [--usage] [--version] PID\n");
  exit (EXIT_SUCCESS);
}

__attribute__((noreturn))
static void
print_version ()
{
  printf ("pldd (cygwin) %d.%d.%d\n"
	  "List dynamic shared objects loaded into process.\n"
	  "Copyright (C) 2012 Cygwin Authors\n\n"
	  "This program comes with NO WARRANTY, to the extent permitted by law.\n"
	  "You may redistribute copies of this program under the terms of\n"
	  "the Cygwin license. Please consult the CYGWIN_LICENSE file for details.\n",
	  CYGWIN_VERSION_DLL_MAJOR / 1000,
	  CYGWIN_VERSION_DLL_MAJOR % 1000,
	  CYGWIN_VERSION_DLL_MINOR);
  exit (EXIT_SUCCESS);
}

__attribute__((noreturn))
static void
print_nargs (void)
{
  fprintf (stderr, "Exactly one parameter with process ID required.\n"
                   "Try `pldd --help' or `pldd --usage' for more information.\n");
  exit (EXIT_FAILURE);
}

int
main (int argc, char *argv[])
{
  int optch, pid, winpid, i;
  char *pidfile, *exefile, *exename;
  FILE *fd;
  HANDLE hProcess;
  HMODULE hModules[1024];
  DWORD cbModules;

  while ((optch = getopt_long (argc, argv, opts, longopts, &optind)) != -1)
    switch (optch)
      {
      case '?':
        print_help ();
        break;
      case 'V':
        print_version ();
        break;
      case 0:
        if (strcmp ("usage", longopts[optind].name) == 0)
          print_usage ();
        break;
      default:
        break;
      }

  argc -= optind;
  argv += optind;
  if (argc != 1)
    print_nargs ();

  pid = atoi (argv[0]);

  if ((pid == 0))
    error (1, 0, "invalid process ID '%s'", argv[0]);

  pidfile = (char *) malloc (32);
  sprintf (pidfile, "/proc/%d/winpid", pid);
  fd = fopen (pidfile, "rb");
  if (!fd)
    error (1, ENOENT, "cannot open /proc/%d", pid);
  fscanf (fd, "%d", &winpid);
  fclose (fd);

  exefile = (char *) malloc (32);
  exename = (char *) malloc (MAX_PATH);
  sprintf (exefile, "/proc/%d/exename", pid);
  fd = fopen (exefile, "rb");
  if (!fd)
    error (1, ENOENT, "cannot open /proc/%d", pid);
  fscanf (fd, "%s", exename);
  fclose (fd);

  hProcess = OpenProcess (PROCESS_QUERY_INFORMATION | PROCESS_VM_READ,
			  0, winpid);
  if (!hProcess)
    error (1, EPERM, "cannot attach to process %d", pid);

  printf ("%d:\t%s\n", pid, exename);

  EnumProcessModules (hProcess, hModules, sizeof (hModules), &cbModules);
  /* start at 1 to skip the executable itself */
  for (i = 1; i < (cbModules / sizeof (HMODULE)); i++)
    {
      TCHAR winname[MAX_PATH];
      char posixname[MAX_PATH];
      GetModuleFileNameEx (hProcess, hModules[i], winname, MAX_PATH);
      cygwin_conv_path (CCP_WIN_W_TO_POSIX, winname, posixname, MAX_PATH);
      printf ("%s\n", posixname);
    }

  return 0;
}
