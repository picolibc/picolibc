/* Linker script for Luminary Micro LM3S315, LM3S316.  */

MEMORY {
  rom (rx) : ORIGIN = 0x00000000, LENGTH = 16K
  ram (w)  : ORIGIN = 0x20000000, LENGTH = 4K
}

SECTIONS {
  _stack = 0x20000000 + 4K;
}

INCLUDE cm3.ld
INCLUDE ldscripts/armelf.xc
