/* This is file go32.h */
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

#ifndef _GO32_H_
#define _GO32_H_

#include <sys/types.h>

#ifdef __cplusplus
extern "C" {
#endif

/* This must match go32/proginfo.h */

typedef struct {
  u_long size_of_this_structure_in_bytes;
  u_long linear_address_of_primary_screen;
  u_long linear_address_of_secondary_screen;
  u_long linear_address_of_transfer_buffer;
  u_long size_of_transfer_buffer; /* >= 4k */
  u_long pid;
  u_char master_interrupt_controller_base;
  u_char slave_interrupt_controller_base;
  u_short selector_for_linear_memory;
  u_long linear_address_of_stub_info_structure;
  u_long linear_address_of_original_psp;
  u_short run_mode;
  u_short run_mode_info;
} Go32_Info_Block;

extern Go32_Info_Block _go32_info_block;

#define _GO32_RUN_MODE_UNDEF	0
#define _GO32_RUN_MODE_RAW	1
#define _GO32_RUN_MODE_XMS	2
#define _GO32_RUN_MODE_VCPI	3
#define _GO32_RUN_MODE_DPMI	4

void dosmemget(int offset, int length, void *buffer);
void dosmemput(const void *buffer, int length, int offset);
void movedata(unsigned source_selector, unsigned source_offset,
              unsigned dest_selector, unsigned dest_offset,
              size_t length);

/* returns number of times hit since last call. (zero first time) */
u_long _go32_was_ctrl_break_hit();
void   _go32_want_ctrl_break(int yes); /* auto-yes if call above function */

u_short _go32_my_cs();
u_short _go32_my_ds();
u_short _go32_my_ss();
u_short _go32_conventional_mem_selector();

#ifdef __cplusplus
}
#endif

#endif

