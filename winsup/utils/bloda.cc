/* bloda.cc

   This file is part of Cygwin.

   This software is a copyrighted work licensed under the terms of the
   Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
   details. */

#define cygwin_internal cygwin_internal_dontuse
#include <stdio.h>
#include <assert.h>
#define WIN32_NO_STATUS	/* Disable status codes in winnt.h since we include
			   ntstatus.h for extended status codes below. */
#include <windows.h>
#undef WIN32_NO_STATUS
#include <psapi.h>
#include <winternl.h>
#include <ntstatus.h>
#undef cygwin_internal

#undef DEBUGGING
#ifdef DEBUGGING
#define dbg_printf(ARGS) printf ARGS ; fflush (NULL)
#else  /* !DEBUGGING */
#define dbg_printf(ARGS) do { } while (0)
#endif /* ?DEBUGGING */

/*  This module detects applications from the Big List of Dodgy Apps,
  a list of applications that have at some given time been shown to
  interfere with the operation of cygwin.  It detects the presence of
  applications on the system by looking for any of four traces an
  installation might leave: 1) registry keys, 2) files on disk
  3) running executables 4) loaded dlls or drivers.

  At the time of writing, the BLODA amounts to:-

    Sonic Solutions burning software containing DLA component
    Norton/MacAffee/Symantec antivirus or antispyware
    Logitech webcam software with "Logitech process monitor" service
    Kerio, Agnitum or ZoneAlarm Personal Firewall
    Iolo System Mechanic/AntiVirus/Firewall
    LanDesk
    Windows Defender
    Embassy Trust Suite fingerprint reader software containing wxvault.dll
    ByteMobile laptop optimization client

  A live version is now being maintained in the Cygwin FAQ, at
    http://cygwin.com/faq/faq.using.html#faq.using.bloda

*/

enum bad_app
{
  SONIC,    NORTON,  MACAFFEE,    SYMANTEC,
  LOGITECH, KERIO,   AGNITUM,     ZONEALARM,
  IOLO,     LANDESK, WINDEFENDER, EMBASSYTS,
  BYTEMOBILE
};

struct bad_app_info
{
  enum bad_app app_id;
  const char *details;
  char found_it;
};

enum bad_app_det_method
{
  HKLMKEY, HKCUKEY, FILENAME, PROCESSNAME, HOOKDLLNAME
};

struct bad_app_det
{
  enum bad_app_det_method type;
  const char *param;
  enum bad_app app;
};

static const struct bad_app_det dodgy_app_detects[] =
{
  { PROCESSNAME, "dlactrlw.exe",                                                 SONIC      },
  { HOOKDLLNAME, "wxvault.dll",                                                  EMBASSYTS  },
  { HKLMKEY,     "SYSTEM\\CurrentControlSet\\Services\\vsdatant",                ZONEALARM  },
  { FILENAME,    "%windir%\\System32\\vsdatant.sys",                             ZONEALARM  },
  { HKLMKEY,     "SYSTEM\\CurrentControlSet\\Services\\lvprcsrv",                LOGITECH   },
  { PROCESSNAME, "LVPrcSrv.exe",                                                 LOGITECH   },
  { FILENAME,    "%programfiles%\\common files\\logitech\\lvmvfm\\LVPrcSrv.exe", LOGITECH   },
  { FILENAME,    "%windir%\\System32\\bmnet.dll",                                BYTEMOBILE },
};

static const size_t num_of_detects = sizeof (dodgy_app_detects) / sizeof (dodgy_app_detects[0]);

static struct bad_app_info big_list_of_dodgy_apps[] =
{
  { SONIC,       "Sonic Solutions burning software containing DLA component"              },
  { NORTON,      "Norton antivirus or antispyware software"                               },
  { MACAFFEE,    "Macaffee antivirus or antispyware software"                             },
  { SYMANTEC,    "Symantec antivirus or antispyware software"                             },
  { LOGITECH,    "Logitech Process Monitor service"                                       },
  { KERIO,       "Kerio Personal Firewall"                                                },
  { AGNITUM,     "Agnitum Personal Firewall"                                              },
  { ZONEALARM,   "ZoneAlarm Personal Firewall"                                            },
  { IOLO,        "Iolo System Mechanic/AntiVirus/Firewall software"                       },
  { LANDESK,     "Landesk"                                                                },
  { WINDEFENDER, "Windows Defender"                                                       },
  { EMBASSYTS,   "Embassy Trust Suite fingerprint reader software containing wxvault.dll" },
  { BYTEMOBILE,  "ByteMobile laptop optimization client"                                  },
};

static const size_t num_of_dodgy_apps = sizeof (big_list_of_dodgy_apps) / sizeof (big_list_of_dodgy_apps[0]);

struct system_module_list
{
  LONG count;
  PVOID *pid;
  PCHAR *name;
};

