/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright © 2019 Keith Packard
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

ENTRY(_start)

MEMORY
{
	flash (rxai!w) : ORIGIN = __flash, LENGTH = __flash_size
	ram (wxa!ri) : ORIGIN = __ram, LENGTH = __ram_size
}

PHDRS
{
	text PT_LOAD;
	ram PT_LOAD;
	ram_init PT_LOAD;
	tls PT_TLS;
}

SECTIONS
{
	__flash = DEFINED(__flash) ? __flash : 0x10000000;
	__flash_size = DEFINED(__flash_size) ? __flash_size : 0x10000;

	__ram = DEFINED(__ram) ? __ram : 0x20000000;
	__ram_size = DEFINED(__ram_size) ? __ram_size : 0x8000;

	__stack_size = DEFINED(__stack_size) ? __stack_size : 0x400;

	PROVIDE(__stack_size = __stack_size);
	PROVIDE(__stack__ = ORIGIN(ram) + __ram_size);

	.init : {
		KEEP (*(.text.init.enter))
		KEEP (*(SORT_BY_NAME(.init) SORT_BY_NAME(.init.*)))
	} >flash AT>flash :text

	.text : {
		*(.text.unlikely .text.unlikely.*)
		*(.text.startup .text.startup.*)
		*(.text .text.*)
		*(.gnu.linkonce.t.*)
	} >flash AT>flash :text

	.fini : {
		KEEP (*(SORT_NONE(.fini)))
	} >flash AT>flash :text

	PROVIDE (__etext = .);
	PROVIDE (_etext = .);
	PROVIDE (etext = .);

	.rodata : {
		*(.rdata)
		*(.rodata .rodata.*)
		*(.gnu.linkonce.r.*)
		. = ALIGN(8);
		*(.srodata.cst16)
		*(.srodata.cst8)
		*(.srodata.cst4)
		*(.srodata.cst2)
		*(.srodata .srodata.*)
	} >flash AT>flash :text

	. = ALIGN(8);

	.preinit_array : {
		PROVIDE_HIDDEN (__preinit_array_start = .);
		KEEP (*(.preinit_array))
		PROVIDE_HIDDEN (__preinit_array_end = .);
	} >flash AT>flash :text

	.init_array : {
		PROVIDE_HIDDEN (__init_array_start = .);
		KEEP (*(SORT_BY_INIT_PRIORITY(.init_array.*) SORT_BY_INIT_PRIORITY(.ctors.*)))
		KEEP (*(.init_array EXCLUDE_FILE (*crtbegin.o *crtbegin?.o *crtend.o *crtend?.o ) .ctors))
		PROVIDE_HIDDEN (__init_array_end = .);
	} >flash AT>flash :text

	.fini_array : {
		PROVIDE_HIDDEN (__fini_array_start = .);
		KEEP (*(SORT_BY_INIT_PRIORITY(.fini_array.*) SORT_BY_INIT_PRIORITY(.dtors.*)))
		KEEP (*(.fini_array EXCLUDE_FILE (*crtbegin.o *crtbegin?.o *crtend.o *crtend?.o ) .dtors))
		PROVIDE_HIDDEN (__fini_array_end = .);
	} >flash AT>flash :text

	.ctors : {
		KEEP (*crtbegin.o(.ctors))
		KEEP (*crtbegin?.o(.ctors))
		KEEP (*(EXCLUDE_FILE (*crtend.o *crtend?.o ) .ctors))
		KEEP (*(SORT(.ctors.*)))
		KEEP (*(.ctors))
	} >flash AT>flash :text

	.dtors : {
		KEEP (*crtbegin.o(.dtors))
		KEEP (*crtbegin?.o(.dtors))
		KEEP (*(EXCLUDE_FILE (*crtend.o *crtend?.o ) .dtors))
		KEEP (*(SORT(.dtors.*)))
		KEEP (*(.dtors))
	} >flash AT>flash :text

	.lalign : {
		. = ALIGN(8);
		PROVIDE(__data_source__ = .);
	} >flash AT>flash :text

	.dalign : {
		. = ALIGN(8);
		PROVIDE(__data_start__ = .);
	} >ram AT>flash :ram_init

	.data : {
		*(.data .data.*)
		*(.gnu.linkonce.d.*)
		. = ALIGN(8);
		PROVIDE( __global_pointer$ = . + 0x800 );
		*(.sdata .sdata.* .sdata2.*)
		*(.gnu.linkonce.s.*)
		. = ALIGN(8);
		*(.srodata.cst16)
		*(.srodata.cst8)
		*(.srodata.cst4)
		*(.srodata.cst2)
		*(.srodata .srodata.*)
	} >ram AT>flash :ram_init

	.tdata : {
		PROVIDE( __tdata_source__ = .);
		*(.tdata .tdata.* .gnu.linkonce.td.*)
	} >flash : tls : text
	PROVIDE(__tdata_size__ = SIZEOF(.tdata));

	.tdata_space : {
		. = . + SIZEOF(.tdata);
		PROVIDE(__tls_base__ = .);
	} >ram

	PROVIDE( __edata = . );
	PROVIDE( _edata = . );
	PROVIDE( edata = . );
	PROVIDE(__data_end__ = .);

	PROVIDE( _fbss = . );
	PROVIDE( __bss_start = . );
	PROVIDE(__bss_start__ = .);

	.tbss (NOLOAD) : {
		*(.tbss .tbss.* .gnu.linkonce.tb.*)
		*(.tcommon)
	} >ram : tls : text
	PROVIDE(__tbss_size__ = SIZEOF(.tbss));

	.bss : {
		*(.sbss*)
		*(.gnu.linkonce.sb.*)
		*(.bss .bss.*)
		*(.gnu.linkonce.b.*)
		*(COMMON)
	} >ram AT>ram :ram
	PROVIDE( __end = . );
	PROVIDE( _end = . );
	PROVIDE( end = . );
	PROVIDE( __bss_end__ = . );

	PROVIDE( __heap_end = __ram + __ram_size );
	PROVIDE( __heap_size = __heap_end - __end );
}

