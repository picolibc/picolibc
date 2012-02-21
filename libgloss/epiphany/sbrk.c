/* EPIPHANY implementation of _sbrk ()

   Copyright (c) 2011, Adapteva, Inc.
   All rights reserved.

   Contributor Jeremy Bennett <jeremy.bennett@embecosm.com> for Adapteva Inc

   Redistribution and use in source and binary forms, with or without
   modification, are permitted provided that the following conditions are met:
    * Redistributions of source code must retain the above copyright notice,
      this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
      notice, this list of conditions and the following disclaimer in the
      documentation and/or other materials provided with the distribution.
    * Neither the name of Adapteva nor the names of its contributors may be
      used to endorse or promote products derived from this software without
      specific prior written permission.

   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
   AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
   IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
   ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
   LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
   CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
   SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
   INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
   CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
   ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
   POSSIBILITY OF SUCH DAMAGE.                                               */
/* ------------------------------------------------------------------------- */
/* This implementation is suitable for use with the Verilator simulation of
   the EPIPHANY.

   Commenting suitable for processing by Doxygen is provided throughout.     */
/* ------------------------------------------------------------------------- */
#include <sys/types.h>
#include <errno.h>
#include <stddef.h>

/* ------------------------------------------------------------------------- */
/*!Increase program data space

   This is a minimal implementation.  Assuming the heap is growing upwards
   from __heap_start towards __heap_end.
   See linker file for definition.

   @param[in] incr  The number of bytes to increment the stack by.

   @return  A pointer to the start of the new block of memory                */
/* ------------------------------------------------------------------------- */
void *
_sbrk (int  incr)
{
	extern char __heap_start;//set by linker
	extern char __heap_end;//set by linker

	static char *heap_end;		/* Previous end of heap or 0 if none */
	char        *prev_heap_end;

	if (0 == heap_end) {
		heap_end = &__heap_start;			/* Initialize first time round */
	}

	prev_heap_end  = heap_end;
	heap_end      += incr;
	//check
	if( heap_end < (&__heap_end)) {

	} else {
		errno = ENOMEM;
		return (char*)-1;
	}
	return (void *) prev_heap_end;

}	/* _sbrk () */



///* sbrk support */
//
///* The current plan is to have one sbrk handler for all cpus.
//   Hence use `asm' for each global variable here to avoid the cpu prefix.
//   We can't intrude on the user's namespace (another reason to use asm).  */
//
//#include <sys/types.h>
///*#include <sys/syscall.h>*/
//#include <errno.h>
//#include <stddef.h>
//
///* These variables are publicly accessible for debugging purposes.
//   The user is also free to set sbrk_size to something different.
//   See mem-layout.c.  */
//
//extern int sbrk_size asm ("sbrk_size");
//
//caddr_t sbrk_start asm ("sbrk_start");
//caddr_t sbrk_loc asm ("sbrk_loc");
//extern char  end;                    /* Defined by linker.  */
//
///*caddr_t _sbrk_r (struct _reent *, size_t) asm ("__sbrk_r");*/
//
//
///* This symbol can be defined with the --defsym linker option.  */
//extern char __heap_end __attribute__((weak));
//
//
///* FIXME: We need a semaphore here.  */
//
//caddr_t
//_sbrk_r (struct _reent *r, size_t nbytes)
//{
//   extern char  end;			/* Defined by linker.  */
//   static char *heap_end;		/* Previous end of heap or 0 if none */
//   char        *prev_heap_end;
//   register char* stack asm("sp");
//
//   if (0 == heap_end)
//     {
//       heap_end = &end;			/* Initialize first time round */
//     }
//
//   prev_heap_end  = heap_end;
//
//   if (stack < (prev_heap_end+nbytes)) {
//     /* heap would overlap the current stack depth.
//      * Future:  use sbrk() system call to make simulator grow memory beyond
//      * the stack and allocate that
//      */
//     errno = ENOMEM;
//     return (char*)-1;
//   }
//
//   heap_end      += nbytes;
//
//   return (caddr_t) prev_heap_end;
//
//}	/* _sbrk () */


//caddr_t
//_sbrk_r (struct _reent *r, size_t nbytes)
//{
//  caddr_t result;
//
//  void *heap_end;
//
//  heap_end = &__heap_end;
//  if (!heap_end)
//    heap_end = sbrk_start + sbrk_size;
//  if (
//      /* Ensure we don't underflow.  */
//      sbrk_loc + nbytes < sbrk_start
//      /* Ensure we don't overflow.  */
//      || sbrk_loc + nbytes > (caddr_t)heap_end)
//    {
//      errno = ENOMEM;
//      return ((caddr_t) -1);
//    }
//
//  if (0 == sbrk_loc)
//    sbrk_loc = &end;                 /* Initialize first time round */
//  result = sbrk_loc;
//  sbrk_loc += nbytes;
//  return result;
//}
