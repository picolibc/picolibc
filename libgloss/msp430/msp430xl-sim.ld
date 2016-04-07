/* Copyright (c) 2013-2015 Red Hat, Inc. All rights reserved.

   This copyrighted material is made available to anyone wishing to use,
   modify, copy, or redistribute it subject to the terms and conditions of
   the BSD License.   This program is distributed in the hope that it will be
   useful, but WITHOUT ANY WARRANTY expressed or implied, including the
   implied warranties of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
   A copy of this license is available at http://www.opensource.org/licenses.

   Any Red Hat trademarks that are incorporated in the source code or
   documentation are not subject to the BSD License and may only be used or
   replicated with the express permission of Red Hat, Inc.  */

/* Example linker script, for large MSP430X executables.  */

OUTPUT_ARCH(msp430)
ENTRY(_start)

INCLUDE intr_vectors.ld

/* Note - These memory regions are just examples.  Real MSP430 MCUs will have
   different varieties and sizes of RAM, ROM and FLASH.  Not all devices will
   have all of these regions either.  Device specific linker scripts are
   provided by TI, so this file is intended to be used as a guide and so that
   toolchain tests can be run against the simulator.  */
MEMORY
{
  RAM (rw)     : ORIGIN = 0x00500, LENGTH = 0x01b00
  ROM (rx)     : ORIGIN = 0x02000, LENGTH = 0x0df00
  /* The regions from intr_vectors.ld go here.  */
  HIFRAM (rw)  : ORIGIN = 0x10000, LENGTH = 0x80000
  HIROM (rx)   : ORIGIN = 0x90000, LENGTH = 0x70000
}

