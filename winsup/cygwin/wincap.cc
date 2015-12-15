/* wincap.cc -- figure out on which OS we're running. Set the
		capability class to the appropriate values.

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

#include "winsup.h"
#include "security.h"
#include "ntdll.h"

/* CV, 2008-10-23: All wincapc's have to be in the .cygwin_dll_common section,
   same as wincap itself.  Otherwise the capability changes made in
   wincapc::init() are not propagated to any subsequently started process
   in the same session.  I'm only writing this longish comment because I'm
   puzzled that this has never been noticed before... */

wincaps wincap_vista __attribute__((section (".cygwin_dll_common"), shared)) = {
  def_guard_pages:1,
  {
    is_server:false,
    needs_count_in_si_lpres2:true,
    has_gaa_largeaddress_bug:true,
    has_broken_alloc_console:false,
    has_console_logon_sid:false,
    has_precise_system_time:false,
    has_microsoft_accounts:false,
    has_processor_groups:false,
    has_broken_prefetchvm:false,
    has_new_pebteb_region:false,
    has_broken_whoami:true,
  },
};

wincaps wincap_7 __attribute__((section (".cygwin_dll_common"), shared)) = {
  def_guard_pages:1,
  {
    is_server:false,
    needs_count_in_si_lpres2:false,
    has_gaa_largeaddress_bug:true,
    has_broken_alloc_console:true,
    has_console_logon_sid:true,
    has_precise_system_time:false,
    has_microsoft_accounts:false,
    has_processor_groups:true,
    has_broken_prefetchvm:false,
    has_new_pebteb_region:false,
    has_broken_whoami:true,
  },
};

wincaps wincap_8 __attribute__((section (".cygwin_dll_common"), shared)) = {
  def_guard_pages:2,
  {
    is_server:false,
    needs_count_in_si_lpres2:false,
    has_gaa_largeaddress_bug:false,
    has_broken_alloc_console:true,
    has_console_logon_sid:true,
    has_precise_system_time:true,
    has_microsoft_accounts:true,
    has_processor_groups:true,
    has_broken_prefetchvm:false,
    has_new_pebteb_region:false,
    has_broken_whoami:false,
  },
};

wincaps wincap_10 __attribute__((section (".cygwin_dll_common"), shared)) = {
  def_guard_pages:2,
  {
    is_server:false,
    needs_count_in_si_lpres2:false,
    has_gaa_largeaddress_bug:false,
    has_broken_alloc_console:true,
    has_console_logon_sid:true,
    has_precise_system_time:true,
    has_microsoft_accounts:true,
    has_processor_groups:true,
    has_broken_prefetchvm:true,
    has_new_pebteb_region:false,
    has_broken_whoami:false,
  },
};

wincaps wincap_10_1511 __attribute__((section (".cygwin_dll_common"), shared)) = {
  def_guard_pages:2,
  {
    is_server:false,
    needs_count_in_si_lpres2:false,
    has_gaa_largeaddress_bug:false,
    has_broken_alloc_console:true,
    has_console_logon_sid:true,
    has_precise_system_time:true,
    has_microsoft_accounts:true,
    has_processor_groups:true,
    has_broken_prefetchvm:false,
    has_new_pebteb_region:true,
    has_broken_whoami:false,
  },
};

wincapc wincap __attribute__((section (".cygwin_dll_common"), shared));

void
wincapc::init ()
{
  if (caps)
    return;		// already initialized

  GetSystemInfo (&system_info);
  version.dwOSVersionInfoSize = sizeof (RTL_OSVERSIONINFOEXW);
  RtlGetVersion (&version);
  /* Overwrite unreliable kernel version with correct values returned by
     RtlGetNtVersionNumbers.  See git log of this change for a description. */
  RtlGetNtVersionNumbers (&version.dwMajorVersion,
			  &version.dwMinorVersion,
			  &version.dwBuildNumber);

  switch (version.dwMajorVersion)
    {
      case 6:
	switch (version.dwMinorVersion)
	  {
	    case 0:
	      caps = &wincap_vista;
	      break;
	    case 1:
	      caps = &wincap_7;
	      break;
	    case 2:
	    case 3:
	      caps = &wincap_8;
	      break;
	    default:
	      caps = &wincap_10;
	      break;
	  }
	break;
      case 10:
      default:
	if (version.dwBuildNumber < 10586)
	  caps = &wincap_10;
	else
	  caps = &wincap_10_1511;
	break;
    }

  ((wincaps *)caps)->is_server = (version.wProductType != VER_NT_WORKSTATION);
#ifdef __x86_64__
  wow64 = 0;
  /* 64 bit systems have one more guard page than their 32 bit counterpart. */
  ++((wincaps *)caps)->def_guard_pages;
#else
  if (NT_SUCCESS (NtQueryInformationProcess (NtCurrentProcess (),
					     ProcessWow64Information,
					     &wow64, sizeof wow64, NULL))
      && !wow64)
#endif
    {
      ((wincaps *)caps)->needs_count_in_si_lpres2 = false;
      ((wincaps *)caps)->has_gaa_largeaddress_bug = false;
      ((wincaps *)caps)->has_broken_prefetchvm = false;
    }

  __small_sprintf (osnam, "NT-%d.%d", version.dwMajorVersion,
		   version.dwMinorVersion);
}
