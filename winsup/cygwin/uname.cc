/* uname.cc

   Copyright 1995, 1996, 1997, 1998, 1999, 2000, 2001, 2002, 2003, 2004,
   2005, 2006, 2007 Red Hat, Inc.
   Written by Steve Chamberlain of Cygnus Support, sac@cygnus.com
   Rewritten by Geoffrey Noer of Cygnus Solutions, noer@cygnus.com

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

#include "winsup.h"
#include <sys/utsname.h>
#include "cygwin_version.h"
#include "cygtls.h"

/* uname: POSIX 4.4.1.1 */
extern "C" int
uname (struct utsname *name)
{
  SYSTEM_INFO sysinfo;

  myfault efault;
  if (efault.faulted (EFAULT))
    return -1;

  char *snp = strstr  (cygwin_version.dll_build_date, "SNP");

  memset (name, 0, sizeof (*name));
  __small_sprintf (name->sysname, "CYGWIN_%s", wincap.osname ());

#if 0
  /* Recognition of the real 64 bit CPU inside of a WOW64 system, irritates
     build systems which think the native system is a 64 bit system.  Since
     we're actually running in a 32 bit environment, it looks more correct
     just to use the CPU info given by WOW64. */
  if (wincap.is_wow64 ())
    GetNativeSystemInfo (&sysinfo);
  else
#else
  /* But it seems ok to add a hint to the sysname, that we're running under
     WOW64.  This might give an early clue if somebody encounters problems. */
  if (wincap.is_wow64 ())
    strncat (name->sysname, "-WOW64",
	     sizeof name->sysname - strlen (name->sysname) - 1);
#endif
    GetSystemInfo (&sysinfo);

  /* Computer name */
  cygwin_gethostname (name->nodename, sizeof (name->nodename) - 1);

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
	if (sysinfo.wProcessorLevel < 3) /* Shouldn't happen. */
	  ptype = 3;
	else if (sysinfo.wProcessorLevel > 9) /* P4 */
	  ptype = 6;
	else
	  ptype = sysinfo.wProcessorLevel;
	__small_sprintf (name->machine, "i%d86", ptype);
	break;
      case PROCESSOR_ARCHITECTURE_IA64:
	strcpy (name->machine, "ia64");
	break;
      case PROCESSOR_ARCHITECTURE_AMD64:
	strcpy (name->machine, "x86_64");
	break;
      case PROCESSOR_ARCHITECTURE_IA32_ON_WIN64:
	strcpy (name->machine, "ia32-win64");
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
