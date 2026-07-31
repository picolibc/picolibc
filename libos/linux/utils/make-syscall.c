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

#define _GNU_SOURCE
#include <sys/syscall.h>
#include <unistd.h>
#include <stdio.h>

int
main(void)
{
#ifdef SYS__llseek
    printf("#define %-32.32s %d\n", "LINUX_SYS__llseek", SYS__llseek);
#endif
#ifdef SYS__newselect
    printf("#define %-32.32s %d\n", "LINUX_SYS__newselect", SYS__newselect);
#endif
#ifdef SYS__sysctl
    printf("#define %-32.32s %d\n", "LINUX_SYS__sysctl", SYS__sysctl);
#endif
#ifdef SYS_accept
    printf("#define %-32.32s %d\n", "LINUX_SYS_accept", SYS_accept);
#endif
#ifdef SYS_accept4
    printf("#define %-32.32s %d\n", "LINUX_SYS_accept4", SYS_accept4);
#endif
#ifdef SYS_access
    printf("#define %-32.32s %d\n", "LINUX_SYS_access", SYS_access);
#endif
#ifdef SYS_acct
    printf("#define %-32.32s %d\n", "LINUX_SYS_acct", SYS_acct);
#endif
#ifdef SYS_add_key
    printf("#define %-32.32s %d\n", "LINUX_SYS_add_key", SYS_add_key);
#endif
#ifdef SYS_adjtimex
    printf("#define %-32.32s %d\n", "LINUX_SYS_adjtimex", SYS_adjtimex);
#endif
#ifdef SYS_alarm
    printf("#define %-32.32s %d\n", "LINUX_SYS_alarm", SYS_alarm);
#endif
#ifdef SYS_alloc_hugepages
    printf("#define %-32.32s %d\n", "LINUX_SYS_alloc_hugepages", SYS_alloc_hugepages);
#endif
#ifdef SYS_arc_gettls
    printf("#define %-32.32s %d\n", "LINUX_SYS_arc_gettls", SYS_arc_gettls);
#endif
#ifdef SYS_arc_settls
    printf("#define %-32.32s %d\n", "LINUX_SYS_arc_settls", SYS_arc_settls);
#endif
#ifdef SYS_arc_usr_cmpxchg
    printf("#define %-32.32s %d\n", "LINUX_SYS_arc_usr_cmpxchg", SYS_arc_usr_cmpxchg);
#endif
#ifdef SYS_arch_prctl
    printf("#define %-32.32s %d\n", "LINUX_SYS_arch_prctl", SYS_arch_prctl);
#endif
#ifdef SYS_atomic_barrier
    printf("#define %-32.32s %d\n", "LINUX_SYS_atomic_barrier", SYS_atomic_barrier);
#endif
#ifdef SYS_atomic_cmpxchg_32
    printf("#define %-32.32s %d\n", "LINUX_SYS_atomic_cmpxchg_32", SYS_atomic_cmpxchg_32);
#endif
#ifdef SYS_bdflush
    printf("#define %-32.32s %d\n", "LINUX_SYS_bdflush", SYS_bdflush);
#endif
#ifdef SYS_bind
    printf("#define %-32.32s %d\n", "LINUX_SYS_bind", SYS_bind);
#endif
#ifdef SYS_bpf
    printf("#define %-32.32s %d\n", "LINUX_SYS_bpf", SYS_bpf);
#endif
#ifdef SYS_brk
    printf("#define %-32.32s %d\n", "LINUX_SYS_brk", SYS_brk);
#endif
#ifdef SYS_breakpoint
    printf("#define %-32.32s %d\n", "LINUX_SYS_breakpoint", SYS_breakpoint);
#endif
#ifdef SYS_cacheflush
    printf("#define %-32.32s %d\n", "LINUX_SYS_cacheflush", SYS_cacheflush);
#endif
#ifdef SYS_capget
    printf("#define %-32.32s %d\n", "LINUX_SYS_capget", SYS_capget);
#endif
#ifdef SYS_capset
    printf("#define %-32.32s %d\n", "LINUX_SYS_capset", SYS_capset);
#endif
#ifdef SYS_chdir
    printf("#define %-32.32s %d\n", "LINUX_SYS_chdir", SYS_chdir);
#endif
#ifdef SYS_chmod
    printf("#define %-32.32s %d\n", "LINUX_SYS_chmod", SYS_chmod);
#endif
#ifdef SYS_chown
    printf("#define %-32.32s %d\n", "LINUX_SYS_chown", SYS_chown);
#endif
#ifdef SYS_chown32
    printf("#define %-32.32s %d\n", "LINUX_SYS_chown32", SYS_chown32);
#endif
#ifdef SYS_chroot
    printf("#define %-32.32s %d\n", "LINUX_SYS_chroot", SYS_chroot);
#endif
#ifdef SYS_clock_adjtime
    printf("#define %-32.32s %d\n", "LINUX_SYS_clock_adjtime", SYS_clock_adjtime);
#endif
#ifdef SYS_clock_getres
    printf("#define %-32.32s %d\n", "LINUX_SYS_clock_getres", SYS_clock_getres);
#endif
#ifdef SYS_clock_gettime
    printf("#define %-32.32s %d\n", "LINUX_SYS_clock_gettime", SYS_clock_gettime);
#endif
#ifdef SYS_clock_nanosleep
    printf("#define %-32.32s %d\n", "LINUX_SYS_clock_nanosleep", SYS_clock_nanosleep);
#endif
#ifdef SYS_clock_settime
    printf("#define %-32.32s %d\n", "LINUX_SYS_clock_settime", SYS_clock_settime);
#endif
#ifdef SYS_clone2
    printf("#define %-32.32s %d\n", "LINUX_SYS_clone2", SYS_clone2);
#endif
#ifdef SYS_clone
    printf("#define %-32.32s %d\n", "LINUX_SYS_clone", SYS_clone);
#endif
#ifdef SYS_clone3
    printf("#define %-32.32s %d\n", "LINUX_SYS_clone3", SYS_clone3);
#endif
#ifdef SYS_close
    printf("#define %-32.32s %d\n", "LINUX_SYS_close", SYS_close);
#endif
#ifdef SYS_close_range
    printf("#define %-32.32s %d\n", "LINUX_SYS_close_range", SYS_close_range);
#endif
#ifdef SYS_connect
    printf("#define %-32.32s %d\n", "LINUX_SYS_connect", SYS_connect);
#endif
#ifdef SYS_copy_file_range
    printf("#define %-32.32s %d\n", "LINUX_SYS_copy_file_range", SYS_copy_file_range);
#endif
#ifdef SYS_creat
    printf("#define %-32.32s %d\n", "LINUX_SYS_creat", SYS_creat);
#endif
#ifdef SYS_create_module
    printf("#define %-32.32s %d\n", "LINUX_SYS_create_module", SYS_create_module);
#endif
#ifdef SYS_delete_module
    printf("#define %-32.32s %d\n", "LINUX_SYS_delete_module", SYS_delete_module);
#endif
#ifdef SYS_dup
    printf("#define %-32.32s %d\n", "LINUX_SYS_dup", SYS_dup);
#endif
#ifdef SYS_dup2
    printf("#define %-32.32s %d\n", "LINUX_SYS_dup2", SYS_dup2);
#endif
#ifdef SYS_dup3
    printf("#define %-32.32s %d\n", "LINUX_SYS_dup3", SYS_dup3);
#endif
#ifdef SYS_epoll_create
    printf("#define %-32.32s %d\n", "LINUX_SYS_epoll_create", SYS_epoll_create);
#endif
#ifdef SYS_epoll_create1
    printf("#define %-32.32s %d\n", "LINUX_SYS_epoll_create1", SYS_epoll_create1);
#endif
#ifdef SYS_epoll_ctl
    printf("#define %-32.32s %d\n", "LINUX_SYS_epoll_ctl", SYS_epoll_ctl);
#endif
#ifdef SYS_epoll_pwait
    printf("#define %-32.32s %d\n", "LINUX_SYS_epoll_pwait", SYS_epoll_pwait);
#endif
#ifdef SYS_epoll_pwait2
    printf("#define %-32.32s %d\n", "LINUX_SYS_epoll_pwait2", SYS_epoll_pwait2);
#endif
#ifdef SYS_epoll_wait
    printf("#define %-32.32s %d\n", "LINUX_SYS_epoll_wait", SYS_epoll_wait);
#endif
#ifdef SYS_eventfd
    printf("#define %-32.32s %d\n", "LINUX_SYS_eventfd", SYS_eventfd);
#endif
#ifdef SYS_eventfd2
    printf("#define %-32.32s %d\n", "LINUX_SYS_eventfd2", SYS_eventfd2);
#endif
#ifdef SYS_execv
    printf("#define %-32.32s %d\n", "LINUX_SYS_execv", SYS_execv);
#endif
#ifdef SYS_execve
    printf("#define %-32.32s %d\n", "LINUX_SYS_execve", SYS_execve);
#endif
#ifdef SYS_execveat
    printf("#define %-32.32s %d\n", "LINUX_SYS_execveat", SYS_execveat);
#endif
#ifdef SYS_exit
    printf("#define %-32.32s %d\n", "LINUX_SYS_exit", SYS_exit);
#endif
#ifdef SYS_exit_group
    printf("#define %-32.32s %d\n", "LINUX_SYS_exit_group", SYS_exit_group);
#endif
#ifdef SYS_faccessat
    printf("#define %-32.32s %d\n", "LINUX_SYS_faccessat", SYS_faccessat);
#endif
#ifdef SYS_faccessat2
    printf("#define %-32.32s %d\n", "LINUX_SYS_faccessat2", SYS_faccessat2);
#endif
#ifdef SYS_fadvise64
    printf("#define %-32.32s %d\n", "LINUX_SYS_fadvise64", SYS_fadvise64);
#endif
#ifdef SYS_fadvise64_64
    printf("#define %-32.32s %d\n", "LINUX_SYS_fadvise64_64", SYS_fadvise64_64);
#endif
#ifdef SYS_fallocate
    printf("#define %-32.32s %d\n", "LINUX_SYS_fallocate", SYS_fallocate);
#endif
#ifdef SYS_fanotify_init
    printf("#define %-32.32s %d\n", "LINUX_SYS_fanotify_init", SYS_fanotify_init);
#endif
#ifdef SYS_fanotify_mark
    printf("#define %-32.32s %d\n", "LINUX_SYS_fanotify_mark", SYS_fanotify_mark);
#endif
#ifdef SYS_fchdir
    printf("#define %-32.32s %d\n", "LINUX_SYS_fchdir", SYS_fchdir);
#endif
#ifdef SYS_fchmod
    printf("#define %-32.32s %d\n", "LINUX_SYS_fchmod", SYS_fchmod);
#endif
#ifdef SYS_fchmodat
    printf("#define %-32.32s %d\n", "LINUX_SYS_fchmodat", SYS_fchmodat);
#endif
#ifdef SYS_fchown
    printf("#define %-32.32s %d\n", "LINUX_SYS_fchown", SYS_fchown);
#endif
#ifdef SYS_fchown32
    printf("#define %-32.32s %d\n", "LINUX_SYS_fchown32", SYS_fchown32);
#endif
#ifdef SYS_fchownat
    printf("#define %-32.32s %d\n", "LINUX_SYS_fchownat", SYS_fchownat);
#endif
#ifdef SYS_fcntl
    printf("#define %-32.32s %d\n", "LINUX_SYS_fcntl", SYS_fcntl);
#endif
#ifdef SYS_fcntl64
    printf("#define %-32.32s %d\n", "LINUX_SYS_fcntl64", SYS_fcntl64);
#endif
#ifdef SYS_fdatasync
    printf("#define %-32.32s %d\n", "LINUX_SYS_fdatasync", SYS_fdatasync);
#endif
#ifdef SYS_fgetxattr
    printf("#define %-32.32s %d\n", "LINUX_SYS_fgetxattr", SYS_fgetxattr);
#endif
#ifdef SYS_finit_module
    printf("#define %-32.32s %d\n", "LINUX_SYS_finit_module", SYS_finit_module);
#endif
#ifdef SYS_flistxattr
    printf("#define %-32.32s %d\n", "LINUX_SYS_flistxattr", SYS_flistxattr);
#endif
#ifdef SYS_flock
    printf("#define %-32.32s %d\n", "LINUX_SYS_flock", SYS_flock);
#endif
#ifdef SYS_fork
    printf("#define %-32.32s %d\n", "LINUX_SYS_fork", SYS_fork);
#endif
#ifdef SYS_free_hugepages
    printf("#define %-32.32s %d\n", "LINUX_SYS_free_hugepages", SYS_free_hugepages);
#endif
#ifdef SYS_fremovexattr
    printf("#define %-32.32s %d\n", "LINUX_SYS_fremovexattr", SYS_fremovexattr);
#endif
#ifdef SYS_fsconfig
    printf("#define %-32.32s %d\n", "LINUX_SYS_fsconfig", SYS_fsconfig);
#endif
#ifdef SYS_fsetxattr
    printf("#define %-32.32s %d\n", "LINUX_SYS_fsetxattr", SYS_fsetxattr);
#endif
#ifdef SYS_fsmount
    printf("#define %-32.32s %d\n", "LINUX_SYS_fsmount", SYS_fsmount);
#endif
#ifdef SYS_fsopen
    printf("#define %-32.32s %d\n", "LINUX_SYS_fsopen", SYS_fsopen);
#endif
#ifdef SYS_fspick
    printf("#define %-32.32s %d\n", "LINUX_SYS_fspick", SYS_fspick);
#endif
#ifdef SYS_fstat
    printf("#define %-32.32s %d\n", "LINUX_SYS_fstat", SYS_fstat);
#endif
#ifdef SYS_fstat64
    printf("#define %-32.32s %d\n", "LINUX_SYS_fstat64", SYS_fstat64);
#endif
#ifdef SYS_fstatat64
    printf("#define %-32.32s %d\n", "LINUX_SYS_fstatat64", SYS_fstatat64);
#endif
#ifdef SYS_fstatfs
    printf("#define %-32.32s %d\n", "LINUX_SYS_fstatfs", SYS_fstatfs);
#endif
#ifdef SYS_fstatfs64
    printf("#define %-32.32s %d\n", "LINUX_SYS_fstatfs64", SYS_fstatfs64);
#endif
#ifdef SYS_fsync
    printf("#define %-32.32s %d\n", "LINUX_SYS_fsync", SYS_fsync);
#endif
#ifdef SYS_ftruncate
    printf("#define %-32.32s %d\n", "LINUX_SYS_ftruncate", SYS_ftruncate);
#endif
#ifdef SYS_ftruncate64
    printf("#define %-32.32s %d\n", "LINUX_SYS_ftruncate64", SYS_ftruncate64);
#endif
#ifdef SYS_futex
    printf("#define %-32.32s %d\n", "LINUX_SYS_futex", SYS_futex);
#endif
#ifdef SYS_futimesat
    printf("#define %-32.32s %d\n", "LINUX_SYS_futimesat", SYS_futimesat);
#endif
#ifdef SYS_get_kernel_syms
    printf("#define %-32.32s %d\n", "LINUX_SYS_get_kernel_syms", SYS_get_kernel_syms);
#endif
#ifdef SYS_get_mempolicy
    printf("#define %-32.32s %d\n", "LINUX_SYS_get_mempolicy", SYS_get_mempolicy);
#endif
#ifdef SYS_get_robust_list
    printf("#define %-32.32s %d\n", "LINUX_SYS_get_robust_list", SYS_get_robust_list);
#endif
#ifdef SYS_get_thread_area
    printf("#define %-32.32s %d\n", "LINUX_SYS_get_thread_area", SYS_get_thread_area);
#endif
#ifdef SYS_get_tls
    printf("#define %-32.32s %d\n", "LINUX_SYS_get_tls", SYS_get_tls);
#endif
#ifdef SYS_getcpu
    printf("#define %-32.32s %d\n", "LINUX_SYS_getcpu", SYS_getcpu);
#endif
#ifdef SYS_getcwd
    printf("#define %-32.32s %d\n", "LINUX_SYS_getcwd", SYS_getcwd);
#endif
#ifdef SYS_getdents
    printf("#define %-32.32s %d\n", "LINUX_SYS_getdents", SYS_getdents);
#endif
#ifdef SYS_getdents64
    printf("#define %-32.32s %d\n", "LINUX_SYS_getdents64", SYS_getdents64);
#endif
#ifdef SYS_getdomainname
    printf("#define %-32.32s %d\n", "LINUX_SYS_getdomainname", SYS_getdomainname);
#endif
#ifdef SYS_getdtablesize
    printf("#define %-32.32s %d\n", "LINUX_SYS_getdtablesize", SYS_getdtablesize);
#endif
#ifdef SYS_getegid
    printf("#define %-32.32s %d\n", "LINUX_SYS_getegid", SYS_getegid);
#endif
#ifdef SYS_getegid32
    printf("#define %-32.32s %d\n", "LINUX_SYS_getegid32", SYS_getegid32);
#endif
#ifdef SYS_geteuid
    printf("#define %-32.32s %d\n", "LINUX_SYS_geteuid", SYS_geteuid);
#endif
#ifdef SYS_geteuid32
    printf("#define %-32.32s %d\n", "LINUX_SYS_geteuid32", SYS_geteuid32);
#endif
#ifdef SYS_getgid
    printf("#define %-32.32s %d\n", "LINUX_SYS_getgid", SYS_getgid);
#endif
#ifdef SYS_getgid32
    printf("#define %-32.32s %d\n", "LINUX_SYS_getgid32", SYS_getgid32);
#endif
#ifdef SYS_getgroups
    printf("#define %-32.32s %d\n", "LINUX_SYS_getgroups", SYS_getgroups);
#endif
#ifdef SYS_getgroups32
    printf("#define %-32.32s %d\n", "LINUX_SYS_getgroups32", SYS_getgroups32);
#endif
#ifdef SYS_gethostname
    printf("#define %-32.32s %d\n", "LINUX_SYS_gethostname", SYS_gethostname);
#endif
#ifdef SYS_getitimer
    printf("#define %-32.32s %d\n", "LINUX_SYS_getitimer", SYS_getitimer);
#endif
#ifdef SYS_getpeername
    printf("#define %-32.32s %d\n", "LINUX_SYS_getpeername", SYS_getpeername);
#endif
#ifdef SYS_getpagesize
    printf("#define %-32.32s %d\n", "LINUX_SYS_getpagesize", SYS_getpagesize);
#endif
#ifdef SYS_getpgid
    printf("#define %-32.32s %d\n", "LINUX_SYS_getpgid", SYS_getpgid);
#endif
#ifdef SYS_getpgrp
    printf("#define %-32.32s %d\n", "LINUX_SYS_getpgrp", SYS_getpgrp);
#endif
#ifdef SYS_getpid
    printf("#define %-32.32s %d\n", "LINUX_SYS_getpid", SYS_getpid);
#endif
#ifdef SYS_getppid
    printf("#define %-32.32s %d\n", "LINUX_SYS_getppid", SYS_getppid);
#endif
#ifdef SYS_getpriority
    printf("#define %-32.32s %d\n", "LINUX_SYS_getpriority", SYS_getpriority);
#endif
#ifdef SYS_getrandom
    printf("#define %-32.32s %d\n", "LINUX_SYS_getrandom", SYS_getrandom);
#endif
#ifdef SYS_getresgid
    printf("#define %-32.32s %d\n", "LINUX_SYS_getresgid", SYS_getresgid);
#endif
#ifdef SYS_getresgid32
    printf("#define %-32.32s %d\n", "LINUX_SYS_getresgid32", SYS_getresgid32);
#endif
#ifdef SYS_getresuid
    printf("#define %-32.32s %d\n", "LINUX_SYS_getresuid", SYS_getresuid);
#endif
#ifdef SYS_getresuid32
    printf("#define %-32.32s %d\n", "LINUX_SYS_getresuid32", SYS_getresuid32);
#endif
#ifdef SYS_getrlimit
    printf("#define %-32.32s %d\n", "LINUX_SYS_getrlimit", SYS_getrlimit);
#endif
#ifdef SYS_getrusage
    printf("#define %-32.32s %d\n", "LINUX_SYS_getrusage", SYS_getrusage);
#endif
#ifdef SYS_getsid
    printf("#define %-32.32s %d\n", "LINUX_SYS_getsid", SYS_getsid);
#endif
#ifdef SYS_getsockname
    printf("#define %-32.32s %d\n", "LINUX_SYS_getsockname", SYS_getsockname);
#endif
#ifdef SYS_getsockopt
    printf("#define %-32.32s %d\n", "LINUX_SYS_getsockopt", SYS_getsockopt);
#endif
#ifdef SYS_gettid
    printf("#define %-32.32s %d\n", "LINUX_SYS_gettid", SYS_gettid);
#endif
#ifdef SYS_gettimeofday
    printf("#define %-32.32s %d\n", "LINUX_SYS_gettimeofday", SYS_gettimeofday);
#endif
#ifdef SYS_getuid
    printf("#define %-32.32s %d\n", "LINUX_SYS_getuid", SYS_getuid);
#endif
#ifdef SYS_getuid32
    printf("#define %-32.32s %d\n", "LINUX_SYS_getuid32", SYS_getuid32);
#endif
#ifdef SYS_getunwind
    printf("#define %-32.32s %d\n", "LINUX_SYS_getunwind", SYS_getunwind);
#endif
#ifdef SYS_getxattr
    printf("#define %-32.32s %d\n", "LINUX_SYS_getxattr", SYS_getxattr);
#endif
#ifdef SYS_getxgid
    printf("#define %-32.32s %d\n", "LINUX_SYS_getxgid", SYS_getxgid);
#endif
#ifdef SYS_getxpid
    printf("#define %-32.32s %d\n", "LINUX_SYS_getxpid", SYS_getxpid);
#endif
#ifdef SYS_getxuid
    printf("#define %-32.32s %d\n", "LINUX_SYS_getxuid", SYS_getxuid);
#endif
#ifdef SYS_init_module
    printf("#define %-32.32s %d\n", "LINUX_SYS_init_module", SYS_init_module);
#endif
#ifdef SYS_inotify_add_watch
    printf("#define %-32.32s %d\n", "LINUX_SYS_inotify_add_watch", SYS_inotify_add_watch);
#endif
#ifdef SYS_inotify_init
    printf("#define %-32.32s %d\n", "LINUX_SYS_inotify_init", SYS_inotify_init);
#endif
#ifdef SYS_inotify_init1
    printf("#define %-32.32s %d\n", "LINUX_SYS_inotify_init1", SYS_inotify_init1);
#endif
#ifdef SYS_inotify_rm_watch
    printf("#define %-32.32s %d\n", "LINUX_SYS_inotify_rm_watch", SYS_inotify_rm_watch);
#endif
#ifdef SYS_io_cancel
    printf("#define %-32.32s %d\n", "LINUX_SYS_io_cancel", SYS_io_cancel);
#endif
#ifdef SYS_io_destroy
    printf("#define %-32.32s %d\n", "LINUX_SYS_io_destroy", SYS_io_destroy);
#endif
#ifdef SYS_io_getevents
    printf("#define %-32.32s %d\n", "LINUX_SYS_io_getevents", SYS_io_getevents);
#endif
#ifdef SYS_io_pgetevents
    printf("#define %-32.32s %d\n", "LINUX_SYS_io_pgetevents", SYS_io_pgetevents);
#endif
#ifdef SYS_io_setup
    printf("#define %-32.32s %d\n", "LINUX_SYS_io_setup", SYS_io_setup);
#endif
#ifdef SYS_io_submit
    printf("#define %-32.32s %d\n", "LINUX_SYS_io_submit", SYS_io_submit);
#endif
#ifdef SYS_io_uring_enter
    printf("#define %-32.32s %d\n", "LINUX_SYS_io_uring_enter", SYS_io_uring_enter);
#endif
#ifdef SYS_io_uring_register
    printf("#define %-32.32s %d\n", "LINUX_SYS_io_uring_register", SYS_io_uring_register);
#endif
#ifdef SYS_io_uring_setup
    printf("#define %-32.32s %d\n", "LINUX_SYS_io_uring_setup", SYS_io_uring_setup);
#endif
#ifdef SYS_ioctl
    printf("#define %-32.32s %d\n", "LINUX_SYS_ioctl", SYS_ioctl);
#endif
#ifdef SYS_ioperm
    printf("#define %-32.32s %d\n", "LINUX_SYS_ioperm", SYS_ioperm);
#endif
#ifdef SYS_iopl
    printf("#define %-32.32s %d\n", "LINUX_SYS_iopl", SYS_iopl);
#endif
#ifdef SYS_ioprio_get
    printf("#define %-32.32s %d\n", "LINUX_SYS_ioprio_get", SYS_ioprio_get);
#endif
#ifdef SYS_ioprio_set
    printf("#define %-32.32s %d\n", "LINUX_SYS_ioprio_set", SYS_ioprio_set);
#endif
#ifdef SYS_ipc
    printf("#define %-32.32s %d\n", "LINUX_SYS_ipc", SYS_ipc);
#endif
#ifdef SYS_kcmp
    printf("#define %-32.32s %d\n", "LINUX_SYS_kcmp", SYS_kcmp);
#endif
#ifdef SYS_kern_features
    printf("#define %-32.32s %d\n", "LINUX_SYS_kern_features", SYS_kern_features);
#endif
#ifdef SYS_kexec_file_load
    printf("#define %-32.32s %d\n", "LINUX_SYS_kexec_file_load", SYS_kexec_file_load);
#endif
#ifdef SYS_kexec_load
    printf("#define %-32.32s %d\n", "LINUX_SYS_kexec_load", SYS_kexec_load);
#endif
#ifdef SYS_keyctl
    printf("#define %-32.32s %d\n", "LINUX_SYS_keyctl", SYS_keyctl);
#endif
#ifdef SYS_kill
    printf("#define %-32.32s %d\n", "LINUX_SYS_kill", SYS_kill);
#endif
#ifdef SYS_landlock_add_rule
    printf("#define %-32.32s %d\n", "LINUX_SYS_landlock_add_rule", SYS_landlock_add_rule);
#endif
#ifdef SYS_landlock_create_ruleset
    printf("#define %-32.32s %d\n", "LINUX_SYS_landlock_create_ruleset",
           SYS_landlock_create_ruleset);
#endif
#ifdef SYS_landlock_restrict_self
    printf("#define %-32.32s %d\n", "LINUX_SYS_landlock_restrict_self", SYS_landlock_restrict_self);
#endif
#ifdef SYS_lchown
    printf("#define %-32.32s %d\n", "LINUX_SYS_lchown", SYS_lchown);
#endif
#ifdef SYS_lchown32
    printf("#define %-32.32s %d\n", "LINUX_SYS_lchown32", SYS_lchown32);
#endif
#ifdef SYS_lgetxattr
    printf("#define %-32.32s %d\n", "LINUX_SYS_lgetxattr", SYS_lgetxattr);
#endif
#ifdef SYS_link
    printf("#define %-32.32s %d\n", "LINUX_SYS_link", SYS_link);
#endif
#ifdef SYS_linkat
    printf("#define %-32.32s %d\n", "LINUX_SYS_linkat", SYS_linkat);
#endif
#ifdef SYS_listen
    printf("#define %-32.32s %d\n", "LINUX_SYS_listen", SYS_listen);
#endif
#ifdef SYS_listxattr
    printf("#define %-32.32s %d\n", "LINUX_SYS_listxattr", SYS_listxattr);
#endif
#ifdef SYS_llistxattr
    printf("#define %-32.32s %d\n", "LINUX_SYS_llistxattr", SYS_llistxattr);
#endif
#ifdef SYS_lookup_dcookie
    printf("#define %-32.32s %d\n", "LINUX_SYS_lookup_dcookie", SYS_lookup_dcookie);
#endif
#ifdef SYS_lremovexattr
    printf("#define %-32.32s %d\n", "LINUX_SYS_lremovexattr", SYS_lremovexattr);
#endif
#ifdef SYS_lseek
    printf("#define %-32.32s %d\n", "LINUX_SYS_lseek", SYS_lseek);
#endif
#ifdef SYS_lsetxattr
    printf("#define %-32.32s %d\n", "LINUX_SYS_lsetxattr", SYS_lsetxattr);
#endif
#ifdef SYS_lstat
    printf("#define %-32.32s %d\n", "LINUX_SYS_lstat", SYS_lstat);
#endif
#ifdef SYS_lstat64
    printf("#define %-32.32s %d\n", "LINUX_SYS_lstat64", SYS_lstat64);
#endif
#ifdef SYS_madvise
    printf("#define %-32.32s %d\n", "LINUX_SYS_madvise", SYS_madvise);
#endif
#ifdef SYS_mbind
    printf("#define %-32.32s %d\n", "LINUX_SYS_mbind", SYS_mbind);
#endif
#ifdef SYS_memory_ordering
    printf("#define %-32.32s %d\n", "LINUX_SYS_memory_ordering", SYS_memory_ordering);
#endif
#ifdef SYS_membarrier
    printf("#define %-32.32s %d\n", "LINUX_SYS_membarrier", SYS_membarrier);
#endif
#ifdef SYS_memfd_create
    printf("#define %-32.32s %d\n", "LINUX_SYS_memfd_create", SYS_memfd_create);
#endif
#ifdef SYS_memfd_secret
    printf("#define %-32.32s %d\n", "LINUX_SYS_memfd_secret", SYS_memfd_secret);
#endif
#ifdef SYS_migrate_pages
    printf("#define %-32.32s %d\n", "LINUX_SYS_migrate_pages", SYS_migrate_pages);
#endif
#ifdef SYS_mincore
    printf("#define %-32.32s %d\n", "LINUX_SYS_mincore", SYS_mincore);
#endif
#ifdef SYS_mkdir
    printf("#define %-32.32s %d\n", "LINUX_SYS_mkdir", SYS_mkdir);
#endif
#ifdef SYS_mkdirat
    printf("#define %-32.32s %d\n", "LINUX_SYS_mkdirat", SYS_mkdirat);
#endif
#ifdef SYS_mknod
    printf("#define %-32.32s %d\n", "LINUX_SYS_mknod", SYS_mknod);
#endif
#ifdef SYS_mknodat
    printf("#define %-32.32s %d\n", "LINUX_SYS_mknodat", SYS_mknodat);
#endif
#ifdef SYS_mlock
    printf("#define %-32.32s %d\n", "LINUX_SYS_mlock", SYS_mlock);
#endif
#ifdef SYS_mlock2
    printf("#define %-32.32s %d\n", "LINUX_SYS_mlock2", SYS_mlock2);
#endif
#ifdef SYS_mlockall
    printf("#define %-32.32s %d\n", "LINUX_SYS_mlockall", SYS_mlockall);
#endif
#ifdef SYS_mmap
    printf("#define %-32.32s %d\n", "LINUX_SYS_mmap", SYS_mmap);
#endif
#ifdef SYS_mmap2
    printf("#define %-32.32s %d\n", "LINUX_SYS_mmap2", SYS_mmap2);
#endif
#ifdef SYS_modify_ldt
    printf("#define %-32.32s %d\n", "LINUX_SYS_modify_ldt", SYS_modify_ldt);
#endif
#ifdef SYS_mount
    printf("#define %-32.32s %d\n", "LINUX_SYS_mount", SYS_mount);
#endif
#ifdef SYS_move_mount
    printf("#define %-32.32s %d\n", "LINUX_SYS_move_mount", SYS_move_mount);
#endif
#ifdef SYS_move_pages
    printf("#define %-32.32s %d\n", "LINUX_SYS_move_pages", SYS_move_pages);
#endif
#ifdef SYS_mprotect
    printf("#define %-32.32s %d\n", "LINUX_SYS_mprotect", SYS_mprotect);
#endif
#ifdef SYS_mq_getsetattr
    printf("#define %-32.32s %d\n", "LINUX_SYS_mq_getsetattr", SYS_mq_getsetattr);
#endif
#ifdef SYS_mq_notify
    printf("#define %-32.32s %d\n", "LINUX_SYS_mq_notify", SYS_mq_notify);
#endif
#ifdef SYS_mq_open
    printf("#define %-32.32s %d\n", "LINUX_SYS_mq_open", SYS_mq_open);
#endif
#ifdef SYS_mq_timedreceive
    printf("#define %-32.32s %d\n", "LINUX_SYS_mq_timedreceive", SYS_mq_timedreceive);
#endif
#ifdef SYS_mq_timedsend
    printf("#define %-32.32s %d\n", "LINUX_SYS_mq_timedsend", SYS_mq_timedsend);
#endif
#ifdef SYS_mq_unlink
    printf("#define %-32.32s %d\n", "LINUX_SYS_mq_unlink", SYS_mq_unlink);
#endif
#ifdef SYS_mremap
    printf("#define %-32.32s %d\n", "LINUX_SYS_mremap", SYS_mremap);
#endif
#ifdef SYS_msgctl
    printf("#define %-32.32s %d\n", "LINUX_SYS_msgctl", SYS_msgctl);
#endif
#ifdef SYS_msgget
    printf("#define %-32.32s %d\n", "LINUX_SYS_msgget", SYS_msgget);
#endif
#ifdef SYS_msgrcv
    printf("#define %-32.32s %d\n", "LINUX_SYS_msgrcv", SYS_msgrcv);
#endif
#ifdef SYS_msgsnd
    printf("#define %-32.32s %d\n", "LINUX_SYS_msgsnd", SYS_msgsnd);
#endif
#ifdef SYS_msync
    printf("#define %-32.32s %d\n", "LINUX_SYS_msync", SYS_msync);
#endif
#ifdef SYS_munlock
    printf("#define %-32.32s %d\n", "LINUX_SYS_munlock", SYS_munlock);
#endif
#ifdef SYS_munlockall
    printf("#define %-32.32s %d\n", "LINUX_SYS_munlockall", SYS_munlockall);
#endif
#ifdef SYS_munmap
    printf("#define %-32.32s %d\n", "LINUX_SYS_munmap", SYS_munmap);
#endif
#ifdef SYS_name_to_handle_at
    printf("#define %-32.32s %d\n", "LINUX_SYS_name_to_handle_at", SYS_name_to_handle_at);
#endif
#ifdef SYS_nanosleep
    printf("#define %-32.32s %d\n", "LINUX_SYS_nanosleep", SYS_nanosleep);
#endif
#ifdef SYS_newfstatat
    printf("#define %-32.32s %d\n", "LINUX_SYS_newfstatat", SYS_newfstatat);
#endif
#ifdef SYS_nfsservctl
    printf("#define %-32.32s %d\n", "LINUX_SYS_nfsservctl", SYS_nfsservctl);
#endif
#ifdef SYS_nice
    printf("#define %-32.32s %d\n", "LINUX_SYS_nice", SYS_nice);
#endif
#ifdef SYS_old_adjtimex
    printf("#define %-32.32s %d\n", "LINUX_SYS_old_adjtimex", SYS_old_adjtimex);
#endif
#ifdef SYS_old_getrlimit
    printf("#define %-32.32s %d\n", "LINUX_SYS_old_getrlimit", SYS_old_getrlimit);
#endif
#ifdef SYS_oldfstat
    printf("#define %-32.32s %d\n", "LINUX_SYS_oldfstat", SYS_oldfstat);
#endif
#ifdef SYS_oldlstat
    printf("#define %-32.32s %d\n", "LINUX_SYS_oldlstat", SYS_oldlstat);
#endif
#ifdef SYS_oldolduname
    printf("#define %-32.32s %d\n", "LINUX_SYS_oldolduname", SYS_oldolduname);
#endif
#ifdef SYS_oldstat
    printf("#define %-32.32s %d\n", "LINUX_SYS_oldstat", SYS_oldstat);
#endif
#ifdef SYS_oldumount
    printf("#define %-32.32s %d\n", "LINUX_SYS_oldumount", SYS_oldumount);
#endif
#ifdef SYS_olduname
    printf("#define %-32.32s %d\n", "LINUX_SYS_olduname", SYS_olduname);
#endif
#ifdef SYS_open
    printf("#define %-32.32s %d\n", "LINUX_SYS_open", SYS_open);
#endif
#ifdef SYS_open_by_handle_at
    printf("#define %-32.32s %d\n", "LINUX_SYS_open_by_handle_at", SYS_open_by_handle_at);
#endif
#ifdef SYS_open_tree
    printf("#define %-32.32s %d\n", "LINUX_SYS_open_tree", SYS_open_tree);
#endif
#ifdef SYS_openat
    printf("#define %-32.32s %d\n", "LINUX_SYS_openat", SYS_openat);
#endif
#ifdef SYS_openat2
    printf("#define %-32.32s %d\n", "LINUX_SYS_openat2", SYS_openat2);
#endif
#ifdef SYS_or1k_atomic
    printf("#define %-32.32s %d\n", "LINUX_SYS_or1k_atomic", SYS_or1k_atomic);
#endif
#ifdef SYS_pause
    printf("#define %-32.32s %d\n", "LINUX_SYS_pause", SYS_pause);
#endif
#ifdef SYS_pciconfig_iobase
    printf("#define %-32.32s %d\n", "LINUX_SYS_pciconfig_iobase", SYS_pciconfig_iobase);
#endif
#ifdef SYS_pciconfig_read
    printf("#define %-32.32s %d\n", "LINUX_SYS_pciconfig_read", SYS_pciconfig_read);
#endif
#ifdef SYS_pciconfig_write
    printf("#define %-32.32s %d\n", "LINUX_SYS_pciconfig_write", SYS_pciconfig_write);
#endif
#ifdef SYS_perf_event_open
    printf("#define %-32.32s %d\n", "LINUX_SYS_perf_event_open", SYS_perf_event_open);
#endif
#ifdef SYS_personality
    printf("#define %-32.32s %d\n", "LINUX_SYS_personality", SYS_personality);
#endif
#ifdef SYS_perfctr
    printf("#define %-32.32s %d\n", "LINUX_SYS_perfctr", SYS_perfctr);
#endif
#ifdef SYS_perfmonctl
    printf("#define %-32.32s %d\n", "LINUX_SYS_perfmonctl", SYS_perfmonctl);
#endif
#ifdef SYS_pidfd_getfd
    printf("#define %-32.32s %d\n", "LINUX_SYS_pidfd_getfd", SYS_pidfd_getfd);
#endif
#ifdef SYS_pidfd_send_signal
    printf("#define %-32.32s %d\n", "LINUX_SYS_pidfd_send_signal", SYS_pidfd_send_signal);
#endif
#ifdef SYS_pidfd_open
    printf("#define %-32.32s %d\n", "LINUX_SYS_pidfd_open", SYS_pidfd_open);
#endif
#ifdef SYS_pipe
    printf("#define %-32.32s %d\n", "LINUX_SYS_pipe", SYS_pipe);
#endif
#ifdef SYS_pipe2
    printf("#define %-32.32s %d\n", "LINUX_SYS_pipe2", SYS_pipe2);
#endif
#ifdef SYS_pivot_root
    printf("#define %-32.32s %d\n", "LINUX_SYS_pivot_root", SYS_pivot_root);
#endif
#ifdef SYS_pkey_alloc
    printf("#define %-32.32s %d\n", "LINUX_SYS_pkey_alloc", SYS_pkey_alloc);
#endif
#ifdef SYS_pkey_free
    printf("#define %-32.32s %d\n", "LINUX_SYS_pkey_free", SYS_pkey_free);
#endif
#ifdef SYS_pkey_mprotect
    printf("#define %-32.32s %d\n", "LINUX_SYS_pkey_mprotect", SYS_pkey_mprotect);
#endif
#ifdef SYS_poll
    printf("#define %-32.32s %d\n", "LINUX_SYS_poll", SYS_poll);
#endif
#ifdef SYS_ppoll
    printf("#define %-32.32s %d\n", "LINUX_SYS_ppoll", SYS_ppoll);
#endif
#ifdef SYS_prctl
    printf("#define %-32.32s %d\n", "LINUX_SYS_prctl", SYS_prctl);
#endif
#ifdef SYS_pread64
    printf("#define %-32.32s %d\n", "LINUX_SYS_pread64", SYS_pread64);
#endif
#ifdef SYS_preadv
    printf("#define %-32.32s %d\n", "LINUX_SYS_preadv", SYS_preadv);
#endif
#ifdef SYS_preadv2
    printf("#define %-32.32s %d\n", "LINUX_SYS_preadv2", SYS_preadv2);
#endif
#ifdef SYS_prlimit64
    printf("#define %-32.32s %d\n", "LINUX_SYS_prlimit64", SYS_prlimit64);
#endif
#ifdef SYS_process_madvise
    printf("#define %-32.32s %d\n", "LINUX_SYS_process_madvise", SYS_process_madvise);
#endif
#ifdef SYS_process_vm_readv
    printf("#define %-32.32s %d\n", "LINUX_SYS_process_vm_readv", SYS_process_vm_readv);
#endif
#ifdef SYS_process_vm_writev
    printf("#define %-32.32s %d\n", "LINUX_SYS_process_vm_writev", SYS_process_vm_writev);
#endif
#ifdef SYS_pselect6
    printf("#define %-32.32s %d\n", "LINUX_SYS_pselect6", SYS_pselect6);
#endif
#ifdef SYS_ptrace
    printf("#define %-32.32s %d\n", "LINUX_SYS_ptrace", SYS_ptrace);
#endif
#ifdef SYS_pwrite64
    printf("#define %-32.32s %d\n", "LINUX_SYS_pwrite64", SYS_pwrite64);
#endif
#ifdef SYS_pwritev
    printf("#define %-32.32s %d\n", "LINUX_SYS_pwritev", SYS_pwritev);
#endif
#ifdef SYS_pwritev2
    printf("#define %-32.32s %d\n", "LINUX_SYS_pwritev2", SYS_pwritev2);
#endif
#ifdef SYS_query_module
    printf("#define %-32.32s %d\n", "LINUX_SYS_query_module", SYS_query_module);
#endif
#ifdef SYS_quotactl
    printf("#define %-32.32s %d\n", "LINUX_SYS_quotactl", SYS_quotactl);
#endif
#ifdef SYS_quotactl_fd
    printf("#define %-32.32s %d\n", "LINUX_SYS_quotactl_fd", SYS_quotactl_fd);
#endif
#ifdef SYS_read
    printf("#define %-32.32s %d\n", "LINUX_SYS_read", SYS_read);
#endif
#ifdef SYS_readahead
    printf("#define %-32.32s %d\n", "LINUX_SYS_readahead", SYS_readahead);
#endif
#ifdef SYS_readdir
    printf("#define %-32.32s %d\n", "LINUX_SYS_readdir", SYS_readdir);
#endif
#ifdef SYS_readlink
    printf("#define %-32.32s %d\n", "LINUX_SYS_readlink", SYS_readlink);
#endif
#ifdef SYS_readlinkat
    printf("#define %-32.32s %d\n", "LINUX_SYS_readlinkat", SYS_readlinkat);
#endif
#ifdef SYS_readv
    printf("#define %-32.32s %d\n", "LINUX_SYS_readv", SYS_readv);
#endif
#ifdef SYS_reboot
    printf("#define %-32.32s %d\n", "LINUX_SYS_reboot", SYS_reboot);
#endif
#ifdef SYS_recv
    printf("#define %-32.32s %d\n", "LINUX_SYS_recv", SYS_recv);
#endif
#ifdef SYS_recvfrom
    printf("#define %-32.32s %d\n", "LINUX_SYS_recvfrom", SYS_recvfrom);
#endif
#ifdef SYS_recvmsg
    printf("#define %-32.32s %d\n", "LINUX_SYS_recvmsg", SYS_recvmsg);
#endif
#ifdef SYS_recvmmsg
    printf("#define %-32.32s %d\n", "LINUX_SYS_recvmmsg", SYS_recvmmsg);
#endif
#ifdef SYS_remap_file_pages
    printf("#define %-32.32s %d\n", "LINUX_SYS_remap_file_pages", SYS_remap_file_pages);
#endif
#ifdef SYS_removexattr
    printf("#define %-32.32s %d\n", "LINUX_SYS_removexattr", SYS_removexattr);
#endif
#ifdef SYS_rename
    printf("#define %-32.32s %d\n", "LINUX_SYS_rename", SYS_rename);
#endif
#ifdef SYS_renameat
    printf("#define %-32.32s %d\n", "LINUX_SYS_renameat", SYS_renameat);
#endif
#ifdef SYS_renameat2
    printf("#define %-32.32s %d\n", "LINUX_SYS_renameat2", SYS_renameat2);
#endif
#ifdef SYS_request_key
    printf("#define %-32.32s %d\n", "LINUX_SYS_request_key", SYS_request_key);
#endif
#ifdef SYS_restart_syscall
    printf("#define %-32.32s %d\n", "LINUX_SYS_restart_syscall", SYS_restart_syscall);
#endif
#ifdef SYS_riscv_flush_icache
    printf("#define %-32.32s %d\n", "LINUX_SYS_riscv_flush_icache", SYS_riscv_flush_icache);
#endif
#ifdef SYS_rmdir
    printf("#define %-32.32s %d\n", "LINUX_SYS_rmdir", SYS_rmdir);
#endif
#ifdef SYS_rseq
    printf("#define %-32.32s %d\n", "LINUX_SYS_rseq", SYS_rseq);
#endif
#ifdef SYS_rt_sigaction
    printf("#define %-32.32s %d\n", "LINUX_SYS_rt_sigaction", SYS_rt_sigaction);
#endif
#ifdef SYS_rt_sigpending
    printf("#define %-32.32s %d\n", "LINUX_SYS_rt_sigpending", SYS_rt_sigpending);
#endif
#ifdef SYS_rt_sigprocmask
    printf("#define %-32.32s %d\n", "LINUX_SYS_rt_sigprocmask", SYS_rt_sigprocmask);
#endif
#ifdef SYS_rt_sigqueueinfo
    printf("#define %-32.32s %d\n", "LINUX_SYS_rt_sigqueueinfo", SYS_rt_sigqueueinfo);
#endif
#ifdef SYS_rt_sigreturn
    printf("#define %-32.32s %d\n", "LINUX_SYS_rt_sigreturn", SYS_rt_sigreturn);
#endif
#ifdef SYS_rt_sigsuspend
    printf("#define %-32.32s %d\n", "LINUX_SYS_rt_sigsuspend", SYS_rt_sigsuspend);
#endif
#ifdef SYS_rt_sigtimedwait
    printf("#define %-32.32s %d\n", "LINUX_SYS_rt_sigtimedwait", SYS_rt_sigtimedwait);
#endif
#ifdef SYS_rt_tgsigqueueinfo
    printf("#define %-32.32s %d\n", "LINUX_SYS_rt_tgsigqueueinfo", SYS_rt_tgsigqueueinfo);
#endif
#ifdef SYS_rtas
    printf("#define %-32.32s %d\n", "LINUX_SYS_rtas", SYS_rtas);
#endif
#ifdef SYS_s390_runtime_instr
    printf("#define %-32.32s %d\n", "LINUX_SYS_s390_runtime_instr", SYS_s390_runtime_instr);
#endif
#ifdef SYS_s390_pci_mmio_read
    printf("#define %-32.32s %d\n", "LINUX_SYS_s390_pci_mmio_read", SYS_s390_pci_mmio_read);
#endif
#ifdef SYS_s390_pci_mmio_write
    printf("#define %-32.32s %d\n", "LINUX_SYS_s390_pci_mmio_write", SYS_s390_pci_mmio_write);
#endif
#ifdef SYS_s390_sthyi
    printf("#define %-32.32s %d\n", "LINUX_SYS_s390_sthyi", SYS_s390_sthyi);
#endif
#ifdef SYS_s390_guarded_storage
    printf("#define %-32.32s %d\n", "LINUX_SYS_s390_guarded_storage", SYS_s390_guarded_storage);
#endif
#ifdef SYS_sched_get_affinity
    printf("#define %-32.32s %d\n", "LINUX_SYS_sched_get_affinity", SYS_sched_get_affinity);
#endif
#ifdef SYS_sched_get_priority_max
    printf("#define %-32.32s %d\n", "LINUX_SYS_sched_get_priority_max", SYS_sched_get_priority_max);
#endif
#ifdef SYS_sched_get_priority_min
    printf("#define %-32.32s %d\n", "LINUX_SYS_sched_get_priority_min", SYS_sched_get_priority_min);
#endif
#ifdef SYS_sched_getaffinity
    printf("#define %-32.32s %d\n", "LINUX_SYS_sched_getaffinity", SYS_sched_getaffinity);
#endif
#ifdef SYS_sched_getattr
    printf("#define %-32.32s %d\n", "LINUX_SYS_sched_getattr", SYS_sched_getattr);
#endif
#ifdef SYS_sched_getparam
    printf("#define %-32.32s %d\n", "LINUX_SYS_sched_getparam", SYS_sched_getparam);
#endif
#ifdef SYS_sched_getscheduler
    printf("#define %-32.32s %d\n", "LINUX_SYS_sched_getscheduler", SYS_sched_getscheduler);
#endif
#ifdef SYS_sched_rr_get_interval
    printf("#define %-32.32s %d\n", "LINUX_SYS_sched_rr_get_interval", SYS_sched_rr_get_interval);
#endif
#ifdef SYS_sched_set_affinity
    printf("#define %-32.32s %d\n", "LINUX_SYS_sched_set_affinity", SYS_sched_set_affinity);
#endif
#ifdef SYS_sched_setaffinity
    printf("#define %-32.32s %d\n", "LINUX_SYS_sched_setaffinity", SYS_sched_setaffinity);
#endif
#ifdef SYS_sched_setattr
    printf("#define %-32.32s %d\n", "LINUX_SYS_sched_setattr", SYS_sched_setattr);
#endif
#ifdef SYS_sched_setparam
    printf("#define %-32.32s %d\n", "LINUX_SYS_sched_setparam", SYS_sched_setparam);
#endif
#ifdef SYS_sched_setscheduler
    printf("#define %-32.32s %d\n", "LINUX_SYS_sched_setscheduler", SYS_sched_setscheduler);
#endif
#ifdef SYS_sched_yield
    printf("#define %-32.32s %d\n", "LINUX_SYS_sched_yield", SYS_sched_yield);
#endif
#ifdef SYS_seccomp
    printf("#define %-32.32s %d\n", "LINUX_SYS_seccomp", SYS_seccomp);
#endif
#ifdef SYS_select
    printf("#define %-32.32s %d\n", "LINUX_SYS_select", SYS_select);
#endif
#ifdef SYS_semctl
    printf("#define %-32.32s %d\n", "LINUX_SYS_semctl", SYS_semctl);
#endif
#ifdef SYS_semget
    printf("#define %-32.32s %d\n", "LINUX_SYS_semget", SYS_semget);
#endif
#ifdef SYS_semop
    printf("#define %-32.32s %d\n", "LINUX_SYS_semop", SYS_semop);
#endif
#ifdef SYS_semtimedop
    printf("#define %-32.32s %d\n", "LINUX_SYS_semtimedop", SYS_semtimedop);
#endif
#ifdef SYS_send
    printf("#define %-32.32s %d\n", "LINUX_SYS_send", SYS_send);
#endif
#ifdef SYS_sendfile
    printf("#define %-32.32s %d\n", "LINUX_SYS_sendfile", SYS_sendfile);
#endif
#ifdef SYS_sendfile64
    printf("#define %-32.32s %d\n", "LINUX_SYS_sendfile64", SYS_sendfile64);
#endif
#ifdef SYS_sendmmsg
    printf("#define %-32.32s %d\n", "LINUX_SYS_sendmmsg", SYS_sendmmsg);
#endif
#ifdef SYS_sendmsg
    printf("#define %-32.32s %d\n", "LINUX_SYS_sendmsg", SYS_sendmsg);
#endif
#ifdef SYS_sendto
    printf("#define %-32.32s %d\n", "LINUX_SYS_sendto", SYS_sendto);
#endif
#ifdef SYS_set_mempolicy
    printf("#define %-32.32s %d\n", "LINUX_SYS_set_mempolicy", SYS_set_mempolicy);
#endif
#ifdef SYS_set_robust_list
    printf("#define %-32.32s %d\n", "LINUX_SYS_set_robust_list", SYS_set_robust_list);
#endif
#ifdef SYS_set_thread_area
    printf("#define %-32.32s %d\n", "LINUX_SYS_set_thread_area", SYS_set_thread_area);
#endif
#ifdef SYS_set_tid_address
    printf("#define %-32.32s %d\n", "LINUX_SYS_set_tid_address", SYS_set_tid_address);
#endif
#ifdef SYS_set_tls
    printf("#define %-32.32s %d\n", "LINUX_SYS_set_tls", SYS_set_tls);
#endif
#ifdef SYS_setdomainname
    printf("#define %-32.32s %d\n", "LINUX_SYS_setdomainname", SYS_setdomainname);
#endif
#ifdef SYS_setfsgid
    printf("#define %-32.32s %d\n", "LINUX_SYS_setfsgid", SYS_setfsgid);
#endif
#ifdef SYS_setfsgid32
    printf("#define %-32.32s %d\n", "LINUX_SYS_setfsgid32", SYS_setfsgid32);
#endif
#ifdef SYS_setfsuid
    printf("#define %-32.32s %d\n", "LINUX_SYS_setfsuid", SYS_setfsuid);
#endif
#ifdef SYS_setfsuid32
    printf("#define %-32.32s %d\n", "LINUX_SYS_setfsuid32", SYS_setfsuid32);
#endif
#ifdef SYS_setgid
    printf("#define %-32.32s %d\n", "LINUX_SYS_setgid", SYS_setgid);
#endif
#ifdef SYS_setgid32
    printf("#define %-32.32s %d\n", "LINUX_SYS_setgid32", SYS_setgid32);
#endif
#ifdef SYS_setgroups
    printf("#define %-32.32s %d\n", "LINUX_SYS_setgroups", SYS_setgroups);
#endif
#ifdef SYS_setgroups32
    printf("#define %-32.32s %d\n", "LINUX_SYS_setgroups32", SYS_setgroups32);
#endif
#ifdef SYS_sethae
    printf("#define %-32.32s %d\n", "LINUX_SYS_sethae", SYS_sethae);
#endif
#ifdef SYS_sethostname
    printf("#define %-32.32s %d\n", "LINUX_SYS_sethostname", SYS_sethostname);
#endif
#ifdef SYS_setitimer
    printf("#define %-32.32s %d\n", "LINUX_SYS_setitimer", SYS_setitimer);
#endif
#ifdef SYS_setns
    printf("#define %-32.32s %d\n", "LINUX_SYS_setns", SYS_setns);
#endif
#ifdef SYS_setpgid
    printf("#define %-32.32s %d\n", "LINUX_SYS_setpgid", SYS_setpgid);
#endif
#ifdef SYS_setpgrp
    printf("#define %-32.32s %d\n", "LINUX_SYS_setpgrp", SYS_setpgrp);
#endif
#ifdef SYS_setpriority
    printf("#define %-32.32s %d\n", "LINUX_SYS_setpriority", SYS_setpriority);
#endif
#ifdef SYS_setregid
    printf("#define %-32.32s %d\n", "LINUX_SYS_setregid", SYS_setregid);
#endif
#ifdef SYS_setregid32
    printf("#define %-32.32s %d\n", "LINUX_SYS_setregid32", SYS_setregid32);
#endif
#ifdef SYS_setresgid
    printf("#define %-32.32s %d\n", "LINUX_SYS_setresgid", SYS_setresgid);
#endif
#ifdef SYS_setresgid32
    printf("#define %-32.32s %d\n", "LINUX_SYS_setresgid32", SYS_setresgid32);
#endif
#ifdef SYS_setresuid
    printf("#define %-32.32s %d\n", "LINUX_SYS_setresuid", SYS_setresuid);
#endif
#ifdef SYS_setresuid32
    printf("#define %-32.32s %d\n", "LINUX_SYS_setresuid32", SYS_setresuid32);
#endif
#ifdef SYS_setreuid
    printf("#define %-32.32s %d\n", "LINUX_SYS_setreuid", SYS_setreuid);
#endif
#ifdef SYS_setreuid32
    printf("#define %-32.32s %d\n", "LINUX_SYS_setreuid32", SYS_setreuid32);
#endif
#ifdef SYS_setrlimit
    printf("#define %-32.32s %d\n", "LINUX_SYS_setrlimit", SYS_setrlimit);
#endif
#ifdef SYS_setsid
    printf("#define %-32.32s %d\n", "LINUX_SYS_setsid", SYS_setsid);
#endif
#ifdef SYS_setsockopt
    printf("#define %-32.32s %d\n", "LINUX_SYS_setsockopt", SYS_setsockopt);
#endif
#ifdef SYS_settimeofday
    printf("#define %-32.32s %d\n", "LINUX_SYS_settimeofday", SYS_settimeofday);
#endif
#ifdef SYS_setuid
    printf("#define %-32.32s %d\n", "LINUX_SYS_setuid", SYS_setuid);
#endif
#ifdef SYS_setuid32
    printf("#define %-32.32s %d\n", "LINUX_SYS_setuid32", SYS_setuid32);
#endif
#ifdef SYS_setup
    printf("#define %-32.32s %d\n", "LINUX_SYS_setup", SYS_setup);
#endif
#ifdef SYS_setxattr
    printf("#define %-32.32s %d\n", "LINUX_SYS_setxattr", SYS_setxattr);
#endif
#ifdef SYS_sgetmask
    printf("#define %-32.32s %d\n", "LINUX_SYS_sgetmask", SYS_sgetmask);
#endif
#ifdef SYS_shmat
    printf("#define %-32.32s %d\n", "LINUX_SYS_shmat", SYS_shmat);
#endif
#ifdef SYS_shmctl
    printf("#define %-32.32s %d\n", "LINUX_SYS_shmctl", SYS_shmctl);
#endif
#ifdef SYS_shmdt
    printf("#define %-32.32s %d\n", "LINUX_SYS_shmdt", SYS_shmdt);
#endif
#ifdef SYS_shmget
    printf("#define %-32.32s %d\n", "LINUX_SYS_shmget", SYS_shmget);
#endif
#ifdef SYS_shutdown
    printf("#define %-32.32s %d\n", "LINUX_SYS_shutdown", SYS_shutdown);
#endif
#ifdef SYS_sigaction
    printf("#define %-32.32s %d\n", "LINUX_SYS_sigaction", SYS_sigaction);
#endif
#ifdef SYS_sigaltstack
    printf("#define %-32.32s %d\n", "LINUX_SYS_sigaltstack", SYS_sigaltstack);
#endif
#ifdef SYS_signal
    printf("#define %-32.32s %d\n", "LINUX_SYS_signal", SYS_signal);
#endif
#ifdef SYS_signalfd
    printf("#define %-32.32s %d\n", "LINUX_SYS_signalfd", SYS_signalfd);
#endif
#ifdef SYS_signalfd4
    printf("#define %-32.32s %d\n", "LINUX_SYS_signalfd4", SYS_signalfd4);
#endif
#ifdef SYS_sigpending
    printf("#define %-32.32s %d\n", "LINUX_SYS_sigpending", SYS_sigpending);
#endif
#ifdef SYS_sigprocmask
    printf("#define %-32.32s %d\n", "LINUX_SYS_sigprocmask", SYS_sigprocmask);
#endif
#ifdef SYS_sigreturn
    printf("#define %-32.32s %d\n", "LINUX_SYS_sigreturn", SYS_sigreturn);
#endif
#ifdef SYS_sigsuspend
    printf("#define %-32.32s %d\n", "LINUX_SYS_sigsuspend", SYS_sigsuspend);
#endif
#ifdef SYS_socket
    printf("#define %-32.32s %d\n", "LINUX_SYS_socket", SYS_socket);
#endif
#ifdef SYS_socketcall
    printf("#define %-32.32s %d\n", "LINUX_SYS_socketcall", SYS_socketcall);
#endif
#ifdef SYS_socketpair
    printf("#define %-32.32s %d\n", "LINUX_SYS_socketpair", SYS_socketpair);
#endif
#ifdef SYS_spill
    printf("#define %-32.32s %d\n", "LINUX_SYS_spill", SYS_spill);
#endif
#ifdef SYS_splice
    printf("#define %-32.32s %d\n", "LINUX_SYS_splice", SYS_splice);
#endif
#ifdef SYS_spu_create
    printf("#define %-32.32s %d\n", "LINUX_SYS_spu_create", SYS_spu_create);
#endif
#ifdef SYS_spu_run
    printf("#define %-32.32s %d\n", "LINUX_SYS_spu_run", SYS_spu_run);
#endif
#ifdef SYS_ssetmask
    printf("#define %-32.32s %d\n", "LINUX_SYS_ssetmask", SYS_ssetmask);
#endif
#ifdef SYS_stat
    printf("#define %-32.32s %d\n", "LINUX_SYS_stat", SYS_stat);
#endif
#ifdef SYS_stat64
    printf("#define %-32.32s %d\n", "LINUX_SYS_stat64", SYS_stat64);
#endif
#ifdef SYS_statfs
    printf("#define %-32.32s %d\n", "LINUX_SYS_statfs", SYS_statfs);
#endif
#ifdef SYS_statfs64
    printf("#define %-32.32s %d\n", "LINUX_SYS_statfs64", SYS_statfs64);
#endif
#ifdef SYS_statx
    printf("#define %-32.32s %d\n", "LINUX_SYS_statx", SYS_statx);
#endif
#ifdef SYS_stime
    printf("#define %-32.32s %d\n", "LINUX_SYS_stime", SYS_stime);
#endif
#ifdef SYS_subpage_prot
    printf("#define %-32.32s %d\n", "LINUX_SYS_subpage_prot", SYS_subpage_prot);
#endif
#ifdef SYS_swapcontext
    printf("#define %-32.32s %d\n", "LINUX_SYS_swapcontext", SYS_swapcontext);
#endif
#ifdef SYS_switch_endian
    printf("#define %-32.32s %d\n", "LINUX_SYS_switch_endian", SYS_switch_endian);
#endif
#ifdef SYS_swapoff
    printf("#define %-32.32s %d\n", "LINUX_SYS_swapoff", SYS_swapoff);
#endif
#ifdef SYS_swapon
    printf("#define %-32.32s %d\n", "LINUX_SYS_swapon", SYS_swapon);
#endif
#ifdef SYS_symlink
    printf("#define %-32.32s %d\n", "LINUX_SYS_symlink", SYS_symlink);
#endif
#ifdef SYS_symlinkat
    printf("#define %-32.32s %d\n", "LINUX_SYS_symlinkat", SYS_symlinkat);
#endif
#ifdef SYS_sync
    printf("#define %-32.32s %d\n", "LINUX_SYS_sync", SYS_sync);
#endif
#ifdef SYS_sync_file_range
    printf("#define %-32.32s %d\n", "LINUX_SYS_sync_file_range", SYS_sync_file_range);
#endif
#ifdef SYS_sync_file_range2
    printf("#define %-32.32s %d\n", "LINUX_SYS_sync_file_range2", SYS_sync_file_range2);
#endif
#ifdef SYS_syncfs
    printf("#define %-32.32s %d\n", "LINUX_SYS_syncfs", SYS_syncfs);
#endif
#ifdef SYS_sys_debug_setcontext
    printf("#define %-32.32s %d\n", "LINUX_SYS_sys_debug_setcontext", SYS_sys_debug_setcontext);
#endif
#ifdef SYS_syscall
    printf("#define %-32.32s %d\n", "LINUX_SYS_syscall", SYS_syscall);
#endif
#ifdef SYS_sysfs
    printf("#define %-32.32s %d\n", "LINUX_SYS_sysfs", SYS_sysfs);
#endif
#ifdef SYS_sysinfo
    printf("#define %-32.32s %d\n", "LINUX_SYS_sysinfo", SYS_sysinfo);
#endif
#ifdef SYS_syslog
    printf("#define %-32.32s %d\n", "LINUX_SYS_syslog", SYS_syslog);
#endif
#ifdef SYS_sysmips
    printf("#define %-32.32s %d\n", "LINUX_SYS_sysmips", SYS_sysmips);
#endif
#ifdef SYS_tee
    printf("#define %-32.32s %d\n", "LINUX_SYS_tee", SYS_tee);
#endif
#ifdef SYS_tgkill
    printf("#define %-32.32s %d\n", "LINUX_SYS_tgkill", SYS_tgkill);
#endif
#ifdef SYS_time
    printf("#define %-32.32s %d\n", "LINUX_SYS_time", SYS_time);
#endif
#ifdef SYS_timer_create
    printf("#define %-32.32s %d\n", "LINUX_SYS_timer_create", SYS_timer_create);
#endif
#ifdef SYS_timer_delete
    printf("#define %-32.32s %d\n", "LINUX_SYS_timer_delete", SYS_timer_delete);
#endif
#ifdef SYS_timer_getoverrun
    printf("#define %-32.32s %d\n", "LINUX_SYS_timer_getoverrun", SYS_timer_getoverrun);
#endif
#ifdef SYS_timer_gettime
    printf("#define %-32.32s %d\n", "LINUX_SYS_timer_gettime", SYS_timer_gettime);
#endif
#ifdef SYS_timer_settime
    printf("#define %-32.32s %d\n", "LINUX_SYS_timer_settime", SYS_timer_settime);
#endif
#ifdef SYS_timerfd_create
    printf("#define %-32.32s %d\n", "LINUX_SYS_timerfd_create", SYS_timerfd_create);
#endif
#ifdef SYS_timerfd_gettime
    printf("#define %-32.32s %d\n", "LINUX_SYS_timerfd_gettime", SYS_timerfd_gettime);
#endif
#ifdef SYS_timerfd_settime
    printf("#define %-32.32s %d\n", "LINUX_SYS_timerfd_settime", SYS_timerfd_settime);
#endif
#ifdef SYS_times
    printf("#define %-32.32s %d\n", "LINUX_SYS_times", SYS_times);
#endif
#ifdef SYS_tkill
    printf("#define %-32.32s %d\n", "LINUX_SYS_tkill", SYS_tkill);
#endif
#ifdef SYS_truncate
    printf("#define %-32.32s %d\n", "LINUX_SYS_truncate", SYS_truncate);
#endif
#ifdef SYS_truncate64
    printf("#define %-32.32s %d\n", "LINUX_SYS_truncate64", SYS_truncate64);
#endif
#ifdef SYS_ugetrlimit
    printf("#define %-32.32s %d\n", "LINUX_SYS_ugetrlimit", SYS_ugetrlimit);
#endif
#ifdef SYS_umask
    printf("#define %-32.32s %d\n", "LINUX_SYS_umask", SYS_umask);
#endif
#ifdef SYS_umount
    printf("#define %-32.32s %d\n", "LINUX_SYS_umount", SYS_umount);
#endif
#ifdef SYS_umount2
    printf("#define %-32.32s %d\n", "LINUX_SYS_umount2", SYS_umount2);
#endif
#ifdef SYS_uname
    printf("#define %-32.32s %d\n", "LINUX_SYS_uname", SYS_uname);
#endif
#ifdef SYS_unlink
    printf("#define %-32.32s %d\n", "LINUX_SYS_unlink", SYS_unlink);
#endif
#ifdef SYS_unlinkat
    printf("#define %-32.32s %d\n", "LINUX_SYS_unlinkat", SYS_unlinkat);
#endif
#ifdef SYS_unshare
    printf("#define %-32.32s %d\n", "LINUX_SYS_unshare", SYS_unshare);
#endif
#ifdef SYS_uselib
    printf("#define %-32.32s %d\n", "LINUX_SYS_uselib", SYS_uselib);
#endif
#ifdef SYS_ustat
    printf("#define %-32.32s %d\n", "LINUX_SYS_ustat", SYS_ustat);
#endif
#ifdef SYS_userfaultfd
    printf("#define %-32.32s %d\n", "LINUX_SYS_userfaultfd", SYS_userfaultfd);
#endif
#ifdef SYS_usr26
    printf("#define %-32.32s %d\n", "LINUX_SYS_usr26", SYS_usr26);
#endif
#ifdef SYS_usr32
    printf("#define %-32.32s %d\n", "LINUX_SYS_usr32", SYS_usr32);
#endif
#ifdef SYS_utime
    printf("#define %-32.32s %d\n", "LINUX_SYS_utime", SYS_utime);
#endif
#ifdef SYS_utimensat
    printf("#define %-32.32s %d\n", "LINUX_SYS_utimensat", SYS_utimensat);
#endif
#ifdef SYS_utimes
    printf("#define %-32.32s %d\n", "LINUX_SYS_utimes", SYS_utimes);
#endif
#ifdef SYS_utrap_install
    printf("#define %-32.32s %d\n", "LINUX_SYS_utrap_install", SYS_utrap_install);
#endif
#ifdef SYS_vfork
    printf("#define %-32.32s %d\n", "LINUX_SYS_vfork", SYS_vfork);
#endif
#ifdef SYS_vhangup
    printf("#define %-32.32s %d\n", "LINUX_SYS_vhangup", SYS_vhangup);
#endif
#ifdef SYS_vm86old
    printf("#define %-32.32s %d\n", "LINUX_SYS_vm86old", SYS_vm86old);
#endif
#ifdef SYS_vm86
    printf("#define %-32.32s %d\n", "LINUX_SYS_vm86", SYS_vm86);
#endif
#ifdef SYS_vmsplice
    printf("#define %-32.32s %d\n", "LINUX_SYS_vmsplice", SYS_vmsplice);
#endif
#ifdef SYS_wait4
    printf("#define %-32.32s %d\n", "LINUX_SYS_wait4", SYS_wait4);
#endif
#ifdef SYS_waitid
    printf("#define %-32.32s %d\n", "LINUX_SYS_waitid", SYS_waitid);
#endif
#ifdef SYS_waitpid
    printf("#define %-32.32s %d\n", "LINUX_SYS_waitpid", SYS_waitpid);
#endif
#ifdef SYS_write
    printf("#define %-32.32s %d\n", "LINUX_SYS_write", SYS_write);
#endif
#ifdef SYS_writev
    printf("#define %-32.32s %d\n", "LINUX_SYS_writev", SYS_writev);
#endif
#ifdef SYS_xtensa
    printf("#define %-32.32s %d\n", "LINUX_SYS_xtensa", SYS_xtensa);
#endif
    return 0;
}
