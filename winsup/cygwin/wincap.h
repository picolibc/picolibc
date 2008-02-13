/* wincap.h: Header for OS capability class.

   Copyright 2001, 2002, 2003, 2004, 2005, 2006 Red Hat, Inc.

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

#ifndef _WINCAP_H
#define _WINCAP_H

struct wincaps
{
  DWORD    lock_file_highword;
  DWORD    chunksize;
  DWORD    heapslop;
  int      shared;
  unsigned is_winnt                                     : 1;
  unsigned is_server                                    : 1;
  unsigned access_denied_on_delete                      : 1;
  unsigned has_delete_on_close                          : 1;
  unsigned has_page_guard                               : 1;
  unsigned has_security                                 : 1;
  unsigned has_security_descriptor_control              : 1;
  unsigned has_get_process_times                        : 1;
  unsigned has_lseek_bug                                : 1;
  unsigned has_lock_file_ex                             : 1;
  unsigned has_signal_object_and_wait                   : 1;
  unsigned has_eventlog                                 : 1;
  unsigned has_ip_helper_lib                            : 1;
  unsigned has_set_handle_information                   : 1;
  unsigned has_set_handle_information_on_console_handles: 1;
  unsigned supports_smp                                 : 1;
  unsigned map_view_of_file_ex_sucks                    : 1;
  unsigned altgr_is_ctrl_alt                            : 1;
  unsigned has_physical_mem_access                      : 1;
  unsigned has_working_copy_on_write                    : 1;
  unsigned share_mmaps_only_by_name                     : 1;
  unsigned virtual_protect_works_on_shared_pages        : 1;
  unsigned has_mmap_alignment_bug			: 1;
  unsigned has_hard_links                               : 1;
  unsigned can_open_directories                         : 1;
  unsigned has_move_file_ex                             : 1;
  unsigned has_negative_pids                            : 1;
  unsigned has_unreliable_pipes                         : 1;
  unsigned has_named_pipes                              : 1;
  unsigned has_try_enter_critical_section		: 1;
  unsigned has_raw_devices				: 1;
  unsigned has_valid_processorlevel			: 1;
  unsigned has_64bit_file_access			: 1;
  unsigned has_process_io_counters                      : 1;
  unsigned supports_reading_modem_output_lines          : 1;
  unsigned needs_memory_protection			: 1;
  unsigned pty_needs_alloc_console			: 1;
  unsigned has_terminal_services			: 1;
  unsigned has_switch_to_thread				: 1;
  unsigned cant_debug_dll_entry				: 1;
  unsigned has_ioctl_storage_get_media_types_ex		: 1;
  unsigned start_proc_suspended				: 1;
  unsigned has_extended_priority_class			: 1;
  unsigned has_guid_volumes				: 1;
  unsigned detect_win16_exe				: 1;
  unsigned has_null_console_handler_routine		: 1;
  unsigned has_disk_ex_ioctls				: 1;
  unsigned has_working_virtual_lock			: 1;
  unsigned has_disabled_user_tos_setting		: 1;
  unsigned has_fileid_dirinfo				: 1;
  unsigned has_exclusiveaddruse				: 1;
  unsigned has_buggy_restart_scan			: 1;
  unsigned needs_count_in_si_lpres2			: 1;
  unsigned has_restricted_stack_args			: 1;
};

class wincapc
{
  OSVERSIONINFOEX  version;
  char             osnam[40];
  bool             wow64;
  void             *caps;

public:
  void init ();

  void set_chunksize (DWORD nchunksize);

  const char *osname () const { return osnam; }
  const bool is_wow64 () const { return wow64; }

#define IMPLEMENT(cap) cap() const { return ((wincaps *) this->caps)->cap; }

  DWORD IMPLEMENT (lock_file_highword)
  DWORD IMPLEMENT (chunksize)
  DWORD IMPLEMENT (heapslop)
  int   IMPLEMENT (shared)
  bool  IMPLEMENT (is_winnt)
  bool  IMPLEMENT (is_server)
  bool  IMPLEMENT (access_denied_on_delete)
  bool  IMPLEMENT (has_delete_on_close)
  bool  IMPLEMENT (has_page_guard)
  bool  IMPLEMENT (has_security)
  bool  IMPLEMENT (has_security_descriptor_control)
  bool  IMPLEMENT (has_get_process_times)
  bool  IMPLEMENT (has_lseek_bug)
  bool  IMPLEMENT (has_lock_file_ex)
  bool  IMPLEMENT (has_signal_object_and_wait)
  bool  IMPLEMENT (has_eventlog)
  bool  IMPLEMENT (has_ip_helper_lib)
  bool  IMPLEMENT (has_set_handle_information)
  bool  IMPLEMENT (has_set_handle_information_on_console_handles)
  bool  IMPLEMENT (supports_smp)
  bool  IMPLEMENT (map_view_of_file_ex_sucks)
  bool  IMPLEMENT (altgr_is_ctrl_alt)
  bool  IMPLEMENT (has_physical_mem_access)
  bool  IMPLEMENT (has_working_copy_on_write)
  bool  IMPLEMENT (share_mmaps_only_by_name)
  bool  IMPLEMENT (virtual_protect_works_on_shared_pages)
  bool	IMPLEMENT (has_mmap_alignment_bug)
  bool  IMPLEMENT (has_hard_links)
  bool  IMPLEMENT (can_open_directories)
  bool  IMPLEMENT (has_move_file_ex)
  bool  IMPLEMENT (has_negative_pids)
  bool  IMPLEMENT (has_unreliable_pipes)
  bool  IMPLEMENT (has_named_pipes)
  bool  IMPLEMENT (has_try_enter_critical_section)
  bool  IMPLEMENT (has_raw_devices)
  bool  IMPLEMENT (has_valid_processorlevel)
  bool  IMPLEMENT (has_64bit_file_access)
  bool  IMPLEMENT (has_process_io_counters)
  bool  IMPLEMENT (supports_reading_modem_output_lines)
  bool  IMPLEMENT (needs_memory_protection)
  bool  IMPLEMENT (pty_needs_alloc_console)
  bool  IMPLEMENT (has_terminal_services)
  bool  IMPLEMENT (has_switch_to_thread)
  bool	IMPLEMENT (cant_debug_dll_entry)
  bool	IMPLEMENT (has_ioctl_storage_get_media_types_ex)
  bool	IMPLEMENT (start_proc_suspended)
  bool	IMPLEMENT (has_extended_priority_class)
  bool	IMPLEMENT (has_guid_volumes)
  bool	IMPLEMENT (detect_win16_exe)
  bool	IMPLEMENT (has_null_console_handler_routine)
  bool	IMPLEMENT (has_disk_ex_ioctls)
  bool	IMPLEMENT (has_working_virtual_lock)
  bool	IMPLEMENT (has_disabled_user_tos_setting)
  bool	IMPLEMENT (has_fileid_dirinfo)
  bool	IMPLEMENT (has_exclusiveaddruse)
  bool	IMPLEMENT (has_buggy_restart_scan)
  bool	IMPLEMENT (needs_count_in_si_lpres2)
  bool	IMPLEMENT (has_restricted_stack_args)

#undef IMPLEMENT
};

extern wincapc wincap;

#endif /* _WINCAP_H */
