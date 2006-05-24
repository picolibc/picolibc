/*
 * bdm semihosting support.
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

/* Semihosting uses a user trap handler containing a HALT
   instruction.  This wakes the debugger to perform some action.  */

/* This is the semihosting trap hander */
#define BDM_TRAPNUM 15

/* This register holds the function enumeration for a semihosting
   command.  */
#define BDM_FUNC_REG "d0"

/* This register holds the argument for the semihosting call.  For most
   functions, this is a pointer to a block of memory that holds the input
   and output parameters for the remote file i/o operation.  */
#define BDM_ARG_REG  "d1"

/* Codes for BDM_FUNC_REG.  */

#define BDM_EXIT  0
#define BDM_PUTCHAR 1 /* Obsolete */
#define BDM_OPEN 2
#define BDM_CLOSE 3
#define BDM_READ 4
#define BDM_WRITE 5
#define BDM_LSEEK 6
#define BDM_RENAME 7
#define BDM_UNLINK 8
#define BDM_STAT 9
#define BDM_FSTAT 10
#define BDM_GETTIMEOFDAY 11
#define BDM_ISATTY 12
#define BDM_SYSTEM 13

/* Here is the macro that generates the trap. */

#define BDM_TRAP(func, arg) \
  __asm__ __volatile__ ("move.l %0,%/" BDM_ARG_REG "\n" \
			"moveq %1,%/" BDM_FUNC_REG "\n" \
			"trap %2" \
			:: "rmi" (arg), "n" (func), "n" (BDM_TRAPNUM) \
			: BDM_FUNC_REG,BDM_ARG_REG,"memory")

