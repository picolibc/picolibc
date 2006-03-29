/* libc/sys/linux/signal.c - Signal handling functions */

/* Written 2000 by Werner Almesberger */


#include <signal.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/syscall.h>


#define __NR___sgetmask __NR_sgetmask /* avoid name space pollution */
#define __NR___ssetmask __NR_ssetmask /* avoid name space pollution */

_syscall2(int,kill,pid_t,pid,int,sig)
_syscall2(__sighandler_t,signal,int,signum,__sighandler_t,handler)
_syscall3(int,sigaction,int,signum,const struct sigaction *,act,
  struct sigaction *,oldact)
_syscall1(int,sigsuspend,const sigset_t *,mask)
_syscall1(int,sigpending,sigset_t *,set)
_syscall0(int,pause)
_syscall1(unsigned int,alarm,unsigned int,seconds)
_syscall3(int,sigprocmask,int,how,const sigset_t *,set,sigset_t *,oldset)

static _syscall0(int,__sgetmask)
static _syscall1(int,__ssetmask,int,newmask)


int sigsetmask(int newmask) /* BSD */
{
    return __ssetmask(newmask);
}


int sigmask(int signum) /* BSD */
{
    return 1 << signum;
}


int sigblock(int mask) /* BSD */
{
    return __ssetmask(mask | __sgetmask());
}


int raise(int sig)
{
    return kill(getpid(),sig);
}


const char *const sys_siglist[] = {
#include "siglist.inc"
};
