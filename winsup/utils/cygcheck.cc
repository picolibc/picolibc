/* cygcheck.cc

   Copyright 1998 Cygnus Solutions.

   This file is part of Cygwin.

   This software is a copyrighted work licensed under the terms of the
   Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
   details. */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>
#include <sys/cygwin.h>
#include <mntent.h>
#include <time.h>

int verbose = 0;
int registry = 0;
int sysinfo = 0;
int givehelp = 0;

#ifdef __GNUC__
typedef long long longlong;
#else
typedef __int64 longlong;
#endif

const char *known_env_vars[] =
{
  "c_include_path",
  "compiler_path",
  "cxx_include_path",
  "cygwin",
  "cygwin32",
  "dejagnu",
  "expect",
  "gcc_default_options",
  "gcc_exec_prefix",
  "home",
  "ld_library_path",
  "library_path",
  "login",
  "lpath",
  "make_mode",
  "makeflags",
  "path",
  "pwd",
  "strace",
  "tcl_library",
  "user",
  0
};

struct {
  const char *name;
  int missing_is_good;
} common_apps[] = {
  { "bash", 0 },
  { "cat", 0 },
  { "cpp", 1 },
  { "find", 0 },
  { "gcc", 0 },
  { "gdb", 0 },
  { "ld", 0 },
  { "ls", 0 },
  { "make", 0 },
  { "sh", 0 },
  { 0, 0 }
};

int num_paths = 0, max_paths = 0;
char **paths = 0;

void
add_path (char *s, int maxlen)
{
  if (num_paths >= max_paths)
    {
      max_paths += 10;
      if (paths)
	paths = (char **) realloc (paths, max_paths * sizeof (char *));
      else
	paths = (char **) malloc (max_paths * sizeof (char *));
    }
  paths[num_paths] = (char *) malloc (maxlen + 1);
  memcpy (paths[num_paths], s, maxlen);
  paths[num_paths][maxlen] = 0;
  char *e = paths[num_paths] + strlen (paths[num_paths]);
  if (e[-1] == '\\' && e[-2] != ':')
    *--e = 0;
  for (int i = 1; i < num_paths; i++)
    if (strcasecmp (paths[num_paths], paths[i]) == 0)
      return;
  num_paths++;
}

void
init_paths ()
{
  char tmp[4000], *sl;
  add_path ((char *) ".", 1);		/* to be replaced later */
  add_path ((char *) ".", 1);		/* the current directory */
  GetSystemDirectory (tmp, 4000);
  add_path (tmp, strlen (tmp));
  sl = strrchr (tmp, '\\');
  if (sl)
    {
      strcpy (sl, "\\SYSTEM");
      add_path (tmp, strlen (tmp));
    }
  GetWindowsDirectory (tmp, 4000);
  add_path (tmp, strlen (tmp));

  char *path = getenv ("PATH");
  if (path)
    {
      char wpath[4000];
      cygwin_posix_to_win32_path_list (path, wpath);
      char *b, *e, sep = ':';
      if (strchr (wpath, ';'))
	sep = ';';
      b = wpath;
      while (1)
	{
	  for (e = b; *e && *e != sep; e++);
	  add_path (b, e - b);
	  if (!*e)
	    break;
	  b = e + 1;
	}
    }
  else
    printf ("WARNING: PATH is not set at all!\n");
}

char *
find_on_path (char *file, char *default_extension,
	      int showall = 0, int search_sysdirs = 0)
{
  static char rv[4000];
  char tmp[4000], *ptr = rv;

  if (strchr (file, ':') || strchr (file, '\\') || strchr (file, '/'))
    return file;

  if (strchr (file, '.'))
    default_extension = (char *)"";

  for (int i = 0; i < num_paths; i++)
    {
      if (!search_sysdirs && (i == 0 || i == 2 || i == 3))
	continue;
      if (i == 0 || !search_sysdirs || strcasecmp (paths[i], paths[0]))
	{
	  sprintf (ptr, "%s\\%s%s", paths[i], file, default_extension);
	  if (GetFileAttributes (ptr) != (DWORD) -1)
	    {
	      if (showall)
		printf ("Found: %s\n", ptr);
	      if (ptr == tmp && verbose)
		printf ("Warning: %s hides %s\n", rv, ptr);
	      ptr = tmp;
	    }
	}
    }

  if (ptr == tmp)
    return rv;

  return 0;
}

