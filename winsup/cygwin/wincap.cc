/* wincap.cc -- figure out on which OS we're running. Set the
		capability class to the appropriate values.

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

#include "winsup.h"
#include "miscfuncs.h"
#include "security.h"
#include "ntdll.h"

/* CV, 2008-10-23: All wincapc's have to be in the .cygwin_dll_common section,
   same as wincap itself.  Otherwise the capability changes made in
   wincapc::init() are not propagated to any subsequently started process
   in the same session.  I'm only writing this longish comment because I'm
   puzzled that this has never been noticed before... */

wincaps wincap_7 __attribute__((section (".cygwin_dll_common"), shared)) = {
  def_guard_pages:1,
  mmap_storage_high:0x070000000000LL,
  {
    is_server:false,
    needs_query_information:true,
    has_gaa_largeaddress_bug:true,
    has_precise_system_time:false,
    has_microsoft_accounts:false,
    has_broken_prefetchvm:false,
    has_new_pebteb_region:false,
    has_broken_whoami:true,
    has_unprivileged_createsymlink:false,
    has_precise_interrupt_time:false,
    has_posix_unlink_semantics:false,
    has_posix_unlink_semantics_with_ignore_readonly:false,
    has_case_sensitive_dirs:false,
    has_posix_rename_semantics:false,
    no_msv1_0_s4u_logon_in_wow64:true,
    has_con_24bit_colors:false,
    has_con_broken_csi3j:false,
    has_con_broken_il_dl:false,
    has_con_esc_rep:false,
    has_extended_mem_api:false,
    has_tcp_fastopen:false,
    has_linux_tcp_keepalive_sockopts:false,
    has_tcp_maxrtms:false,
    has_query_process_handle_info:false,
    has_con_broken_tabs:false,
  },
};

wincaps wincap_8 __attribute__((section (".cygwin_dll_common"), shared)) = {
  def_guard_pages:2,
  mmap_storage_high:0x070000000000LL,
  {
    is_server:false,
    needs_query_information:true,
    has_gaa_largeaddress_bug:false,
    has_precise_system_time:true,
    has_microsoft_accounts:true,
    has_broken_prefetchvm:false,
    has_new_pebteb_region:false,
    has_broken_whoami:false,
    has_unprivileged_createsymlink:false,
    has_precise_interrupt_time:false,
    has_posix_unlink_semantics:false,
    has_posix_unlink_semantics_with_ignore_readonly:false,
    has_case_sensitive_dirs:false,
    has_posix_rename_semantics:false,
    no_msv1_0_s4u_logon_in_wow64:false,
    has_con_24bit_colors:false,
    has_con_broken_csi3j:false,
    has_con_broken_il_dl:false,
    has_con_esc_rep:false,
    has_extended_mem_api:false,
    has_tcp_fastopen:false,
    has_linux_tcp_keepalive_sockopts:false,
    has_tcp_maxrtms:false,
    has_query_process_handle_info:true,
    has_con_broken_tabs:false,
  },
};

wincaps wincap_8_1 __attribute__((section (".cygwin_dll_common"), shared)) = {
  def_guard_pages:2,
  mmap_storage_high:0x700000000000LL,
  {
    is_server:false,
    needs_query_information:false,
    has_gaa_largeaddress_bug:false,
    has_precise_system_time:true,
    has_microsoft_accounts:true,
    has_broken_prefetchvm:false,
    has_new_pebteb_region:false,
    has_broken_whoami:false,
    has_unprivileged_createsymlink:false,
    has_precise_interrupt_time:false,
    has_posix_unlink_semantics:false,
    has_posix_unlink_semantics_with_ignore_readonly:false,
    has_case_sensitive_dirs:false,
    has_posix_rename_semantics:false,
    no_msv1_0_s4u_logon_in_wow64:false,
    has_con_24bit_colors:false,
    has_con_broken_csi3j:false,
    has_con_broken_il_dl:false,
    has_con_esc_rep:false,
    has_extended_mem_api:false,
    has_tcp_fastopen:false,
    has_linux_tcp_keepalive_sockopts:false,
    has_tcp_maxrtms:false,
    has_query_process_handle_info:true,
    has_con_broken_tabs:false,
  },
};

