/*
 * Version of sbrk for no operating system.
 */

#include "config.h"
#include <_ansi.h>
#include <_syslist.h>
#include <sys/types.h>
#include <errno.h>
#undef errno
extern int errno;

caddr_t 
_DEFUN (_sbrk, (incr),
        int incr)
{ 
   extern char end; /* set by linker */
   static char *heap_end; 
   char *prev_heap_end; 

   if (heap_end == 0) { 
      heap_end = &end; 
   } 
   prev_heap_end = heap_end; 
   heap_end += incr; 
   return (caddr_t) prev_heap_end; 
} 