#define DID_NEW		1
#define DID_ACTIVE	2
#define DID_INACTIVE	3

struct Did
{
  Did *next;
  char *file;
  int state;
};
Did *did = 0;

Did *
already_did (char *file)
{
  Did *d;
  for (d = did; d; d = d->next)
    if (strcasecmp (d->file, file) == 0)
      return d;
  d = new Did;
  d->file = strdup (file);
  d->next = did;
  d->state = DID_NEW;
  did = d;
  return d;
}

int
get_word (HANDLE fh, int offset)
{
  short rv;
  unsigned r;
  SetFilePointer (fh, offset, 0, FILE_BEGIN);
  ReadFile (fh, &rv, 2, (DWORD *) &r, 0);
  return rv;
}

int
get_dword (HANDLE fh, int offset)
{
  int rv;
  unsigned r;
  SetFilePointer (fh, offset, 0, FILE_BEGIN);
  ReadFile (fh, &rv, 4, (DWORD *) &r, 0);
  return rv;
}

struct Section
{
  char name[8];
  int virtual_size;
  int virtual_address;
  int size_of_raw_data;
  int pointer_to_raw_data;
};

int
rva_to_offset (int rva, char *sections, int nsections, int *sz)
{
  int i;
  for (i = 0; i < nsections; i++)
    {
      Section *s = (Section *) (sections + i * 40);
#if 0
      printf ("%08x < %08x < %08x ? %08x\n",
	      s->virtual_address, rva,
	      s->virtual_address + s->virtual_size, s->pointer_to_raw_data);
#endif
      if (rva >= s->virtual_address
	  && rva < s->virtual_address + s->virtual_size)
	{
	  if (sz)
	    *sz = s->virtual_address + s->virtual_size - rva;
	  return rva - s->virtual_address + s->pointer_to_raw_data;
	}
    }
  return 0;			/* punt */
}

struct ExpDirectory
{
  int flags;
  int timestamp;
  short major_ver;
  short minor_ver;
  int name_rva;
};

struct ImpDirectory
  {
    unsigned characteristics;
    unsigned timestamp;
    unsigned forwarder_chain;
    unsigned name_rva;
    unsigned iat_rva;
  };


void track_down (char *file, char *suffix, int lvl);

#define CYGPREFIX (sizeof ("%%% Cygwin ") - 1)
static void
cygwin_info (HANDLE h)
{
  char *buf, *bufend;
  char *major, *minor;
  const char *hello = "    Cygwin DLL version info:\n";
  DWORD size = GetFileSize (h, NULL);
  DWORD n;

  if (size == 0xffffffff)
    return;

  buf = (char *) malloc (size);
  if (!buf)
    return;

  (void) SetFilePointer (h, 0, NULL, FILE_BEGIN);
  if (!ReadFile (h, buf, size, &n, NULL))
    return;

  bufend = buf + size;
  major = minor = NULL;
  while (buf < bufend)
    if ((buf = (char *) memchr (buf, '%', bufend - buf)) == NULL)
	break;
    else if (strncmp ("%%% Cygwin ", buf, CYGPREFIX) != 0)
	buf++;
    else
      {
	char *p = strchr (buf += CYGPREFIX, '\n');
	fputs (hello, stdout);
	fputs ("        ", stdout);
	fwrite (buf, 1 + p - buf, 1, stdout);
	hello = "";
      }

  if (!*hello)
    puts ("");
  return;
}