wincaps  wincap_10_1507 __attribute__((section (".cygwin_dll_common"), shared)) = {
  def_guard_pages:2,
  mmap_storage_high:0x700000000000LL,
  {
    is_server:false,
    needs_query_information:false,
    has_gaa_largeaddress_bug:false,
    has_precise_system_time:true,
    has_microsoft_accounts:true,
    has_broken_prefetchvm:true,
    has_new_pebteb_region:false,
    has_broken_whoami:false,
    has_unprivileged_createsymlink:false,
    has_precise_interrupt_time:true,
    has_posix_unlink_semantics:false,
    has_posix_unlink_semantics_with_ignore_readonly:false,
    has_case_sensitive_dirs:false,
    has_posix_rename_semantics:false,
    no_msv1_0_s4u_logon_in_wow64:false,
    has_con_24bit_colors:false,
    has_con_broken_csi3j:false,
    has_con_broken_il_dl:false,
    has_con_esc_rep:false,
    has_extended_mem_api:false,
    has_tcp_fastopen:false,
    has_linux_tcp_keepalive_sockopts:false,
    has_tcp_maxrtms:false,
    has_query_process_handle_info:true,
    has_con_broken_tabs:false,
  },
};

wincaps  wincap_10_1607 __attribute__((section (".cygwin_dll_common"), shared)) = {
  def_guard_pages:2,
  mmap_storage_high:0x700000000000LL,
  {
    is_server:false,
    needs_query_information:false,
    has_gaa_largeaddress_bug:false,
    has_precise_system_time:true,
    has_microsoft_accounts:true,
    has_broken_prefetchvm:true,
    has_new_pebteb_region:false,
    has_broken_whoami:false,
    has_unprivileged_createsymlink:false,
    has_precise_interrupt_time:true,
    has_posix_unlink_semantics:false,
    has_posix_unlink_semantics_with_ignore_readonly:false,
    has_case_sensitive_dirs:false,
    has_posix_rename_semantics:false,
    no_msv1_0_s4u_logon_in_wow64:false,
    has_con_24bit_colors:false,
    has_con_broken_csi3j:false,
    has_con_broken_il_dl:false,
    has_con_esc_rep:false,
    has_extended_mem_api:false,
    has_tcp_fastopen:true,
    has_linux_tcp_keepalive_sockopts:false,
    has_tcp_maxrtms:true,
    has_query_process_handle_info:true,
    has_con_broken_tabs:false,
  },
};

wincaps wincap_10_1703 __attribute__((section (".cygwin_dll_common"), shared)) = {
  def_guard_pages:2,
  mmap_storage_high:0x700000000000LL,
  {
    is_server:false,
    needs_query_information:false,
    has_gaa_largeaddress_bug:false,
    has_precise_system_time:true,
    has_microsoft_accounts:true,
    has_broken_prefetchvm:false,
    has_new_pebteb_region:true,
    has_broken_whoami:false,
    has_unprivileged_createsymlink:true,
    has_precise_interrupt_time:true,
    has_posix_unlink_semantics:false,
    has_posix_unlink_semantics_with_ignore_readonly:false,
    has_case_sensitive_dirs:false,
    has_posix_rename_semantics:false,
    no_msv1_0_s4u_logon_in_wow64:false,
    has_con_24bit_colors:true,
    has_con_broken_csi3j:false,
    has_con_broken_il_dl:false,
    has_con_esc_rep:false,
    has_extended_mem_api:false,
    has_tcp_fastopen:true,
    has_linux_tcp_keepalive_sockopts:false,
    has_tcp_maxrtms:true,
    has_query_process_handle_info:true,
    has_con_broken_tabs:true,
  },
};

