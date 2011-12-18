/*
 * C library support files for the Blackfin processor
 *
 * Copyright (C) 2010 Analog Devices, Inc.
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

/* This is a callback which gcc itself wants to flush jump tables.

   Map it into L1 Text because of anomalies 05-00-0312 and 05-00-0419.  */

__attribute__ ((l1_text))
void __clear_cache_range (char *beg, char *end)
{
  char *ptr = beg;
  do {
    __asm__ __volatile__ ("FLUSH [%0++];" : "+a" (ptr) : : "memory");
  } while (ptr <= end);
  ptr = beg;
  __asm__ __volatile__ ("SSYNC;");
  do {
    __asm__ __volatile__ ("IFLUSH [%0++];" : "+a" (ptr) : : "memory");
  } while (ptr <= end);
}
