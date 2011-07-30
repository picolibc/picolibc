/* wincap.cc -- figure out on which OS we're running. Set the
		capability class to the appropriate values.

   Copyright 2001, 2002, 2003, 2004, 2005, 2006, 2007, 2008,
   2009, 2010, 2011 Red Hat, Inc.

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

/* Minimal set of capabilities required to run Cygwin. */
#define wincap_minimal wincap_2000

wincaps wincap_2000 __attribute__((section (".cygwin_dll_common"), shared)) = {
  max_sys_priv:SE_MANAGE_VOLUME_PRIVILEGE,
  is_server:false,
  has_physical_mem_access:true,
  has_create_global_privilege:false,
  has_ioctl_storage_get_media_types_ex:false,
  has_disk_ex_ioctls:false,
  has_buggy_restart_scan:true,
  has_mandatory_integrity_control:false,
  needs_logon_sid_in_sid_list:true,
  needs_count_in_si_lpres2:false,
  has_recycle_dot_bin:false,
  has_gaa_prefixes:false,
  has_gaa_on_link_prefix:false,
  supports_all_posix_ai_flags:false,
  has_restricted_stack_args:false,
  has_transactions:false,
  has_recvmsg:false,
  has_sendmsg:false,
  has_broken_udf:true,
  has_console_handle_problem:false,
  has_broken_alloc_console:false,
  has_always_all_codepages:false,
  has_localenames:false,
  has_fast_cwd:false,
  has_restricted_raw_disk_access:false,
  use_dont_resolve_hack:false,
  has_stack_size_param_is_a_reservation:false,
};

wincaps wincap_2000sp4 __attribute__((section (".cygwin_dll_common"), shared)) = {
  max_sys_priv:SE_CREATE_GLOBAL_PRIVILEGE,
  is_server:false,
  has_physical_mem_access:true,
  has_create_global_privilege:true,
  has_ioctl_storage_get_media_types_ex:false,
  has_disk_ex_ioctls:false,
  has_buggy_restart_scan:true,
  has_mandatory_integrity_control:false,
  needs_logon_sid_in_sid_list:true,
  needs_count_in_si_lpres2:false,
  has_recycle_dot_bin:false,
  has_gaa_prefixes:false,
  has_gaa_on_link_prefix:false,
  supports_all_posix_ai_flags:false,
  has_restricted_stack_args:false,
  has_transactions:false,
  has_recvmsg:false,
  has_sendmsg:false,
  has_broken_udf:true,
  has_console_handle_problem:false,
  has_broken_alloc_console:false,
  has_always_all_codepages:false,
  has_localenames:false,
  has_fast_cwd:false,
  has_restricted_raw_disk_access:false,
  use_dont_resolve_hack:false,
  has_stack_size_param_is_a_reservation:false,
};

wincaps wincap_xp __attribute__((section (".cygwin_dll_common"), shared)) = {
  max_sys_priv:SE_MANAGE_VOLUME_PRIVILEGE,
  is_server:false,
  has_physical_mem_access:true,
  has_create_global_privilege:false,
  has_ioctl_storage_get_media_types_ex:true,
  has_disk_ex_ioctls:true,
  has_buggy_restart_scan:false,
  has_mandatory_integrity_control:false,
  needs_logon_sid_in_sid_list:false,
  needs_count_in_si_lpres2:false,
  has_recycle_dot_bin:false,
  has_gaa_prefixes:false,
  has_gaa_on_link_prefix:false,
  supports_all_posix_ai_flags:false,
  has_restricted_stack_args:false,
  has_transactions:false,
  has_recvmsg:true,
  has_sendmsg:false,
  has_broken_udf:true,
  has_console_handle_problem:false,
  has_broken_alloc_console:false,
  has_always_all_codepages:false,
  has_localenames:false,
  has_fast_cwd:false,
  has_restricted_raw_disk_access:false,
  use_dont_resolve_hack:true,
  has_stack_size_param_is_a_reservation:true,
};

