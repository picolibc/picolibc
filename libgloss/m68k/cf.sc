# a linker script template.
# RAM - start of board's ram
# RAM_SIZE - size of board's ram
# ROM - start of board's rom
# ROM_SIZE - size of board's rom

test -z "${ROM:+1}" && NOROM=1

cat <<EOF
STARTUP(bdm-crt0.o)
OUTPUT_ARCH(m68k)
ENTRY(__start)
SEARCH_DIR(.)
GROUP(-lc -lbdm)
__DYNAMIC  =  0;

MEMORY
{
  ${ROM:+rom (rx) : ORIGIN = ${ROM}, LENGTH = ${ROM_SIZE}}
  ram (rwx) : ORIGIN = ${RAM}, LENGTH = ${RAM_SIZE}
}

/* Place the stack at the end of memory, unless specified otherwise. */
PROVIDE (__stack = ${RAM} + ${RAM_SIZE});

SECTIONS
{
  .text :
  {
    CREATE_OBJECT_SYMBOLS
    bdm-crt0.o(.text)
    *(.text .text.*)

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

    *(.rodata .rodata.*)

    . = ALIGN(0x4);
    *(.gcc_except_table) 

    . = ALIGN(0x4);
    *(.eh_frame)

    . = ALIGN(0x4);
    __INIT_SECTION__ = . ;
    LONG (0x4e560000)	/* linkw %fp,#0 */
    *(.init)
    SHORT (0x4e5e)	/* unlk %fp */
    SHORT (0x4e75)	/* rts */

    . = ALIGN(0x4);
    __FINI_SECTION__ = . ;
    LONG (0x4e560000)	/* linkw %fp,#0 */
    *(.fini)
    SHORT (0x4e5e)	/* unlk %fp */
    SHORT (0x4e75)	/* rts */

    *(.lit)

    . = ALIGN(4);
    _etext = .;
  } >${ROM:+rom}${NOROM:+ram}

  .data :
  {
    __data_load = LOADADDR (.data);
    __data_start = .;
    *(.got.plt) *(.got)
    *(.shdata)
    *(.data .data.*)
    . = ALIGN (4);
    _edata = .;
  } >ram ${ROM:+AT>rom}

  .bss :
  {
    __bss_start = . ;
    *(.shbss)
    *(.bss .bss.*)
    *(COMMON)
    . = ALIGN (8);
    _end = .;
    __end = _end;
  } >ram ${ROM:+AT>rom}

  .stab 0 (NOLOAD) :
  {
    *(.stab)
  }

  .stabstr 0 (NOLOAD) :
  {
    *(.stabstr)
  }
}
EOF
