#ifndef _RV_MEMMOVE_H_
#define _RV_MEMMOVE_H_

#include <picolibc.h>

#if !defined(__PREFER_SIZE_OVER_SPEED) && !defined(__OPTIMIZE_SIZE__)
# define _MACHINE_RISCV_MEMMOVE_GENERIC_
#else
# define _MACHINE_RISCV_MEMMOVE_ASM_
#endif

#endif /* _RV_MEMMOVE_H_ */