void
dll_info (const char *path, HANDLE fh, int lvl, int recurse)
{
  DWORD junk;
  int i;
  int pe_header_offset = get_dword (fh, 0x3c);
  int opthdr_ofs = pe_header_offset + 4 + 20;
  unsigned short v[6];
  SetFilePointer (fh, opthdr_ofs + 40, 0, FILE_BEGIN);
  ReadFile (fh, &v, sizeof (v), &junk, 0);
  if (verbose)
    printf (" - os=%d.%d img=%d.%d sys=%d.%d\n",
	    v[0], v[1], v[2], v[3], v[4], v[5]);
  else
    printf ("\n");
  int num_entries = get_dword (fh, opthdr_ofs + 92);
  int export_rva = get_dword (fh, opthdr_ofs + 96);
  int export_size = get_dword (fh, opthdr_ofs + 100);
  int import_rva = get_dword (fh, opthdr_ofs + 104);
  int import_size = get_dword (fh, opthdr_ofs + 108);

  int nsections = get_word (fh, pe_header_offset + 4 + 2);
  char *sections = (char *) malloc (nsections * 40);
  SetFilePointer (fh, pe_header_offset + 4 + 20 + get_word (fh, pe_header_offset + 4 + 16),
		  0, FILE_BEGIN);
  ReadFile (fh, sections, nsections * 40, &junk, 0);

  if (verbose && num_entries >= 1 && export_size > 0)
    {
      int expsz;
      int expbase = rva_to_offset (export_rva, sections, nsections, &expsz);
      if (expbase)
	{
	  SetFilePointer (fh, expbase, 0, FILE_BEGIN);
	  unsigned char *exp = (unsigned char *) malloc (expsz);
	  ReadFile (fh, exp, expsz, &junk, 0);
	  ExpDirectory *ed = (ExpDirectory *) exp;
	  int ofs = ed->name_rva - export_rva;
	  struct tm *tm = localtime ((const time_t *) &(ed->timestamp));
	  if (tm->tm_year < 60)
	    tm->tm_year += 2000;
	  if (tm->tm_year < 200)
	    tm->tm_year += 1900;
	  printf ("%*c", lvl + 2, ' ');
	  printf ("\"%s\" v%d.%d ts=", exp + ofs,
		  ed->major_ver, ed->minor_ver);
	  printf ("%d/%d/%d %d:%02d\n",
		  tm->tm_year, tm->tm_mon + 1, tm->tm_mday,
		  tm->tm_hour, tm->tm_min);
	}
    }

  if (num_entries >= 2 && import_size > 0 && recurse)
    {
      int impsz;
      int impbase = rva_to_offset (import_rva, sections, nsections, &impsz);
      if (impbase)
	{
	  SetFilePointer (fh, impbase, 0, FILE_BEGIN);
	  unsigned char *imp = (unsigned char *) malloc (impsz);
	  ReadFile (fh, imp, impsz, &junk, 0);
	  ImpDirectory *id = (ImpDirectory *) imp;
	  for (i = 0; id[i].name_rva; i++)
	    {
	      /* int ofs = id[i].name_rva - import_rva; */
	      track_down ((char *) imp + id[i].name_rva - import_rva,
			  (char *) ".dll", lvl + 2);
	    }
	}
    }
  if (strstr (path, "\\cygwin1.dll"))
    cygwin_info (fh);
}

