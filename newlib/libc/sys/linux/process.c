/* libc/sys/linux/process.c - Process-related system calls */

/* Written 2000 by Werner Almesberger */


#include <sys/unistd.h>
#include <sys/wait.h>
#include <machine/syscall.h>


#define __NR__exit __NR_exit
#define __NR__execve __NR_execve

_syscall0(int,fork)
_syscall3(int,_execve,const char *,file,char * const *,argv,char * const *,envp)
_syscall0(int,getpid)
_syscall2(int,setpgid,pid_t,pid,pid_t,pgid)
_syscall0(pid_t,getppid)
_syscall0(pid_t,getpgrp)
_syscall0(pid_t,setsid)

weak_alias(__libc_getpid,__getpid);

pid_t vfork(void)
{
  pid_t pid;

  pid = fork();
  
  if(!pid)
    {
      /* In child. */
      return 0;
    }
  else
    {
      /* In parent.  Wait for child to finish. */
      if (waitpid (pid, NULL, 0) < 0)
        return pid;
    }
}

/* FIXME: get rid of noreturn warning */

#define return for (;;)
_syscall1(void,_exit,int,exitcode)
#undef return
