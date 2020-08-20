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
srcs_machine_tls = {
			'riscv' : ['machine/riscv/tls.c'],
			'riscv32' : ['machine/riscv/tls.c'],
			'riscv64' : ['machine/riscv/tls.c'],
			'arm' : ['machine/arm/tls.c', 'machine/arm/read_tp.S'],
			'aarch64' : [ 'machine/aarch64/tls.c' ]
		      }

srcs_machine = {
			'arm' : ['machine/arm/interrupt.c']
		      }

srcs_tls = [
  'inittls.c'
]

srcs_picolib = [
  'picosbrk.c',
  'dso_handle.c',
  'getauxval.c'
]

if srcs_machine.has_key(host_cpu_family)
  srcs_picolib += srcs_machine[host_cpu_family]
endif

if thread_local_storage
  srcs_picolib += srcs_tls
  if srcs_machine_tls.has_key(host_cpu_family)
    has_picolibc_tls_api = true
    srcs_picolib += srcs_machine_tls[host_cpu_family]
  endif
endif

foreach target : targets
	value = get_variable('target_' + target)
	set_variable('lib_picolib' + target,
		     static_library('picolib' + target,
				    srcs_picolib,
				    pic: false,
				    include_directories: inc,
				    c_args: value[1]))
endforeach
