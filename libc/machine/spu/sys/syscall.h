/* Copyright (c) 2007 Patrick Mansfield <patmans@us.ibm.com> */
#ifndef _SYS_SYSCALL_H
#define _SYS_SYSCALL_H

#include <sys/cdefs.h>

_BEGIN_STD_C

int __send_to_ppe(unsigned int signalcode, unsigned int opcode, void *data);

_END_STD_C

#endif
