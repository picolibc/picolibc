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
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/syscall.h>
#include <_ansi.h>
#include <errno.h>
#include <fcntl.h>
#include <stdarg.h>
#include <reent.h>



void _exit(int n)
{
asm volatile("trap #0");
}

int isatty(file)
     int file;
{
  return 1;
}
