/* wincap.cc -- figure out on which OS we're running. Set the
		capability class to the appropriate values.

   Copyright 2001, 2002, 2003, 2004, 2005, 2006, 2007, 2008 Red Hat, Inc.

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

#include "winsup.h"
#include "security.h"

/* Minimal set of capabilities which is equivalent to NT4. */
static NO_COPY wincaps wincap_unknown = {
  chunksize:0,
  heapslop:0x0,
  max_sys_priv:SE_CHANGE_NOTIFY_PRIVILEGE,
  is_server:false,
  has_dacl_protect:false,
  has_ip_helper_lib:false,
  has_broken_if_oper_status:false,
  has_physical_mem_access:true,
  has_process_io_counters:false,
  has_terminal_services:false,
  has_create_global_privilege:false,
  has_ioctl_storage_get_media_types_ex:false,
  has_extended_priority_class:false,
  has_guid_volumes:false,
  has_disk_ex_ioctls:false,
  has_disabled_user_tos_setting:false,
  has_fileid_dirinfo:false,
  has_exclusiveaddruse:false,
  has_buggy_restart_scan:false,
  has_mandatory_integrity_control:false,
  needs_logon_sid_in_sid_list:true,
  needs_count_in_si_lpres2:false,
  has_recycle_dot_bin:false,
  has_gaa_prefixes:false,
  has_gaa_on_link_prefix:false,
  supports_all_posix_ai_flags:false,
  has_restricted_stack_args:false,
};

static NO_COPY wincaps wincap_nt4 = {
  chunksize:0,
  heapslop:0x0,
  max_sys_priv:SE_CHANGE_NOTIFY_PRIVILEGE,
  is_server:false,
  has_dacl_protect:false,
  has_ip_helper_lib:false,
  has_broken_if_oper_status:false,
  has_physical_mem_access:true,
  has_process_io_counters:false,
  has_terminal_services:false,
  has_create_global_privilege:false,
  has_ioctl_storage_get_media_types_ex:false,
  has_extended_priority_class:false,
  has_guid_volumes:false,
  has_disk_ex_ioctls:false,
  has_disabled_user_tos_setting:false,
  has_fileid_dirinfo:false,
  has_exclusiveaddruse:false,
  has_buggy_restart_scan:false,
  has_mandatory_integrity_control:false,
  needs_logon_sid_in_sid_list:true,
  needs_count_in_si_lpres2:false,
  has_recycle_dot_bin:false,
  has_gaa_prefixes:false,
  has_gaa_on_link_prefix:false,
  supports_all_posix_ai_flags:false,
  has_restricted_stack_args:false,
};

static NO_COPY wincaps wincap_nt4sp4 = {
  chunksize:0,
  heapslop:0x0,
  max_sys_priv:SE_CHANGE_NOTIFY_PRIVILEGE,
  is_server:false,
  has_dacl_protect:false,
  has_ip_helper_lib:true,
  has_broken_if_oper_status:true,
  has_physical_mem_access:true,
  has_process_io_counters:false,
  has_terminal_services:false,
  has_create_global_privilege:false,
  has_ioctl_storage_get_media_types_ex:false,
  has_extended_priority_class:false,
  has_guid_volumes:false,
  has_disk_ex_ioctls:false,
  has_disabled_user_tos_setting:false,
  has_fileid_dirinfo:false,
  has_exclusiveaddruse:true,
  has_buggy_restart_scan:false,
  has_mandatory_integrity_control:false,
  needs_logon_sid_in_sid_list:true,
  needs_count_in_si_lpres2:false,
  has_recycle_dot_bin:false,
  has_gaa_prefixes:false,
  has_gaa_on_link_prefix:false,
  supports_all_posix_ai_flags:false,
  has_restricted_stack_args:false,
};

static NO_COPY wincaps wincap_2000 = {
  chunksize:0,
  heapslop:0x0,
  max_sys_priv:SE_MANAGE_VOLUME_PRIVILEGE,
  is_server:false,
  has_dacl_protect:true,
  has_ip_helper_lib:true,
  has_broken_if_oper_status:false,
  has_physical_mem_access:true,
  has_process_io_counters:true,
  has_terminal_services:true,
  has_create_global_privilege:false,
  has_ioctl_storage_get_media_types_ex:false,
  has_extended_priority_class:true,
  has_guid_volumes:true,
  has_disk_ex_ioctls:false,
  has_disabled_user_tos_setting:true,
  has_fileid_dirinfo:true,
  has_exclusiveaddruse:true,
  has_buggy_restart_scan:true,
  has_mandatory_integrity_control:false,
  needs_logon_sid_in_sid_list:true,
  needs_count_in_si_lpres2:false,
  has_recycle_dot_bin:false,
  has_gaa_prefixes:false,
  has_gaa_on_link_prefix:false,
  supports_all_posix_ai_flags:false,
  has_restricted_stack_args:false,
};