wincaps wincap_xpsp1 __attribute__((section (".cygwin_dll_common"), shared)) = {
  max_sys_priv:SE_MANAGE_VOLUME_PRIVILEGE,
  is_server:false,
  has_physical_mem_access:true,
  has_create_global_privilege:false,
  has_ioctl_storage_get_media_types_ex:true,
  has_disk_ex_ioctls:true,
  has_buggy_restart_scan:false,
  has_mandatory_integrity_control:false,
  needs_logon_sid_in_sid_list:false,
  needs_count_in_si_lpres2:false,
  has_recycle_dot_bin:false,
  has_gaa_prefixes:true,
  has_gaa_on_link_prefix:false,
  supports_all_posix_ai_flags:false,
  has_restricted_stack_args:false,
  has_transactions:false,
  has_recvmsg:true,
  has_sendmsg:false,
  has_broken_udf:true,
  has_console_handle_problem:false,
  has_broken_alloc_console:false,
  has_always_all_codepages:false,
  has_localenames:false,
  has_fast_cwd:false,
  has_restricted_raw_disk_access:false,
  use_dont_resolve_hack:true,
  has_stack_size_param_is_a_reservation:true,
};

wincaps wincap_xpsp2 __attribute__((section (".cygwin_dll_common"), shared)) = {
  max_sys_priv:SE_CREATE_GLOBAL_PRIVILEGE,
  is_server:false,
  has_physical_mem_access:true,
  has_create_global_privilege:true,
  has_ioctl_storage_get_media_types_ex:true,
  has_disk_ex_ioctls:true,
  has_buggy_restart_scan:false,
  has_mandatory_integrity_control:false,
  needs_logon_sid_in_sid_list:false,
  needs_count_in_si_lpres2:false,
  has_recycle_dot_bin:false,
  has_gaa_prefixes:true,
  has_gaa_on_link_prefix:false,
  supports_all_posix_ai_flags:false,
  has_restricted_stack_args:false,
  has_transactions:false,
  has_recvmsg:true,
  has_sendmsg:false,
  has_broken_udf:true,
  has_console_handle_problem:false,
  has_broken_alloc_console:false,
  has_always_all_codepages:false,
  has_localenames:false,
  has_fast_cwd:false,
  has_restricted_raw_disk_access:false,
  use_dont_resolve_hack:true,
  has_stack_size_param_is_a_reservation:true,
};

wincaps wincap_2003 __attribute__((section (".cygwin_dll_common"), shared)) = {
  max_sys_priv:SE_CREATE_GLOBAL_PRIVILEGE,
  is_server:false,
  has_physical_mem_access:false,
  has_create_global_privilege:true,
  has_ioctl_storage_get_media_types_ex:true,
  has_disk_ex_ioctls:true,
  has_buggy_restart_scan:false,
  has_mandatory_integrity_control:false,
  needs_logon_sid_in_sid_list:false,
  needs_count_in_si_lpres2:false,
  has_recycle_dot_bin:false,
  has_gaa_prefixes:true,
  has_gaa_on_link_prefix:false,
  supports_all_posix_ai_flags:false,
  has_restricted_stack_args:true,
  has_transactions:false,
  has_recvmsg:true,
  has_sendmsg:false,
  has_broken_udf:true,
  has_console_handle_problem:false,
  has_broken_alloc_console:false,
  has_always_all_codepages:false,
  has_localenames:false,
  has_fast_cwd:false,
  has_restricted_raw_disk_access:false,
  use_dont_resolve_hack:true,
  has_stack_size_param_is_a_reservation:true,
};

wincaps wincap_vista __attribute__((section (".cygwin_dll_common"), shared)) = {
  max_sys_priv:SE_CREATE_SYMBOLIC_LINK_PRIVILEGE,
  is_server:false,
  has_physical_mem_access:false,
  has_create_global_privilege:true,
  has_ioctl_storage_get_media_types_ex:true,
  has_disk_ex_ioctls:true,
  has_buggy_restart_scan:false,
  has_mandatory_integrity_control:true,
  needs_logon_sid_in_sid_list:false,
  needs_count_in_si_lpres2:true,
  has_recycle_dot_bin:true,
  has_gaa_prefixes:true,
  has_gaa_on_link_prefix:true,
  supports_all_posix_ai_flags:true,
  has_restricted_stack_args:false,
  has_transactions:true,
  has_recvmsg:true,
  has_sendmsg:true,
  has_broken_udf:false,
  has_console_handle_problem:false,
  has_broken_alloc_console:false,
  has_always_all_codepages:true,
  has_localenames:true,
  has_fast_cwd:true,
  has_restricted_raw_disk_access:true,
  use_dont_resolve_hack:false,
  has_stack_size_param_is_a_reservation:true,
};

