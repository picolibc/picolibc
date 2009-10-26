#define PROFILE_SUPPORT 1

#include "crt0.S"

	.global __mcount						
__mcount:								
	;; When a function is compiled for profiling, gcc creates code	
	;; like this at the start of each profiled function:		
	;;								
	;;      .global <func_name>					
	;; <func_name>:							
	;;      bsr __mcount						
	;;	<...function's prologue...>				
	;; 	<...function's body...>					
	;;								
	;; We must save all of the argument registers, extract the	
	;; address of <func_name>, call _mcount_internal to do the	
	;; real work and then restore the argument registers before	
	;; returning.							
									
	pushm	r1-r4							
	mov.l	16[r0], r1						
	bsr	__mcount_internal					
	rtsd	#16, r1-r4
	