void
track_down (char *file, char *suffix, int lvl)
{
  char *path = find_on_path (file, suffix, 0, 1);
  if (!path)
    {
      printf ("Error: could not find %s\n", file);
      return;
    }

  Did *d = already_did (file);
  switch (d->state)
    {
    case DID_NEW:
      break;
    case DID_ACTIVE:
      if (verbose)
	{
	  if (lvl)
	    printf ("%*c", lvl, ' ');
	  printf ("%s", path);
	  printf (" (recursive)\n");
	}
      return;
    case DID_INACTIVE:
      if (verbose)
	{
	  if (lvl)
	    printf ("%*c", lvl, ' ');
	  printf ("%s", path);
	  printf (" (already done)\n");
	}
      return;
    }

  if (lvl)
    printf ("%*c", lvl, ' ');

  if (!path)
    {
      printf ("%s not found\n", file);
      return;
    }

  printf ("%s", path);

  HANDLE fh = CreateFile (path, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE,
			  NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
  if (fh == INVALID_HANDLE_VALUE)
    {
      printf (" - Cannot open\n");
      return;
    }

  d->state = DID_ACTIVE;

  dll_info (path, fh, lvl, 1);
  d->state = DID_INACTIVE;
  CloseHandle (fh);
}

void
ls (char *f)
{
  HANDLE h = CreateFile (f, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE,
			 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
  BY_HANDLE_FILE_INFORMATION info;
  GetFileInformationByHandle (h, &info);
  SYSTEMTIME systime;
  FileTimeToSystemTime (&info.ftLastWriteTime, &systime);
  printf ("%5dk %04d/%02d/%02d %s",
	  (((int) info.nFileSizeLow) + 512) / 1024,
	  systime.wYear, systime.wMonth, systime.wDay,
	  f);
  dll_info (f, h, 16, 0);
  CloseHandle (h);

}

void
cygcheck (char *app)
{
  char *papp = find_on_path (app, (char *) ".exe", 1, 0);
  if (!papp)
    {
      printf ("Error: could not find %s\n", app);
      return;
    }
  char *s = strdup (papp);
  char *sl = 0, *t;
  for (t = s; *t; t++)
    if (*t == '/' || *t == '\\' || *t == ':')
      sl = t;
  if (sl == 0)
    paths[0] = (char *) ".";
  else
    {
      *sl = 0;
      paths[0] = s;
    }
  did = 0;
  track_down (papp, (char *) ".exe", 0);
}


extern char **environ;

struct RegInfo
  {
    RegInfo *prev;
    char *name;
    HKEY key;
  };

void
show_reg (RegInfo * ri, int nest)
{
  if (!ri)
    return;
  show_reg (ri->prev, 1);
  if (nest)
    printf ("%s\\", ri->name);
  else
    printf ("%s\n", ri->name);
}

void
scan_registry (RegInfo * prev, HKEY hKey, char *name, int cygnus)
{
  RegInfo ri;
  ri.prev = prev;
  ri.name = name;
  ri.key = hKey;

  char *cp;
  for (cp = name; *cp; cp++)
    if (strncasecmp (cp, "cygnus", 6) == 0)
      cygnus = 1;

  DWORD num_subkeys, max_subkey_len, num_values;
  DWORD max_value_len, max_valdata_len, i;
  if (RegQueryInfoKey (hKey, 0, 0, 0, &num_subkeys, &max_subkey_len, 0,
		       &num_values, &max_value_len, &max_valdata_len, 0, 0)
      != ERROR_SUCCESS)
    {
#if 0
      char tmp[400];
      FormatMessage (FORMAT_MESSAGE_FROM_SYSTEM, 0, GetLastError (),
		     MAKELANGID (LANG_NEUTRAL, SUBLANG_DEFAULT), tmp,
		     400, 0);
      printf ("RegQueryInfoKey: %s\n", tmp);
#endif
      return;
    }

  if (cygnus)
    {
      show_reg (&ri, 0);
      char *value_name = (char *) malloc (max_value_len + 1);
      char *value_data = (char *) malloc (max_valdata_len + 1);

      for (i = 0; i < num_values; i++)
	{
	  DWORD dlen = max_valdata_len + 1;
	  DWORD nlen = max_value_len + 1;
	  DWORD type;
	  RegEnumValue (hKey, i, value_name, &nlen, 0,
			&type, (BYTE *) value_data, &dlen);
	  {
	    printf ("  %s = ", i ? value_name : "(default)");
	    switch (type)
	      {
	      case REG_DWORD:
		printf ("0x%08x\n", *(unsigned *) value_data);
		break;
	      case REG_EXPAND_SZ:
	      case REG_SZ:
		printf ("`%s'\n", value_data);
		break;
	      default:
		printf ("(unsupported type)\n");
		break;
	      }
	  }
#if 0
        else
          {
	    char tmp[400];
	    FormatMessage (FORMAT_MESSAGE_FROM_SYSTEM, 0, GetLastError (),
			   MAKELANGID (LANG_NEUTRAL, SUBLANG_DEFAULT), tmp,
			   400, 0);
	    printf ("RegEnumValue: %s\n", tmp);
	  }
#endif
	}
      free (value_name);
      free (value_data);
    }

  char *subkey_name = (char *) malloc (max_subkey_len + 1);
  for (i = 0; i < num_subkeys; i++)
    {
      if (RegEnumKey (hKey, i, subkey_name, max_subkey_len + 1) == ERROR_SUCCESS)
	{
	  HKEY sKey;
	  if (RegOpenKeyEx (hKey, subkey_name, 0, KEY_ALL_ACCESS, &sKey)
	      == ERROR_SUCCESS)
	    {
	      scan_registry (&ri, sKey, subkey_name, cygnus);
	      RegCloseKey (sKey);
	    }
	}
    }
  free (subkey_name);
}

void
dump_sysinfo ()
{
  int i, j;
  char tmp[4000];
  time_t now;
  char *found_cygwin_dll;

  printf ("\nCygnus Win95/NT Configuration Diagnostics\n");
  time (&now);
  printf ("Current System Time: %s\n", ctime (&now));

  OSVERSIONINFO osversion;
  osversion.dwOSVersionInfoSize = sizeof (osversion);
  GetVersionEx (&osversion);
  char *osname = (char *) "unknown OS";
  switch (osversion.dwPlatformId)
    {
    case VER_PLATFORM_WIN32s:
      osname = (char *) "win32s";
      break;
    case VER_PLATFORM_WIN32_WINDOWS:
      switch (osversion.dwMinorVersion)
	{
	case 0:
	  osname = (char *) "Win95";
	  break;
	case 1:
	  osname = (char *) "Win98";
	  break;
	default:
	  osname = (char *) "Win9X";
	  break;
	}
      break;
    case VER_PLATFORM_WIN32_NT:
      osname = (char *) "WinNT";
      break;
    default:
      osname = (char *) "uknown-os";
      break;
    }
  printf ("%s Ver %d.%d build %d %s\n\n", osname,
	  (int) osversion.dwMajorVersion, (int) osversion.dwMinorVersion,
	  (int) osversion.dwBuildNumber, osversion.szCSDVersion);

  printf ("Path:");
  char *s = getenv ("PATH"), *e;
  char sep = strchr (s, ';') ? ';' : ':';
  int count_path_items = 0;
  while (1)
    {
      for (e = s; *e && *e != sep; e++);
      printf ("\t%.*s\n", e - s, s);
      count_path_items++;
      if (!*e)
	break;
      s = e + 1;
    }

  GetSystemDirectory (tmp, 4000);
  printf ("\nSysDir: %s\n", tmp);
  GetWindowsDirectory (tmp, 4000);
  printf ("WinDir: %s\n\n", tmp);


  if (givehelp)
    printf ("Here's some environment variables that may affect cygwin:\n");
  for (i = 0; environ[i]; i++)
    {
      char *eq = strchr (environ[i], '=');
      if (!eq)
	continue;
      /* int len = eq - environ[i]; */
      for (j = 0; known_env_vars[j]; j++)
	{
	  *eq = 0;
	  if (strcmp (environ[i], "PATH") == 0)
	    continue;		/* we handle this one specially */
	  if (strcasecmp (environ[i], known_env_vars[j]) == 0)
	    printf ("%s = `%s'\n", environ[i], eq + 1);
	  *eq = '=';
	}
    }
  printf ("\n");

  if (verbose)
    {
      if (givehelp)
	printf ("Here's the rest of your environment variables:\n");
      for (i = 0; environ[i]; i++)
	{
	  int found = 0;
	  char *eq = strchr (environ[i], '=');
	  if (!eq)
	    continue;
	  /* int len = eq - environ[i]; */
	  for (j = 0; known_env_vars[j]; j++)
	    {
	      *eq = 0;
	      if (strcasecmp (environ[i], known_env_vars[j]) == 0)
		found = 1;
	      *eq = '=';
	    }
	  if (!found)
	    {
	      *eq = 0;
	      printf ("%s = `%s'\n", environ[i], eq + 1);
	      *eq = '=';
	    }
	}
      printf ("\n");
    }

  if (registry)
    {
      if (givehelp)
	printf ("Scanning registry for keys with `Cygnus' in them...\n");
#if 0
      /* big and not generally useful */
      scan_registry (0, HKEY_CLASSES_ROOT, (char *) "HKEY_CLASSES_ROOT", 0);
#endif
      scan_registry (0, HKEY_CURRENT_CONFIG,
		     (char *) "HKEY_CURRENT_CONFIG", 0);
      scan_registry (0, HKEY_CURRENT_USER, (char *) "HKEY_CURRENT_USER", 0);
      scan_registry (0, HKEY_LOCAL_MACHINE, (char *) "HKEY_LOCAL_MACHINE", 0);
#if 0
      /* the parts we need are duplicated in HKEY_CURRENT_USER anyway */
      scan_registry (0, HKEY_USERS, (char *) "HKEY_USERS", 0);
#endif
      printf ("\n");
    }
  else
    printf ("Use `-r' to scan registry\n\n");

  if (givehelp)
    {
      printf ("Listing available drives...\n");
      printf ("Drv Type        Size   Free Flags              Name\n");
    }
  int prev_mode = SetErrorMode (SEM_FAILCRITICALERRORS | SEM_NOOPENFILEERRORBOX);
  int drivemask = GetLogicalDrives ();

  HINSTANCE k32 = LoadLibrary ("kernel32.dll");
  BOOL (WINAPI *gdfse) (LPCSTR, long long *, long long *, long long *) =
    (BOOL (WINAPI *) (LPCSTR, long long *, long long *, long long *))
      GetProcAddress (k32, "GetDiskFreeSpaceExA");

  for (i = 0; i < 26; i++)
    {
      if (!(drivemask & (1 << i)))
	continue;
      char drive[4], name[200], fsname[200];
      DWORD serno = 0, maxnamelen = 0, flags = 0;
      name[0] = name[0] = fsname[0] = 0;
      sprintf (drive, "%c:\\", i + 'a');
      GetVolumeInformation (drive, name, sizeof (name), &serno, &maxnamelen,
			    &flags, fsname, sizeof (fsname));

      int dtype = GetDriveType (drive);
      char drive_type[4] = "unk";
      switch (dtype)
	{
	case DRIVE_REMOVABLE:
	  strcpy (drive_type, "fd ");
	  break;
	case DRIVE_FIXED:
	  strcpy (drive_type, "hd ");
	  break;
	case DRIVE_REMOTE:
	  strcpy (drive_type, "net");
	  break;
	case DRIVE_CDROM:
	  strcpy (drive_type, "cd ");
	  break;
	case DRIVE_RAMDISK:
	  strcpy (drive_type, "ram");
	  break;
	}

      long capacity_mb = -1;
      int percent_full = -1;

      long long free_me = 0ULL, free_bytes = 0ULL, total_bytes = 1ULL;
      if (gdfse != NULL
	  && gdfse (drive, & free_me, & total_bytes, & free_bytes))
	{
	  capacity_mb = total_bytes / (1024L * 1024L);
	  percent_full = 100 - (int) ((100.0 * free_me) / total_bytes);
	}
      else
	{
	  DWORD spc = 0, bps = 0, fc = 0, tc = 1;
	  if (GetDiskFreeSpace (drive, &spc, &bps, &fc, &tc))
	    {
	      capacity_mb = (spc * bps * tc) / (1024 * 1024);
	      percent_full = 100 - (int) ((100.0 * fc) / tc);
	    }
	}

      printf ("%.2s  %s %-6s ", drive, drive_type, fsname);
      if (capacity_mb >= 0)
	printf ("%5dMb %3d%% ", (int) capacity_mb, (int) percent_full);
      else
	printf ("  N/A    N/A ");
      printf ("%s %s %s %s %s %s  %s\n",
	      flags & FS_CASE_IS_PRESERVED ? "CP" : "  ",
	      flags & FS_CASE_SENSITIVE ? "CS" : "  ",
	      flags & FS_UNICODE_STORED_ON_DISK ? "UN" : "  ",
	      flags & FS_PERSISTENT_ACLS ? "PA" : "  ",
	      flags & FS_FILE_COMPRESSION ? "FC" : "  ",
	      flags & FS_VOL_IS_COMPRESSED ? "VC" : "  ",
#if 0
	      flags & FILE_SUPPORTS_ENCRYPTION ? "EN" : "  ",
	      flags & FILE_SUPPORTS_OBJECT_IDS ? "OI" : "  ",
	      flags & FILE_SUPPORTS_REPARSE_POINTS ? "RP" : "  ",
	      flags & FILE_SUPPORTS_SPARSE_FILES ? "SP" : "  ",
	      flags & FILE_VOLUME_QUOTAS ? "QU" : "  ",
#endif
	      name);
    }

  FreeLibrary (k32);
  SetErrorMode (prev_mode);
  if (givehelp)
    {
      printf ("fd=floppy, hd=hard drive, cd=CD-ROM, net=Network Share\n");
      printf ("CP=Case Preserving, CS=Case Sensitive, UN=Unicode\n");
      printf ("PA=Persistent ACLS, FC=File Compression, VC=Volume Compression\n");
    }
  printf ("\n");

  unsigned int ml_fsname = 4, ml_dir = 7, ml_type = 6;

  if (givehelp)
    {
      printf ("Mount entries: these map POSIX directories to your NT drives.\n");
      printf ("%-*s  %-*s  %-*s  %s\n",
	      ml_fsname, "-NT-",
	      ml_dir, "-POSIX-",
	      ml_type, "-Type-", "-Flags-");
    }

  struct mntent *mnt;
  setmntent (0, 0);

  while ((mnt = getmntent (0)))
    {
      printf ("%-*s  %-*s  %-*s  %s\n",
	      ml_fsname, mnt->mnt_fsname,
	      ml_dir, mnt->mnt_dir,
	      ml_type, mnt->mnt_type,
	      mnt->mnt_opts);
    }
  printf ("\n");

  add_path ((char *) "\\bin", 4);	/* just in case */

  if (givehelp)
    printf ("Looking to see where common programs can be found, if at all...\n");
  for (i = 0; common_apps[i].name; i++)
    if (!find_on_path ((char *) common_apps[i].name, (char *) ".exe", 1, 0))
    {
      if (common_apps[i].missing_is_good)
        printf ("Not Found: %s (good!)\n", common_apps[i].name);
      else
        printf ("Not Found: %s\n", common_apps[i].name);
    }
  printf ("\n");

  if (givehelp)
    printf ("Looking for various Cygnus DLLs...  (-v gives version info)\n");
  for (i = 0; i < num_paths; i++)
    {
      WIN32_FIND_DATA ffinfo;
      sprintf (tmp, "%s/*.*", paths[i]);
      HANDLE ff = FindFirstFile (tmp, &ffinfo);
      int found = (ff != INVALID_HANDLE_VALUE);
      found_cygwin_dll = NULL;
      while (found)
	{
	  char *f = ffinfo.cFileName;
	  if (strcasecmp (f + strlen (f) - 4, ".dll") == 0)
	    {
	      if (strncasecmp (f, "cyg", 3) == 0)
		{
		  sprintf (tmp, "%s\\%s", paths[i], f);
		  if (strcasecmp (f, "cygwin1.dll") == 0)
		    found_cygwin_dll = strdup (tmp);
		  else
		    ls (tmp);
		}
	    }
	  found = FindNextFile (ff, &ffinfo);
	}
      if (found_cygwin_dll)
	{
	  ls (found_cygwin_dll);
	  free (found_cygwin_dll);
	}

      FindClose (ff);
    }
}

void
usage ()
{
  fprintf (stderr, "Usage: cygcheck [-s] [-v] [-r] [-h] [program ...]\n");
  fprintf (stderr, "  -s = system information\n");
  fprintf (stderr, "  -v = verbose output (indented) (for -s or programs)\n");
  fprintf (stderr, "  -r = registry search (requires -s)\n");
  fprintf (stderr, "  -h = give help about the info\n");
  fprintf (stderr, "You must at least give either -s or a program name\n");
  exit (1);
}

int
main (int argc, char **argv)
{
  int i;
  while (argc > 1 && argv[1][0] == '-')
    {
      if (strcmp (argv[1], "-v") == 0)
	verbose = 1;
      if (strcmp (argv[1], "-r") == 0)
	registry = 1;
      if (strcmp (argv[1], "-s") == 0)
	sysinfo = 1;
      if (strcmp (argv[1], "-h") == 0)
	givehelp = 1;
      argc--;
      argv++;
    }

  if (argc == 1 && !sysinfo)
    usage ();

  init_paths ();

  if (argc > 1 && givehelp)
    {
      if (argc == 2)
	{
	  printf ("Here is where the OS will find your program, and which dlls\n");
	  printf ("will be used for it.  Use -v to see DLL version info\n");
	}
      else
	{
	  printf ("Here is where the OS will find your programs, and which dlls\n");
	  printf ("will be used for them.  Use -v to see DLL version info\n");
	}

      if (!sysinfo)
	printf ("\n");
    }

  for (i = 1; i < argc; i++)
    {
      cygcheck (argv[i]);
      printf ("\n");
    }

  if (sysinfo)
    dump_sysinfo ();

  if (!givehelp)
    printf ("Use -h to see help about each section\n");

  return 0;
}
