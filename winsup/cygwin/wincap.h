/* wincap.h: Header for OS capability class.

   Copyright 2001, 2002, 2003, 2004, 2005, 2006, 2007 Red Hat, Inc.

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
  unsigned is_server                                    : 1;
  unsigned has_security                                 : 1;
  unsigned has_security_descriptor_control              : 1;
  unsigned has_ip_helper_lib                            : 1;
  unsigned has_physical_mem_access                      : 1;
  unsigned has_move_file_ex                             : 1;
  unsigned has_unreliable_pipes                         : 1;
  unsigned has_raw_devices				: 1;
  unsigned has_valid_processorlevel			: 1;
  unsigned has_64bit_file_access			: 1;
  unsigned has_process_io_counters                      : 1;
  unsigned supports_reading_modem_output_lines          : 1;
  unsigned needs_memory_protection			: 1;
  unsigned pty_needs_alloc_console			: 1;
  unsigned has_terminal_services			: 1;
  unsigned has_switch_to_thread				: 1;
  unsigned has_ioctl_storage_get_media_types_ex		: 1;
  unsigned start_proc_suspended				: 1;
  unsigned has_extended_priority_class			: 1;
  unsigned has_guid_volumes				: 1;
  unsigned has_null_console_handler_routine		: 1;
  unsigned has_disk_ex_ioctls				: 1;
  unsigned has_disabled_user_tos_setting		: 1;
  unsigned has_fileid_dirinfo				: 1;
  unsigned has_exclusiveaddruse				: 1;
  unsigned has_buggy_restart_scan			: 1;
  unsigned has_mandatory_integrity_control		: 1;
  unsigned needs_logon_sid_in_sid_list			: 1;
  unsigned needs_count_in_si_lpres2			: 1;
  unsigned has_recycle_dot_bin				: 1;
  unsigned has_gaa_prefixes				: 1;
  unsigned has_gaa_on_link_prefix			: 1;
  unsigned supports_all_posix_ai_flags			: 1;
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
  bool  IMPLEMENT (is_server)
  bool  IMPLEMENT (has_security)
  bool  IMPLEMENT (has_security_descriptor_control)
  bool  IMPLEMENT (has_ip_helper_lib)
  bool  IMPLEMENT (has_physical_mem_access)
  bool  IMPLEMENT (has_move_file_ex)
  bool  IMPLEMENT (has_unreliable_pipes)
  bool  IMPLEMENT (has_raw_devices)
  bool  IMPLEMENT (has_valid_processorlevel)
  bool  IMPLEMENT (has_64bit_file_access)
  bool  IMPLEMENT (has_process_io_counters)
  bool  IMPLEMENT (supports_reading_modem_output_lines)
  bool  IMPLEMENT (needs_memory_protection)
  bool  IMPLEMENT (pty_needs_alloc_console)
  bool  IMPLEMENT (has_terminal_services)
  bool  IMPLEMENT (has_switch_to_thread)
  bool	IMPLEMENT (has_ioctl_storage_get_media_types_ex)
  bool	IMPLEMENT (start_proc_suspended)
  bool	IMPLEMENT (has_extended_priority_class)
  bool	IMPLEMENT (has_guid_volumes)
  bool	IMPLEMENT (has_null_console_handler_routine)
  bool	IMPLEMENT (has_disk_ex_ioctls)
  bool	IMPLEMENT (has_disabled_user_tos_setting)
  bool	IMPLEMENT (has_fileid_dirinfo)
  bool	IMPLEMENT (has_exclusiveaddruse)
  bool	IMPLEMENT (has_buggy_restart_scan)
  bool	IMPLEMENT (has_mandatory_integrity_control)
  bool	IMPLEMENT (needs_logon_sid_in_sid_list)
  bool	IMPLEMENT (needs_count_in_si_lpres2)
  bool	IMPLEMENT (has_recycle_dot_bin)
  bool	IMPLEMENT (has_gaa_prefixes)
  bool	IMPLEMENT (has_gaa_on_link_prefix)
  bool	IMPLEMENT (supports_all_posix_ai_flags)

#undef IMPLEMENT
};

extern wincapc wincap;

#endif /* _WINCAP_H */
