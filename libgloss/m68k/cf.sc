# a linker script template.
# RAMSTART - start of board's ram
# RAMSIZE - size of board's ram
# RAMDBUG - bytes at start of RAM for DBUG use
# ISV - nonnull if interrupt service vector should be provided.

cat <<EOF
STARTUP(crt0.o)
OUTPUT_ARCH(m68k)
ENTRY(start)
SEARCH_DIR(.)
__DYNAMIC  =  0;

MEMORY
{
  ram (rwx) : ORIGIN = ${RAMSTART} + ${RAMDBUG:-0},
		 LENGTH = ${RAMSIZE} - ${RAMDBUG:-0}
}

/* Place the stack at the end of memory, unless specified otherwise. */
PROVIDE (__stack = ${RAMSTART} + ${RAMSIZE});

/* Inhibit an interrupt vector, if one is not specified.  */
PROVIDE (__interrupt_vector = -1);

/*
 * Initalize some symbols to be zero so we can reference them in the
 * crt0 without core dumping. These functions are all optional, but
 * we do this so we can have our crt0 always use them if they exist. 
 */
PROVIDE (hardware_init_hook = 0);
PROVIDE (software_init_hook = 0);
/*
 * stick everything in ram (of course)
 */
SECTIONS
{
  .text :
  {
    CREATE_OBJECT_SYMBOLS
    ${ISV+__interrupt_vector = .; . += 256 * 4;}
    *(.text .text.*)

    . = ALIGN(0x4);
    /* These are for running static constructors and destructors under ELF.  */
    KEEP (*crtbegin.o(.ctors))
    KEEP (*(EXCLUDE_FILE (*crtend.o) .ctors))
    KEEP (*(SORT(.ctors.*)))
    KEEP (*(.ctors))
    KEEP (*crtbegin.o(.dtors))
    KEEP (*(EXCLUDE_FILE (*crtend.o) .dtors))
    KEEP (*(SORT(.dtors.*)))
    KEEP (*(.dtors))

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

    _etext = .;
    *(.lit)
  } > ram

  .data :
  {
    *(.got.plt) *(.got)
    *(.shdata)
    *(.data .data.*)
    _edata = .;
  } > ram

  .bss :
  {
    . = ALIGN(0x4);
    __bss_start = . ;
    *(.shbss)
    *(.bss .bss.*)
    *(COMMON)
    _end =  ALIGN (0x8);
    __end = _end;
  } > ram

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
