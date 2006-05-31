/* Linker script fragment for ARM V7M.  */

SECTIONS {
  /* ARM V7M uses a vector at address zero to dispatch interrupts.  */
  .isr_vector : { *(.isr_vector) } > rom
} 