wincaps wincap_7 __attribute__((section (".cygwin_dll_common"), shared)) = {
  max_sys_priv:SE_CREATE_SYMBOLIC_LINK_PRIVILEGE,
  is_server:false,
  has_physical_mem_access:false,
  has_create_global_privilege:true,
  has_ioctl_storage_get_media_types_ex:true,
  has_disk_ex_ioctls:true,
  has_buggy_restart_scan:false,
  has_mandatory_integrity_control:true,
  needs_logon_sid_in_sid_list:false,
  needs_count_in_si_lpres2:false,
  has_recycle_dot_bin:true,
  has_gaa_prefixes:true,
  has_gaa_on_link_prefix:true,
  supports_all_posix_ai_flags:true,
  has_restricted_stack_args:false,
  has_transactions:true,
  has_recvmsg:true,
  has_sendmsg:true,
  has_broken_udf:false,
  has_console_handle_problem:true,
  has_broken_alloc_console:true,
  has_always_all_codepages:true,
  has_localenames:true,
  has_fast_cwd:true,
  has_restricted_raw_disk_access:true,
  use_dont_resolve_hack:false,
  has_stack_size_param_is_a_reservation:true,
};

wincapc wincap __attribute__((section (".cygwin_dll_common"), shared));

void
wincapc::init ()
{
  if (caps)
    return;		// already initialized

  GetSystemInfo (&system_info);
  memset (&version, 0, sizeof version);
  version.dwOSVersionInfoSize = sizeof (OSVERSIONINFOEX);
  if (!GetVersionEx (reinterpret_cast<LPOSVERSIONINFO>(&version)))
    api_fatal ("Cygwin requires at least Windows 2000.");

  switch (version.dwPlatformId)
    {
      case VER_PLATFORM_WIN32_NT:
	switch (version.dwMajorVersion)
	  {
	    case 4:
	      /* I'd be very surprised if this code is ever hit, but it doesn't
		 hurt to keep it. */
	      api_fatal ("Cygwin requires at least Windows 2000.");
	      break;
	    case 5:
	      switch (version.dwMinorVersion)
		{
		  case 0:
		    if (version.wServicePackMajor < 4)
		      caps = &wincap_2000;
		    else
		      caps = &wincap_2000sp4;
		    break;

		  case 1:
		    caps = &wincap_xp;
		    switch (version.wServicePackMajor)
		      {
		      case 0:
			caps = &wincap_xp;
		      case 1:
			caps = &wincap_xpsp1;
		      default:
			caps = &wincap_xpsp2;
		      }
		    break;

		  default:
		    caps = &wincap_2003;
		}
	      break;
	    case 6:
	      switch (version.dwMinorVersion)
		{
		  case 0:
		    caps = &wincap_vista;
		    break;
		  default:
		    caps = &wincap_7;
		    break;
		}
	      break;
	    default:
	      caps = &wincap_minimal;
	      break;
	  }
	break;
      case VER_PLATFORM_WIN32_WINDOWS:
	/* I'd be very surprised if this code is ever hit, but it doesn't
	   hurt to keep it. */
	api_fatal ("Windows 95/98/Me are not supported.");
	break;
      default:
	caps = &wincap_minimal;
	break;
    }

  ((wincaps *)caps)->is_server = (version.wProductType != VER_NT_WORKSTATION);
  if (NT_SUCCESS (NtQueryInformationProcess (NtCurrentProcess (),
					     ProcessWow64Information,
					     &wow64, sizeof wow64, NULL))
      && !wow64)
    {
      ((wincaps *)caps)->needs_count_in_si_lpres2 = false;
      ((wincaps *)caps)->has_restricted_stack_args = false;
    }

  __small_sprintf (osnam, "NT-%d.%d", version.dwMajorVersion,
		   version.dwMinorVersion);
}
