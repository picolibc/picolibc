#include "sys/syscall.h"

int _read(file, ptr, len)
     int file;
     char *ptr;
     int len;
{
	register int ret asm("r0") ;

	/* Type cast int as short so that we can copy int values into 16 bit 
	   registers in case of -mint32 switch is given.
	   This is not going to affect data as file= 0 for stdin and len=1024 */

	asm("mov.b %0, r0l"::  "i" (SYS_read)) ; /* Syscall Number */
	asm("mov.w %0, r1" :: "r"((short)file) :"r1", "r2", "r3") ;
	asm("mov.w %0, r3" :: "r"((short)len) :"r1", "r2", "r3") ;
#if defined(__H8300__) || defined(__NORMAL_MODE__)
	asm("mov.w %0, r2" :: "r"(ptr) :"r1", "r2", "r3") ;
#else
	asm("mov.l %0, er2" :: "r"(ptr) :"r1", "er2", "r3") ;
#endif
	// This is magic trap similar to _write for simulator
	asm("jsr @@0xc8") ;
  return ret;
}


