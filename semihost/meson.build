#
# SPDX-License-Identifier: BSD-3-Clause
#
# Copyright © 2019 Keith Packard
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
src_semihost_machine = {
			'riscv' : 'riscv/semihost-riscv.s',
			'riscv32' : 'riscv/semihost-riscv.s',
			'riscv64' : 'riscv/semihost-riscv.s',
			'arm' : 'arm/semihost-arm.S',
			'aarch64' : 'aarch64/semihost-aarch64.S'
		      }

if src_semihost_machine.has_key(host_cpu_family)
  srcs_semihost = [
    'close.c',
    'exit.c',
    'fstat.c',
    'isatty.c',
    'kill.c',
    'lseek.c',
    'lseek64.c',
    'open.c',
    'read.c',
    'unlink.c',
    'write.c',
    'sys_clock.c',
    'sys_close.c',
    'sys_elapsed.c',
    'sys_errno.c',
    'sys_exit.c',
    'sys_exit_extended.c',
    'sys_feature.c',
    'sys_flen.c',
    'sys_get_cmdline.c',
    'sys_getc.c',
    'sys_heapinfo.c',
    'sys_iserror.c',
    'sys_istty.c',
    'sys_open.c',
    'sys_putc.c',
    'sys_read.c',
    'sys_remove.c',
    'sys_rename.c',
    'sys_seek.c',
    'sys_system.c',
    'sys_tickfreq.c',
    'sys_time.c',
    'sys_tmpnam.c',
    'sys_write.c',
    'sys_write0.c',
    join_paths('machine', src_semihost_machine[host_cpu_family])
  ]

  if tinystdio
    srcs_semihost += 'iob.c'
  else
    srcs_semihost += 'mapstdio.c'
  endif

  install_headers(
    'semihost.h'
  )

  inc = [inc, include_directories('.')]

  has_semihost = true

  has_arm_semihost = true

  foreach target : targets
    value = get_variable('target_' + target)

    instdir = join_paths(get_option('libdir'), value[0])

    if target == ''
      libsemihost_name = 'semihost'
    else
      libsemihost_name = join_paths(target, 'libsemihost')
    endif

    set_variable('lib_semihost' + target,
		 static_library(libsemihost_name,
				srcs_semihost,
				install : true,
				install_dir : instdir,
				include_directories : inc,
				c_args : value[1]))
    
  endforeach
endif

#
# Systems with non-arm semihosting support
#
semihost_custom = {
		    'x86' : 'i386',
		    'x86_64' : 'i386',
		  }

message('host_cpu_family ' + host_cpu_family)

if semihost_custom.has_key(host_cpu_family)
  subdir(join_paths('machine', semihost_custom[host_cpu_family]))
endif
