/*
 * bdm-unlink.c -- 
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
#include "bdm-gdb.h"
#include <stdio.h>
#include <string.h>
#include <errno.h>

/*
 * unlink -- unlink (delete) a file
 * input parameters:
 *   0 : filename ptr
 *   1 : filename length
 * output parameters:
 *   0 : result
 *   1 : errno
 */

int unlink (const char *path)
{
  gdb_parambuf_t parameters;
  parameters[0] = (uint32_t) path;
  parameters[1] = (uint32_t) strlen (path) + 1;
  __bdm_semihost (BDM_UNLINK, parameters);
  errno = convert_from_gdb_errno (parameters[1]);
  return parameters[0];
}
