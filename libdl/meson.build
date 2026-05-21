#
# Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
# SPDX-License-Identifier: BSD-3-Clause-Clear
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted (subject to the limitations in the
# disclaimer below) provided that the following conditions are met:
#
#   * Redistributions of source code must retain the above copyright
#     notice, this list of conditions and the following disclaimer.
#
#   * Redistributions in binary form must reproduce the above
#     copyright notice, this list of conditions and the following
#     disclaimer in the documentation and/or other materials provided
#     with the distribution.
#
#   * Neither the name of Qualcomm Technologies, Inc. nor the names of its
#     contributors may be used to endorse or promote products derived
#     from this software without specific prior written permission.
#
# NO EXPRESS OR IMPLIED LICENSES TO ANY PARTY'S PATENT RIGHTS ARE
# GRANTED BY THIS LICENSE. THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT
# HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED
# WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
# MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
# IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
# ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
# DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
# GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
# INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER
# IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
# OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN
# IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
#

libdl_machine_dir = 'machine' / host_cpu_family

if not fs.is_dir(libdl_machine_dir)
  error('libdl enabled (-Dlibdl=true) but no machine port exists for "' + host_cpu_family + '" (expected libdl/' + libdl_machine_dir + ')')
endif

libdl_srcs = files(
  'dl.c',
  libdl_machine_dir / 'dl_relocate.c',
  libdl_machine_dir / 'dl_syms.c',
)

foreach params : targets
  target      = params['name']
  target_dir  = params['dir']
  target_c_args = params['c_args']

  instdir = join_paths(lib_dir, target_dir)

  if meson.version().version_compare('>=1.10')
    _lib = static_library('dl' + target,
                          libdl_srcs,
                          build_subdir : target_dir,
                          install : really_install,
                          install_dir : instdir,
                          include_directories : inc,
                          c_args : target_c_args + c_args)
  else
    target_lib_prefix = params['lib_prefix']
    _lib = static_library(
              join_paths(target_dir, target_lib_prefix + 'dl'),
              libdl_srcs,
              install : really_install,
              install_dir : instdir,
              include_directories : inc,
              c_args : target_c_args + c_args)
  endif

  set_variable('lib_dl' + target, _lib)

endforeach
