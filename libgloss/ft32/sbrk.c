/* sbrk.c -- allocate memory dynamically.
 *
 *
 * Copyright (C) 2015 FTDI (support@ftdichip.com)
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
#include <errno.h>
#include "glue.h"

char *__heap_ptr = (char *)&_end;

/*
 * sbrk -- changes heap size size. Get nbytes more
 *         RAM. We just increment a pointer in what's
 *         left of memory on the board.
 */
char *
_sbrk (nbytes)
     int nbytes;
{
  char *base;
  char *sp;
  char canary;

  if ((__heap_ptr + nbytes) < (&canary - 2000))
    {
      base = __heap_ptr;
      __heap_ptr += nbytes;
      return base;
    }
    else
    {
      errno = ENOMEM;
      return ((char *)-1);
    }
}
