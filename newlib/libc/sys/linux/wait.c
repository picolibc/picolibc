/* libc/sys/linux/wait.c - Wait function wrappers */

/* Written 2000 by Werner Almesberger */


#include <sys/wait.h>
#include <sys/syscall.h>


_syscall3(pid_t,waitpid,pid_t,pid,int *,wait_stat,int,options)
_syscall4(pid_t,wait4,pid_t,pid,int *,status,int,options,struct rusage *,rusage)


pid_t wait3(int *status,int options,struct rusage *rusage)
{
    return wait4(-1,status,options,rusage);
}


pid_t wait(int *status)
{
    return waitpid(-1,status,0);
}
