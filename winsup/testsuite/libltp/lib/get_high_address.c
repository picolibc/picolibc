/* $Header$ */

/*
 *	(C) COPYRIGHT CRAY RESEARCH, INC.
 *	UNPUBLISHED PROPRIETARY INFORMATION.
 *	ALL RIGHTS RESERVED.
 */

#include <unistd.h> 

char *
get_high_address()
{
       return (char *)sbrk(0) + 16384;
}
