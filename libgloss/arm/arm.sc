# a linker script template.
# RAM - start of board's ram
# RAM_SIZE - size of board's ram
# ROM - start of board's rom
# ROM_SIZE - size of board's rom
# $1 whether to set _stack

test "x$1" = "xyes" && SETSTACK=1
test -z "${ROM_SIZE:+1}" && NOROM=1

cat <<EOF
OUTPUT_FORMAT("elf32-littlearm", "elf32-bigarm",
	      "elf32-littlearm")
STARTUP(${CRT0})
GROUP(-lc ${BSP} -lgcc)
OUTPUT_ARCH(arm)
ENTRY(_start)
SEARCH_DIR(.)
__DYNAMIC  =  0;

MEMORY
{
  ${ROM:+rom (rx) : ORIGIN = ${ROM}, LENGTH = ${ROM_SIZE}}
  ram (rwx) : ORIGIN = ${RAM}, LENGTH = ${RAM_SIZE}
}

/* Place the stack at the end of memory, unless specified otherwise. */
${SETSTACK:+PROVIDE (__stack = ${RAM} + ${RAM_SIZE});}

SECTIONS
{
  .text :
  {
    CREATE_OBJECT_SYMBOLS
    ${ROM:+*(.isr_vector)}
    *(.text .text.* .gnu.linkonce.t.*)
    *(.plt)
    *(.gnu.warning)
    *(.glue_7t) *(.glue_7)

    . = ALIGN(0x4);
    /* These are for running static constructors and destructors under ELF.  */
    KEEP (*crtbegin.o(.ctors))
    KEEP (*(EXCLUDE_FILE (*crtend.o) .ctors))
    KEEP (*(SORT(.ctors.*)))
    KEEP (*crtend.o(.ctors))
    KEEP (*crtbegin.o(.dtors))
    KEEP (*(EXCLUDE_FILE (*crtend.o) .dtors))
    KEEP (*(SORT(.dtors.*)))
    KEEP (*crtend.o(.dtors))

    *(.rodata .rodata.* .gnu.linkonce.r.*)

    *(.ARM.extab .gnu.linkonce.armextab.*)
    *(.gcc_except_table) 
    *(.eh_frame_hdr)
    *(.eh_frame)

    *(.init)
    *(.fini)

    PROVIDE_HIDDEN (__preinit_array_start = .);
    KEEP (*(.preinit_array))
    PROVIDE_HIDDEN (__preinit_array_end = .);
    PROVIDE_HIDDEN (__init_array_start = .);
    KEEP (*(SORT(.init_array.*)))
    KEEP (*(.init_array))
    PROVIDE_HIDDEN (__init_array_end = .);
    PROVIDE_HIDDEN (__fini_array_start = .);
    KEEP (*(.fini_array))
    KEEP (*(SORT(.fini_array.*)))
    PROVIDE_HIDDEN (__fini_array_end = .);
  } >${ROM:+rom}${NOROM:+ram}
  /* .ARM.exidx is sorted, so has to go in its own output section.  */
   __exidx_start = .;
  .ARM.exidx :
  {
    *(.ARM.exidx* .gnu.linkonce.armexidx.*)
  } >${ROM:+rom}${NOROM:+ram}
  __exidx_end = .;
  _etext = .;

  .data :
  {
    __data_load = LOADADDR (.data);
    __data_start = .;
    KEEP(*(.jcr))
    *(.got.plt) *(.got)
    *(.shdata)
    *(.data .data.* .gnu.linkonce.d.*)
    . = ALIGN (4);
    _edata = .;
  } >ram ${ROM:+AT>rom}

  .bss :
  {
    __bss_start__ = . ;
    *(.shbss)
    *(.bss .bss.* .gnu.linkonce.b.*)
    *(COMMON)
    . = ALIGN (8);
    __bss_end__ = .;
    _end = .;
    __end = _end;
    PROVIDE(end = .);
  } >ram ${ROM:+AT>rom}

  .stab 0 (NOLOAD) :
  {
    *(.stab)
  }

  .stabstr 0 (NOLOAD) :
  {
    *(.stabstr)
  }
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
  .note.gnu.arm.ident 0 : { KEEP (*(.note.gnu.arm.ident)) }
  .ARM.attributes 0 : { KEEP (*(.ARM.attributes)) }
  /DISCARD/ : { *(.note.GNU-stack) ${NOROM:+*(.isr_vector)} }
}
EOF

exit 0
