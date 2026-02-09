#
# SPDX-License-Identifier: BSD-3-Clause
#
# Copyright © 2025 Keith Packard
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions
# are met:
#
# 1. Redistributions of source code must retain the above copyright
#    notice, this list of conditions and the following disclaimer.
#
# 2. Redistributions in binary form must reproduce the above
#    copyright notice, this list of conditions and the following
#    disclaimer in the documentation and/or other materials provided
#    with the distribution.
#
# 3. Neither the name of the copyright holder nor the names of its
#    contributors may be used to endorse or promote products derived
#    from this software without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
# "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
# LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
# FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
# COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
# INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
# (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
# SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
# HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
# STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
# ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
# OF THE POSSIBILITY OF SUCH DAMAGE.
#

machine_dir = 'machine' / host_cpu_family

if fs.is_dir(machine_dir)

  has_os_linux = true

  has_os_library = true

  has_os_posix = true

  has_os_fileio = true

  has_os_process = true

  has_os_id = true

  srcs_os_linux = [
    'access.c',
    'alarm.c',
    'cfgetispeed.c',
    'cfgetospeed.c',
    'chdir.c',
    'chmod.c',
    'clock_gettime.c',
    'close.c',
    'closedir.c',
    'dup.c',
    'dup2.c',
    'execve.c',
    '_exit.c',
    'fcntl.c',
    'fork.c',
    'fpathconf.c',
    'fstat.c',
    'getcwd.c',
    'getegid.c',
    'getentropy.c',
    'geteuid.c',
    'getgid.c',
    'getgroups.c',
    'getpgrp.c',
    'getpid.c',
    'gettimeofday.c',
    'getuid.c',
    'getwd.c',
    'iob_stderr.c',
    'iob_stdin.c',
    'iob_stdout.c',
    'isatty.c',
    'kill.c',
    'killpg.c',
    'lseek.c',
    'mkdir.c',
    'nanosleep.c',
    'open.c',
    'opendir.c',
    'pathconf.c',
    'pipe.c',
    'poll.c',
    'raise.c',
    'read.c',
    'readdir.c',
    'readlink.c',
    'rename.c',
    'rmdir.c',
    'sbrk.c',
    'setegid.c',
    'setgid.c',
    'setgroups.c',
    'setitimer.c',
    'seteuid.c',
    'setuid.c',
    'signal.c',
    '_signal_from_linux.c',
    '_signal_to_linux.c',
    'sigprocmask.c',
    'sigsuspend.c',
    'stat.c',
    '_statbuf.c',
    '_syscall_error.c',
    'tcflush.c',
    'tcgetattr.c',
    'tcgetpgrp.c',
    'tcsetattr.c',
    'times.c',
    'ttyname.c',
    'ttyname_r.c',
    'umask.c',
    'unlink.c',
    'wait3.c',
    'waitpid.c',
    'write.c',
  ]

  subdir('include')

  subdir(machine_dir)

  src_os_linux = files(srcs_os_linux) + src_os_linux_machine

  linux_inc = include_directories(machine_dir, 'include')

  foreach params : targets
    target = params['name']
    target_dir = params['dir']
    target_c_args = params['c_args']

    instdir = join_paths(lib_dir, target_dir)

    liblinux_name = 'linux'

    if meson.version().version_compare('>=1.10')
      _lib = static_library(liblinux_name,
			    src_os_linux,
                            build_subdir : target_dir,
			    install : really_install,
			    install_dir : instdir,
			    include_directories : [linux_inc, inc],
			    c_args : target_c_args + c_args)
    else
      target_lib_prefix = params['lib_prefix']
      _lib = static_library(join_paths(target_dir, target_lib_prefix + liblinux_name),
			    src_os_linux,
			    install : really_install,
			    install_dir : instdir,
			    include_directories : [linux_inc, inc],
			    c_args : target_c_args + c_args)
    endif

    set_variable('lib_linux' + target, _lib)
    
  endforeach

endif
