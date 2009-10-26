/*

Copyright (c) 2005,2008,2009 Red Hat Incorporated.
All rights reserved.

Redistribution and use in source and binary forms, with or without 
modification, are permitted provided that the following conditions are met: 

    Redistributions of source code must retain the above copyright 
    notice, this list of conditions and the following disclaimer.

    Redistributions in binary form must reproduce the above copyright
    notice, this list of conditions and the following disclaimer in the
    documentation and/or other materials provided with the distribution.

    The name of Red Hat Incorporated may not be used to endorse 
    or promote products derived from this software without specific 
    prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" 
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE 
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED.  IN NO EVENT SHALL RED HAT INCORPORATED BE LIABLE FOR ANY
DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND 
ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE. 

*/

/* Default linker script, for normal executables */
OUTPUT_ARCH(rx)
ENTRY(_start)

/* Do we need any of these for elf?
   __DYNAMIC = 0;    */

/* This memory layout corresponds to the smallest predicted RX600 chip.  */
MEMORY {
	RAM (w)   : ORIGIN = 0x00000000, LENGTH = 0x00020000 /* 128k */
	STACK (w) : ORIGIN = 0x00020000, LENGTH = 16 /* top of RAM */

	ROM (w)   : ORIGIN = 0xfff40000, LENGTH = 0x000bffd0 /* 768k */
/* This is the largest RX6000: */
/*	ROM (w)   : ORIGIN = 0xffe00000, LENGTH = 0x001fffd0 */ /* 2Mb */
}