static NO_COPY wincaps wincap_2000sp4 = {
  chunksize:0,
  heapslop:0x0,
  max_sys_priv:SE_CREATE_GLOBAL_PRIVILEGE,
  is_server:false,
  has_dacl_protect:true,
  has_ip_helper_lib:true,
  has_broken_if_oper_status:false,
  has_physical_mem_access:true,
  has_process_io_counters:true,
  has_terminal_services:true,
  has_create_global_privilege:true,
  has_ioctl_storage_get_media_types_ex:false,
  has_extended_priority_class:true,
  has_guid_volumes:true,
  has_disk_ex_ioctls:false,
  has_disabled_user_tos_setting:true,
  has_fileid_dirinfo:true,
  has_exclusiveaddruse:true,
  has_buggy_restart_scan:true,
  has_mandatory_integrity_control:false,
  needs_logon_sid_in_sid_list:true,
  needs_count_in_si_lpres2:false,
  has_recycle_dot_bin:false,
  has_gaa_prefixes:false,
  has_gaa_on_link_prefix:false,
  supports_all_posix_ai_flags:false,
  has_restricted_stack_args:false,
};

static NO_COPY wincaps wincap_xp = {
  chunksize:0,
  heapslop:0x0,
  max_sys_priv:SE_MANAGE_VOLUME_PRIVILEGE,
  is_server:false,
  has_dacl_protect:true,
  has_ip_helper_lib:true,
  has_broken_if_oper_status:false,
  has_physical_mem_access:true,
  has_process_io_counters:true,
  has_terminal_services:true,
  has_create_global_privilege:false,
  has_ioctl_storage_get_media_types_ex:true,
  has_extended_priority_class:true,
  has_guid_volumes:true,
  has_disk_ex_ioctls:true,
  has_disabled_user_tos_setting:true,
  has_fileid_dirinfo:true,
  has_exclusiveaddruse:true,
  has_buggy_restart_scan:false,
  has_mandatory_integrity_control:false,
  needs_logon_sid_in_sid_list:false,
  needs_count_in_si_lpres2:false,
  has_recycle_dot_bin:false,
  has_gaa_prefixes:false,
  has_gaa_on_link_prefix:false,
  supports_all_posix_ai_flags:false,
  has_restricted_stack_args:false,
};

static NO_COPY wincaps wincap_xpsp1 = {
  chunksize:0,
  heapslop:0x0,
  max_sys_priv:SE_MANAGE_VOLUME_PRIVILEGE,
  is_server:false,
  has_dacl_protect:true,
  has_ip_helper_lib:true,
  has_broken_if_oper_status:false,
  has_physical_mem_access:true,
  has_process_io_counters:true,
  has_terminal_services:true,
  has_create_global_privilege:false,
  has_ioctl_storage_get_media_types_ex:true,
  has_extended_priority_class:true,
  has_guid_volumes:true,
  has_disk_ex_ioctls:true,
  has_disabled_user_tos_setting:true,
  has_fileid_dirinfo:true,
  has_exclusiveaddruse:true,
  has_buggy_restart_scan:false,
  has_mandatory_integrity_control:false,
  needs_logon_sid_in_sid_list:false,
  needs_count_in_si_lpres2:false,
  has_recycle_dot_bin:false,
  has_gaa_prefixes:true,
  has_gaa_on_link_prefix:false,
  supports_all_posix_ai_flags:false,
  has_restricted_stack_args:false,
};

static NO_COPY wincaps wincap_xpsp2 = {
  chunksize:0,
  heapslop:0x0,
  max_sys_priv:SE_CREATE_GLOBAL_PRIVILEGE,
  is_server:false,
  has_dacl_protect:true,
  has_ip_helper_lib:true,
  has_broken_if_oper_status:false,
  has_physical_mem_access:true,
  has_process_io_counters:true,
  has_terminal_services:true,
  has_create_global_privilege:true,
  has_ioctl_storage_get_media_types_ex:true,
  has_extended_priority_class:true,
  has_guid_volumes:true,
  has_disk_ex_ioctls:true,
  has_disabled_user_tos_setting:true,
  has_fileid_dirinfo:true,
  has_exclusiveaddruse:true,
  has_buggy_restart_scan:false,
  has_mandatory_integrity_control:false,
  needs_logon_sid_in_sid_list:false,
  needs_count_in_si_lpres2:false,
  has_recycle_dot_bin:false,
  has_gaa_prefixes:true,
  has_gaa_on_link_prefix:false,
  supports_all_posix_ai_flags:false,
  has_restricted_stack_args:false,
};

