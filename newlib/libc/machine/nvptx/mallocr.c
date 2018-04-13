/*
 * Support file for nvptx in newlib.
 * Copyright (c) 2014-2018 Mentor Graphics.
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

void *_malloc_r (struct _reent *r, size_t n)
{
  return malloc (n);
}

void _free_r (struct _reent *r, void *p)
{
  free (p);
}