static PSYSTEM_PROCESS_INFORMATION
get_process_list (void)
{
  int n_procs = 0x100;
  PSYSTEM_PROCESS_INFORMATION pslist = (PSYSTEM_PROCESS_INFORMATION) malloc (n_procs * sizeof *pslist);

  while (NtQuerySystemInformation (SystemProcessInformation,
    pslist, n_procs * sizeof *pslist, 0) == STATUS_INFO_LENGTH_MISMATCH)
    {
      n_procs *= 2;
      free (pslist);
      pslist = (PSYSTEM_PROCESS_INFORMATION) malloc (n_procs * sizeof *pslist);
    }
  return pslist;
}

static system_module_list *
get_module_list (void)
{
  DWORD modsize = 0;
  system_module_list *modlist = (system_module_list *)
				calloc (1, sizeof (system_module_list));
  while (!EnumDeviceDrivers (modlist->pid, modsize, &modsize))
    {
      free (modlist->pid);
      free (modlist->name);
      modlist->count = modsize / sizeof (PVOID);
      modlist->pid = (PVOID *) calloc (modlist->count, sizeof (PVOID));
      modlist->name = (PCHAR *) calloc (modlist->count, sizeof (PCHAR));
    }
  for (int i = 0; i < modlist->count; ++i)
    {
      modlist->name[0] = (PCHAR) calloc (256, sizeof (CHAR));
      GetDeviceDriverBaseNameA (modlist->pid[i], modlist->name[i], 256);
    }
  return modlist;
}

static bool
find_process_in_list (PSYSTEM_PROCESS_INFORMATION pslist, PUNICODE_STRING psname)
{
  while (1)
    {
      if (pslist->ImageName.Length && pslist->ImageName.Buffer)
	{
	  dbg_printf (("%S\n", pslist->ImageName.Buffer));
	  if (!_wcsicmp (pslist->ImageName.Buffer, psname->Buffer))
	    return true;
	}
      if (!pslist->NextEntryOffset)
	break;
      pslist = (PSYSTEM_PROCESS_INFORMATION)(pslist->NextEntryOffset + (char *)pslist);
    };
  return false;
}

static bool
find_module_in_list (system_module_list * modlist, const char * const modname)
{
  for (int i = 0; i < modlist->count; ++i)
    {
      dbg_printf (("name '%s' ", modlist->name[i]));
      if (!_stricmp (modlist->name[i], modname))
	return true;
    }
  return false;
}

static bool
expand_path (const char *path, char *outbuf)
{
  char *dst = outbuf;
  const char *end, *envval;
  char envvar[MAX_PATH];
  size_t len;

  while ((dst - outbuf) < MAX_PATH)
    {
      if (*path != '%')
	{
	  if ((*dst++ = *path++) != 0)
	    continue;
	  break;
	}
      /* Expand an environ var.  */
      end = path + 1;
      while (*end != '%')
	{
	  /* Watch out for unterminated %  */
	  if (*end++ == 0)
	    {
	      end = NULL;
	      break;
	    }
	}
      /* If we didn't find the end, can't expand it.  */
      if ((end == NULL) || (end == (path + 1)))
	{
	  /* Unterminated % so copy verbatim.  */
	  *dst++ = *path++;
	  continue;
	}
      /* Expand the environment var into the new path.  */
      if ((end - (path + 1)) >= MAX_PATH)
	return -1;
      memcpy (envvar, path + 1, end - (path + 1));
      envvar[end - (path + 1)] = 0;
      envval = getenv (envvar);
      /* If not found, copy env var name verbatim.  */
      if (envval == NULL)
	{
	  *dst++ = *path++;
	  continue;
	}
      /* Check enough room before copying.  */
      len = strlen (envval);
      if ((dst + len - outbuf) >= MAX_PATH)
	return false;
      memcpy (dst, envval, len);
      dst += len;
      /* And carry on past the end of env var name.  */
      path = end + 1;
    }
  return (dst - outbuf) < MAX_PATH;
}

