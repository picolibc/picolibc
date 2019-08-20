ENTRY(_start)

MEMORY
{
	flash (rxai!w) : ORIGIN = 0x20400000, LENGTH = 0x1fc00000
	ram (wxa!ri) : ORIGIN = 0x80000000, LENGTH = 0x4000
}

PHDRS
{
	flash PT_LOAD;
	ram PT_LOAD;
	ram_init PT_LOAD;
}

SECTIONS
{

	__stack_size = DEFINED(__stack_size) ? __stack_size : 0x400;
	PROVIDE(__stack_size = __stack_size);
	__heap_size = DEFINED(__heap_size) ? __heap_size : 0x400;

	.init : {
		KEEP (*(.text.metal.init.enter))
		KEEP (*(SORT_NONE(.init)))
	} >flash AT>flash :flash

	.text : {
		*(.text.unlikely .text.unlikely.*)
		*(.text.startup .text.startup.*)
		*(.text .text.*)
		*(.gnu.linkonce.t.*)
	} >flash AT>flash :flash

	.fini : {
		KEEP (*(SORT_NONE(.fini)))
	} >flash AT>flash :flash

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
	} >flash AT>flash :flash

	. = ALIGN(8);

	.preinit_array : {
		PROVIDE_HIDDEN (__preinit_array_start = .);
		KEEP (*(.preinit_array))
		PROVIDE_HIDDEN (__preinit_array_end = .);
	} >flash AT>flash :flash

	.init_array : {
		PROVIDE_HIDDEN (__init_array_start = .);
		KEEP (*(SORT_BY_INIT_PRIORITY(.init_array.*) SORT_BY_INIT_PRIORITY(.ctors.*)))
		KEEP (*(.init_array EXCLUDE_FILE (*crtbegin.o *crtbegin?.o *crtend.o *crtend?.o ) .ctors))
		PROVIDE_HIDDEN (__init_array_end = .);
	} >flash AT>flash :flash

	.fini_array : {
		PROVIDE_HIDDEN (__fini_array_start = .);
		KEEP (*(SORT_BY_INIT_PRIORITY(.fini_array.*) SORT_BY_INIT_PRIORITY(.dtors.*)))
		KEEP (*(.fini_array EXCLUDE_FILE (*crtbegin.o *crtbegin?.o *crtend.o *crtend?.o ) .dtors))
		PROVIDE_HIDDEN (__fini_array_end = .);
	} >flash AT>flash :flash

	.ctors : {
		KEEP (*crtbegin.o(.ctors))
		KEEP (*crtbegin?.o(.ctors))
		KEEP (*(EXCLUDE_FILE (*crtend.o *crtend?.o ) .ctors))
		KEEP (*(SORT(.ctors.*)))
		KEEP (*(.ctors))
	} >flash AT>flash :flash

	.dtors : {
		KEEP (*crtbegin.o(.dtors))
		KEEP (*crtbegin?.o(.dtors))
		KEEP (*(EXCLUDE_FILE (*crtend.o *crtend?.o ) .dtors))
		KEEP (*(SORT(.dtors.*)))
		KEEP (*(.dtors))
	} >flash AT>flash :flash

	.lalign : {
		. = ALIGN(8);
		PROVIDE(__data_source__ = .);
	} >flash AT>flash :flash

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

	PROVIDE( _edata = . );
	PROVIDE( edata = . );
	PROVIDE(__data_end__ = .);

	. = ALIGN(8);
	PROVIDE( _fbss = . );
	PROVIDE( __bss_start = . );
	PROVIDE(__bss_start__ = .);

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

