/* wincap.cc -- figure out on which OS we're running. Set the
		capability class to the appropriate values.

   Copyright 2001 Red Hat, Inc.

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

#include "winsup.h"

static NO_COPY wincaps wincap_unknown = {
  lock_file_highword:0x0,
  chunksize:0x0,
  shared:FILE_SHARE_READ | FILE_SHARE_WRITE,
  is_winnt:false,
  access_denied_on_delete:false,
  has_delete_on_close:false,
  has_page_guard:false,
  has_security:false,
  has_security_descriptor_control:false,
  has_get_process_times:false,
  has_specific_access_rights:false,
  has_lseek_bug:false,
  has_lock_file_ex:false,
  has_signal_object_and_wait:false,
  has_eventlog:false,
  has_ip_helper_lib:false,
  has_set_handle_information:false,
  has_set_handle_information_on_console_handles:false,
  supports_smp:false,
  map_view_of_file_ex_sucks:false,
  altgr_is_ctrl_alt:false,
  has_physical_mem_access:false,
  has_working_copy_on_write:false,
  share_mmaps_only_by_name:false,
  virtual_protect_works_on_shared_pages:false,
  has_hard_links:false,
  can_open_directories:false,
  has_move_file_ex:false,
  has_negative_pids:false,
  has_unreliable_pipes:false,
  has_try_enter_critical_section:false,
  has_raw_devices:false,
  has_valid_processorlevel:false,
};

static NO_COPY wincaps wincap_95 = {
  lock_file_highword:0x0,
  chunksize:32 * 1024 * 1024,
  shared:FILE_SHARE_READ | FILE_SHARE_WRITE,
  is_winnt:false,
  access_denied_on_delete:true,
  has_delete_on_close:false,
  has_page_guard:false,
  has_security:false,
  has_security_descriptor_control:false,
  has_get_process_times:false,
  has_specific_access_rights:false,
  has_lseek_bug:true,
  has_lock_file_ex:false,
  has_signal_object_and_wait:false,
  has_eventlog:false,
  has_ip_helper_lib:false,
  has_set_handle_information:false,
  has_set_handle_information_on_console_handles:false,
  supports_smp:false,
  map_view_of_file_ex_sucks:true,
  altgr_is_ctrl_alt:false,
  has_physical_mem_access:false,
  has_working_copy_on_write:false,
  share_mmaps_only_by_name:true,
  virtual_protect_works_on_shared_pages:false,
  has_hard_links:false,
  can_open_directories:false,
  has_move_file_ex:false,
  has_negative_pids:true,
  has_unreliable_pipes:true,
  has_try_enter_critical_section:false,
  has_raw_devices:false,
  has_valid_processorlevel:false,
};

static NO_COPY wincaps wincap_95osr2 = {
  lock_file_highword:0x0,
  chunksize:32 * 1024 * 1024,
  shared:FILE_SHARE_READ | FILE_SHARE_WRITE,
  is_winnt:false,
  access_denied_on_delete:true,
  has_delete_on_close:false,
  has_page_guard:false,
  has_security:false,
  has_security_descriptor_control:false,
  has_get_process_times:false,
  has_specific_access_rights:false,
  has_lseek_bug:true,
  has_lock_file_ex:false,
  has_signal_object_and_wait:false,
  has_eventlog:false,
  has_ip_helper_lib:false,
  has_set_handle_information:false,
  has_set_handle_information_on_console_handles:false,
  supports_smp:false,
  map_view_of_file_ex_sucks:true,
  altgr_is_ctrl_alt:false,
  has_physical_mem_access:false,
  has_working_copy_on_write:false,
  share_mmaps_only_by_name:true,
  virtual_protect_works_on_shared_pages:false,
  has_hard_links:false,
  can_open_directories:false,
  has_move_file_ex:false,
  has_negative_pids:true,
  has_unreliable_pipes:true,
  has_try_enter_critical_section:false,
  has_raw_devices:false,
  has_valid_processorlevel:false,
};

static NO_COPY wincaps wincap_98 = {
  lock_file_highword:0x0,
  chunksize:32 * 1024 * 1024,
  shared:FILE_SHARE_READ | FILE_SHARE_WRITE,
  is_winnt:false,
  access_denied_on_delete:true,
  has_delete_on_close:false,
  has_page_guard:false,
  has_security:false,
  has_security_descriptor_control:false,
  has_get_process_times:false,
  has_specific_access_rights:false,
  has_lseek_bug:true,
  has_lock_file_ex:false,
  has_signal_object_and_wait:false,
  has_eventlog:false,
  has_ip_helper_lib:true,
  has_set_handle_information:false,
  has_set_handle_information_on_console_handles:false,
  supports_smp:false,
  map_view_of_file_ex_sucks:true,
  altgr_is_ctrl_alt:false,
  has_physical_mem_access:false,
  has_working_copy_on_write:false,
  share_mmaps_only_by_name:true,
  virtual_protect_works_on_shared_pages:false,
  has_hard_links:false,
  can_open_directories:false,
  has_move_file_ex:false,
  has_negative_pids:true,
  has_unreliable_pipes:true,
  has_try_enter_critical_section:false,
  has_raw_devices:false,
  has_valid_processorlevel:true,
};

static NO_COPY wincaps wincap_98se = {
  lock_file_highword:0x0,
  chunksize:32 * 1024 * 1024,
  shared:FILE_SHARE_READ | FILE_SHARE_WRITE,
  is_winnt:false,
  access_denied_on_delete:true,
  has_delete_on_close:false,
  has_page_guard:false,
  has_security:false,
  has_security_descriptor_control:false,
  has_get_process_times:false,
  has_specific_access_rights:false,
  has_lseek_bug:true,
  has_lock_file_ex:false,
  has_signal_object_and_wait:false,
  has_eventlog:false,
  has_ip_helper_lib:true,
  has_set_handle_information:false,
  has_set_handle_information_on_console_handles:false,
  supports_smp:false,
  map_view_of_file_ex_sucks:true,
  altgr_is_ctrl_alt:false,
  has_physical_mem_access:false,
  has_working_copy_on_write:false,
  share_mmaps_only_by_name:true,
  virtual_protect_works_on_shared_pages:false,
  has_hard_links:false,
  can_open_directories:false,
  has_move_file_ex:false,
  has_negative_pids:true,
  has_unreliable_pipes:true,
  has_try_enter_critical_section:false,
  has_raw_devices:false,
  has_valid_processorlevel:true,
};

static NO_COPY wincaps wincap_me = {
  lock_file_highword:0x0,
  chunksize:32 * 1024 * 1024,
  shared:FILE_SHARE_READ | FILE_SHARE_WRITE,
  is_winnt:false,
  access_denied_on_delete:true,
  has_delete_on_close:false,
  has_page_guard:false,
  has_security:false,
  has_security_descriptor_control:false,
  has_get_process_times:false,
  has_specific_access_rights:false,
  has_lseek_bug:true,
  has_lock_file_ex:false,
  has_signal_object_and_wait:false,
  has_eventlog:false,
  has_ip_helper_lib:true,
  has_set_handle_information:false,
  has_set_handle_information_on_console_handles:false,
  supports_smp:false,
  map_view_of_file_ex_sucks:true,
  altgr_is_ctrl_alt:false,
  has_physical_mem_access:false,
  has_working_copy_on_write:false,
  share_mmaps_only_by_name:true,
  virtual_protect_works_on_shared_pages:false,
  has_hard_links:false,
  can_open_directories:false,
  has_move_file_ex:false,
  has_negative_pids:true,
  has_unreliable_pipes:true,
  has_try_enter_critical_section:false,
  has_raw_devices:false,
  has_valid_processorlevel:true,
};

static NO_COPY wincaps wincap_nt3 = {
  lock_file_highword:0xffffffff,
  chunksize:0,
  shared:FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
  is_winnt:true,
  access_denied_on_delete:false,
  has_delete_on_close:true,
  has_page_guard:true,
  has_security:true,
  has_security_descriptor_control:false,
  has_get_process_times:true,
  has_specific_access_rights:true,
  has_lseek_bug:false,
  has_lock_file_ex:true,
  has_signal_object_and_wait:false,
  has_eventlog:true,
  has_ip_helper_lib:false,
  has_set_handle_information:true,
  has_set_handle_information_on_console_handles:false,
  supports_smp:false,
  map_view_of_file_ex_sucks:false,
  altgr_is_ctrl_alt:true,
  has_physical_mem_access:true,
  has_working_copy_on_write:true,
  share_mmaps_only_by_name:false,
  virtual_protect_works_on_shared_pages:true,
  has_hard_links:true,
  can_open_directories:true,
  has_move_file_ex:true,
  has_negative_pids:false,
  has_unreliable_pipes:false,
  has_try_enter_critical_section:false,
  has_raw_devices:true,
  has_valid_processorlevel:true,
};

static NO_COPY wincaps wincap_nt4 = {
  lock_file_highword:0xffffffff,
  chunksize:0,
  shared:FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
  is_winnt:true,
  access_denied_on_delete:false,
  has_delete_on_close:true,
  has_page_guard:true,
  has_security:true,
  has_security_descriptor_control:false,
  has_get_process_times:true,
  has_specific_access_rights:true,
  has_lseek_bug:false,
  has_lock_file_ex:true,
  has_signal_object_and_wait:true,
  has_eventlog:true,
  has_ip_helper_lib:false,
  has_set_handle_information:true,
  has_set_handle_information_on_console_handles:false,
  supports_smp:true,
  map_view_of_file_ex_sucks:false,
  altgr_is_ctrl_alt:true,
  has_physical_mem_access:true,
  has_working_copy_on_write:true,
  share_mmaps_only_by_name:false,
  virtual_protect_works_on_shared_pages:true,
  has_hard_links:true,
  can_open_directories:true,
  has_move_file_ex:true,
  has_negative_pids:false,
  has_unreliable_pipes:false,
  has_try_enter_critical_section:true,
  has_raw_devices:true,
  has_valid_processorlevel:true,
};

static NO_COPY wincaps wincap_nt4sp4 = {
  lock_file_highword:0xffffffff,
  chunksize:0,
  shared:FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
  is_winnt:true,
  access_denied_on_delete:false,
  has_delete_on_close:true,
  has_page_guard:true,
  has_security:true,
  has_security_descriptor_control:false,
  has_get_process_times:true,
  has_specific_access_rights:true,
  has_lseek_bug:false,
  has_lock_file_ex:true,
  has_signal_object_and_wait:true,
  has_eventlog:true,
  has_ip_helper_lib:true,
  has_set_handle_information:true,
  has_set_handle_information_on_console_handles:false,
  supports_smp:true,
  map_view_of_file_ex_sucks:false,
  altgr_is_ctrl_alt:true,
  has_physical_mem_access:true,
  has_working_copy_on_write:true,
  share_mmaps_only_by_name:false,
  virtual_protect_works_on_shared_pages:true,
  has_hard_links:true,
  can_open_directories:true,
  has_move_file_ex:true,
  has_negative_pids:false,
  has_unreliable_pipes:false,
  has_try_enter_critical_section:true,
  has_raw_devices:true,
  has_valid_processorlevel:true,
};

static NO_COPY wincaps wincap_2000 = {
  lock_file_highword:0xffffffff,
  chunksize:0,
  shared:FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
  is_winnt:true,
  access_denied_on_delete:false,
  has_delete_on_close:true,
  has_page_guard:true,
  has_security:true,
  has_security_descriptor_control:true,
  has_get_process_times:true,
  has_specific_access_rights:true,
  has_lseek_bug:false,
  has_lock_file_ex:true,
  has_signal_object_and_wait:true,
  has_eventlog:true,
  has_ip_helper_lib:true,
  has_set_handle_information:true,
  has_set_handle_information_on_console_handles:true,
  supports_smp:true,
  map_view_of_file_ex_sucks:false,
  altgr_is_ctrl_alt:true,
  has_physical_mem_access:true,
  has_working_copy_on_write:true,
  share_mmaps_only_by_name:false,
  virtual_protect_works_on_shared_pages:true,
  has_hard_links:true,
  can_open_directories:true,
  has_move_file_ex:true,
  has_negative_pids:false,
  has_unreliable_pipes:false,
  has_try_enter_critical_section:true,
  has_raw_devices:true,
  has_valid_processorlevel:true,
};

static NO_COPY wincaps wincap_xp = {
  lock_file_highword:0xffffffff,
  chunksize:0,
  shared:FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
  is_winnt:true,
  access_denied_on_delete:false,
  has_delete_on_close:true,
  has_page_guard:true,
  has_security:true,
  has_security_descriptor_control:true,
  has_get_process_times:true,
  has_specific_access_rights:true,
  has_lseek_bug:false,
  has_lock_file_ex:true,
  has_signal_object_and_wait:true,
  has_eventlog:true,
  has_ip_helper_lib:true,
  has_set_handle_information:true,
  has_set_handle_information_on_console_handles:true,
  supports_smp:true,
  map_view_of_file_ex_sucks:false,
  altgr_is_ctrl_alt:true,
  has_physical_mem_access:true,
  has_working_copy_on_write:true,
  share_mmaps_only_by_name:false,
  virtual_protect_works_on_shared_pages:true,
  has_hard_links:true,
  can_open_directories:true,
  has_move_file_ex:true,
  has_negative_pids:false,
  has_unreliable_pipes:false,
  has_try_enter_critical_section:true,
  has_raw_devices:true,
  has_valid_processorlevel:true,
};

wincapc NO_COPY wincap;

void
wincapc::init ()
{
  const char *os;

  memset (&version, 0, sizeof version);
  version.dwOSVersionInfoSize = sizeof version;
  GetVersionEx (&version);

  switch (version.dwPlatformId)
    {
      case VER_PLATFORM_WIN32_NT:
	switch (version.dwMajorVersion)
	  {
	    case 3:
	      os = "NT";
	      caps = &wincap_nt3;
	      break;
	    case 4:
	      os = "NT";
	      if (strcmp (version.szCSDVersion, "Service Pack 4") < 0)
		caps = &wincap_nt4;
	      else
		caps = &wincap_nt4sp4;
	      break;
	    case 5:
	      os = "NT";
	      if (version.dwMinorVersion == 0)
		caps = &wincap_2000;
	      else
		caps = &wincap_xp;
	      break;
	    default:
	      os = "??";
	      caps = &wincap_unknown;
	      break;
	  }
	break;
      case VER_PLATFORM_WIN32_WINDOWS:
	switch (version.dwMinorVersion)
	  {
	    case 0:
	      os = "95";
	      if (strchr(version.szCSDVersion, 'C'))
		caps = &wincap_95osr2;
	      else
		caps = &wincap_95;
	      break;
	    case 10:
	      os = "98";
	      if (strchr(version.szCSDVersion, 'A'))
		caps = &wincap_98se;
	      else
		caps = &wincap_98;
	      break;
	    case 90:
	      os = "ME";
	      caps = &wincap_me;
	      break;
	    default:
	      os = "??";
	      caps = &wincap_unknown;
	      break;
	  }
	break;
      default:
	os = "??";
	caps = &wincap_unknown;
	break;
    }
  __small_sprintf (osnam, "%s-%d.%d", os, version.dwMajorVersion,
		   version.dwMinorVersion);
}

void
wincapc::set_chunksize (DWORD nchunksize)
{
  ((wincaps *)this->caps)->chunksize = nchunksize;
}