static NO_COPY wincaps wincap_2003 = {
  chunksize:0,
  heapslop:0x4,
  max_sys_priv:SE_CREATE_GLOBAL_PRIVILEGE,
  is_server:true,
  has_dacl_protect:true,
  has_ip_helper_lib:true,
  has_broken_if_oper_status:false,
  has_physical_mem_access:false,
  has_process_io_counters:true,
  has_terminal_services:true,
  has_create_global_privilege:true,
  has_ioctl_storage_get_media_types_ex:true,
  has_extended_priority_class:true,
  has_guid_volumes:true,
  has_disk_ex_ioctls:true,
  has_disabled_user_tos_setting:true,
  has_fileid_dirinfo:true,
  has_exclusiveaddruse:true,
  has_buggy_restart_scan:false,
  has_mandatory_integrity_control:false,
  needs_logon_sid_in_sid_list:false,
  needs_count_in_si_lpres2:false,
  has_recycle_dot_bin:false,
  has_gaa_prefixes:true,
  has_gaa_on_link_prefix:false,
  supports_all_posix_ai_flags:false,
  has_restricted_stack_args:true,
};

static NO_COPY wincaps wincap_vista = {
  chunksize:0,
  heapslop:0x4,
  max_sys_priv:SE_CREATE_SYMBOLIC_LINK_PRIVILEGE,
  is_server:false,
  has_dacl_protect:true,
  has_ip_helper_lib:true,
  has_broken_if_oper_status:false,
  has_physical_mem_access:false,
  has_process_io_counters:true,
  has_terminal_services:true,
  has_create_global_privilege:true,
  has_ioctl_storage_get_media_types_ex:true,
  has_extended_priority_class:true,
  has_guid_volumes:true,
  has_disk_ex_ioctls:true,
  has_disabled_user_tos_setting:true,
  has_fileid_dirinfo:true,
  has_exclusiveaddruse:true,
  has_buggy_restart_scan:false,
  has_mandatory_integrity_control:true,
  needs_logon_sid_in_sid_list:false,
  needs_count_in_si_lpres2:true,
  has_recycle_dot_bin:true,
  has_gaa_prefixes:true,
  has_gaa_on_link_prefix:true,
  supports_all_posix_ai_flags:true,
  has_restricted_stack_args:false,
};

wincapc wincap __attribute__((section (".cygwin_dll_common"), shared));

void
wincapc::init ()
{
  bool has_osversioninfoex = true;

  if (caps)
    return;		// already initialized

  memset (&version, 0, sizeof version);
  /* Request versionex info first, which is available on all systems since
     NT4 SP6 anyway.  If that fails, call the simple version. */
  version.dwOSVersionInfoSize = sizeof (OSVERSIONINFOEX);
  if (!GetVersionEx (reinterpret_cast<LPOSVERSIONINFO>(&version)))
    {
      has_osversioninfoex = false;
      version.dwOSVersionInfoSize = sizeof (OSVERSIONINFO);
      GetVersionEx (reinterpret_cast<LPOSVERSIONINFO>(&version));
    }

  switch (version.dwPlatformId)
    {
      case VER_PLATFORM_WIN32_NT:
	switch (version.dwMajorVersion)
	  {
	    case 4:
	      if (!has_osversioninfoex
		  && strcmp (version.szCSDVersion, "Service Pack 4") < 0)
		caps = &wincap_nt4;
	      else
		caps = &wincap_nt4sp4;
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
	      caps = &wincap_vista;
	      break;
	    default:
	      caps = &wincap_unknown;
	      break;
	  }
	break;
      case VER_PLATFORM_WIN32_WINDOWS:
	/* I'd be very surprised if this code is ever hit, but it doesn't
	   hurt to keep it. */
	api_fatal ("Windows 95/98/Me are not supported.");
	break;
      default:
	caps = &wincap_unknown;
	break;
    }

  if (has_osversioninfoex && version.wProductType != VER_NT_WORKSTATION)
    ((wincaps *)this->caps)->is_server = true;

  BOOL is_wow64_proc = FALSE;
  if (IsWow64Process (GetCurrentProcess (), &is_wow64_proc))
    wow64 = is_wow64_proc;
  else
    {
      ((wincaps *)this->caps)->needs_count_in_si_lpres2 = false;
      ((wincaps *)this->caps)->has_restricted_stack_args = false;
    }

  __small_sprintf (osnam, "NT-%d.%d", version.dwMajorVersion,
		   version.dwMinorVersion);
}

void
wincapc::set_chunksize (DWORD nchunksize)
{
  ((wincaps *)this->caps)->chunksize = nchunksize;
}
