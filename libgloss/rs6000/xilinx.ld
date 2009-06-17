/* Default linker script, for normal executables 

Copyright (c) 2008, 2009 Xilinx, Inc.  All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are
met:

1.  Redistributions source code must retain the above copyright notice,
this list of conditions and the following disclaimer. 

2.  Redistributions in binary form must reproduce the above copyright
notice, this list of conditions and the following disclaimer in the
documentation and/or other materials provided with the distribution. 

3.  Neither the name of Xilinx nor the names of its contributors may be
used to endorse or promote products derived from this software without
specific prior written permission. 

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDER AND CONTRIBUTORS "AS
IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED
TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

OUTPUT_FORMAT("elf32-powerpc", "elf32-powerpc",
	      "elf32-powerpc")
OUTPUT_ARCH(powerpc:common)
ENTRY(_boot)
_START_ADDR = DEFINED(_START_ADDR) ? _START_ADDR : 0xFFFF0000;
_STACK_SIZE = DEFINED(_STACK_SIZE) ? _STACK_SIZE : 4k;
_HEAP_SIZE = DEFINED(_HEAP_SIZE) ? _HEAP_SIZE : 4k;
STARTUP(boot.o)
/* Do we need any of these for elf?
   __DYNAMIC = 0;    */
PROVIDE (__stack = 0); PROVIDE (___stack = 0);
SECTIONS
{

  . = _START_ADDR;
  .vectors  BLOCK (64k):
  {
    *(.vectors)
  }
  /* Read-only sections, merged into text segment: */
  .hash		  : { *(.hash)		}
  .dynsym	  : { *(.dynsym)		}
  .dynstr	  : { *(.dynstr)		}
  .gnu.version   : { *(.gnu.version)      }
  .gnu.version_d   : { *(.gnu.version_d)  }
  .gnu.version_r   : { *(.gnu.version_r)  }
  .rela.text     :
    {
      *(.rela.text)
      *(.rela.text.*)
      *(.rela.gnu.linkonce.t*)
    }
  .rela.data     :
    {
      *(.rela.data)
      *(.rela.data.*)
      *(.rela.gnu.linkonce.d*)
    }
  .rela.rodata   :
    {
      *(.rela.rodata)
      *(.rela.rodata.*)
      *(.rela.gnu.linkonce.r*)
    }
  .rela.got	  : { *(.rela.got)	}
  .rela.got1	  : { *(.rela.got1)	}
  .rela.got2	  : { *(.rela.got2)	}
  .rela.ctors	  : { *(.rela.ctors)	}
  .rela.dtors	  : { *(.rela.dtors)	}
  .rela.init	  : { *(.rela.init)	}
  .rela.fini	  : { *(.rela.fini)	}
  .rela.bss	  : { *(.rela.bss)	}
  .rela.plt	  : { *(.rela.plt)	}
  .rela.sdata	  : { *(.rela.sdata)	}
  .rela.sbss	  : { *(.rela.sbss)	}
  .rela.sdata2	  : { *(.rela.sdata2)	}
  .rela.sbss2	  : { *(.rela.sbss2)	}
  .text      :
  {
    *(.text)
    *(.text.*)
    /* .gnu.warning sections are handled specially by elf32.em.  */
    *(.gnu.warning)
    *(.gnu.linkonce.t.*)
  } =0
  .init		  : { KEEP (*(.init))	} =0
  .fini		  : { KEEP (*(.fini))	} =0
  .rodata    :
  {
    *(.rodata)
    *(.rodata.*)
    *(.gnu.linkonce.r.*)
  }
  .rodata1	  : { *(.rodata1) }
  _etext = .;
  PROVIDE (etext = .);
  PROVIDE (__etext = .);
  .sdata2   : { *(.sdata2) *(.sdata2.*) *(.gnu.linkonce.s2.*)}
  __SDATA2_START__ = ADDR(.sdata2);
  __SDATA2_END__ = ADDR(.sdata2) + SIZEOF(.sdata2);
  .sbss2   : { *(.sbss2) *(.gnu.linkonce.sb2.*)}
  __SBSS2_START__ = ADDR(.sbss2);
  __SBSS2_END__ = ADDR(.sbss2) + SIZEOF(.sbss2);
  /* Adjust the address for the data segment.  We want to adjust up to
     the same address within the page on the next page up.  It would
     be more correct to do this:
       . = ALIGN(0x08) + (ALIGN(8) & (0x08 - 1));
     The current expression does not correctly handle the case of a
     text segment ending precisely at the end of a page; it causes the
     data segment to skip a page.  The above expression does not have
     this problem, but it will currently (2/95) cause BFD to allocate
     a single segment, combining both text and data, for this case.
     This will prevent the text segment from being shared among
     multiple executions of the program; I think that is more
     important than losing a page of the virtual address space (note
     that no actual memory is lost; the page which is skipped can not
     be referenced).  */
/*  . =  ALIGN(8) + 0x08;*/
  .data    :
  {
    *(.data)
    *(.data.*)
    *(.gnu.linkonce.d*)
    CONSTRUCTORS
  }
  .data1   : { *(.data1) }
  .got1		  : { *(.got1) }
  .dynamic	  : { *(.dynamic) }
  /* Put .ctors and .dtors next to the .got2 section, so that the pointers
     get relocated with -mrelocatable. Also put in the .fixup pointers.
     The current compiler no longer needs this, but keep it around for 2.7.2  */
		PROVIDE (_GOT2_START_ = .);
		PROVIDE (__GOT2_START_ = .);
  .got2		  :  { *(.got2) }
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
		PROVIDE (_FIXUP_START_ = .);
		PROVIDE (__FIXUP_START_ = .);
  .fixup	  : { *(.fixup) }
		PROVIDE (_FIXUP_END_ = .);
		PROVIDE (__FIXUP_END_ = .);
		PROVIDE (_GOT2_END_ = .);
		PROVIDE (__GOT2_END_ = .);
		PROVIDE (_GOT_START_ = .);
		PROVIDE (__GOT_START_ = .);
  .got		  : { *(.got) }
  .got.plt	  : { *(.got.plt) }
		PROVIDE (_GOT_END_ = .);
		PROVIDE (__GOT_END_ = .);
  /* Added by Sathya to handle C++ exceptions */
  .eh_frame   : {
    *(.eh_frame)
  }
  .jcr   : {
    *(.jcr)
  }
  .gcc_except_table   : {
    *(.gcc_except_table)
  }
  /* We want the small data sections together, so single-instruction offsets
     can access them all, and initialized data all before uninitialized, so
     we can shorten the on-disk segment size.  */
  .sdata	  : { *(.sdata) *(.sdata.*) *(.gnu.linkonce.s.*)}
  __SDATA_START__ = ADDR(.sdata);
  __SDATA_END__ = ADDR(.sdata) + SIZEOF(.sdata);
  _edata  =  .;
  PROVIDE (edata = .);
  PROVIDE (__edata = .);
  .sbss      :
  {
    __sbss_start = .;
    ___sbss_start = .;
    *(.sbss)
    *(.sbss.*)
    *(.gnu.linkonce.sb.*)
    *(.scommon)
    *(.dynsbss)
    __sbss_end = .;
    ___sbss_end = .;
  }
  .plt   : { *(.plt) }
  .bss       :
  {
   __bss_start = .;
   ___bss_start = .;
   *(.dynbss)
   *(.bss)
   *(.bss.*)
   *(.gnu.linkonce.b.*)
   *(COMMON)
   . = ALIGN(4);
   __bss_end = .;
  }

  .stack     :
  { 
    _stack_end = .;
    . = . + _STACK_SIZE;
    . = ALIGN(16);
    __stack = .;
  }

  .heap      :
  {
    _heap_start = .;
    . = . + _HEAP_SIZE;
    . = ALIGN(16);
    _heap_end = .;
  }

  .tdata   : {
    *(.tdata)
    *(.tdata.*)
    *(.gnu.linkonce.td.*)
  }
  .tbss   : {
    *(.tbss)
    *(.gnu.linkonce.tb.*)
  }
  /* Check to see if boot0 is going to be within 24bits from boot */
  /* If so, then leave the location counter as is. */
  /* If not, then make location counter to be 0x20 locations above boot location */
  /* This assumes that a BRAM is present for boot and hence 0x20 locations above it are valid */
  /* This fixes CR 183394 */
 __boot0_flag__ = ((. > 0x00ffffff) && (. < 0xff000000)) ? 1 : 0;
 . = (__boot0_flag__) ? 0xffffffdc : .;
  .boot0  : { *(.boot0)}
  _end = . ;
  end = .;
  __end = .;
  .boot  0xFFFFFFFC  : { *(.boot) }
  /* These are needed for ELF backends which have not yet been
     converted to the new style linker.  */
  .stab 0 : { *(.stab) }
  .stabstr 0 : { *(.stabstr) }
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
  .debug_info     0 : { *(.debug_info) }
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
  /* These must appear regardless of  .  */
}
