/* uname.cc

   Copyright 1996, 1997, 1998, 1999, 2000, 2001 Red Hat, Inc.
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

  if (check_null_invalid_struct_errno (name))
    return -1;

  char *snp = strstr  (cygwin_version.dll_build_date, "SNP");

  memset (name, 0, sizeof (*name));
  __small_sprintf (name->sysname, "CYGWIN_%s", wincap.osname ());

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
	unsigned int ptype;
	if (wincap.has_valid_processorlevel ())
	  {
	    if (sysinfo.wProcessorLevel < 3) /* Shouldn't happen. */
	      ptype = 3;
	    else if (sysinfo.wProcessorLevel > 9) /* P4 */
	      ptype = 6;
	    else
	      ptype = sysinfo.wProcessorLevel;
	  }
	else
	  {
	    if (sysinfo.dwProcessorType == PROCESSOR_INTEL_386 ||
	        sysinfo.dwProcessorType == PROCESSOR_INTEL_486)
	      ptype = sysinfo.dwProcessorType / 100;
	    else
	      ptype = PROCESSOR_INTEL_PENTIUM / 100;
	  }
	__small_sprintf (name->machine, "i%d86", ptype);
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
