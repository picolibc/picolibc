/* libc/sys/linux/ids.c - System calls related to user and group ids  */

/* Written 2000 by Werner Almesberger */


#include <sys/types.h>
#include <sys/unistd.h>
#include <sys/syscall.h>


_syscall1(int,setuid,uid_t,uid)
_syscall0(uid_t,getuid)
_syscall1(int,setgid,gid_t,gid)
_syscall0(gid_t,getgid)
_syscall0(uid_t,geteuid)
_syscall0(gid_t,getegid)
_syscall2(int,getgroups,int,size,gid_t *,list)
