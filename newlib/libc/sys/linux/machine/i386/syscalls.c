/* miscellaneous i386-specific linux syscalls */

/* Copyright 2002, Red Hat Inc. */

#include <sys/resource.h>
#include <machine/syscall.h>
#include <errno.h>

_syscall2(int,getrlimit,int,resource,struct rlimit *,rlp);
_syscall2(int,setrlimit,int,resource,const struct rlimit *,rlp);

