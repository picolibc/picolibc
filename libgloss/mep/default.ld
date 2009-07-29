/******************************************************************************
begin-header

  DO NOT EDIT. THIS FILE IS MACHINE-GENERATED 
  FROM THE TEMPLATE FILE gmap_default.ld.
  ALL EDITS WILL BE ERASED WITH NEXT MeP-Integrator RUN. 

  Custom linker script for the MeP Module default.
end-header
*******************************************************************************/

OUTPUT_FORMAT("elf32-mep-little", "elf32-mep",
	      "elf32-mep-little")
OUTPUT_ARCH(mep)
ENTRY(_reset)

MEMORY
{
/* begin-memory */
  VEC	(r) : ORIGIN = 0x00000000, LENGTH = 0x000000b8
  VEC_WARM	(w) : ORIGIN = 0x00200000, LENGTH = 0x000000b8
  HWINIT	(r) : ORIGIN = 0x000000b8, LENGTH = 0x00000148
  RAM1	(r) : ORIGIN = 0x00000200, LENGTH = 0x001ffe00
  RAM2	(w) : ORIGIN = 0x00800000, LENGTH = 0x00800000
/*RAM3	(w) : ORIGIN = 0x80000000, LENGTH = 0x00800000 */
  IMEM0	(w) : ORIGIN = 0x002000b8, LENGTH = 0x00003f48
  IMEM1	(w) : ORIGIN = 0x00204000, LENGTH = 0x00004000
  DMEM0	(w) : ORIGIN = 0x00220000, LENGTH = 0x00008000
  DMEM1	(w) : ORIGIN = 0x00228000, LENGTH = 0x00008000
  DMEM2	(w) : ORIGIN = 0x00230000, LENGTH = 0x00008000
  DMEM3	(w) : ORIGIN = 0x00238000, LENGTH = 0x00008000
/* end-memory */
}

