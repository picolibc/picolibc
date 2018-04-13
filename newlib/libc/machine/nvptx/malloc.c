/*
 * Support file for nvptx in newlib.
 * Copyright (c) 2016-2018 Mentor Graphics.
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

#include <stdlib.h>

/* The CUDA-provided malloc.  */
void *sys_malloc (size_t) __asm__ ("malloc");

/* The user-visible malloc (renamed by compiler).  */
void *malloc (size_t size)
{
  long long *ptr = sys_malloc (size + sizeof (long long));
  if (ptr)
    *(size_t *)ptr++ = size;

  return ptr;
}
