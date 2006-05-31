/* Linker script for Luminary Micro LM3S101, LM3S102.  */

MEMORY {
  rom (rx) : ORIGIN = 0x00000000, LENGTH = 8K
  ram (w)  : ORIGIN = 0x20000000, LENGTH = 2K
}

SECTIONS {
  _stack = 0x20000000 + 2K;
}

INCLUDE cm3.ld
INCLUDE ldscripts/armelf.xc
