/*
 * bdm-semihost.c -- 
 *
 * Copyright (c) 2006 CodeSourcery Inc
 *
 * The authors hereby grant permission to use, copy, modify, distribute,
 * and license this software and its documentation for any purpose, provided
 * that existing copyright notices are retained in all copies and that this
 * notice is included verbatim in any distributions. No written agreement,
 * license, or royalty fee is required for any of the authorized uses.
 * Modifications to this software may be copyrighted by their authors
 * and need not follow the licensing terms described here, provided that
 * the new terms are clearly indicated on the first page of each file where
 * they apply.
 */

/* Semihosting trap.  The debugger intercepts this and
   performs the semihosting action.  Then the program resumes as
   usual.  This function must be compiled without a frame pointer, so
   we know the halt instruction is the very first instuction.  */
	
void __attribute__ ((interrupt_handler)) __bdm_semihosting (void) 
{
  __asm__ __volatile__ ("halt" ::: "memory");
}
