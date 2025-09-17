#ifndef _RV_MEMCPY_H_
#define _RV_MEMCPY_H_

#include <picolibc.h>

#if defined(__PREFER_SIZE_OVER_SPEED) || defined(__OPTIMIZE_SIZE__)
# define _MACHINE_RISCV_MEMCPY_ASM_
#else
# define _MACHINE_RISCV_MEMCPY_C_
#endif

#endif /* _RV_MEMCPY_H_ */
