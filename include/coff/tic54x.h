/* TI COFF information for Texas Instruments TMS320C54X.  This file customizes 
   the settings in coff/ti.h. */
#ifndef COFF_TIC54X_H
#define COFF_TIC54X_H
#define TIC54X_TARGET_ID 0x98
#define TIC54XALGMAGIC  0x009B  /* c54x algebraic assembler output */
#define TIC5X_TARGET_ID  0x92
#define TI_TARGET_ID TIC54X_TARGET_ID
#define OCTETS_PER_BYTE_POWER 1 /* octets per byte, as a power of two */
#define HOWTO_BANK 6 /* add to howto to get absolute/sect-relative version */
#define TICOFF_TARGET_ARCH bfd_arch_tic54x
#define TICOFF_DEFAULT_MAGIC TICOFF1MAGIC /* we use COFF1 for compatibility */
#include "coff/ti.h"
#endif /* COFF_TIC54X_H */
