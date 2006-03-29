/*
 * bdm-outbyte.c -- 
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

#include "bdm-semihost.h"

/*
 * outbyte -- output a byte to a console.
 */

void outbyte (char c)
{
  int code = c & 0xff;
  
  __asm__ __volatile__ ("move.l %0,%/" BDM_ARG_REG "\n"
			"moveq %1,%/" BDM_FUNC_REG "\n"
			"trap %2"
			:: "rmi" (code), "n" (BDM_PUTCHAR), "n" (BDM_TRAP)
			: BDM_FUNC_REG,BDM_ARG_REG,BDM_RESULT_REG,"memory");
}
