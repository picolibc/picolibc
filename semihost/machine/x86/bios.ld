MEMORY
{
	bios (rxai!w) : ORIGIN = 0xffff0000, LENGTH = 0x10000
}

PHDRS
{
	text PT_LOAD;
}

SECTIONS {
	.text : {
		*(.text*)
	} >bios AT>bios :text

	/DISCARD/ : {
		*(*)
	}
}