SECTIONS
{
  /* Sections to be placed in the vec area.  */
  .vec : { *(.vec) } >VEC /* VEC-section */

  /* Sections to be placed in the HWINIT area.  */
  .hwinit : { *(.hwinit) } >HWINIT /* HWINIT-section */

  /* Sections to be placed in the ROM area.  */
  .gnu.version   : { *(.gnu.version)	} >RAM1 /* ROM-section */
  .gnu.version_d : { *(.gnu.version_d)	} >RAM1 /* ROM-section */
  .gnu.version_r : { *(.gnu.version_r)	} >RAM1 /* ROM-section */

  /* Sections to be placed in the romdata.s area.  */
  .srodata :
  {
    __sdabase = . + 0x8000;
    *(.srodata) *(.srodata.*) *(.gnu.linkonce.srd.*)
  } >RAM2 /* DATA-section */

  /* Sections to be placed in the data.s area.  */
  .sdata     : 
  {
    *(.sdata) 
    *(.sdata.*)
    *(.gnu.linkonce.s.*)
  } >RAM2 /* DATA-section */
  .sbss      :
  {
    PROVIDE (__sbss_start = .);
    PROVIDE (___sbss_start = .);
    *(.dynsbss)
    *(.sbss)
    *(.sbss.*)
    *(.gnu.linkonce.sb.*)
    *(.scommon)
    PROVIDE (__sbss_end = .);
    PROVIDE (___sbss_end = .);
    /* Assert maximum size */
    __assert_tiny_size = ASSERT ((. < __sdabase) || ((. - __sdabase) <= 0x8000),
	 		         "tiny section overflow");
  } >RAM2 /* DATA-section */

  /* Sections to be placed in the data.m area.  */
  .data    :
  {
    __data_start = . ;
    *(.data)
    *(.data.*)
    *(.gnu.linkonce.d.*)
    SORT(CONSTRUCTORS)
  } >RAM2 /* DATA-section */
  .data1   : { *(.data1) } >RAM2 /* DATA-section */
  .eh_frame : { KEEP (*(.eh_frame))} >RAM2 /* DATA-section */
  .gcc_except_table : { *(.gcc_except_table) *(.gcc_except_table.*) } >RAM2 /* DATA-section */
  .dynamic       : { *(.dynamic) } >RAM2 /* DATA-section */
  .ctors   : 
  {
    /* gcc uses crtbegin.o to find the start of
       the constructors, so we make sure it is
       first.  Because this is a wildcard, it
       doesn't matter if the user does not
       actually link against crtbegin.o; the
       linker won't look for a file to match a
       wildcard.  The wildcard also means that it
       doesn't matter which directory crtbegin.o
       is in.  */
    KEEP (*crtbegin.o(.ctors))
    /* We don't want to include the .ctor section from
       from the crtend.o file until after the sorted ctors.
       The .ctor section from the crtend file contains the
       end of ctors marker and it must be last */
    KEEP (*(EXCLUDE_FILE (*crtend.o ) .ctors))
    KEEP (*(SORT(.ctors.*)))
    KEEP (*(.ctors))
  } >RAM2 /* DATA-section */
  .dtors         :
  {
    KEEP (*crtbegin.o(.dtors))
    KEEP (*(EXCLUDE_FILE (*crtend.o ) .dtors))
    KEEP (*(SORT(.dtors.*)))
    KEEP (*(.dtors))
  } >RAM2 /* DATA-section */
  .jcr :
  {
    . = ALIGN(4);
    *(.jcr)
  } >RAM2 /* DATA-section */
  .got :
  {
    *(.got.plt) *(.got)
    _edata = .;
    PROVIDE (edata = .);
  } >RAM2 /* DATA-section */
  .based :
  {
    __tpbase = .;
    *(.based) *(.based.*) *(.gnu.linkonce.based.*)
    /* Assert maximum size */
    __assert_based_size = ASSERT ((. - __tpbase) <= 0x80,
	                          "based section overflow");
  } >RAM2 /* DATA-section */

  .bss       :
  {
    __bss_start = .;
    *(.dynbss)
    *(.bss)
    *(.bss.*)
    *(.gnu.linkonce.b.*)
    *(COMMON)
    /* Align here to ensure that the .bss section occupies space up to
      _end.  Align after .bss to ensure correct alignment even if the
      .bss section disappears because there are no input sections.  */
    . = ALIGN(32 / 8);
    _end = .;
    PROVIDE (end = .);
  } >RAM2 /* DATA-section */

  /* Sections to be placed in the code.m area.  */
  .init          : 
  { 
    KEEP (*(.init))
  } >RAM2 /* CODE-section */
  .plt      : { *(.plt)	} >RAM2 /* DATA-section */
  .text      :
  {
    *(.text)
    *(.text.*)
    *(.stub)
    /* .gnu.warning sections are handled specially by elf32.em.  */
    *(.gnu.warning)
    *(.gnu.linkonce.t.*)
  } >RAM2 /* CODE-section */ =0
  .vtext ALIGN(8)    :
  {
    *(.vtext) *(.vtext.*) *(.gnu.linkonce.v.*)
  } >RAM2 /* CODE-section */
  .fini      :
  {
    KEEP (*(.fini))
    PROVIDE (__etext = .);
    PROVIDE (_etext = .);
    PROVIDE (etext = .);
  } >RAM2 /* CODE-section */ =0

  /* Sections to be placed in the romdata.m area.  */
  .rodata   : { *(.rodata) *(.rodata.*) *(.gnu.linkonce.r.*) } >RAM2 /* DATA-section */
  .rodata1   : { *(.rodata1) } >RAM2 /* DATA-section */

  /* Sections to be placed in the code.l area.  */
  .ftext : {  *(.ftext) *(.ftext.*) *(.gnu.linkonce.ft.*) } >RAM2 /* CODE-section */
  .vftext ALIGN(8)    :
  { 
    *(.vftext) *(.vftext.*) *(.gnu.linkonce.vf.*)
  } >RAM2 /* CODE-section */

  /* Sections to be placed in the romdata.l area.  */
  .frodata :
  {
    *(.frodata) *(.frodata.*) *(.gnu.linkonce.frd.*)
    __assert_near_size = ASSERT (. <= 0x1000000, "near section overflow");
  } >RAM2 /* DATA-section */

  /* Sections to be placed in the data.l area.  */
  .far : { *(.far) *(.far.*) *(.gnu.linkonce.far.*) } >RAM2 /* DATA-section */
  .farbss :
  { PROVIDE (__farbss_start = .);
    *(.farbss) *(.farbss.*)
    PROVIDE (__farbss_end = .);
  } >RAM2 /* DATA-section */

  /* END-mep-sections */

  .vec_warm :
  {
    /* vec_warm is a place for the startup code to write the interrupt
       vectors.  Allow 0xb8 bytes of space aligned on a 4 byte boundary.  */
    . = ALIGN(4);
    vec_warm = .;
    . += 0xb8;
  } >VEC_WARM

  /* begin-stack-table */
  __stack50 = (__stack - (0 *  (__stack_size / 1)) + 15) / 16 * 16;

  .rostacktab :
  {
    /* Emit a table describing the location of the different stacks.  */
    . = ALIGN(4);
    __stack_table = .;
    LONG (__stack50);
    LONG (__stack50);
    LONG (__stack50);
    LONG (__stack50);
    LONG (__stack50);
    LONG (__stack50);
    LONG (__stack50);
    LONG (__stack50);
    LONG (__stack50);
    LONG (__stack50);
    LONG (__stack50);
    LONG (__stack50);
    LONG (__stack50);
    LONG (__stack50);
    LONG (__stack50);
    LONG (__stack50);
    LONG (__stack50);
    LONG (__stack50);
    LONG (__stack50);
    LONG (__stack50);
    LONG (__stack50);
    LONG (__stack50);
    LONG (__stack50);
    LONG (__stack50);
    LONG (__stack50);
    LONG (__stack50);
    LONG (__stack50);
    LONG (__stack50);
    LONG (__stack50);
    LONG (__stack50);
    LONG (__stack50);
    LONG (__stack50);
    LONG (__stack50);
    LONG (__stack50);
    LONG (__stack50);
    LONG (__stack50);
    LONG (__stack50);
    LONG (__stack50);
    LONG (__stack50);
    LONG (__stack50);
    LONG (__stack50);
    LONG (__stack50);
    LONG (__stack50);
    LONG (__stack50);
    LONG (__stack50);
    LONG (__stack50);
    LONG (__stack50);
    LONG (__stack50);
    LONG (__stack50);
    LONG (__stack50);
    LONG (__stack50);
    __stack_end = .;
  } >RAM2
  /* end-stack-table */

  /* begin-heap */
  /* End of DATA is 0x00800000 + 0x00800000. That's where the heap will end.  */
  __heap_end = 0x00800000 + 0x00800000 - 1;
  /* end-heap */

  /* Default stack size is 1M.  That's where the heap will start if there's
     room.  If there's not enough room, allocate half of the remaining space
     for stack and half for heap.  Align the heap on a 16 byte boundary.  */
  __stack_size = (__stack_end + 0x100000 <= __heap_end + 1 
	          ? 0x100000
 		  : ((__heap_end + 1 - __stack_end) / 2));
  __heap = (__stack_end + __stack_size + 15) / 16 * 16;

  /* Leave 16 bytes between the stack and the heap.  */
  __stack = __heap - 0x10;

  /* Stabs debugging sections.  */
  .stab 0 : { *(.stab) }
  .stabstr 0 : { *(.stabstr) }
  .stab.excl 0 : { *(.stab.excl) }
  .stab.exclstr 0 : { *(.stab.exclstr) }
  .stab.index 0 : { *(.stab.index) }
  .stab.indexstr 0 : { *(.stab.indexstr) }
  .comment 0 : { *(.comment) }
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
  .debug_info     0 : { *(.debug_info) *(.gnu.linkonce.wi.*) }
  .debug_abbrev   0 : { *(.debug_abbrev) }
  .debug_line     0 : { *(.debug_line) }
  .debug_frame    0 : { *(.debug_frame) }
  .debug_str      0 : { *(.debug_str) }
  .debug_loc      0 : { *(.debug_loc) }
  .debug_macinfo  0 : { *(.debug_macinfo) }
  .debug_ranges   0 : { *(.debug_ranges) }
  /* SGI/MIPS DWARF 2 extensions */
  .debug_weaknames 0 : { *(.debug_weaknames) }
  .debug_funcnames 0 : { *(.debug_funcnames) }
  .debug_typenames 0 : { *(.debug_typenames) }
  .debug_varnames  0 : { *(.debug_varnames) }
  /* These must appear regardless of  .  */
}
