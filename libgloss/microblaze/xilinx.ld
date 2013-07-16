/* Default linker script, for normal executables */
/*SEARCH_DIR("/proj/fv1/gnu_builds/head/mb/release/lin/mb/microblaze-xilinx-elf/lib");*/
/*SEARCH_DIR("/proj/fv1/gnu_builds/head/mb/release/lin/mb/microblaze-xilinx-elf/lib");*/
ENTRY(_start)
_TEXT_START_ADDR = DEFINED(_TEXT_START_ADDR) ? _TEXT_START_ADDR : 0x50;
_HEAP_SIZE = DEFINED(_HEAP_SIZE) ? _HEAP_SIZE : 0x0;
_STACK_SIZE = DEFINED(_STACK_SIZE) ? _STACK_SIZE : 0x400;
SECTIONS
{
  .vectors.reset 0x0 : { KEEP (*(.vectors.reset)) } = 0
  .vectors.sw_exception 0x8 : { KEEP (*(.vectors.sw_exception)) } = 0
  .vectors.interrupt 0x10 : { KEEP (*(.vectors.interrupt)) } = 0
  .vectors.debug_sw_break 0x18 : { KEEP (*(.vectors.debug_sw_break)) } = 0
  .vectors.hw_exception 0x20 : { KEEP (*(.vectors.hw_exception)) } = 0
  . = _TEXT_START_ADDR;
   _ftext  =  .;
  .text : {
    *(.text)
    *(.text.*)
    *(.gnu.linkonce.t.*)
  }
   _etext  =  .;
  .init : { KEEP (*(.init))	} =0
  .fini : { KEEP (*(.fini))	} =0
  PROVIDE (__CTOR_LIST__ = .);
  PROVIDE (___CTOR_LIST__ = .);
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
    KEEP (*(EXCLUDE_FILE (*crtend.o) .ctors))
    KEEP (*(SORT(.ctors.*)))
    KEEP (*(.ctors))
  }
  PROVIDE (__CTOR_END__ = .);
  PROVIDE (___CTOR_END__ = .);
  PROVIDE (__DTOR_LIST__ = .);
  PROVIDE (___DTOR_LIST__ = .);
   .dtors         :
  {
    KEEP (*crtbegin.o(.dtors))
    KEEP (*(EXCLUDE_FILE (*crtend.o) .dtors))
    KEEP (*(SORT(.dtors.*)))
    KEEP (*(.dtors))
  }
  PROVIDE (__DTOR_END__ = .);
  PROVIDE (___DTOR_END__ = .);
   . = ALIGN(4);
    _frodata = . ;
  .rodata : {
    *(.rodata)
    *(.rodata.*)
    *(.gnu.linkonce.r.*)
    CONSTRUCTORS; /* Is this needed? */
  }
   _erodata = .;
  /* Alignments by 8 to ensure that _SDA2_BASE_ on a word boundary */
  /* Note that .sdata2 and .sbss2 must be contiguous */
  . = ALIGN(8);
   _ssrw = .;
  .sdata2 : {
    *(.sdata2)
    *(.sdata2.*)
    *(.gnu.linkonce.s2.*)
  }
  . = ALIGN(4);
  .sbss2 : {
    PROVIDE (__sbss2_start = .);
    *(.sbss2)
    *(.sbss2.*)
    *(.gnu.linkonce.sb2.*)
    PROVIDE (__sbss2_end = .);
  }
  . = ALIGN(8);
   _essrw = .;
   _ssrw_size = _essrw - _ssrw;
   PROVIDE (_SDA2_BASE_ = _ssrw + (_ssrw_size / 2 ));
   . = ALIGN(4);
   _fdata = .;
  .data : {
    *(.data)
    *(.gnu.linkonce.d.*)
    CONSTRUCTORS; /* Is this needed? */
  }
   _edata = . ;
   /* Added to handle pic code */
  .got : {
    *(.got)
  }
  .got1 : {
    *(.got1)
  }
  .got2 : {
    *(.got2)
  }
  /* Added by Sathya to handle C++ exceptions */
  .eh_frame : {
    *(.eh_frame)
  }
  .jcr : {
    *(.jcr)
  }
  .gcc_except_table : {
    *(.gcc_except_table)
  }
  /* Alignments by 8 to ensure that _SDA_BASE_ on a word boundary */
  /* Note that .sdata and .sbss must be contiguous */
  . = ALIGN(8);
   _ssro = .;
  .sdata : {
    *(.sdata)
    *(.sdata.*)
    *(.gnu.linkonce.s.*)
  }
  . = ALIGN(4);
  .sbss : {
    PROVIDE (__sbss_start = .);
    *(.sbss)
    *(.sbss.*)
    *(.gnu.linkonce.sb.*)
    PROVIDE (__sbss_end = .);
  }
  . = ALIGN(8);
   _essro = .;
   _ssro_size = _essro - _ssro;
  PROVIDE (_SDA_BASE_ = _ssro + (_ssro_size / 2 ));
  . = ALIGN(4);
   _fbss = .;
  .bss : {
    PROVIDE (__bss_start = .);
    *(.bss)
    *(.bss.*)
    *(.gnu.linkonce.b.*)
    *(COMMON)
    . = ALIGN(4);
    PROVIDE (__bss_end = .);
  }

  . = ALIGN(4);
  .heap : {
     _heap = .;
     _heap_start = .;
     . += _HEAP_SIZE;
     _heap_end = .;
  }

  . = ALIGN(4);
  .stack : {
     _stack_end = .;
     . += _STACK_SIZE;
     . = ALIGN(8);
     _stack = .;
     _end = .;
  }

  .tdata : {
    *(.tdata)
    *(.tdata.*)
    *(.gnu.linkonce.td.*)
  }
  .tbss : {
    *(.tbss)
    *(.tbss.*)
    *(.gnu.linkonce.tb.*)
  }
}
