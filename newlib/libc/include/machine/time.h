/*
Copyright (C) 1991 DJ Delorie
All rights reserved.

Redistribution, modification, and use in source and binary forms is permitted
provided that the above copyright notice and following paragraph are
duplicated in all such forms.

This file is distributed WITHOUT ANY WARRANTY; without even the implied
warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 */
#ifndef	_MACHTIME_H_
#define	_MACHTIME_H_

#if defined(__rtems__) || defined(__VISIUM__) || defined(__riscv)
#define _CLOCKS_PER_SEC_ 1000000
#elif defined(__aarch64__) || defined(__arm__) || defined(__thumb__)
#define _CLOCKS_PER_SEC_ 100
#endif

#ifdef __SPU__
#include <sys/_timespec.h>
int nanosleep (const struct timespec *, struct timespec *);
#endif

#endif	/* _MACHTIME_H_ */
