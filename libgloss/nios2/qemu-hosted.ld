/* qemu-hosted.ld -- Linker script for QEMU generic-nommu board emulation.

   Copyright (c) 2018 Mentor Graphics

   The authors hereby grant permission to use, copy, modify, distribute,
   and license this software and its documentation for any purpose, provided
   that existing copyright notices are retained in all copies and that this
   notice is included verbatim in any distributions. No written agreement,
   license, or royalty fee is required for any of the authorized uses.
   Modifications to this software may be copyrighted by their authors
   and need not follow the licensing terms described here, provided that
   the new terms are clearly indicated on the first page of each file where
   they apply.
 
   Parts of this file were derived from code written by Altera Corporation
   and distributed under the following license:

   Copyright (c) 2008
   Altera Corporation, San Jose, California, USA.
   All rights reserved.

   Permission is hereby granted, free of charge, to any person obtaining a
   copy of this software and associated documentation files (the "Software"),
   to deal in the Software without restriction, including without limitation
   the rights to use, copy, modify, merge, publish, distribute, sublicense,
   and/or sell copies of the Software, and to permit persons to whom the
   Software is furnished to do so, subject to the following conditions:

   The above copyright notice and this permission notice shall be included in
   all copies or substantial portions of the Software.

   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
   IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
   FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
   AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
   LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
   FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
   DEALINGS IN THE SOFTWARE.

   This agreement shall be governed in all respects by the laws of the State
   of California and by the laws of the United States of America.

*/

ENTRY(__start)
SEARCH_DIR(.)
GROUP(-lgcc -lc -lnios2)

MEMORY
{
  rom (rx) : ORIGIN = 0x0, LENGTH = 64M
  ram (rwx) : ORIGIN = 0x10000000, LENGTH = 128M
  tlbmiss (rwx) : ORIGIN = 0x7fff400, LENGTH = 1K
  descriptor (rwx) : ORIGIN = 0x8002000, LENGTH = 8K
}

/* Force the linker to search for these symbols from
   the start of the link process.  */
EXTERN(__reset)
EXTERN(__start)
EXTERN(main __stack __heap_start __heap_end __heap_end)

/* Where to put the stack and heap.  Allocate 1M heap space.  */
PROVIDE(__heap_start = _end);
PROVIDE(__heap_end = _end + 100000);
PROVIDE(__stack = 0x18000000);

/* Force exit to be picked up in a hosted environment.  */
EXTERN(exit atexit)

/* Ensure that newlib runs the finalizers.  */
__libc_fini = _fini;

SECTIONS
{
  .text :
  {
    _ftext = .;
    *(.text.entry)
    *(.text.start)
    *(.text .text.* .gnu.linkonce.t.*)

    . = ALIGN(0x4);
    KEEP (*crtbegin.o(.jcr))
    KEEP (*(EXCLUDE_FILE (*crtend.o) .jcr))
    KEEP (*crtend.o(.jcr))

    . = ALIGN(0x4);
    *(.gcc_except_table .gcc_except_table.*)
    *(.gnu_extab .gnu_extab.*)
  } > ram
  .eh_frame_hdr : ALIGN (4)
  {
    KEEP (*(.eh_frame_hdr))
    *(.eh_frame_entry .eh_frame_entry.*)
  } > ram
  .eh_frame : ALIGN (4)
  {
    KEEP (*(.eh_frame)) *(.eh_frame.*)
  } > ram
  .note.gnu.build-id : ALIGN (4)
  {
    KEEP (*(.note.gnu.build-id))
  } > ram
  .rodata : ALIGN (4)
  {
    *(.rodata .rodata.* .gnu.linkonce.r.*)

    . = ALIGN(4);
    KEEP(*(.init))

    . = ALIGN(4);
    __preinit_array_start = .;
    KEEP (*(.preinit_array))
    __preinit_array_end = .;

    . = ALIGN(4);
    __init_array_start = .;
    KEEP (*(SORT(.init_array.*)))
    KEEP (*(.init_array))
    __init_array_end = .;

    . = ALIGN(4);
    KEEP(*(.fini))

    . = ALIGN(4);
    __fini_array_start = .;
    KEEP (*(.fini_array))
    KEEP (*(SORT(.fini_array.*)))
    __fini_array_end = .;

    . = ALIGN(0x4);
    KEEP (*crtbegin.o(.ctors))
    KEEP (*(EXCLUDE_FILE (*crtend.o) .ctors))
    KEEP (*(SORT(.ctors.*)))
    KEEP (*crtend.o(.ctors))

    . = ALIGN(0x4);
    KEEP (*crtbegin.o(.dtors))
    KEEP (*(EXCLUDE_FILE (*crtend.o) .dtors))
    KEEP (*(SORT(.dtors.*)))
    KEEP (*crtend.o(.dtors))

    *(.lit)

    . = ALIGN (8);
    _etext = .;
  } > ram

  .data : ALIGN (8)
  {
    *(.shdata)
    *(.data .data.* .gnu.linkonce.d.*)
    . = ALIGN(8);
    _gp = ABSOLUTE(. + 0x8000);
    *(.got.plt) *(.got)
    *(.lit8)
    *(.lit4)
    *(.sdata .sdata.* .gnu.linkonce.s.*)
    . = ALIGN (8);
    *(.ram)
    . = ALIGN (8);
    _edata = .;
  } > ram
  .bss : ALIGN (8)
  {
     __bss_start = ABSOLUTE(.);
    *(.sbss .sbss.* .gnu.linkonce.sb.*)
    *(.scommon)
    *(.bss .bss.* .gnu.linkonce.b.*)
    *(COMMON)
    . = ALIGN (8);
    __bss_end = ABSOLUTE(.);
     _end = .;
    __end = .;
  } > ram

  .stab 0 (NOLOAD) : { *(.stab) }
  .stabstr 0 (NOLOAD) : { *(.stabstr) }
  /* DWARF debug sections.
   * Symbols in the DWARF debugging sections are relative to
   * the beginning of the section so we begin them at 0.
   */
  /* DWARF 1 */
  .debug          0 : { *(.debug) }
  .line           0 : { *(.line) }
  /* GNU DWARF 1 extensions */
  .debug_srcinfo  0 : { *(.debug_srcinfo) }
  .debug_sfnames  0 : { *(.debug_sfnames) }
  /* DWARF 1.1 and DWARF 2 */
  .debug_aranges  0 : { *(.debug_aranges) }
  .debug_pubnames 0 : { *(.debug_pubnames) }
  /* DWARF 2 */
  .debug_info     0 : { *(.debug_info .gnu.linkonce.wi.*) }
  .debug_abbrev   0 : { *(.debug_abbrev) }
  .debug_line     0 : { *(.debug_line) }
  .debug_frame    0 : { *(.debug_frame) }
  .debug_str      0 : { *(.debug_str) }
  .debug_loc      0 : { *(.debug_loc) }
  .debug_macinfo  0 : { *(.debug_macinfo) }
  /* DWARF 2.1 */
  .debug_ranges   0 : { *(.debug_ranges) }
  /* SGI/MIPS DWARF 2 extensions */
  .debug_weaknames 0 : { *(.debug_weaknames) }
  .debug_funcnames 0 : { *(.debug_funcnames) }
  .debug_typenames 0 : { *(.debug_typenames) }
  .debug_varnames  0 : { *(.debug_varnames) }

  .debug_alt_sim_info 0 : { *(.debug_alt_sim_info) }
}
