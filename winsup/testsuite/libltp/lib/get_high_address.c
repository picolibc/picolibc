/*
 *	(C) COPYRIGHT CRAY RESEARCH, INC.
 *	UNPUBLISHED PROPRIETARY INFORMATION.
 *	ALL RIGHTS RESERVED.
 */

#include <unistd.h> 
#ifdef __CYGWIN__
#include <windows.h>
#endif

char *
get_high_address()
{
#ifdef __CYGWIN__
       return VirtualAlloc (NULL, 4096, MEM_COMMIT, PAGE_NOACCESS) + 2048;
#else
       return (char *)sbrk(0) + 16384;
#endif
}
