/* Support files for GNU libc.  Files in the C namespace go here.
   Files in the system namespace (ie those that start with an underscore)
   go in syscalls.c.
   
   Note: These functions are in a seperate file so that OS providers can
   overrride the system call stubs (defined in syscalls.c) without having
   to provide libc funcitons as well.  */
#include "swi.h"



void
alarm (void)
{
}
