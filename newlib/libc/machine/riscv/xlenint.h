#ifndef _XLENINT_H
#define _XLENINT_H

#include <stdint.h>

#if __riscv_xlen == 64
typedef uint64_t uintxlen_t;
#elif __riscv_xlen == 32
typedef uint32_t uintxlen_t;
#else
#error __riscv_xlen must equal 32 or 64
#endif

#endif /* _XLENINT_H */
