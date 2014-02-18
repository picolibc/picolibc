/* minidumper.cc

   Copyright 2014 Red Hat Inc.

   This file is part of Cygwin.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License (file COPYING.dumper) for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin Street - Fifth Floor, Boston, MA 02110-1301, USA.
*/

#include <sys/cygwin.h>
#include <cygwin/version.h>
#include <getopt.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <windows.h>

BOOL verbose = FALSE;
BOOL nokill = FALSE;

typedef DWORD MINIDUMP_TYPE;

typedef BOOL (WINAPI *MiniDumpWriteDump_type)(
                                              HANDLE hProcess,
                                              DWORD dwPid,
                                              HANDLE hFile,
                                              MINIDUMP_TYPE DumpType,
                                              CONST void *ExceptionParam,
                                              CONST void *UserStreamParam,
                                              CONST void *allbackParam);

static void
minidump(DWORD pid, MINIDUMP_TYPE dump_type, const char *minidump_file)
{
  HANDLE dump_file;
  HANDLE process;
  MiniDumpWriteDump_type MiniDumpWriteDump_fp;
  HMODULE module;

  module = LoadLibrary("dbghelp.dll");
  if (!module)
    {
      fprintf (stderr, "error loading DbgHelp\n");
      return;
    }

  MiniDumpWriteDump_fp = (MiniDumpWriteDump_type)GetProcAddress(module, "MiniDumpWriteDump");
  if (!MiniDumpWriteDump_fp)
    {
      fprintf (stderr, "error getting the address of MiniDumpWriteDump\n");
      return;
    }

  dump_file = CreateFile(minidump_file,
                         GENERIC_READ | GENERIC_WRITE,
                         0,
                         NULL,
                         CREATE_ALWAYS,
                         FILE_FLAG_BACKUP_SEMANTICS,
                         NULL);
  if (dump_file == INVALID_HANDLE_VALUE)
    {
      fprintf (stderr, "error opening file\n");
      return;
    }

  process = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ | PROCESS_TERMINATE,
                        FALSE,
                        pid);
  if (dump_file == INVALID_HANDLE_VALUE)
    {
      fprintf (stderr, "error opening process\n");
      return;
    }

  BOOL success = (*MiniDumpWriteDump_fp)(process,
                                         pid,
                                         dump_file,
                                         dump_type,
                                         NULL,
                                         NULL,
                                         NULL);
  if (success)
    {
      if (verbose)
        printf ("minidump created successfully\n");
    }
  else
    {
      fprintf (stderr, "error creating minidump\n");
    }

  /* Unless nokill is given, behave like dumper and terminate the dumped
     process */
  if (!nokill)
    {
      TerminateProcess(process, 128 + 9);
      WaitForSingleObject(process, INFINITE);
    }

  CloseHandle(process);
  CloseHandle(dump_file);
  FreeLibrary(module);
}

static void
usage (FILE *stream, int status)
{
  fprintf (stream, "\
Usage: %s [OPTION] FILENAME WIN32PID\n\
\n\
Write minidump from WIN32PID to FILENAME.dmp\n\
\n\
 -t, --type     minidump type flags\n\
 -n, --nokill   don't terminate the dumped process\n\
 -d, --verbose  be verbose while dumping\n\
 -h, --help     output help information and exit\n\
 -q, --quiet    be quiet while dumping (default)\n\
 -V, --version  output version information and exit\n\
\n", program_invocation_short_name);
  exit (status);
}

struct option longopts[] = {
  {"type", required_argument, NULL, 't'},
  {"nokill", no_argument, NULL, 'n'},
  {"verbose", no_argument, NULL, 'd'},
  {"help", no_argument, NULL, 'h'},
  {"quiet", no_argument, NULL, 'q'},
  {"version", no_argument, 0, 'V'},
  {0, no_argument, NULL, 0}
};
const char *opts = "tndhqV";

static void
print_version ()
{
  printf ("minidumper (cygwin) %d.%d.%d\n"
          "Minidump write for Cygwin\n"
          "Copyright (C) 1999 - %s Red Hat, Inc.\n"
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
  int opt;
  const char *p = "";
  DWORD pid;
  MINIDUMP_TYPE dump_type = 0; // MINIDUMP_NORMAL

  while ((opt = getopt_long (argc, argv, opts, longopts, NULL) ) != EOF)
    switch (opt)
      {
      case 't':
        {
          char *endptr;
          dump_type = strtoul(optarg, &endptr, 0);
          if (*endptr != '\0')
            {
              fprintf (stderr, "syntax error in minidump type \"%s\" near character #%d.\n", optarg, (int) (endptr - optarg));
              exit(1);
            }
        }
        break;
      case 'n':
        nokill = TRUE;
        break;
      case 'd':
        verbose = TRUE;
        break;
      case 'q':
        verbose = FALSE;
        break;
      case 'h':
        usage (stdout, 0);
      case 'V':
        print_version ();
        exit (0);
      default:
        fprintf (stderr, "Try `%s --help' for more information.\n",
                 program_invocation_short_name);
        exit (1);
      }

  if (argv && *(argv + optind) && *(argv + optind +1))
    {
      ssize_t len = cygwin_conv_path (CCP_POSIX_TO_WIN_A | CCP_RELATIVE,
                                      *(argv + optind), NULL, 0);
      char *win32_name = (char *) alloca (len);
      cygwin_conv_path (CCP_POSIX_TO_WIN_A | CCP_RELATIVE,  *(argv + optind),
                        win32_name, len);
      if ((p = strrchr (win32_name, '\\')))
        p++;
      else
        p = win32_name;

      pid = strtoul (*(argv + optind + 1), NULL, 10);
    }
  else
    {
      usage (stderr, 1);
      return -1;
    }

  char *minidump_file = (char *) malloc (strlen (p) + sizeof (".dmp"));
  if (!minidump_file)
    {
      fprintf (stderr, "error allocating memory\n");
      return -1;
    }
  sprintf (minidump_file, "%s.dmp", p);

  if (verbose)
    printf ("dumping process %u to %s using dump type flags 0x%x\n", (unsigned int)pid, minidump_file, (unsigned int)dump_type);

  minidump(pid, dump_type, minidump_file);

  free (minidump_file);
};
