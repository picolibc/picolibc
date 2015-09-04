OUTPUT_FORMAT("elf32-ft32")
OUTPUT_ARCH(ft32)

SEARCH_DIR(.)
GROUP(-lsim -lc -lgcc -lsim)

MEMORY
{
  flash     (rx)   : ORIGIN = 0, LENGTH = 256K
  ram       (rw!x) : ORIGIN = 0x800000, LENGTH = 64K
}
SECTIONS
{
  .text :
  {
    *(.text*)
    *(.strings)
    *(._pm*)
    *(.init)
    *(.fini)
     _etext = . ;
  }  > flash
  .tors :
  {
    ___ctors = . ;
    *(.ctors)
    ___ctors_end = . ;
    ___dtors = . ;
    *(.dtors)
    ___dtors_end = . ;
  } > ram
  .data          : AT (ADDR (.text) + SIZEOF (.text))
  {
    *(.data)
    *(.rodata)
    *(.rodata*)
     _edata = . ;
  }  > ram
  .bss  SIZEOF(.data) + ADDR(.data) :
  {
     _bss_start = . ;
    *(.bss)
    *(COMMON)
     _end = . ;
  }  > ram
   __data_load_start = LOADADDR(.data);
   __data_load_end = __data_load_start + SIZEOF(.data);
  .stab 0 (NOLOAD) :
  {
    *(.stab)
  }
  .stabstr 0 (NOLOAD) :
  {
    *(.stabstr)
  }
}