SECTIONS
{
  .text           :
  {
    PROVIDE (_start = .);
    *(.text P .stub .text.* .gnu.linkonce.t.*)
    KEEP (*(.text.*personality*))
    /* .gnu.warning sections are handled specially by elf32.em.  */
    *(.gnu.warning)
    *(.interp .hash .dynsym .dynstr .gnu.version*)
    PROVIDE (__etext = .);
    PROVIDE (_etext = .);
    PROVIDE (etext = .);
    . = ALIGN(4);
    KEEP (*(.init))
    KEEP (*(.fini))
  } > ROM

  .rodata : {
    . = ALIGN(4);
    *(.plt)
    *(.rodata C C_2 C_1 .rodata.* .gnu.linkonce.r.*)
    *(.rodata1)
    *(.eh_frame_hdr)
    KEEP (*(.eh_frame))
    KEEP (*(.gcc_except_table)) *(.gcc_except_table.*)
    . = ALIGN(4);
    PROVIDE(__romdatastart = .);
  } > ROM

  .data : {
    . = ALIGN(4);
    PROVIDE (__datastart = .); /* IF_ROROM */
    PROVIDE (__preinit_array_start = .);
    KEEP (*(.preinit_array))
    PROVIDE (__preinit_array_end = .);
    PROVIDE (__init_array_start = .);
    KEEP (*(SORT(.init_array.*)))
    KEEP (*(.init_array))
    PROVIDE (__init_array_end = .);
    PROVIDE (__fini_array_start = .);
    KEEP (*(.fini_array))
    KEEP (*(SORT(.fini_array.*)))
    PROVIDE (__fini_array_end = .);
    LONG(0); /* Sentinel.  */

    /* gcc uses crtbegin.o to find the start of the constructors, so
       we make sure it is first.  Because this is a wildcard, it
       doesn't matter if the user does not actually link against
       crtbegin.o; the linker won't look for a file to match a
       wildcard.  The wildcard also means that it doesn't matter which
       directory crtbegin.o is in.  */
    KEEP (*crtbegin*.o(.ctors))

    /* We don't want to include the .ctor section from from the
       crtend.o file until after the sorted ctors.  The .ctor section
       from the crtend file contains the end of ctors marker and it
       must be last */
    KEEP (*(EXCLUDE_FILE (*crtend*.o ) .ctors))
    KEEP (*(SORT(.ctors.*)))
    KEEP (*(.ctors))

    KEEP (*crtbegin*.o(.dtors))
    KEEP (*(EXCLUDE_FILE (*crtend*.o ) .dtors))
    KEEP (*(SORT(.dtors.*)))
    KEEP (*(.dtors))

    KEEP (*(.jcr))
    *(.data.rel.ro.local) *(.data.rel.ro*)
    *(.dynamic)

    *(.data D .data.* .gnu.linkonce.d.*)
    KEEP (*(.gnu.linkonce.d.*personality*))
    SORT(CONSTRUCTORS)
    *(.data1)
    *(.got.plt) *(.got)

    /* We want the small data sections together, so single-instruction offsets
       can access them all, and initialized data all before uninitialized, so
       we can shorten the on-disk segment size.  */
    . = ALIGN(4);
    *(.sdata .sdata.* .gnu.linkonce.s.* D_2 D_1)

    . = ALIGN(4);
    _edata = .;
    PROVIDE (edata = .);
    PROVIDE (__dataend = .);
  } > RAM AT> ROM

  /* Note that __romdatacopysize may be ZERO for the simulator, which
     knows how to intialize RAM directly.  It should ONLY be used for
     copying data from ROM to RAM; if you need to know the size of the
     data section, subtract the end symbol from the start symbol.  */
  /* Note that crt0 assumes this is a multiple of four; all the
     start/stop symbols are also assumed long-aligned.  */
  PROVIDE (__romdatacopysize = SIZEOF(.data));

  .bss : {
    . = ALIGN(4);
    PROVIDE (__bssstart = .);
    *(.dynbss)
    *(.bss B B_2 B_1 .bss.* .gnu.linkonce.b.*)
    . = ALIGN(4);
    *(COMMON)
    . = ALIGN(4);
    PROVIDE (__bssend = .);
    _end = .;
    PROVIDE (end = .);
  } > RAM
  PROVIDE (__bsssize = SIZEOF(.bss) / 4);

  .stack (ORIGIN (STACK)) :
  {
    PROVIDE (__stack = .);
    *(.stack)
  }

  /* Providing one of these symbols in your code is sufficient to have
     it linked in to the fixed vector table.  */

  PROVIDE (__rx_priviledged_exception_handler = 0x00000000);
  PROVIDE (__rx_access_exception_handler = 0x00000000);
  PROVIDE (__rx_undefined_exception_handler = 0x00000000);
  PROVIDE (__rx_floating_exception_handler = 0x00000000);
  PROVIDE (__rx_nonmaskable_exception_handler = 0x00000000);

  .vectors (0xFFFFFFD0) :
  {
    PROVIDE (__vectors = .);
    LONG (__rx_priviledged_exception_handler);
    LONG (__rx_access_exception_handler);
    LONG (0);
    LONG (__rx_undefined_exception_handler);
    LONG (0);
    LONG (__rx_floating_exception_handler);
    LONG (0);
    LONG (0);
    LONG (0);
    LONG (0);
    LONG (__rx_nonmaskable_exception_handler);
    LONG (_start);
  }

  /* The rest are all not normally part of the runtime image.  */

  /* Stabs debugging sections.  */
  .stab          0 : { *(.stab) }
  .stabstr       0 : { *(.stabstr) }
  .stab.excl     0 : { *(.stab.excl) }
  .stab.exclstr  0 : { *(.stab.exclstr) }
  .stab.index    0 : { *(.stab.index) }
  .stab.indexstr 0 : { *(.stab.indexstr) }
  .comment       0 : { *(.comment) }
  /* DWARF debug sections.
     Symbols in the DWARF debugging sections are relative to the beginning
     of the section so we begin them at 0.  */
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
  /* SGI/MIPS DWARF 2 extensions */
  .debug_weaknames 0 : { *(.debug_weaknames) }
  .debug_funcnames 0 : { *(.debug_funcnames) }
  .debug_typenames 0 : { *(.debug_typenames) }
  .debug_varnames  0 : { *(.debug_varnames) }
  /DISCARD/ : { *(.note.GNU-stack) }
}