wincaps wincap_10_1709 __attribute__((section (".cygwin_dll_common"), shared)) = {
  def_guard_pages:2,
  mmap_storage_high:0x700000000000LL,
  {
    is_server:false,
    needs_query_information:false,
    has_gaa_largeaddress_bug:false,
    has_precise_system_time:true,
    has_microsoft_accounts:true,
    has_broken_prefetchvm:false,
    has_new_pebteb_region:true,
    has_broken_whoami:false,
    has_unprivileged_createsymlink:true,
    has_precise_interrupt_time:true,
    has_posix_unlink_semantics:true,
    has_posix_unlink_semantics_with_ignore_readonly:false,
    has_case_sensitive_dirs:false,
    has_posix_rename_semantics:false,
    no_msv1_0_s4u_logon_in_wow64:false,
    has_con_24bit_colors:true,
    has_con_broken_csi3j:false,
    has_con_broken_il_dl:false,
    has_con_esc_rep:false,
    has_extended_mem_api:false,
    has_tcp_fastopen:true,
    has_linux_tcp_keepalive_sockopts:true,
    has_tcp_maxrtms:true,
    has_query_process_handle_info:true,
    has_con_broken_tabs:true,
  },
};

wincaps wincap_10_1803 __attribute__((section (".cygwin_dll_common"), shared)) = {
  def_guard_pages:2,
  mmap_storage_high:0x700000000000LL,
  {
    is_server:false,
    needs_query_information:false,
    has_gaa_largeaddress_bug:false,
    has_precise_system_time:true,
    has_microsoft_accounts:true,
    has_broken_prefetchvm:false,
    has_new_pebteb_region:true,
    has_broken_whoami:false,
    has_unprivileged_createsymlink:true,
    has_precise_interrupt_time:true,
    has_posix_unlink_semantics:true,
    has_posix_unlink_semantics_with_ignore_readonly:false,
    has_case_sensitive_dirs:true,
    has_posix_rename_semantics:false,
    no_msv1_0_s4u_logon_in_wow64:false,
    has_con_24bit_colors:true,
    has_con_broken_csi3j:false,
    has_con_broken_il_dl:false,
    has_con_esc_rep:false,
    has_extended_mem_api:true,
    has_tcp_fastopen:true,
    has_linux_tcp_keepalive_sockopts:true,
    has_tcp_maxrtms:true,
    has_query_process_handle_info:true,
    has_con_broken_tabs:true,
  },
};

wincaps wincap_10_1809 __attribute__((section (".cygwin_dll_common"), shared)) = {
  def_guard_pages:2,
  mmap_storage_high:0x700000000000LL,
  {
    is_server:false,
    needs_query_information:false,
    has_gaa_largeaddress_bug:false,
    has_precise_system_time:true,
    has_microsoft_accounts:true,
    has_broken_prefetchvm:false,
    has_new_pebteb_region:true,
    has_broken_whoami:false,
    has_unprivileged_createsymlink:true,
    has_precise_interrupt_time:true,
    has_posix_unlink_semantics:true,
    has_posix_unlink_semantics_with_ignore_readonly:true,
    has_case_sensitive_dirs:true,
    has_posix_rename_semantics:true,
    no_msv1_0_s4u_logon_in_wow64:false,
    has_con_24bit_colors:true,
    has_con_broken_csi3j:true,
    has_con_broken_il_dl:false,
    has_con_esc_rep:false,
    has_extended_mem_api:true,
    has_tcp_fastopen:true,
    has_linux_tcp_keepalive_sockopts:true,
    has_tcp_maxrtms:true,
    has_query_process_handle_info:true,
    has_con_broken_tabs:true,
  },
};

wincaps wincap_10_1903 __attribute__((section (".cygwin_dll_common"), shared)) = {
  def_guard_pages:2,
  mmap_storage_high:0x700000000000LL,
  {
    is_server:false,
    needs_query_information:false,
    has_gaa_largeaddress_bug:false,
    has_precise_system_time:true,
    has_microsoft_accounts:true,
    has_broken_prefetchvm:false,
    has_new_pebteb_region:true,
    has_broken_whoami:false,
    has_unprivileged_createsymlink:true,
    has_precise_interrupt_time:true,
    has_posix_unlink_semantics:true,
    has_posix_unlink_semantics_with_ignore_readonly:true,
    has_case_sensitive_dirs:true,
    has_posix_rename_semantics:true,
    no_msv1_0_s4u_logon_in_wow64:false,
    has_con_24bit_colors:true,
    has_con_broken_csi3j:false,
    has_con_broken_il_dl:true,
    has_con_esc_rep:true,
    has_extended_mem_api:true,
    has_tcp_fastopen:true,
    has_linux_tcp_keepalive_sockopts:true,
    has_tcp_maxrtms:true,
    has_query_process_handle_info:true,
    has_con_broken_tabs:true,
  },
};

