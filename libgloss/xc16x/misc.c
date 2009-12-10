/*
 * Copyright (C) 2006 KPIT Cummins
 * Copyright (C) 2009 Conny Marco Menebröcker
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms is permitted
 * provided that the above copyright notice and following paragraph are
 * duplicated in all such forms.
 *
 * This file is distributed WITHOUT ANY WARRANTY; without even the implied
 * warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 */
#include <_ansi.h>
#include <sys/types.h>
#include <sys/stat.h>

/* _raise(), getpid(), and kill() are required by abort().
   getpid/kill are prefixed with '_' because of MISSING_SYSCALL_NAMES.  */

int _DEFUN(_raise,(sig),
	   int sig)
{
  errno = ENOSYS;
  return -1;
}

int _DEFUN(_getpid,(),)
{
  errno = ENOSYS;
  return -1;
}

int _DEFUN(_kill,(pid, sig),
	   int pid _AND
	   int sig)
{
  errno = ENOSYS;
  return -1;
}
