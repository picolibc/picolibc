/*
(C) Copyright IBM Corp. 2008

All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

* Redistributions of source code must retain the above copyright notice,
this list of conditions and the following disclaimer.
* Redistributions in binary form must reproduce the above copyright
notice, this list of conditions and the following disclaimer in the
documentation and/or other materials provided with the distribution.
* Neither the name of IBM nor the names of its contributors may be
used to endorse or promote products derived from this software without
specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
POSSIBILITY OF SUCH DAMAGE.

Author: Ken Werner <ken.werner@de.ibm.com>
*/

#ifndef __LINUX_SYSCALLS_H
#define __LINUX_SYSCALLS_H
#ifdef __cplusplus
extern "C" {
#endif

/* The system call numbers. See kernel source file
   include/asm-powerpc/unistd.h.  */
#define __NR_read                   3
#define __NR_write                  4
#define __NR_open                   5
#define __NR_close                  6
#define __NR_waitpid                7
#define __NR_creat                  8
#define __NR_link                   9
#define __NR_unlink                 10

#define __NR_chdir                  12
#define __NR_time                   13
#define __NR_mkdod                  14
#define __NR_chmod                  15
#define __NR_lchown                 16

#define __NR_lseek                  19
#define __NR_getpid                 20

#define __NR_setuid                 23
#define __NR_getuid                 24
#define __NR_stime                  25

#define __NR_alarm                  27

#define __NR_access                 33
#define __NR_nice                   34

#define __NR_sync                   36
#define __NR_kill                   37
#define __NR_rename                 38
#define __NR_mkdir                  39
#define __NR_rmdir                  40
#define __NR_dup                    41
#define __NR_pipe                   42
#define __NR_times                  43

#define __NR_brk                    45
#define __NR_setgid                 46
#define __NR_getgid                 47

#define __NR_geteuid                49
#define __NR_getegid                50

#define __NR_ioctl                  54
#define __NR_fcntl                  55

#define __NR_setpgid                57

#define __NR_umask                  60
#define __NR_chroot                 61

#define __NR_dup2                   63
#define __NR_getppid                64
#define __NR_getpgrp                65
#define __NR_setsid                 66

#define __NR_sgetmask               68
#define __NR_ssetmask               69
#define __NR_setreuid               70
#define __NR_setregid               71

#define __NR_sethostname            74
#define __NR_setrlimit              75

#define __NR_getrusage              77
#define __NR_gettimeofday           78
#define __NR_settimeofday           79
#define __NR_getgroups              80
#define __NR_setgroups              81

#define __NR_symlink                83

#define __NR_readlink               85

#define __NR_mmap                   90
#define __NR_munmap                 91
#define __NR_truncate               92
#define __NR_ftruncate              93
#define __NR_fchmod                 94
#define __NR_fchown                 95
#define __NR_getpriority            96
#define __NR_setpriority            97

#define __NR_socketcall             102
#define __NR_syslog                 103
#define __NR_setitimer              104
#define __NR_getitimer              105
#define __NR_newstat                106
#define __NR_newlstat               107
#define __NR_newfstat               108

#define __NR_vhangup                111

#define __NR_wait4                  114

#define __NR_sysinfo                116

#define __NR_fsync                  118

#define __NR_setdomainname          121
#define __NR_newuname               122

#define __NR_adjtimex               124
#define __NR_mprotect               125

#define __NR_getpgid                132
#define __NR_fchdir                 133
#define __NR_bdflush                134

#define __NR_personality            136

#define __NR_setfsuid               138
#define __NR_setfsgid               139
#define __NR__llseek                140
#define __NR_getdents               141
#define __NR__newselect             142
#define __NR_flock                  143
#define __NR_msync                  144
#define __NR_readv                  145
#define __NR_writev                 146
#define __NR_getsid                 147
#define __NR_fdatasync              148

#define __NR_mlock                  150
#define __NR_munlock                151
#define __NR_mlockall               152
#define __NR_munlockall             153
#define __NR_sched_setparam         154
#define __NR_sched_getparam         155
#define __NR_sched_setscheduler     156
#define __NR_sched_getscheduler     157
#define __NR_sched_yield            158
#define __NR_sched_get_priority_max 159
#define __NR_sched_get_priority_min 160
#define __NR_sched_rr_get_interval  161
#define __NR_nanosleep              162
#define __NR_mremap                 163
#define __NR_setresuid              164
#define __NR_getresuid              165

#define __NR_poll                   167

#define __NR_setresgid              169
#define __NR_getresgid              170
#define __NR_prctl                  171

#define __NR_pread64                179
#define __NR_pwrite64               180
#define __NR_chown                  181
#define __NR_getcwd                 182
#define __NR_capget                 183
#define __NR_capset                 184

#define __NR_sendfile               185

#define __NR_getrlimit              190
#define __NR_readahead              191

#define __NR_getdents64             202
#define __NR_pivot_root             203

#define __NR_madvise                205
#define __NR_mincore                206
#define __NR_gettid                 207
#define __NR_tkill                  208
#define __NR_setxattr               209
#define __NR_lsetxattr              210
#define __NR_fsetxattr              211
#define __NR_getxattr               212
#define __NR_lgetxattr              213
#define __NR_fgetxattr              214
#define __NR_listxattr              215
#define __NR_llistxattr             216
#define __NR_flistxattr             217
#define __NR_removexattr            218
#define __NR_lremovexattr           219
#define __NR_fremovexattr           220
#define __NR_futex                  221
#define __NR_sched_setaffinity      222
#define __NR_sched_getaffinity      223

#define __NR_io_setup               227
#define __NR_io_destroy             228
#define __NR_io_getevents           229
#define __NR_io_submit              230
#define __NR_io_cancel              231

#define __NR_fadvise64              233

#define __NR_epoll_create           236
#define __NR_epoll_ctl              237
#define __NR_epoll_wait             238
#define __NR_remap_file_pages       239
#define __NR_timer_create           240
#define __NR_timer_settime          241
#define __NR_timer_gettime          242
#define __NR_timer_getoverrun       243
#define __NR_timer_delete           244
#define __NR_clock_settime          245
#define __NR_clock_gettime          246
#define __NR_clock_getres           247
#define __NR_clock_nanosleep        248

#define __NR_tgkill                 250
#define __NR_utimes                 251
#define __NR_statfs64               252
#define __NR_fstatfs64              253

#define __NR_rtas                   255

#define __NR_unshare                282
#define __NR_splice                 283
#define __NR_tee                    284
#define __NR_vmsplice               285
#define __NR_openat                 286
#define __NR_mkdirat                287
#define __NR_mknodat                288
#define __NR_fchownat               289
#define __NR_futimesat              290
#define __NR_fstatat64              291
#define __NR_unlinkat               292
#define __NR_renameat               293
#define __NR_linkat                 294
#define __NR_symlinkat              295
#define __NR_readlinkat             296
#define __NR_fchmodat               297
#define __NR_faccessat              298
#define __NR_get_robust_list        299
#define __NR_set_robust_list        300
#define __NR_move_pages             301
#define __NR_getcpu                 302

#define __NR_utimensat              304
#define __NR_signalfd               305
#define __NR_timerfd                306
#define __NR_eventfd                307
#define __NR_sync_file_range2       308


/* System callbacks from the SPU. See kernel source file
   include/asm-powerpc/spu.h.  */
struct spu_syscall_block
{
  unsigned long long nr_ret;	/* System call nr and return value.  */
  unsigned long long parm[6];	/* System call arguments.  */
};

/* Issues a Linux system call.  */
int __linux_syscall (struct spu_syscall_block *s);

#ifdef __cplusplus
}
#endif
#endif
