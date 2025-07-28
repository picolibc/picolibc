#include "hexagon_semihost.h"

int
hexagon_semihost(enum hexagon_system_call_code code, int *args)
{
    // Store system call number in r0
    register uintptr_t r0 __asm__("r0") = code;
    // Store pointer to array of system call arguments in r1
    register uintptr_t r1 __asm__("r1") = (int)args;
    __asm__ __volatile__(SWI : "=r"(r0), "=r"(r1) : "r"(r0), "r"(r1));
    int retval = r0;
    if (retval == -1) {
        hexagon_semihost_errno(r1);
    }
    return retval;
}
