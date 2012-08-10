/* wincap.h: Header for OS capability class.

   Copyright 2001, 2002, 2003, 2004, 2005, 2006, 2007, 2008,
   2009, 2010, 2011 Red Hat, Inc.

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

#ifndef _WINCAP_H
#define _WINCAP_H

struct wincaps
{
  DWORD    max_sys_priv;
  unsigned is_server                                    : 1;
  unsigned has_physical_mem_access                      : 1;
  unsigned has_create_global_privilege			: 1;
  unsigned has_ioctl_storage_get_media_types_ex		: 1;
  unsigned has_disk_ex_ioctls				: 1;
  unsigned has_buggy_restart_scan			: 1;
  unsigned has_mandatory_integrity_control		: 1;
  unsigned needs_logon_sid_in_sid_list			: 1;
  unsigned needs_count_in_si_lpres2			: 1;
  unsigned has_recycle_dot_bin				: 1;
  unsigned has_gaa_prefixes				: 1;
  unsigned has_gaa_on_link_prefix			: 1;
  unsigned supports_all_posix_ai_flags			: 1;
  unsigned has_restricted_stack_args			: 1;
  unsigned has_transactions				: 1;
  unsigned has_recvmsg					: 1;
  unsigned has_sendmsg					: 1;
  unsigned has_broken_udf				: 1;
  unsigned has_console_handle_problem			: 1;
  unsigned has_broken_alloc_console			: 1;
  unsigned has_always_all_codepages			: 1;
  unsigned has_localenames				: 1;
  unsigned has_fast_cwd					: 1;
  unsigned has_restricted_raw_disk_access		: 1;
  unsigned use_dont_resolve_hack			: 1;
  unsigned has_stack_size_param_is_a_reservation	: 1;
  unsigned has_console_logon_sid			: 1;
  unsigned wow64_has_secondary_stack			: 1;
};

class wincapc
{
  SYSTEM_INFO      system_info;
  OSVERSIONINFOEX  version;
  char             osnam[40];
  ULONG            wow64;
  void             *caps;

public:
  void init ();

  const DWORD cpu_count () const { return system_info.dwNumberOfProcessors; }
  const DWORD page_size () const { return system_info.dwPageSize; }
  const DWORD allocation_granularity () const
			      { return system_info.dwAllocationGranularity; }
  const char *osname () const { return osnam; }
  const bool is_wow64 () const { return wow64; }

#define IMPLEMENT(cap) cap() const { return ((wincaps *) this->caps)->cap; }

  DWORD IMPLEMENT (max_sys_priv)
  bool  IMPLEMENT (is_server)
  bool  IMPLEMENT (has_physical_mem_access)
  bool  IMPLEMENT (has_create_global_privilege)
  bool	IMPLEMENT (has_ioctl_storage_get_media_types_ex)
  bool	IMPLEMENT (has_disk_ex_ioctls)
  bool	IMPLEMENT (has_buggy_restart_scan)
  bool	IMPLEMENT (has_mandatory_integrity_control)
  bool	IMPLEMENT (needs_logon_sid_in_sid_list)
  bool	IMPLEMENT (needs_count_in_si_lpres2)
  bool	IMPLEMENT (has_recycle_dot_bin)
  bool	IMPLEMENT (has_gaa_prefixes)
  bool	IMPLEMENT (has_gaa_on_link_prefix)
  bool	IMPLEMENT (supports_all_posix_ai_flags)
  bool	IMPLEMENT (has_restricted_stack_args)
  bool	IMPLEMENT (has_transactions)
  bool	IMPLEMENT (has_recvmsg)
  bool	IMPLEMENT (has_sendmsg)
  bool	IMPLEMENT (has_broken_udf)
  bool	IMPLEMENT (has_console_handle_problem)
  bool	IMPLEMENT (has_broken_alloc_console)
  bool	IMPLEMENT (has_always_all_codepages)
  bool	IMPLEMENT (has_localenames)
  bool	IMPLEMENT (has_fast_cwd)
  bool	IMPLEMENT (has_restricted_raw_disk_access)
  bool	IMPLEMENT (use_dont_resolve_hack)
  bool	IMPLEMENT (has_stack_size_param_is_a_reservation)
  bool	IMPLEMENT (has_console_logon_sid)
  bool	IMPLEMENT (wow64_has_secondary_stack)

#undef IMPLEMENT
};

extern wincapc wincap;

#endif /* _WINCAP_H */
