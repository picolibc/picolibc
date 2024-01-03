/*
 * Copyright (C) 2020 Embecosm Limited
 * SPDX-License-Identifier: BSD-2-Clause
 */
#include "semihost_fdtable.h"
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>

#ifndef RISCV_MAX_OPEN_FILES
#define RISCV_MAX_OPEN_FILES 16
#endif

extern int errno;
extern int _open (const char *, int, ...);

/* fdtable keeps track of the position of each file and is used to map stdin,
   stdout and stderr to STDIN_FILENO, STDOUT_FILENO and STDERR_FILENO.  */

static struct fdentry fdtable[RISCV_MAX_OPEN_FILES];

/* Initialize fdtable.  A handle of -1 denotes an empty entry.  */

void __attribute__ ((constructor))
init_semihosting ()
{
  int i;

  for (i=0; i<RISCV_MAX_OPEN_FILES; i++)
    fdtable[i].handle = -1;

  /* Set up std streams. Note that the semihost _open() call returns an index
     into the fdtable.  */
  /* stdin.  */
  i = _open (":tt", O_RDONLY);
  fdtable[STDIN_FILENO].handle = fdtable[i].handle;
  fdtable[STDIN_FILENO].pos = 0;

  /* stdout.  */
  i = _open (":tt", O_WRONLY|O_CREAT|O_TRUNC);
  fdtable[STDOUT_FILENO].handle = fdtable[i].handle;
  fdtable[STDOUT_FILENO].pos = 0;

  /* stderr.  */
  i = _open (":tt", O_WRONLY|O_CREAT|O_APPEND);
  fdtable[STDERR_FILENO].handle = fdtable[i].handle;
  fdtable[STDERR_FILENO].pos = 0;
}

/* Add entry to fdtable.  */

int
__add_fdentry (int handle)
{
  for (int i=0; i<RISCV_MAX_OPEN_FILES; i++)
    if (fdtable[i].handle == -1)
      {
	fdtable[i].handle = handle;
	fdtable[i].pos = 0;
	return i;
      }
  /* Too many open files.  */
  errno = ENFILE;
  return -1;
}

/* Return the fdentry for file or NULL if not found.  */

struct fdentry *
__get_fdentry (int file)
{
  if (file<0 || file>=RISCV_MAX_OPEN_FILES || fdtable[file].handle == -1)
    {
      errno = EBADF;
      return NULL;
    }
  return &fdtable[file];
}

/* Remove entry from fdtable.  */

void
__remove_fdentry (int file)
{
  fdtable[file].handle = -1;
}