static bool
detect_dodgy_app (const struct bad_app_det *det, PSYSTEM_PROCESS_INFORMATION pslist, system_module_list * modlist)
{
  HANDLE fh;
  HKEY hk;
  UNICODE_STRING unicodename;
  ANSI_STRING ansiname;
  NTSTATUS rv;
  bool found;
  char expandedname[MAX_PATH];

  switch (det->type)
    {
    case HKLMKEY:
      dbg_printf (("Detect reg key hklm '%s'... ", det->param));
      if (RegOpenKeyEx (HKEY_LOCAL_MACHINE, det->param, 0, STANDARD_RIGHTS_READ, &hk) == ERROR_SUCCESS)
	{
	  RegCloseKey (hk);
	  dbg_printf (("found!\n"));
	  return true;
	}
      break;

    case HKCUKEY:
      dbg_printf (("Detect reg key hkcu '%s'... ", det->param));
      if (RegOpenKeyEx (HKEY_CURRENT_USER, det->param, 0, STANDARD_RIGHTS_READ, &hk) == ERROR_SUCCESS)
	{
	  RegCloseKey (hk);
	  dbg_printf (("found!\n"));
	  return true;
	}
      break;

    case FILENAME:
      dbg_printf (("Detect filename '%s'... ", det->param));
      if (!expand_path (det->param, expandedname))
	{
	  printf ("Expansion failure!\n");
	  break;
	}
      dbg_printf (("('%s' after expansion)... ", expandedname));
      fh = CreateFile (expandedname, 0, FILE_SHARE_READ | FILE_SHARE_WRITE
	| FILE_SHARE_DELETE, NULL, OPEN_EXISTING, 0, NULL);
      if (fh != INVALID_HANDLE_VALUE)
	{
	  CloseHandle (fh);
	  dbg_printf (("found!\n"));
	  return true;
	}
      break;

    case PROCESSNAME:
      dbg_printf (("Detect proc name '%s'... ", det->param));
      /* Equivalent of RtlInitAnsiString.  */
      ansiname.Length = ansiname.MaximumLength = strlen (det->param);
      ansiname.Buffer = (CHAR *) det->param;
      rv = RtlAnsiStringToUnicodeString (&unicodename, &ansiname, TRUE);
      if (rv != STATUS_SUCCESS)
	{
	  printf ("Ansi to unicode conversion failure $%08x\n", (unsigned int) rv);
	  break;
	}
      found = find_process_in_list (pslist, &unicodename);
      RtlFreeUnicodeString (&unicodename);
      if (found)
	{
	  dbg_printf (("found!\n"));
	  return true;
	}
      break;

    case HOOKDLLNAME:
      dbg_printf (("Detect hookdll '%s'... ", det->param));
      if (find_module_in_list (modlist, det->param))
	{
	  dbg_printf (("found!\n"));
	  return true;
	}
      break;

    }
  dbg_printf (("not found.\n"));
  return false;
}

static struct bad_app_info *
find_dodgy_app_info (enum bad_app which_app)
{
  size_t i;
  for (i = 0; i < num_of_dodgy_apps; i++)
    {
      if (big_list_of_dodgy_apps[i].app_id == which_app)
	return &big_list_of_dodgy_apps[i];
    }
  return NULL;
}

/* External entrypoint called from cygcheck.cc/dump_sysinfo.  */
void
dump_dodgy_apps (int verbose)
{
  size_t i, n_det = 0;
  PSYSTEM_PROCESS_INFORMATION pslist;
  system_module_list * modlist;

  /* Read system info for detect testing.  */
  pslist = get_process_list ();
  modlist = get_module_list ();

  /* Go with builtin list for now; later may enhance to
  read dodgy apps from a file or download from an URL.  */
  for (i = 0; i < num_of_dodgy_apps; i++)
    {
      big_list_of_dodgy_apps[i].found_it = false;
    }

  for (i = 0; i < num_of_detects; i++)
    {
      const struct bad_app_det *det = &dodgy_app_detects[i];
      struct bad_app_info *found = find_dodgy_app_info (det->app);
      bool detected = detect_dodgy_app (det, pslist, modlist);

      /* Not found would mean we coded the lists bad. */
      assert (found);
      if (detected)
	{
	  ++n_det;
	  found->found_it |= (1 << det->type);
	}
    }
  if (n_det)
    {
      printf ("\nPotential app conflicts:\n\n");
      for (i = 0; i < num_of_dodgy_apps; i++)
	{
	  if (big_list_of_dodgy_apps[i].found_it)
	    {
	      printf ("%s%s", big_list_of_dodgy_apps[i].details,
		verbose ? "\nDetected: " : ".\n");
	      if (!verbose)
		continue;
	      const char *sep = "";
	      if (big_list_of_dodgy_apps[i].found_it & (1 << HKLMKEY))
		{
		  printf ("HKLM Registry Key");
		  sep = ", ";
		}
	      if (big_list_of_dodgy_apps[i].found_it & (1 << HKCUKEY))
		{
		  printf ("%sHKCU Registry Key", sep);
		  sep = ", ";
		}
	      if (big_list_of_dodgy_apps[i].found_it & (1 << FILENAME))
		{
		  printf ("%sNamed file", sep);
		  sep = ", ";
		}
	      if (big_list_of_dodgy_apps[i].found_it & (1 << PROCESSNAME))
		{
		  printf ("%sNamed process", sep);
		  sep = ", ";
		}
	      if (big_list_of_dodgy_apps[i].found_it & (1 << HOOKDLLNAME))
		{
		  printf ("%sLoaded hook DLL", sep);
		}
	      printf (".\n\n");
	    }
	}
    }
  /* Tidy up allocations.  */
  free (pslist);
  for (int i = 0; i < modlist->count; ++i)
    free (modlist->name[i]);
  free (modlist->name);
  free (modlist->pid);
}

