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
#define BDM_TRAP 15

/* This register holds the function enumeration for a semihosting
   command.  */
#define BDM_FUNC_REG "d0"
/* This register holds the argument for the semihosting call.  */
#define BDM_ARG_REG  "d1"
/* This register holds the result of a semihosting call.  */
#define BDM_RESULT_REG  "d0"

/* Program exit.  Argument is exit code.  */
#define BDM_EXIT  0

/* Output char to console.  Argument is char to print.  */
#define BDM_PUTCHAR  1
