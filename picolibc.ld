ENTRY(_start)

MEMORY
{
	flash (rxai!w) : ORIGIN = DEFINED(__flash) ? __flash : 0x20400000, LENGTH = DEFINED(__flash_size) ? __flash_size : 0x1fc00000
	ram (wxa!ri) : ORIGIN = DEFINED(__ram) ? __ram : 0x80000000, LENGTH = DEFINED(__ram_size) ? __ram_size : 0x4000
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
	__ram_size = DEFINED(__ram_size) ? __ram_size : 0x4000;
	__flash_size = DEFINED(__flash_size) ? __flash_size : 0x8000;
	__stack_size = DEFINED(__stack_size) ? __stack_size : 0x400;
	__heap_size = DEFINED(__heap_size) ? __heap_size : 0x400;

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
	PROVIDE( _end = . );
	PROVIDE( end = . );
	PROVIDE( __bss_end__ = . );
}

