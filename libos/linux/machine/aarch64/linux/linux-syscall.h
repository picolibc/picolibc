/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright © 2026 Keith Packard
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above
 *    copyright notice, this list of conditions and the following
 *    disclaimer in the documentation and/or other materials provided
 *    with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its
 *    contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
 * INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
 * OF THE POSSIBILITY OF SUCH DAMAGE.
 */
#ifndef _LINUX_SYSCALL_H_
#define _LINUX_SYSCALL_H_
#define LINUX_SYS_io_setup               0
#define LINUX_SYS_io_destroy             1
#define LINUX_SYS_io_submit              2
#define LINUX_SYS_io_cancel              3
#define LINUX_SYS_io_getevents           4
#define LINUX_SYS_setxattr               5
#define LINUX_SYS_lsetxattr              6
#define LINUX_SYS_fsetxattr              7
#define LINUX_SYS_getxattr               8
#define LINUX_SYS_lgetxattr              9
#define LINUX_SYS_fgetxattr              10
#define LINUX_SYS_listxattr              11
#define LINUX_SYS_llistxattr             12
#define LINUX_SYS_flistxattr             13
#define LINUX_SYS_removexattr            14
#define LINUX_SYS_lremovexattr           15
#define LINUX_SYS_fremovexattr           16
#define LINUX_SYS_getcwd                 17
#define LINUX_SYS_lookup_dcookie         18
#define LINUX_SYS_eventfd2               19
#define LINUX_SYS_epoll_create1          20
#define LINUX_SYS_epoll_ctl              21
#define LINUX_SYS_epoll_pwait            22
#define LINUX_SYS_dup                    23
#define LINUX_SYS_dup3                   24
#define LINUX_SYS_fcntl                  25
#define LINUX_SYS_inotify_init1          26
#define LINUX_SYS_inotify_add_watch      27
#define LINUX_SYS_inotify_rm_watch       28
#define LINUX_SYS_ioctl                  29
#define LINUX_SYS_ioprio_set             30
#define LINUX_SYS_ioprio_get             31
#define LINUX_SYS_flock                  32
#define LINUX_SYS_mknodat                33
#define LINUX_SYS_mkdirat                34
#define LINUX_SYS_unlinkat               35
#define LINUX_SYS_symlinkat              36
#define LINUX_SYS_linkat                 37
#define LINUX_SYS_renameat               38
#define LINUX_SYS_umount2                39
#define LINUX_SYS_mount                  40
#define LINUX_SYS_pivot_root             41
#define LINUX_SYS_nfsservctl             42
#define LINUX_SYS_statfs                 43
#define LINUX_SYS_fstatfs                44
#define LINUX_SYS_truncate               45
#define LINUX_SYS_ftruncate              46
#define LINUX_SYS_fallocate              47
#define LINUX_SYS_faccessat              48
#define LINUX_SYS_chdir                  49
#define LINUX_SYS_fchdir                 50
#define LINUX_SYS_chroot                 51
#define LINUX_SYS_fchmod                 52
#define LINUX_SYS_fchmodat               53
#define LINUX_SYS_fchownat               54
#define LINUX_SYS_fchown                 55
#define LINUX_SYS_openat                 56
#define LINUX_SYS_close                  57
#define LINUX_SYS_vhangup                58
#define LINUX_SYS_pipe2                  59
#define LINUX_SYS_quotactl               60
#define LINUX_SYS_getdents64             61
#define LINUX_SYS_lseek                  62
#define LINUX_SYS_read                   63
#define LINUX_SYS_write                  64
#define LINUX_SYS_readv                  65
#define LINUX_SYS_writev                 66
#define LINUX_SYS_pread64                67
#define LINUX_SYS_pwrite64               68
#define LINUX_SYS_preadv                 69
#define LINUX_SYS_pwritev                70
#define LINUX_SYS_sendfile               71
#define LINUX_SYS_pselect6               72
#define LINUX_SYS_ppoll                  73
#define LINUX_SYS_signalfd4              74
#define LINUX_SYS_vmsplice               75
#define LINUX_SYS_splice                 76
#define LINUX_SYS_tee                    77
#define LINUX_SYS_readlinkat             78
#define LINUX_SYS_newfstatat             79
#define LINUX_SYS_fstat                  80
#define LINUX_SYS_sync                   81
#define LINUX_SYS_fsync                  82
#define LINUX_SYS_fdatasync              83
#define LINUX_SYS_sync_file_range        84
#define LINUX_SYS_timerfd_create         85
#define LINUX_SYS_timerfd_settime        86
#define LINUX_SYS_timerfd_gettime        87
#define LINUX_SYS_utimensat              88
#define LINUX_SYS_acct                   89
#define LINUX_SYS_capget                 90
#define LINUX_SYS_capset                 91
#define LINUX_SYS_personality            92
#define LINUX_SYS_exit                   93
#define LINUX_SYS_exit_group             94
#define LINUX_SYS_waitid                 95
#define LINUX_SYS_set_tid_address        96
#define LINUX_SYS_unshare                97
#define LINUX_SYS_futex                  98
#define LINUX_SYS_set_robust_list        99
#define LINUX_SYS_get_robust_list        100
#define LINUX_SYS_nanosleep              101
#define LINUX_SYS_getitimer              102
#define LINUX_SYS_setitimer              103
#define LINUX_SYS_kexec_load             104
#define LINUX_SYS_init_module            105
#define LINUX_SYS_delete_module          106
#define LINUX_SYS_timer_create           107
#define LINUX_SYS_timer_gettime          108
#define LINUX_SYS_timer_getoverrun       109
#define LINUX_SYS_timer_settime          110
#define LINUX_SYS_timer_delete           111
#define LINUX_SYS_clock_settime          112
#define LINUX_SYS_clock_gettime          113
#define LINUX_SYS_clock_getres           114
#define LINUX_SYS_clock_nanosleep        115
#define LINUX_SYS_syslog                 116
#define LINUX_SYS_ptrace                 117
#define LINUX_SYS_sched_setparam         118
#define LINUX_SYS_sched_setscheduler     119
#define LINUX_SYS_sched_getscheduler     120
#define LINUX_SYS_sched_getparam         121
#define LINUX_SYS_sched_setaffinity      122
#define LINUX_SYS_sched_getaffinity      123
#define LINUX_SYS_sched_yield            124
#define LINUX_SYS_sched_get_priority_max 125
#define LINUX_SYS_sched_get_priority_min 126
#define LINUX_SYS_sched_rr_get_interval  127
#define LINUX_SYS_restart_syscall        128
#define LINUX_SYS_kill                   129
#define LINUX_SYS_tkill                  130
#define LINUX_SYS_tgkill                 131
#define LINUX_SYS_sigaltstack            132
#define LINUX_SYS_rt_sigsuspend          133
#define LINUX_SYS_rt_sigaction           134
#define LINUX_SYS_rt_sigprocmask         135
#define LINUX_SYS_rt_sigpending          136
#define LINUX_SYS_rt_sigtimedwait        137
#define LINUX_SYS_rt_sigqueueinfo        138
#define LINUX_SYS_rt_sigreturn           139
#define LINUX_SYS_setpriority            140
#define LINUX_SYS_getpriority            141
#define LINUX_SYS_reboot                 142
#define LINUX_SYS_setregid               143
#define LINUX_SYS_setgid                 144
#define LINUX_SYS_setreuid               145
#define LINUX_SYS_setuid                 146
#define LINUX_SYS_setresuid              147
#define LINUX_SYS_getresuid              148
#define LINUX_SYS_setresgid              149
#define LINUX_SYS_getresgid              150
#define LINUX_SYS_setfsuid               151
#define LINUX_SYS_setfsgid               152
#define LINUX_SYS_times                  153
#define LINUX_SYS_setpgid                154
#define LINUX_SYS_getpgid                155
#define LINUX_SYS_getsid                 156
#define LINUX_SYS_setsid                 157
#define LINUX_SYS_getgroups              158
#define LINUX_SYS_setgroups              159
#define LINUX_SYS_uname                  160
#define LINUX_SYS_sethostname            161
#define LINUX_SYS_setdomainname          162
#define LINUX_SYS_getrlimit              163
#define LINUX_SYS_setrlimit              164
#define LINUX_SYS_getrusage              165
#define LINUX_SYS_umask                  166
#define LINUX_SYS_prctl                  167
#define LINUX_SYS_getcpu                 168
#define LINUX_SYS_gettimeofday           169
#define LINUX_SYS_settimeofday           170
#define LINUX_SYS_adjtimex               171
#define LINUX_SYS_getpid                 172
#define LINUX_SYS_getppid                173
#define LINUX_SYS_getuid                 174
#define LINUX_SYS_geteuid                175
#define LINUX_SYS_getgid                 176
#define LINUX_SYS_getegid                177
#define LINUX_SYS_gettid                 178
#define LINUX_SYS_sysinfo                179
#define LINUX_SYS_mq_open                180
#define LINUX_SYS_mq_unlink              181
#define LINUX_SYS_mq_timedsend           182
#define LINUX_SYS_mq_timedreceive        183
#define LINUX_SYS_mq_notify              184
#define LINUX_SYS_mq_getsetattr          185
#define LINUX_SYS_msgget                 186
#define LINUX_SYS_msgctl                 187
#define LINUX_SYS_msgrcv                 188
#define LINUX_SYS_msgsnd                 189
#define LINUX_SYS_semget                 190
#define LINUX_SYS_semctl                 191
#define LINUX_SYS_semtimedop             192
#define LINUX_SYS_semop                  193
#define LINUX_SYS_shmget                 194
#define LINUX_SYS_shmctl                 195
#define LINUX_SYS_shmat                  196
#define LINUX_SYS_shmdt                  197
#define LINUX_SYS_socket                 198
#define LINUX_SYS_socketpair             199
#define LINUX_SYS_bind                   200
#define LINUX_SYS_listen                 201
#define LINUX_SYS_accept                 202
#define LINUX_SYS_connect                203
#define LINUX_SYS_getsockname            204
#define LINUX_SYS_getpeername            205
#define LINUX_SYS_sendto                 206
#define LINUX_SYS_recvfrom               207
#define LINUX_SYS_setsockopt             208
#define LINUX_SYS_getsockopt             209
#define LINUX_SYS_shutdown               210
#define LINUX_SYS_sendmsg                211
#define LINUX_SYS_recvmsg                212
#define LINUX_SYS_readahead              213
#define LINUX_SYS_brk                    214
#define LINUX_SYS_munmap                 215
#define LINUX_SYS_mremap                 216
#define LINUX_SYS_add_key                217
#define LINUX_SYS_request_key            218
#define LINUX_SYS_keyctl                 219
#define LINUX_SYS_clone                  220
#define LINUX_SYS_execve                 221
#define LINUX_SYS_mmap                   222
#define LINUX_SYS_fadvise64              223
#define LINUX_SYS_swapon                 224
#define LINUX_SYS_swapoff                225
#define LINUX_SYS_mprotect               226
#define LINUX_SYS_msync                  227
#define LINUX_SYS_mlock                  228
#define LINUX_SYS_munlock                229
#define LINUX_SYS_mlockall               230
#define LINUX_SYS_munlockall             231
#define LINUX_SYS_mincore                232
#define LINUX_SYS_madvise                233
#define LINUX_SYS_remap_file_pages       234
#define LINUX_SYS_mbind                  235
#define LINUX_SYS_get_mempolicy          236
#define LINUX_SYS_set_mempolicy          237
#define LINUX_SYS_migrate_pages          238
#define LINUX_SYS_move_pages             239
#define LINUX_SYS_rt_tgsigqueueinfo      240
#define LINUX_SYS_perf_event_open        241
#define LINUX_SYS_accept4                242
#define LINUX_SYS_recvmmsg               243
#define LINUX_SYS_wait4                  260
#define LINUX_SYS_prlimit64              261
#define LINUX_SYS_fanotify_init          262
#define LINUX_SYS_fanotify_mark          263
#define LINUX_SYS_name_to_handle_at      264
#define LINUX_SYS_open_by_handle_at      265
#define LINUX_SYS_clock_adjtime          266
#define LINUX_SYS_syncfs                 267
#define LINUX_SYS_setns                  268
#define LINUX_SYS_sendmmsg               269
#define LINUX_SYS_process_vm_readv       270
#define LINUX_SYS_process_vm_writev      271
#define LINUX_SYS_kcmp                   272
#define LINUX_SYS_finit_module           273
#define LINUX_SYS_sched_setattr          274
#define LINUX_SYS_sched_getattr          275
#define LINUX_SYS_renameat2              276
#define LINUX_SYS_seccomp                277
#define LINUX_SYS_getrandom              278
#define LINUX_SYS_memfd_create           279
#define LINUX_SYS_bpf                    280
#define LINUX_SYS_execveat               281
#define LINUX_SYS_userfaultfd            282
#define LINUX_SYS_membarrier             283
#define LINUX_SYS_mlock2                 284
#define LINUX_SYS_copy_file_range        285
#define LINUX_SYS_preadv2                286
#define LINUX_SYS_pwritev2               287
#define LINUX_SYS_pkey_mprotect          288
#define LINUX_SYS_pkey_alloc             289
#define LINUX_SYS_pkey_free              290
#define LINUX_SYS_statx                  291
#define LINUX_SYS_io_pgetevents          292
#define LINUX_SYS_rseq                   293
#define LINUX_SYS_kexec_file_load        294
#define LINUX_SYS_pidfd_send_signal      424
#define LINUX_SYS_io_uring_setup         425
#define LINUX_SYS_io_uring_enter         426
#define LINUX_SYS_io_uring_register      427
#define LINUX_SYS_open_tree              428
#define LINUX_SYS_move_mount             429
#define LINUX_SYS_fsopen                 430
#define LINUX_SYS_fsconfig               431
#define LINUX_SYS_fsmount                432
#define LINUX_SYS_fspick                 433
#define LINUX_SYS_pidfd_open             434
#define LINUX_SYS_clone3                 435
#define LINUX_SYS_close_range            436
#define LINUX_SYS_openat2                437
#define LINUX_SYS_pidfd_getfd            438
#define LINUX_SYS_faccessat2             439
#define LINUX_SYS_process_madvise        440
#define LINUX_SYS_epoll_pwait2           441
#define LINUX_SYS_quotactl_fd            443
#define LINUX_SYS_landlock_create_rulese 444
#define LINUX_SYS_landlock_add_rule      445
#define LINUX_SYS_landlock_restrict_self 446
#define LINUX_SYS_memfd_secret           447
#endif /* _LINUX_SYSCALL_H_ */
