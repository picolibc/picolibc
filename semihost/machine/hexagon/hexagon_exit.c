#include "hexagon_semihost.h"
#include <unistd.h>

__noreturn void
_exit(int status)
{
    /* simulator reads exit status from r2 */
    /* simulator reads system call code from r0 */
    register uintptr_t r0 __asm__("r0") = status;
    __asm__ __volatile__("r2 = %0\n"
                         "r0 = #24\n" SWI
                         : "=r"(r0)
                         :
                         : "memory");
    /* Should never execute */
    __builtin_unreachable();
}
