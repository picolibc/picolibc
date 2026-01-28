/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright © 2025 Keith Packard
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above
 *    copyright notice, this list of conditions and the following
 *    disclaimer in the documentation and/or other materials provided
 *    with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its
 *    contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
 * INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
 * OF THE POSSIBILITY OF SUCH DAMAGE.
 */

ENTRY(__start)

/*
 * These values should be provided by the application. We'll include
 * some phony values here to make things link for testing
 */

MEMORY
{
	fvector (rx!w) :
		ORIGIN = DEFINED(__fvector) ? __fvector : 0xffffff80,
		LENGTH = DEFINED(__fvector_size) ? __fvector_size : 0x00000080
	flash (rx!w) :
		ORIGIN = DEFINED(___flash) ? ___flash : 0x01000000,
		LENGTH = DEFINED(___flash_size) ? ___flash_size : 0x00800000
	ram (w!rx) :
		ORIGIN = DEFINED(___ram) ? ___ram : 0x01800000,
		LENGTH = DEFINED(___ram_size) ? ___ram_size : 0x00800000
}

PHDRS
{
	text_fvector PT_LOAD;
	text PT_LOAD;
	ram_init PT_LOAD;
	ram PT_LOAD;
	tls_init PT_TLS;
        tls PT_TLS;
}

