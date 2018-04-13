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

void *
realloc (void *old_ptr, size_t new_size)
{
  void *new_ptr = malloc (new_size);

  if (old_ptr && new_ptr)
    {
      size_t old_size = *(size_t *)((long long *)old_ptr - 1);
      size_t copy_size = old_size > new_size ? new_size : old_size;
      __builtin_memcpy (new_ptr, old_ptr, copy_size);
      free (old_ptr);
    }

  return new_ptr;
}
