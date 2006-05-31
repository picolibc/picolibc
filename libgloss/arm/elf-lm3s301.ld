/* Linker script for Luminary Micro LM3S301.  */

MEMORY {
  rom (rx) : ORIGIN = 0x00000000, LENGTH = 16K
  ram (w)  : ORIGIN = 0x20000000, LENGTH = 2K
}

SECTIONS {
  _stack = 0x20000000 + 2K;
}

INCLUDE ldscripts/armelf.xc
