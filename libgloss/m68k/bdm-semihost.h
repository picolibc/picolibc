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

extern int __bdm_semihost (int func, void *args);
