/* wincap.h: Header for OS capability class.

   Copyright 2001 Red Hat, Inc.

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
  int      shared;
  unsigned is_winnt                                     : 1;
  unsigned access_denied_on_delete                      : 1;
  unsigned has_delete_on_close                          : 1;
  unsigned has_page_guard                               : 1;
  unsigned has_security                                 : 1;
  unsigned has_security_descriptor_control              : 1;
  unsigned has_get_process_times                        : 1;
  unsigned has_specific_access_rights                   : 1;
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
  unsigned has_hard_links                               : 1;
  unsigned can_open_directories                         : 1;
  unsigned has_move_file_ex                             : 1;
  unsigned has_negative_pids                            : 1;
  unsigned has_unreliable_pipes                         : 1;
  unsigned has_try_enter_critical_section		: 1;
  unsigned has_raw_devices				: 1;
  unsigned has_valid_processorlevel			: 1;
};

class wincapc
{
  OSVERSIONINFO version;
  char          osnam[40];
  void          *caps;

public:
  void init ();

  void set_chunksize (DWORD nchunksize);

  const char *osname () const { return osnam; }

#define IMPLEMENT(cap) cap() const { return ((wincaps *)this->caps)->cap; }

  DWORD IMPLEMENT (lock_file_highword)
  DWORD IMPLEMENT (chunksize)
  int   IMPLEMENT (shared)
  bool  IMPLEMENT (is_winnt)
  bool  IMPLEMENT (access_denied_on_delete)
  bool  IMPLEMENT (has_delete_on_close)
  bool  IMPLEMENT (has_page_guard)
  bool  IMPLEMENT (has_security)
  bool  IMPLEMENT (has_security_descriptor_control)
  bool  IMPLEMENT (has_get_process_times)
  bool  IMPLEMENT (has_specific_access_rights)
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
  bool  IMPLEMENT (has_hard_links)
  bool  IMPLEMENT (can_open_directories)
  bool  IMPLEMENT (has_move_file_ex)
  bool  IMPLEMENT (has_negative_pids)
  bool  IMPLEMENT (has_unreliable_pipes)
  bool  IMPLEMENT (has_try_enter_critical_section)
  bool  IMPLEMENT (has_raw_devices)
  bool  IMPLEMENT (has_valid_processorlevel)

#undef IMPLEMENT
};

extern wincapc wincap;

#endif /* _WINCAP_H */