SECTIONS
{
  .resetvec :
  {
    *(.resetvec)
  } > VECT31

  .rodata :
  {
    . = ALIGN(2);
    *(.plt)

    . = ALIGN(2);
    *(.lower.rodata.* .lower.rodata)

    /* Note: By default we do not have this line:

         *(.either.rodata.*) *(.either.rodata)

       defined here, or anywhere else in this script.  This is deliberate.
       The algorithm in the linker that automatically places rodata into
       either the .rodata or the .upper.rodata sections relies upon the
       fact that the .either.rodata section is not defined, and that the
       .upper.rodata section is defined.  If the .upper.rodata is not
       defined in this script then the line above should be restored so that
       code compiled with -mdata-region=either enabled will still work.

       The same reasoning applies to the absence of definitions for the
       .either.text, .either.data and .either.bss sections as well.  */

    . = ALIGN(2);
    *(.rodata .rodata.* .gnu.linkonce.r.* .const .const:*)
    *(.rodata1)

    . = ALIGN(2);
    KEEP (*(.gcc_except_table)) *(.gcc_except_table.*)
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

  } > ROM

  /* Note: This is a separate .rodata section for sections which are
     read only but which older linkers treat as read-write.
     This prevents older linkers from marking the entire .rodata
     section as read-write.  */
  .rodata2 : {
    . = ALIGN(2);
    *(.eh_frame_hdr)
    KEEP (*(.eh_frame))

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
  } > ROM

  .upper.rodata :
  {
    /* Note: If this section is not defined then please add:
    
         *(.either.rodata.*) *(.either.rodata)

       to the definition of the .rodata section above.  This
       will allow code compiled with -mdata-region=either to
       work properly.  */

    . = ALIGN(2);
    *(.upper.rodata.* .upper.rodata)
  } > HIROM

  .data :
  {
    . = ALIGN(2);
    PROVIDE (__datastart = .);
    *(.lower.data.* .lower.data)

    . = ALIGN(2);
    KEEP (*(.jcr))
    *(.data.rel.ro.local) *(.data.rel.ro*)
    *(.dynamic)

    . = ALIGN(2);
    *(.data .data.* .gnu.linkonce.d.*)

    /* See the note in .rodata section about why we do not have this line here:

        *(.either.data.* .either.data)
    */

    KEEP (*(.gnu.linkonce.d.*personality*))
    SORT(CONSTRUCTORS)
    *(.data1)
    *(.got.plt) *(.got)

    /* We want the small data sections together, so single-instruction offsets
       can access them all, and initialized data all before uninitialized, so
       we can shorten the on-disk segment size.  */
    . = ALIGN(2);
    *(.sdata .sdata.* .gnu.linkonce.s.* D_2 D_1)

    . = ALIGN(2);
    _edata = .;
    PROVIDE (edata = .);
    PROVIDE (__dataend = .);

    /* See the comment in the .upper.data section about the need
       to copy data from ROM into RAM at program start up.  */
  } > RAM AT> ROM

  /* Note that crt0 assumes that __romdatacopysize is a multiple of two.
     All the start/stop symbols are also assumed to be word-aligned.  */
  __romdatastart = LOADADDR(.data);
  __romdatacopysize = SIZEOF(.data);

  /* ------------------- start of .upper.data sections.----------------  */
  /* Note: If both HIROM and HIFRAM are available then the .upper.data
     section should look like this:

        . = ALIGN(2);
        .upper.data :
        {
          __upper_data_init = LOADADDR (.upper.data);
          /* Status word.  * /
          SHORT(1);
          __high_datastart = .;
          *(.upper.data.* .upper.data)
          __high_dataend = .;
	} > HIFRAM AT> HIROM

        __rom_highdatacopysize = SIZEOF(.upper.data) - 2;
        __rom_highdatastart = LOADADDR(.upper.data) + 2;
  
      If only HIFRAM is available then the layout below must look like this:

        .upper.data :
        {
          . = ALIGN(2);
          __high_datastart = .;
          *(.upper.data.* .upper.data)
          __high_dataend = .;
        } > HIFRAM

	__rom_highdatacopysize = SIZEOF(.upper.data);

	.shadow.upper.data :
        {
          . = ALIGN(2);
          __upper_data_init = .;
          /* Status word.  * /
          SHORT(0);
          /* Space for the copy of .upper.data.  * /
          . = . + SIZEOF(.upper.data) - 2;
         } > HIFRAM

        __rom_highdatastart = LOADADDR(.shadow.upper.data) + 2;

      Note - remove the space in this sequence: * / (twice) when you copy one
      of the script fragments above into your script.

      Note - the symbols defined here are *not* enclosed by the PROVIDE
      keyword.  This is deliberate.  The crt0 library provides weak
      definitions of these symbols and those weak definitions *must* be
      overriden by the correct values.

      The status word is used to control how the .upper.data section
      is initialized at application start up.  If the word is non-zero
      then data is copied from __rom_highdatastart to __high_datastart.
      This corresponds with copying the contents of .upper.data from its
      load address (HIROM) to its run-time address (HIFRAM) in the first
      scenario, or from the .shadow.upper.section to the .upper.data in
      the second scenario.

      If the status word is zero then the data is copied the other way
      and the word is set to one.  This only happens when the second
      scenario is in play, and only the very first time the application
      starts running.  This makes sure that the .shadow.upper.data section
      contains a pristine copy of the .upper.data section that can be used
      to reinitialize the .upper.data section upon device reset.

      The status word is necessary as this allows us to have one routine
      in crt0 that can handle either form of .upper.data layout.  IE crt0
      is linker script agnostic.

      Note - if the .upper.data section is not going to be defined at all
      then please add this line back into the .data section above:

        *(.either.data.* .either.data)
    */

  . = ALIGN(2);
  .upper.data :
  {
    __upper_data_init = LOADADDR (.upper.data);
    /* Status word.  */
    SHORT(1);
    __high_datastart = .;
    *(.upper.data.* .upper.data)
    __high_dataend = .;
  } > HIFRAM AT> HIROM
  
  __rom_highdatacopysize = SIZEOF(.upper.data) - 2;
  __rom_highdatastart = LOADADDR(.upper.data) + 2;

  /* ------------------- end of .upper.data sections.----------------  */

  .bss :
  {
    . = ALIGN(2);
    PROVIDE (__bssstart = .);
    *(.lower.bss.* .lower.bss)
    *(.dynbss)
    *(.sbss .sbss.*)
    *(.bss .bss.* .gnu.linkonce.b.*)
    /* See the note in .rodata section about why we do not have this line here:

        *(.either.bss.* .either.bss)
    */
    . = ALIGN(2);
    *(COMMON)
    . = ALIGN(2);
    PROVIDE (__bssend = .);
  } > RAM
  PROVIDE (__bsssize = SIZEOF(.bss));

  /* This section contains data that is not initialised during load
     *or* application reset.  */
  .noinit (NOLOAD) :
  {
    . = ALIGN(2);
    PROVIDE (__noinit_start = .);
    *(.noinit)
    . = ALIGN(2);
    PROVIDE (__noinit_end = .);
  } > RAM

  /* This section contains data that *is* initialised during load
     but *not* on application reset.  This section should be in FLASH.  */
  .persistent :
  {
    . = ALIGN(2);
    PROVIDE (__persistent_start = .);
    *(.persistent)
    . = ALIGN(2);
    PROVIDE (__persistent_end = .);
  } > HIFRAM
 
  .upper.bss :
  {
    /* Note - if this section is not going to be defined then please
       add this line back into the definition of the .bss section above:
       
      *(.either.bss.* .either.bss)
    */
    . = ALIGN(2);
    __high_bssstart = .;
    *(.upper.bss.* .upper.bss)
    . = ALIGN(2);
    __high_bssend = .;
  } > HIFRAM
  __high_bsssize = SIZEOF(.upper.bss);

  /* We create this section so that "end" will always be in the
     HIFRAM region (matching .stack below), even if the .upper.bss
     section is empty.  */
  .heap_start :
  {
    . = ALIGN(2);
    _end = .;
    PROVIDE (end = .);
    LONG(0);
  } > HIFRAM

  /* The __stack_size value of 0x100 is just a guess, but since it is
     PROVIDEd the user can override it on the command line.  It has to be
     set here, rather than inside the .stack section, as symbols defined
     inside sections are only evaluated during the final phase of the link,
     long after the ASSERT is checked.  An ASSERT referencing a PROVIDED but
     not yet evaluated symbol will automatically fail.

     FIXME: It would be nice if this value could be automatically set via
     gcc's -fstack-usage command line option somehow.  */
  PROVIDE (__stack_size = 0x100);

  /* Note: We place the stack in HIFRAM because then there is less
     chance that it will collide with allocated data in the RAM region.
     In scripts targeted at real MCUs however it may be better to place
     the stack and heap in RAM, as flash does have a limited number of
     writes before failure.

     Note - if the location of .stack is changed, then be sure to change
     the definition of .heap_start above as well.  */
  .stack (ORIGIN (HIFRAM) + LENGTH (HIFRAM)) :
  {
    PROVIDE (__stack = .);

    /* Linker section checking ignores empty sections like
       this one so we have to have our own test here.  */
    ASSERT ((__stack > (_end + __stack_size)),
            "Error: Too much data - no room left for the stack");
  }

  .text :
  {
    PROVIDE (_start = .);

    . = ALIGN(2);
    KEEP (*(SORT(.crt_*)))

    . = ALIGN(2);
    KEEP (*(.lowtext))

    . = ALIGN(2);
    *(.lower.text.* .lower.text)

    . = ALIGN(2);
    *(.text .stub .text.* .gnu.linkonce.t.* .text:*)

    /* See the note in .rodata section about why we do not have this line here:

        *(.either.text.* .either.text)
    */
    
    KEEP (*(.text.*personality*))
    /* .gnu.warning sections are handled specially by elf32.em.  */
    *(.gnu.warning)
    *(.interp .hash .dynsym .dynstr .gnu.version*)
    PROVIDE (__etext = .);
    PROVIDE (_etext = .);
    PROVIDE (etext = .);
    . = ALIGN(2);
    KEEP (*(.init))
    KEEP (*(.fini))
    KEEP (*(.tm_clone_table))
  } > ROM

  .upper.text :
  {
    /* Note - if this section is not going to be included in the script
       then please add this line back into the definition of the .text
       section above:

       *(.either.text.* .either.text)
    */
    . = ALIGN(2);
    *(.upper.text.* .upper.text)
  } > HIROM

  /* The rest are all not normally part of the runtime image.  */

  .MSP430.attributes 0 :
  {
    KEEP (*(.MSP430.attributes))
    KEEP (*(.gnu.attributes))
    KEEP (*(__TI_build_attributes))
  }

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
  /* DWARF 1.  */
  .debug          0 : { *(.debug) }
  .line           0 : { *(.line) }
  /* GNU DWARF 1 extensions.  */
  .debug_srcinfo  0 : { *(.debug_srcinfo) }
  .debug_sfnames  0 : { *(.debug_sfnames) }
  /* DWARF 1.1 and DWARF 2.  */
  .debug_aranges  0 : { *(.debug_aranges) }
  .debug_pubnames 0 : { *(.debug_pubnames) }
  /* DWARF 2.  */
  .debug_info     0 : { *(.debug_info .gnu.linkonce.wi.*) }
  .debug_abbrev   0 : { *(.debug_abbrev) }
  .debug_line     0 : { *(.debug_line .debug_line.* .debug_line_end ) }
  .debug_frame    0 : { *(.debug_frame) }
  .debug_str      0 : { *(.debug_str) }
  .debug_loc      0 : { *(.debug_loc) }
  .debug_macinfo  0 : { *(.debug_macinfo) }
  /* SGI/MIPS DWARF 2 extensions.  */
  .debug_weaknames 0 : { *(.debug_weaknames) }
  .debug_funcnames 0 : { *(.debug_funcnames) }
  .debug_typenames 0 : { *(.debug_typenames) }
  .debug_varnames  0 : { *(.debug_varnames) }
  /* DWARF 3 */
  .debug_pubtypes 0 : { *(.debug_pubtypes) }
  .debug_ranges   0 : { *(.debug_ranges) }
  /* DWARF Extension.  */
  .debug_macro    0 : { *(.debug_macro) }

  /DISCARD/ : { *(.note.GNU-stack) }
}
