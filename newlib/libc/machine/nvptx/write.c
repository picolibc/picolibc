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
#include <stdio.h>
#include <unistd.h>
#include <errno.h>

_READ_WRITE_RETURN_TYPE write (int fd, const void *buf, size_t count)
{
  size_t i;
  char *b = (char *)buf;
  if (fd != 1 && fd != 2)
    {
      errno = EBADF;
      return -1;
    }
  for (i = 0; i < count; i++)
    printf ("%c", b[i]);
  return count;
}
