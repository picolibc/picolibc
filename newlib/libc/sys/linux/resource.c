/* libc/sys/linux/resource.c - Process resource functions */

/* Copyright 2002, Red Hat Inc. */

#include <sys/resource.h>
#include <machine/syscall.h>

_syscall2(int,getrusage,int,who,struct rusage *,r_usage)
_syscall2(int,getrlimit,int,resource,struct rlimit *,rlp)
_syscall2(int,setrlimit,int,resource,const struct rlimit *,rlp)

weak_alias(__libc_getrlimit,__getrlimit)
weak_alias(__libc_setrlimit,__setrlimit)
