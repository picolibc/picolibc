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
#include <libgen.h>

#define _WIN32_WINNT 0x0501
#include <windows.h>
#include <imagehlp.h>
#include <psapi.h>

#ifndef STATUS_DLL_NOT_FOUND
#define STATUS_DLL_NOT_FOUND (0xC0000135L)
#endif

#define VERSION "1.0"

struct option longopts[] =
{
  {"help", no_argument, NULL, 0},
  {"version", no_argument, NULL, 0},
  {"data-relocs", no_argument, NULL, 'd'},
  {"dll", no_argument, NULL, 'D'},
  {"function-relocs", no_argument, NULL, 'r'},
  {"unused", no_argument, NULL, 'u'},
  {0, no_argument, NULL, 0}
};

static int process_file (const char *);

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

static char *
get_module_filename (HANDLE hp, HMODULE hm)
{
  size_t len;
  char *buf = NULL;
  DWORD res;
  for (len = 1024; (res = GetModuleFileNameEx (hp, hm, (buf = (char *) realloc (buf, len)), len)) == len; len += 1024)
    continue;
  if (!res)
    {
      free (buf);
      buf = NULL;
    }
  return buf;
}

static char *
load_dll (const char *fn)
{
  char *buf = get_module_filename (GetCurrentProcess (), NULL);
  if (!buf)
    {
      printf ("ldd: GetModuleFileName returned an error %lu\n", GetLastError ());
      exit (1);		/* FIXME */
    }
  buf = (char *) realloc (buf, sizeof (" \"--dll\" \"\"") + strlen (buf));
  strcat (buf, " --dll \"");
  strcat (buf, fn);
  strcat (buf, "\"");
  return buf;
}


