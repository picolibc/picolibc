#include "hexagon_semihost.h"

struct hexagon_semihost_return
hexagon_semihost(enum hexagon_system_call_code code, int *args)
{
    // Store system call number in r0
    register uintptr_t r0 __asm__("r0") = code;
    // Store pointer to array of system call arguments in r1
    register uintptr_t r1 __asm__("r1") = (int)args;
    __asm__ __volatile__(SWI : "=r"(r0), "=r"(r1) : "r"(r0), "r"(r1));
    struct hexagon_semihost_return retval_errno = { r0, r1 };
    return retval_errno;
}
