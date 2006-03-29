/* This is file dpmi.h */
/*
** Copyright (C) 1993 DJ Delorie
**
** This file is distributed under the terms listed in the document
** "copying.dj".
** A copy of "copying.dj" should accompany this file; if not, a copy
** should be available from where this file was obtained.  This file
** may not be distributed without a verbatim copy of "copying.dj".
**
** This file is distributed WITHOUT ANY WARRANTY; without even the implied
** warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
*/

#ifndef _DPMI_H_
#define _DPMI_H_


#include <sys/types.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef union {
  struct {
    u_long edi;
    u_long esi;
    u_long ebp;
    u_long res;
    u_long ebx;
    u_long edx;
    u_long ecx;
    u_long eax;
  } d;
  struct {
    u_short di, di_hi;
    u_short si, si_hi;
    u_short bp, bp_hi;
    u_short res, res_hi;
    u_short bx, bx_hi;
    u_short dx, dx_hi;
    u_short cx, cx_hi;
    u_short ax, ax_hi;
    u_short flags;
    u_short es;
    u_short ds;
    u_short fs;
    u_short gs;
    u_short ip;
    u_short cs;
    u_short sp;
    u_short ss;
  } x;
  struct {
    u_char edi[4];
    u_char esi[4];
    u_char ebp[4];
    u_char res[4];
    u_char bl, bh, ebx_b2, ebx_b3;
    u_char dl, dh, edx_b2, edx_b3;
    u_char cl, ch, ecx_b2, ecx_b3;
    u_char al, ah, eax_b2, eax_b3;
  } h;
} _go32_dpmi_registers;

typedef struct {
  u_long  size;
  u_long  pm_offset;
  u_short pm_selector;
  u_short rm_offset;
  u_short rm_segment;
} _go32_dpmi_seginfo;

typedef struct {
  u_long available_memory;
  u_long available_pages;
  u_long available_lockable_pages;
  u_long linear_space;
  u_long unlocked_pages;
  u_long available_physical_pages;
  u_long total_physical_pages;
  u_long free_linear_space;
  u_long max_pages_in_paging_file;
  u_long reserved[3];
} _go32_dpmi_meminfo;

/* returns zero if success, else dpmi error and info->size is max size */
int _go32_dpmi_allocate_dos_memory(_go32_dpmi_seginfo *info);
	/* set size to bytes/16, call, use rm_segment.  Do not
	   change anthing but size until the memory is freed.
	   If error, max size is returned in size as bytes/16. */
int _go32_dpmi_free_dos_memory(_go32_dpmi_seginfo *info);
	/* set new size to bytes/16, call.  If error, max size
	   is returned in size as bytes/16 */
int _go32_dpmi_resize_dos_memory(_go32_dpmi_seginfo *info);
	/* uses pm_selector to free memory */

/* These both use the rm_segment:rm_offset fields only */
int _go32_dpmi_get_real_mode_interrupt_vector(int vector, _go32_dpmi_seginfo *info);
int _go32_dpmi_set_real_mode_interrupt_vector(int vector, _go32_dpmi_seginfo *info);

/* These do NOT wrap the function in pm_offset in an iret handler.
   You must provide an assembler interface yourself, or alloc one below.
   You may NOT longjmp out of an interrupt handler. */
int _go32_dpmi_get_protected_mode_interrupt_vector(int vector, _go32_dpmi_seginfo *info);
	/* puts vector in pm_selector:pm_offset. */
int _go32_dpmi_set_protected_mode_interrupt_vector(int vector, _go32_dpmi_seginfo *info);
	/* sets vector from pm_offset and pm_selector */
int _go32_dpmi_chain_protected_mode_interrupt_vector(int vector, _go32_dpmi_seginfo *info);
	/* sets up wrapper that calls function in pm_offset, chaining to old
	   handler when it returns */

/* These can be used to generate assember IRET-style wrappers for functions */
int _go32_dpmi_allocate_iret_wrapper(_go32_dpmi_seginfo *info);
	/* Put function ptr in pm_offset, call, returns wrapper entry in pm_offset. */
int _go32_dpmi_free_iret_wrapper(_go32_dpmi_seginfo *info);
	/* assumes pm_offset points to wrapper, frees it */

/* simulate real mode calls.  CS:IP from regs for non-interrupt */
int _go32_dpmi_simulate_int(int vector, _go32_dpmi_registers *regs);
int _go32_dpmi_simulate_fcall(_go32_dpmi_registers *regs);
int _go32_dpmi_simulate_fcall_iret(_go32_dpmi_registers *regs);

/* These automatically handle the tasks of restructuring the
   real-mode stack for the proper return type.  The callback
   (info->pm_offset) is called as (*pmcb)(_go32_dpmi_registers *regs); */
int _go32_dpmi_allocate_real_mode_callback_retf(_go32_dpmi_seginfo *info, _go32_dpmi_registers *regs);
	/* points callback at pm_offset, returns seg:ofs of callback addr
	   in rm_segment:rm_offset.  Do not change any fields until freed.
	   Interface is added to simulate far return */
int _go32_dpmi_allocate_real_mode_callback_iret(_go32_dpmi_seginfo *info, _go32_dpmi_registers *regs);
	/* same, but simulates iret */
int _go32_dpmi_free_real_mode_callback(_go32_dpmi_seginfo *info);
	/* frees callback */

/* Only available_memory is guaranteed to be valid.  Try
   available_physical_pages for phys mem left */
int _go32_dpmi_get_free_memory_information(_go32_dpmi_meminfo *info);

/* Convenience functions.  These use the above memory info call.
   The return value is *bytes* */
u_long _go32_dpmi_remaining_physical_memory(void);
u_long _go32_dpmi_remaining_virtual_memory(void);


#ifdef __cplusplus
}
#endif

#endif