static int
start_process (const char *fn, bool& isdll)
{
  STARTUPINFO si = {};
  PROCESS_INFORMATION pi;
  si.cb = sizeof (si);
  CHAR *cmd;
  if (strlen (fn) < 4 || strcasecmp (strchr (fn, '\0') - 4, ".dll") != 0)
    {
      cmd = strdup (fn);
      isdll = false;
    }
  else
    {
      cmd = load_dll (fn);
      isdll = true;
    }
  if (CreateProcess (NULL, cmd, NULL, NULL, FALSE, DEBUG_PROCESS, NULL, NULL, &si, &pi))
    {
      free (cmd);
      hProcess = pi.hProcess;
      DebugSetProcessKillOnExit (true);
      return 0;
    }

  free (cmd);
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

#define SLOP strlen (" (?)")
char *
tocyg (char *win_fn)
{
  win_fn[MAX_PATH] = '\0';
  ssize_t cwlen = cygwin_conv_path (CCP_WIN_A_TO_POSIX, win_fn, NULL, 0);
  char *fn;
  if (cwlen <= 0)
    fn = strdup (win_fn);
  else
    {
      char *fn_cyg = (char *) malloc (cwlen + SLOP + 1);
      if (cygwin_conv_path (CCP_WIN_A_TO_POSIX, win_fn, fn_cyg, cwlen) == 0)
	fn = fn_cyg;
      else
	{
	  free (fn_cyg);
	  fn = (char *) malloc (strlen (win_fn) + SLOP + 1);
	  strcpy (fn, win_fn);
	}
    }
  return fn;
}

#define CYGWIN_DLL_LEN (strlen ("\\cygwin1.dll"))
static int
print_dlls_and_kill_inferior (dlls *dll, const char *dllfn, const char *process_fn)
{
  bool printit = !dllfn;
  while ((dll = dll->next))
    {
      char *fn;
      char *fullpath = get_module_filename (hProcess, (HMODULE) dll->lpBaseOfDll);
      if (!fullpath)
	fn = strdup ("???");
      else
	{
	  if (printit)
	    fn = tocyg (fullpath);
	  else if (strcasecmp (fullpath, dllfn) != 0)
	    continue;
	  else
	    {
	      printit = true;
	      free (fullpath);
	      continue;
	    }
	  free (fullpath);
	}
      printf ("\t%s => %s (%p)\n", basename (fn), fn, dll->lpBaseOfDll);
      free (fn);
    }
  TerminateProcess (hProcess, 0);
  if (process_fn)
    return process_file (process_fn);
  return 0;
}

static int
report (const char *in_fn, bool multiple)
{
  if (multiple)
    printf ("%s:\n", in_fn);
  char *fn = realpath (in_fn, NULL);
  if (!fn)
    print_errno_error_and_return (in_fn);

  ssize_t len = cygwin_conv_path (CCP_POSIX_TO_WIN_A, fn, NULL, 0);
  if (len <= 0)
    print_errno_error_and_return (fn);

  bool isdll;
  char fn_win[len + 1];
  if (cygwin_conv_path (CCP_POSIX_TO_WIN_A, fn, fn_win, len))
    print_errno_error_and_return (fn);

  if (!fn || start_process (fn_win, isdll))
    print_errno_error_and_return (in_fn);

  DEBUG_EVENT ev;

  unsigned dll_count = 0;

  dlls dll_list = {};
  dlls *dll_last = &dll_list;
  const char *process_fn = NULL;
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
	  if (!isdll && ++dll_count == 2)
	    get_entry_point ();
	  dll_last->next = (dlls *) malloc (sizeof (dlls));
	  dll_last->next->lpBaseOfDll = ev.u.LoadDll.lpBaseOfDll;
	  dll_last->next->next = NULL;
	  dll_last = dll_last->next;
	  break;
	case EXCEPTION_DEBUG_EVENT:
	  switch (ev.u.Exception.ExceptionRecord.ExceptionCode)
	    {
	    case STATUS_DLL_NOT_FOUND:
	      process_fn = fn_win;
	      break;
	    case STATUS_BREAKPOINT:
	      if (!isdll)
		print_dlls_and_kill_inferior (&dll_list, isdll ? fn_win : NULL, process_fn);
	      break;
	    }
	  break;
	case EXIT_PROCESS_DEBUG_EVENT:
	  print_dlls_and_kill_inferior (&dll_list, isdll ? fn_win : NULL, process_fn);
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
      case 'D':
	if (!LoadLibrary (argv[optind]))
	  exit (1);
	exit (0);
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

static struct filelist
{
  struct filelist *next;
  char *name;
} *head;

static bool printing = false;

static bool
saw_file (char *name)
{

  struct filelist *p;

  for (p=head; p; p = p->next)
    if (strcasecmp (name, p->name) == 0)
      return true;

  p = (filelist *) malloc(sizeof (struct filelist));
  p->next = head;
  p->name = strdup (name);
  head = p;
  return false;
}


/* dump of import directory
   section begins at pointer 'section base'
   section RVA is 'section_rva'
   import directory begins at pointer 'imp' */
static int
dump_import_directory (const void *const section_base,
		       const DWORD section_rva,
		       const IMAGE_IMPORT_DESCRIPTOR *imp)
{
  /* get memory address given the RVA */
  #define adr(rva) ((const void*) ((char*) section_base+((DWORD) (rva))-section_rva))

  /* continue until address inaccessible or there's no DLL name */
  for (; !IsBadReadPtr (imp, sizeof (*imp)) && imp->Name; imp++)
    {
      char full_path[MAX_PATH];
      char *dummy;
      char *fn = (char *) adr (imp->Name);

      if (saw_file (fn))
	continue;

      /* output DLL's name */
      char *print_fn;
      if (!SearchPath (NULL, fn, NULL, sizeof (full_path), full_path, &dummy))
	{
	  print_fn = strdup ("not found");
	  printing = true;
	}
      else if (!printing)
	continue;
      else
	{
	  print_fn = tocyg (full_path);
	  strcat (print_fn, " (?)");
	}

      printf ("\t%s => %s\n", (char *) fn, print_fn);
      free (print_fn);
    }
  #undef adr

  return 0;
}

/* load a file in RAM (memory-mapped)
   return pointer to loaded file
   0 if no success  */
static void *
map_file (const char *filename)
{
  HANDLE hFile, hMapping;
  void *basepointer;
  if ((hFile = CreateFile (filename, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE,
			   0, OPEN_EXISTING, FILE_FLAG_SEQUENTIAL_SCAN, 0)) == INVALID_HANDLE_VALUE)
    {
      fprintf (stderr, "couldn't open %s\n", filename);
      return 0;
    }
  if (!(hMapping = CreateFileMapping (hFile, 0, PAGE_READONLY | SEC_COMMIT, 0, 0, 0)))
    {
      fprintf (stderr, "CreateFileMapping failed with windows error %lu\n", GetLastError ());
      CloseHandle (hFile);
      return 0;
    }
  if (!(basepointer = MapViewOfFile (hMapping, FILE_MAP_READ, 0, 0, 0)))
    {
      fprintf (stderr, "MapViewOfFile failed with windows error %lu\n", GetLastError ());
      CloseHandle (hMapping);
      CloseHandle (hFile);
      return 0;
    }
  
  CloseHandle (hMapping);
  CloseHandle (hFile);

  return basepointer;
}


/* this will return a pointer immediatly behind the DOS-header
   0 if error */
static void *
skip_dos_stub (const IMAGE_DOS_HEADER *dos_ptr)
{
  /* look there's enough space for a DOS-header */
  if (IsBadReadPtr (dos_ptr, sizeof (*dos_ptr)))
      {
	fprintf (stderr, "not enough space for DOS-header\n");
	return 0;
      }

   /* validate MZ */
   if (dos_ptr->e_magic != IMAGE_DOS_SIGNATURE)
      {
	fprintf (stderr, "not a DOS-stub\n");
	return 0;
      }

  /* ok, then, go get it */
  return (char*) dos_ptr + dos_ptr->e_lfanew;
}


/* find the directory's section index given the RVA
   Returns -1 if impossible */
static int
get_directory_index (const unsigned dir_rva,
		     const unsigned dir_length,
		     const int number_of_sections,
		     const IMAGE_SECTION_HEADER *sections)
{
  int sect;
  for (sect = 0; sect < number_of_sections; sect++)
  {
    /* compare directory RVA to section RVA */
    if (sections[sect].VirtualAddress <= dir_rva
       && dir_rva < sections[sect].VirtualAddress+sections[sect].SizeOfRawData)
      return sect;
  }

  return -1;
}

/* dump imports of a single file
   Returns 0 if successful, !=0 else */
static int
process_file (const char *filename)
{
  void *basepointer;    /* Points to loaded PE file
			 * This is memory mapped stuff
			 */
  int number_of_sections;
  DWORD import_rva;           /* RVA of import directory */
  DWORD import_length;        /* length of import directory */
  int import_index;           /* index of section with import directory */

  /* ensure byte-alignment for struct tag_header */
  #include <pshpack1.h>

  const struct tag_header
    {
      DWORD signature;
      IMAGE_FILE_HEADER file_head;
      IMAGE_OPTIONAL_HEADER opt_head;
      IMAGE_SECTION_HEADER section_header[1];  /* this is an array of unknown length
					          actual number in file_head.NumberOfSections
					          if your compiler objects to it length 1 should work */
    } *header;

  /* revert to regular alignment */
  #include <poppack.h>

  head = NULL;			/* FIXME: memory leak */
  printing = false;

  /* first, load file */
  basepointer = map_file (filename);
  if (!basepointer)
      {
	puts ("cannot load file");
	return 1;
      }

  /* get header pointer; validate a little bit */
  header = (struct tag_header *) skip_dos_stub ((IMAGE_DOS_HEADER *) basepointer);
  if (!header)
      {
	puts ("cannot skip DOS stub");
	UnmapViewOfFile (basepointer);
	return 2;
      }

  /* look there's enough space for PE headers */
  if (IsBadReadPtr (header, sizeof (*header)))
      {
	puts ("not enough space for PE headers");
	UnmapViewOfFile (basepointer);
	return 3;
      }

  /* validate PE signature */
  if (header->signature!=IMAGE_NT_SIGNATURE)
      {
	puts ("not a PE file");
	UnmapViewOfFile (basepointer);
	return 4;
      }

  /* get number of sections */
  number_of_sections = header->file_head.NumberOfSections;

  /* check there are sections... */
  if (number_of_sections<1)
      {
	UnmapViewOfFile (basepointer);
	return 5;
      }

  /* validate there's enough space for section headers */
  if (IsBadReadPtr (header->section_header, number_of_sections*sizeof (IMAGE_SECTION_HEADER)))
      {
	puts ("not enough space for section headers");
	UnmapViewOfFile (basepointer);
	return 6;
      }

  /* get RVA and length of import directory */
  import_rva = header->opt_head.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress;
  import_length = header->opt_head.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].Size;

  /* check there's stuff to care about */
  if (!import_rva || !import_length)
      {
	UnmapViewOfFile (basepointer);
	return 0;       /* success! */
    }

  /* get import directory pointer */
  import_index = get_directory_index (import_rva,import_length,number_of_sections,header->section_header);

  /* check directory was found */
  if (import_index <0)
      {
	puts ("couldn't find import directory in sections");
	UnmapViewOfFile (basepointer);
	return 7;
      }

  /* ok, we've found the import directory... action! */
  {
      /* The pointer to the start of the import directory's section */
    const void *section_address = (char*) basepointer + header->section_header[import_index].PointerToRawData;
    if (dump_import_directory (section_address,
			     header->section_header[import_index].VirtualAddress,
				      /* the last parameter is the pointer to the import directory:
				         section address + (import RVA - section RVA)
				         The difference is the offset of the import directory in the section */
			     (const IMAGE_IMPORT_DESCRIPTOR *) ((char *) section_address+import_rva-header->section_header[import_index].VirtualAddress)))
      {
	UnmapViewOfFile (basepointer);
	return 8;
      }
  }
  
  UnmapViewOfFile (basepointer);
  return 0;
}
