/* cr16 syscall.h file. This is used only by the simulator. These numbers
   match the syscalls used by the sim port which are different from linux
   system calls.
   This will allow correct generation of sim/common/nltvals.def  */
   

#ifndef _SYS_SYSCALL_H_
#define _SYS_SYSCALL_H_

/* Note: This file may be included by assembler source.  */

#define SYS_fork        2
#define SYS_wait4       7
#define SYS_create      8
#define SYS_link        9
#define SYS_execv       11
#define SYS_chdir       12
#define SYS_mknod       14
#define SYS_chmod       15
#define SYS_chown       16
#define SYS_getpid      20
#define SYS_isatty      21
#define SYS_fstat       22
#define SYS_ARG         24
#define SYS_stat        38
#define SYS_pipe        42
#define SYS_execve      59
#define SYS_kill        60
#define SYS_utime       201
#define SYS_wait        202
#define SYS_time        0x300
#define SYS_open        0x401
#define SYS_close       0x402
#define SYS_read        0x403
#define SYS_write       0x404
#define SYS_lseek       0x405
#define SYS_rename      0x406
#define SYS_unlink      0x407
#define SYS_exit        0x410

#endif