SECTIONS
{
	PROVIDE(___stack = ORIGIN(ram) + LENGTH(ram));

	.fvector : {
		KEEP (*(.rodata.fvector*))
	} >fvector AT>fvector :text_fvector

	.text : {

		/* code */
		*(.literal.unlikely .text.unlikely .literal.unlikely.* .text.unlikely.*)
		*(.literal.startup .text.startup .literal.startup.* .text.startup.*)
		*(.literal .text .literal.* .text.* .opd .opd.* P P.*)
		*(.gnu.linkonce.t.*)
		KEEP (*(.fini .fini.*))
		___text_end = .;

		PROVIDE (___etext = ___text_end);
		PROVIDE (__etext = ___text_end);
		PROVIDE (_etext = ___text_end);

		/* Need to pre-align so that the symbols come after padding */
		. = ALIGN(8);

		/* lists of constructors and destructors */
		PROVIDE_HIDDEN ( ___preinit_array_start = . );
		KEEP (*(.preinit_array))
		PROVIDE_HIDDEN ( ___preinit_array_end = . );

		PROVIDE_HIDDEN ( ___init_array_start = . );
		KEEP (*(SORT_BY_INIT_PRIORITY(.init_array.*) SORT_BY_INIT_PRIORITY(.ctors.*)))
		KEEP (*(.init_array .ctors))
		PROVIDE_HIDDEN ( ___init_array_end = . );

		PROVIDE_HIDDEN ( ___fini_array_start = . );
		KEEP (*(SORT_BY_INIT_PRIORITY(.fini_array.*) SORT_BY_INIT_PRIORITY(.dtors.*)))
		KEEP (*(.fini_array .dtors))
		KEEP (*(.fini_array*))
		PROVIDE_HIDDEN ( ___fini_array_end = . );

	} >flash AT>flash :text

	.rodata : {

		/* read-only data */
		*(.rdata)
		*(.rodata .rodata.* C C.*)
		*(.gnu.linkonce.r.*)

		*(.srodata.cst16)
		*(.srodata.cst8)
		*(.srodata.cst4)
		*(.srodata.cst2)
		*(.srodata .srodata.*)

	} >flash AT>flash :text
	PROVIDE(__pid_base = ADDR(.rodata));

	.data.rel.ro : {

		/* data that needs relocating */
		*(.data.rel.ro .data.rel.ro.*)

	} >flash AT>flash :text

	/*
	 * Needs to be in its own segment with the PLT entries first
	 * so that the linker will compute the offsets to those
	 * entries correctly.
	 */
	.got : {
		*(.got.plt)
		*(.got)
	} >flash AT>flash :text

	.toc : {
		*(.toc .toc.*)
	} >flash AT>flash :text

	/* additional sections when compiling with C++ exception support */
	/*
	.except_ordered : {
		*(.gcc_except_table *.gcc_except_table.*)
		*(.ARM.extab* .gnu.linkonce.armextab.*)
	} >flash AT>flash :text
	.eh_frame_hdr : {
		PROVIDE_HIDDEN ( ___eh_frame_hdr_start = . );
		*(.eh_frame_hdr) *(.eh_frame_entry .eh_frame_entry.*)
		PROVIDE_HIDDEN ( ___eh_frame_hdr_end = . );
	} >flash AT>flash :text
	.eh_frame : {
		PROVIDE_HIDDEN ( ___eh_frame_start = . );
		KEEP (*(.eh_frame .eh_frame.*))
		PROVIDE_HIDDEN ( ___eh_frame_end = . );
	} >flash AT>flash :text

	.except_unordered : {
		. = ALIGN(8);

		PROVIDE(___exidx_start = .);
		*(.ARM.exidx*)
		PROVIDE(___exidx_end = .);
	} >flash AT>flash :text
	*/

	/*
	 * Data values which are preserved across reset
	 */
	.preserve (NOLOAD) : {
		PROVIDE(___preserve_start__ = .);
		KEEP(*(SORT_BY_NAME(.preserve.*)))
		KEEP(*(.preserve))
		PROVIDE(___preserve_end__ = .);
	} >ram AT>ram :ram

	.data :  ALIGN_WITH_INPUT  {
		*(.data .data.* D D_1 D.* D_1.*)
		*(.gnu.linkonce.d.*)

		/* Need to pre-align so that the symbols come after padding */
		. = ALIGN(8);

		PROVIDE( ___global_pointer$ = . + 0x800 );
		PROVIDE( __gp = . + 0x8000);
		*(.sdata .sdata.* .sdata2.* D_2 D_2.*)
		*(.gnu.linkonce.s.*)
	} >ram AT>flash :ram_init
	PROVIDE(___data_start = ADDR(.data));
	PROVIDE(___data_source = LOADADDR(.data));

	/* Thread local initialized data. This gets
	 * space allocated as it is expected to be placed
	 * in ram to be used as a template for TLS data blocks
	 * allocated at runtime. We're slightly abusing that
	 * by placing the data in flash where it will be copied
	 * into the allocate ram addresses by the existing
	 * data initialization code in crt0.
	 * BFD includes .tbss alignment when computing .tdata
	 * alignment, but for ld.lld we have to explicitly pad
	 * as it only guarantees usage as a TLS template works
	 * rather than supporting this use case.
	 */
	.tdata : /* For ld.lld:  ALIGN(___tls_align) */  ALIGN_WITH_INPUT  {
		*(.tdata .tdata.* .gnu.linkonce.td.*)
		PROVIDE(___data_end = .);
		PROVIDE(___tdata_end = .);
	} >ram AT>flash :tls_init :ram_init
	PROVIDE( ___tls_base = ADDR(.tdata));
	PROVIDE( ___tdata_start = ADDR(.tdata));
	PROVIDE( ___tdata_source = LOADADDR(.tdata) );
	PROVIDE( ___tdata_source_end = LOADADDR(.tdata) + SIZEOF(.tdata) );
	PROVIDE( ___data_source_end = ___tdata_source_end );
	PROVIDE( ___tdata_size = SIZEOF(.tdata) );
	PROVIDE( ___tls_inittls = 0 );

	PROVIDE( ___edata = ___data_end );
	PROVIDE( __edata = ___data_end );
	PROVIDE( _edata = ___data_end );
	PROVIDE( ___data_size = ___data_end - ___data_start );
	PROVIDE( ___data_source_size = ___data_source_end - ___data_source );

	.tbss (NOLOAD) : {
		*(.tbss .tbss.* .gnu.linkonce.tb.*)
		*(.tcommon)
		PROVIDE( ___tls_end = . );
		PROVIDE( ___tbss_end = . );
	} >ram AT>ram :tls :ram
	PROVIDE( ___bss_start = ADDR(.tbss));
	PROVIDE( ___tbss_start = ADDR(.tbss));
	PROVIDE( ___tbss_offset = ADDR(.tbss) - ADDR(.tdata) );
	PROVIDE( ___tbss_size = SIZEOF(.tbss) );
	PROVIDE( ___tls_size = ___tls_end - ___tls_base );
	PROVIDE( ___tls_align = MAX(ALIGNOF(.tdata), ALIGNOF(.tbss)) );
	PROVIDE( ___tls_size_align = (___tls_size + ___tls_align - 1) & ~(___tls_align - 1));
	PROVIDE( ___arm32_tls_tcb_offset = MAX(8, ___tls_align) );
	PROVIDE( ___arm64_tls_tcb_offset = MAX(16, ___tls_align) );

	/*
	 * Unlike ld.lld, ld.bfd does not advance the location counter for
	 * .tbss, but we actually need memory allocated for .tbss as we use
	 * it for the initial TLS storage.
	 * Create special section here just to make room.
	 */
        
        .tbss_space (NOLOAD) : {
		. = ADDR(.tbss);
		. = . + SIZEOF(.tbss);
	} >ram AT>ram :ram
	
	.bss (NOLOAD) : {
		*(.sbss* B_2 B_2.*)
		*(.gnu.linkonce.sb.*)
		*(.bss .bss.* B B_1 B.* B_1.*)
		*(.gnu.linkonce.b.*)
		*(COMMON)

		/* Align the heap */
		. = ALIGN(8);
		___bss_end = .;
	} >ram AT>ram :ram
	PROVIDE( ___non_tls_bss_start = ADDR(.bss) );
	PROVIDE( ___end = ___bss_end );
	__end = ___bss_end;
	PROVIDE( _end = ___bss_end );
	PROVIDE( ___bss_size = ___bss_end - ___bss_start );

	/* Make the rest of memory available for heap storage */
	PROVIDE (___heap_start = ___end);
	PROVIDE (___heap_end = ___stack - (DEFINED(___stack_size) ? ___stack_size : 0x00001000));
	PROVIDE (___heap_size = ___heap_end - ___heap_start);

        /* Allow a minimum heap size to be specified */
        .heap (NOLOAD) : {
                . += (DEFINED(___heap_size_min) ? ___heap_size_min : 0);
        } >ram :ram

	/* Define a stack region to make sure it fits in memory */
	.stack (NOLOAD) : {
		. += (DEFINED(___stack_size) ? ___stack_size : 0x00001000);
	} >ram :ram

	/* Throw away C++ exception handling information */

	

	/DISCARD/ : {
		*(.note .note.*)
		*(.eh_frame .eh_frame.*)
		*(.ARM.extab* .gnu.linkonce.armextab.*)
		*(.ARM.exidx*)
	}

	

	/* Stabs debugging sections.  */
	.stab          0 : { *(.stab) }
	.stabstr       0 : { *(.stabstr) }
	.stab.excl     0 : { *(.stab.excl) }
	.stab.exclstr  0 : { *(.stab.exclstr) }
	.stab.index    0 : { *(.stab.index) }
	.stab.indexstr 0 : { *(.stab.indexstr) }
	.comment       0 : { *(.comment) }
	.gnu.build.attributes : { *(.gnu.build.attributes .gnu.build.attributes.*) }
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
	.debug_line     0 : { *(.debug_line .debug_line.* .debug_line_end) }
	.debug_frame    0 : { *(.debug_frame) }
	.debug_str      0 : { *(.debug_str) }
	.debug_loc      0 : { *(.debug_loc) }
	.debug_macinfo  0 : { *(.debug_macinfo) }
	/* SGI/MIPS DWARF 2 extensions.  */
	.debug_weaknames 0 : { *(.debug_weaknames) }
	.debug_funcnames 0 : { *(.debug_funcnames) }
	.debug_typenames 0 : { *(.debug_typenames) }
	.debug_varnames  0 : { *(.debug_varnames) }
	/* DWARF 3.  */
	.debug_pubtypes 0 : { *(.debug_pubtypes) }
	.debug_ranges   0 : { *(.debug_ranges) }
	/* DWARF 5.  */
	.debug_addr     0 : { *(.debug_addr) }
	.debug_line_str 0 : { *(.debug_line_str) }
	.debug_loclists 0 : { *(.debug_loclists) }
	.debug_macro    0 : { *(.debug_macro) }
	.debug_names    0 : { *(.debug_names) }
	.debug_rnglists 0 : { *(.debug_rnglists) }
	.debug_str_offsets 0 : { *(.debug_str_offsets) }
	.debug_sup      0 : { *(.debug_sup) }
	.gnu.attributes 0 : { KEEP (*(.gnu.attributes)) }
}
/*
 * Check that sections that are copied from flash to RAM have matching
 * padding, so that a single memcpy() of __data_size copies the correct bytes.
 */
ASSERT( ___data_size == ___data_source_size,
	"ERROR: .data/.tdata flash size does not match RAM size");
