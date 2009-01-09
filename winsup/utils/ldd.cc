/* Copyright (c) 2009, Chris Faylor

  All rights reserved.

  Redistribution and use in source and binary forms, with or without
  modification, are permitted provided that the following conditions are met:

	* Redistributions of source code must retain the above copyright
	notice, this list of conditions and the following disclaimer.
	* Neither the name of the owner nor the names of its
	contributors may be used to endorse or promote products derived from
	this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDER AND CONTRIBUTORS "AS
  IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
  TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
  PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT
  OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
  SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED
  TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
  PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
  LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
  NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
  SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include <errno.h>
#include <getopt.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/cygwin.h>
#include <unistd.h>

#define _WIN32_WINNT 0x0501
#include <windows.h>
#include <imagehlp.h>
#include <psapi.h>

#define VERSION "1.0"

struct option longopts[] =
{
  {"help", no_argument, NULL, 0},
  {"version", no_argument, NULL, 0},
  {"data-relocs", no_argument, NULL, 'd'},
  {"function-relocs", no_argument, NULL, 'r'},
  {"unused", no_argument, NULL, 'u'},
  {0, no_argument, NULL, 0}
};

static int
usage (const char *fmt, ...)
{
  va_list ap;
  va_start (ap, fmt);
  fprintf (stderr, "ldd: ");
  vfprintf (stderr, fmt, ap);
  fprintf (stderr, "\nTry `ldd --help' for more information.\n");
  exit (1);
}

#define print_errno_error_and_return(__fn) \
  do {\
    fprintf (stderr, "ldd: %s: %s\n", (__fn), strerror (errno));\
    return 1;\
  } while (0)

#define set_errno_and_return(x) \
  do {\
    cygwin_internal (CW_SETERRNO, __FILE__, __LINE__ - 2);\
    return (x);\
  } while (0)


static HANDLE hProcess;

static int
start_process (const char *fn)
{
  ssize_t len = cygwin_conv_path (CCP_POSIX_TO_WIN_A, fn, NULL, 0);
  if (len <= 0)
    print_errno_error_and_return (fn);

  char fn_win[len + 1];
  if (cygwin_conv_path (CCP_POSIX_TO_WIN_A, fn, fn_win, len))
    print_errno_error_and_return (fn);

  STARTUPINFO si = {};
  PROCESS_INFORMATION pi;
  si.cb = sizeof (si);
  if (CreateProcess (NULL, fn_win, NULL, NULL, FALSE, DEBUG_PROCESS, NULL, NULL, &si, &pi))
    {
      hProcess = pi.hProcess;
      DebugSetProcessKillOnExit (true);
      return 0;
    }

  set_errno_and_return (1);
}

static int
get_entry_point ()
{
  HMODULE hm;
  DWORD cb;
  if (!EnumProcessModules (hProcess, &hm, sizeof (hm), &cb) || !cb)
    set_errno_and_return (1);

  MODULEINFO mi = {};
  if (!GetModuleInformation (hProcess, hm, &mi, sizeof (mi)) || !mi.EntryPoint)
    set_errno_and_return (1);

  static const unsigned char int3 = 0xcc;
  if (!WriteProcessMemory (hProcess, mi.EntryPoint, &int3, 1, &cb) || cb != 1)
    set_errno_and_return (1);
  return 0;
}

struct dlls
  {
    LPVOID lpBaseOfDll;
    struct dlls *next;
  };

static int
print_dlls_and_kill_inferior (dlls *dll)
{
  while ((dll = dll->next))
    {
      char *fn;
      char fnbuf[MAX_PATH + 1];
      DWORD len = GetModuleFileNameEx (hProcess, (HMODULE) dll->lpBaseOfDll, fnbuf, sizeof(fnbuf) - 1);
      if (!len)
	fn = strdup ("???");
      else
	{
	  fnbuf[MAX_PATH] = '\0';
	  ssize_t cwlen = cygwin_conv_path (CCP_WIN_A_TO_POSIX, fnbuf, NULL, 0);
	  if (cwlen <= 0)
	    fn = strdup (fnbuf);
	  else
	    {
	      char *fn_cyg = (char *) malloc (cwlen + 1);
	      if (cygwin_conv_path (CCP_WIN_A_TO_POSIX, fnbuf, fn_cyg, cwlen) == 0)
		fn = fn_cyg;
	      else
		{
		  free (fn_cyg);
		  fn = strdup (fnbuf);
		}
	    }
	}
      printf ("\t%s (%p)\n", fn, dll->lpBaseOfDll);
      free (fn);
    }
  TerminateProcess (hProcess, 0);
  return 0;
}

static int
report (const char *in_fn, bool multiple)
{
  if (multiple)
    printf ("%s:\n", in_fn);
  char *fn = realpath (in_fn, NULL);
  if (!fn || start_process (fn))
    print_errno_error_and_return (in_fn);

  DEBUG_EVENT ev;

  unsigned dll_count = 0;

  dlls dll_list = {};
  dlls *dll_last = &dll_list;
  while (1)
    {
      if (WaitForDebugEvent (&ev, 1000))
	/* ok */;
      else
	switch (GetLastError ())
	  {
	  case WAIT_TIMEOUT:
	    continue;
	  default:
	    usleep (100000);
	    goto out;
	  }
      switch (ev.dwDebugEventCode)
	{
	case LOAD_DLL_DEBUG_EVENT:
	  if (++dll_count == 2)
	    get_entry_point ();
	  dll_last->next = (dlls *) malloc (sizeof (dlls));
	  dll_last->next->lpBaseOfDll = ev.u.LoadDll.lpBaseOfDll;
	  dll_last->next->next = NULL;
	  dll_last = dll_last->next;
	  break;
	case EXCEPTION_DEBUG_EVENT:
	  print_dlls_and_kill_inferior (&dll_list);
	  break;
	default:
	  break;
	}
      if (!ContinueDebugEvent (ev.dwProcessId, ev.dwThreadId, DBG_CONTINUE))
	{
	  cygwin_internal (CW_SETERRNO, __FILE__, __LINE__ - 2);
	  print_errno_error_and_return (in_fn);
	}
    }

out:
  return 0;
}


int
main (int argc, char **argv)
{
  int optch;
  int index;
  while ((optch = getopt_long (argc, argv, "dru", longopts, &index)) != -1)
    switch (optch)
      {
      case 'd':
      case 'r':
      case 'u':
	usage ("option not implemented `-%c'", optch);
	exit (1);
      case 0:
	if (index == 1)
	  {
	    printf ("ldd (Cygwin) %s\nCopyright (C) 2009 Chris Faylor\n"
		    "This is free software; see the source for copying conditions.  There is NO\n"
		    "warranty; not even for MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.\n",
		    VERSION);
	    exit (0);
	  }
	else
	  {
	    puts ("Usage: ldd [OPTION]... FILE...\n\
      --help              print this help and exit\n\
      --version           print version information and exit\n\
  -r, --function-relocs   process data and function relocations\n\
                          (currently unimplemented)\n\
  -u, --unused            print unused direct dependencies\n\
                          (currently unimplemented)\n\
  -v, --verbose           print all information\n\
                          (currently unimplemented)");
	    exit (0);
	  }
      }
  argv += optind;
  if (!*argv)
    usage("missing file arguments");

  int ret = 0;
  bool multiple = !!argv[1];
  char *fn;
  while ((fn = *argv++))
    if (report (fn, multiple))
      ret = 1;
  exit (ret);
}
