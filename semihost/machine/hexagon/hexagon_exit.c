#include "hexagon_semihost.h"
#include <unistd.h>

__noreturn void
_exit(int status)
{
    /* simulator reads exit status from r2 */
    /* simulator reads system call code from r0 */
    register uintptr_t r2 __asm__("r2") = status;
    register uintptr_t r0 __asm__("r0") = SYS_EXIT;
    __asm__ __volatile__(SWI : : "r"(r0), "r"(r2) : "memory");
    /* Should never execute */
    __builtin_unreachable();
}
