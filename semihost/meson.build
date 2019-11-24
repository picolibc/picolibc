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
			'riscv' : 'riscv/semihost-riscv.c',
			'riscv64' : 'riscv/semihost-riscv.c',
			'arm' : 'arm/semihost-arm.c'
		      }

if src_semihost_machine.has_key(host_cpu)
  srcs_semihost = [
    'errno.c',
    'putc.c',
    'getc.c',
    'exit.c',
    'iob.c',
    'get_cmdline.c',
    'open.c',
    'close.c',
    'lseek.c',
    'write.c',
    'read.c',
    'clock.c',
    join_paths('machine', src_semihost_machine[host_cpu])
  ]

  install_headers(
    'semihost.h'
  )

  semihost_inc = [include_directories('.'), inc]

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
				include_directories : semihost_inc,
				c_args : value[1]))
    
  endforeach
endif
