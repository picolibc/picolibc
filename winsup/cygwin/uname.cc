/* uname.cc

   Copyright 1996, 1997, 1998, 1999, 2000 Cygnus Solutions.
   Written by Steve Chamberlain of Cygnus Support, sac@cygnus.com
   Rewritten by Geoffrey Noer of Cygnus Solutions, noer@cygnus.com

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

#include "winsup.h"
#include <stdio.h>
#include <sys/utsname.h>
#include "cygwin_version.h"

/* uname: POSIX 4.4.1.1 */
extern "C" int
uname (struct utsname *name)
{
  DWORD len;
  SYSTEM_INFO sysinfo;
  extern char osname[];
  char *snp = strstr  (cygwin_version.dll_build_date, "SNP");

  memset (name, 0, sizeof (*name));
  __small_sprintf (name->sysname, "CYGWIN_%s", osname);

  GetSystemInfo (&sysinfo);

  /* Computer name */
  len = sizeof (name->nodename) - 1;
  GetComputerNameA (name->nodename, &len);

  /* Cygwin dll release */
  __small_sprintf (name->release, "%d.%d.%d%s(%d.%d/%d/%d)",
		   cygwin_version.dll_major / 1000,
		   cygwin_version.dll_major % 1000,
		   cygwin_version.dll_minor,
		   snp ? "s" : "",
		   cygwin_version.api_major,
		   cygwin_version.api_minor,
		   cygwin_version.shared_data,
		   cygwin_version.mount_registry);

  /* Cygwin "version" aka build date */
  strcpy (name->version, cygwin_version.dll_build_date);
  if (snp)
    name->version[snp - cygwin_version.dll_build_date] = '\0';

  /* CPU type */
  switch (sysinfo.wProcessorArchitecture)
    {
      case PROCESSOR_ARCHITECTURE_INTEL:
	/* But which of the x86 chips are we? */
	/* Default to i386 if the specific chip cannot be determined */
	switch (os_being_run)
	  {
	    case win95:
	    case win98:
	    case winME:
	      /* dwProcessorType only valid in Windows 95 */
	      if ((sysinfo.dwProcessorType == PROCESSOR_INTEL_386) ||
		  (sysinfo.dwProcessorType == PROCESSOR_INTEL_486) ||
		  (sysinfo.dwProcessorType == PROCESSOR_INTEL_PENTIUM))
		__small_sprintf (name->machine, "i%d", sysinfo.dwProcessorType);
	      else
		strcpy (name->machine, "i386");
	      break;
	    case winNT:
	      /* wProcessorLevel only valid in Windows NT */
	      __small_sprintf (name->machine, "i%d86", sysinfo.wProcessorLevel);
	      break;
	    default:
	      strcpy (name->machine, "i386");
	      break;
	  }
	break;
      case PROCESSOR_ARCHITECTURE_ALPHA:
	strcpy (name->machine, "alpha");
	break;
      case PROCESSOR_ARCHITECTURE_MIPS:
	strcpy (name->machine, "mips");
	break;
      default:
	strcpy (name->machine, "unknown");
	break;
    }

  return 0;
}