wincaps wincap_10_2004 __attribute__((section (".cygwin_dll_common"), shared)) = {
  def_guard_pages:2,
  mmap_storage_high:0x700000000000LL,
  {
    is_server:false,
    needs_query_information:false,
    has_gaa_largeaddress_bug:false,
    has_precise_system_time:true,
    has_microsoft_accounts:true,
    has_broken_prefetchvm:false,
    has_new_pebteb_region:true,
    has_broken_whoami:false,
    has_unprivileged_createsymlink:true,
    has_precise_interrupt_time:true,
    has_posix_unlink_semantics:true,
    has_posix_unlink_semantics_with_ignore_readonly:true,
    has_case_sensitive_dirs:true,
    has_posix_rename_semantics:true,
    no_msv1_0_s4u_logon_in_wow64:false,
    has_con_24bit_colors:true,
    has_con_broken_csi3j:false,
    has_con_broken_il_dl:false,
    has_con_esc_rep:true,
    has_extended_mem_api:true,
    has_tcp_fastopen:true,
    has_linux_tcp_keepalive_sockopts:true,
    has_tcp_maxrtms:true,
    has_query_process_handle_info:true,
    has_con_broken_tabs:true,
  },
};

wincaps wincap_11 __attribute__((section (".cygwin_dll_common"), shared)) = {
  def_guard_pages:2,
  mmap_storage_high:0x700000000000LL,
  {
    is_server:false,
    needs_query_information:false,
    has_gaa_largeaddress_bug:false,
    has_precise_system_time:true,
    has_microsoft_accounts:true,
    has_broken_prefetchvm:false,
    has_new_pebteb_region:true,
    has_broken_whoami:false,
    has_unprivileged_createsymlink:true,
    has_precise_interrupt_time:true,
    has_posix_unlink_semantics:true,
    has_posix_unlink_semantics_with_ignore_readonly:true,
    has_case_sensitive_dirs:true,
    has_posix_rename_semantics:true,
    no_msv1_0_s4u_logon_in_wow64:false,
    has_con_24bit_colors:true,
    has_con_broken_csi3j:false,
    has_con_broken_il_dl:false,
    has_con_esc_rep:true,
    has_extended_mem_api:true,
    has_tcp_fastopen:true,
    has_linux_tcp_keepalive_sockopts:true,
    has_tcp_maxrtms:true,
    has_query_process_handle_info:true,
    has_con_broken_tabs:false,
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
  version.dwBuildNumber &= 0xffff;

  switch (version.dwMajorVersion)
    {
      case 6:
	switch (version.dwMinorVersion)
	  {
	    case 1:
	      caps = &wincap_7;
	      break;
	    case 2:
	      caps = &wincap_8;
	      break;
	    case 3:
	    default:
	      caps = &wincap_8_1;
	      break;
	  }
	break;
      case 10:
      default:
	if (likely (version.dwBuildNumber >= 22000))
	  caps = &wincap_11;
	else if (version.dwBuildNumber >= 19041)
	  caps = &wincap_10_2004;
	else if (version.dwBuildNumber >= 18362)
	  caps = &wincap_10_1903;
	else if (version.dwBuildNumber >= 17763)
	  caps = &wincap_10_1809;
	else if (version.dwBuildNumber >= 17134)
	  caps = &wincap_10_1803;
	else if (version.dwBuildNumber >= 16299)
	  caps = &wincap_10_1709;
	else if (version.dwBuildNumber >= 15063)
	  caps = &wincap_10_1703;
	else if (version.dwBuildNumber >= 14393)
	  caps = &wincap_10_1607;
	else
	  caps = & wincap_10_1507;
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
      ((wincaps *)caps)->has_gaa_largeaddress_bug = false;
      ((wincaps *)caps)->has_broken_prefetchvm = false;
      ((wincaps *)caps)->no_msv1_0_s4u_logon_in_wow64 = false;
    }

  __small_sprintf (osnam, "NT-%d.%d", version.dwMajorVersion,
		   version.dwMinorVersion);
}
