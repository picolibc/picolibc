#
# SPDX-License-Identifier: BSD-3-Clause
#
# Copyright © 2021 Keith Packard
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
src_semihost += files('semihost-aarch64.S')
has_arm_semihost = true

lib_semihost_raw_srcs = [
  'aarch64_stub.c',
  'aarch64_exit.c',
  'aarch64_iob.c',
  ]

foreach params : targets
  target = params['name']
  target_dir = params['dir']
  target_c_args = params['c_args']

  instdir = join_paths(lib_dir, target_dir)

  libsemihost_raw_name = 'semihost_raw'

  if meson.version().version_compare('>=1.10')
    _lib = static_library(libsemihost_raw_name,
			  lib_semihost_raw_srcs,
                          build_subdir : target_dir,
			  install : really_install,
			  install_dir : instdir,
			  include_directories : inc,
			  c_args : target_c_args + c_args,
			  pic: false)
  else
    target_lib_prefix = params['lib_prefix']
    _lib = static_library(join_paths(target_dir, target_lib_prefix + libsemihost_raw_name),
			  lib_semihost_raw_srcs,
			  install : really_install,
			  install_dir : instdir,
			  include_directories : inc,
			  c_args : target_c_args + c_args,
			  pic: false)
  endif

  set_variable('lib_semihost_raw' + target, _lib)

endforeach
