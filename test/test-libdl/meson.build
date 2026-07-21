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
# Individual libdl tests — one executable per test, matching the style
# of other picolibc test suites.
#
# Only built when:
#   - shared-library=true  (libc.so exists)
#   - host_cpu_family == 'hexagon'  (arch has dl_relocate + dl_syms)
#   - has_semihost  (semihosting I/O needed to read libc.so at runtime)
#   - lib_dl<target> was built by libdl/meson.build
#

libdl_tests = [
  'test-libdl-dlerror',
  'test-libdl-open-error',
  'test-libdl-null-args',
  'test-libdl-strlen',
  'test-libdl-malloc',
  'test-libdl-sprintf',
  'test-libdl-memcpy',
  'test-libdl-memset',
  'test-libdl-strcmp',
  'test-libdl-atoi',
  'test-libdl-dlsym-unknown',
  'test-libdl-open-twice',
]

if build_shared and host_cpu_family == 'hexagon' and has_semihost

  foreach params : targets
    target        = params['name']
    target_dir    = params['dir']
    target_c_args = params['c_args']

    if not is_variable('lib_dl' + target)
      continue
    endif
    if not is_variable(oslib_test + target)
      continue
    endif
    if not is_variable(crt0_test + target)
      continue
    endif

    _libs = [
      get_variable('lib_c_link' + target),
      get_variable(oslib_test + target),
      get_variable('lib_dl' + target),
    ]
    if is_variable(libfallback_var + target)
      _libs += [get_variable(libfallback_var + target)]
    endif

    _lib_files = []
    foreach _lib : _libs
      _lib_files += _lib.full_path()
    endforeach

    _objs      = [get_variable(crt0_test + target)]
    _link_args = target_c_args + _lib_files + get_variable('test_link_args' + target, test_link_args)
    _link_deps = get_variable('test_link_depends' + target, test_link_depends) + _libs

    # Embed the absolute path to libc.so at compile time so each test
    # binary is self-contained (no --args needed at run time).
    _libc_so     = get_variable('lib_c' + target).get_shared_lib()
    _libc_so_def = '-DLIBC_SO_PATH="' + _libc_so.full_path() + '"'

    _c_args = target_c_args + get_variable('test_c_args' + target, test_c_args) + [_libc_so_def]

    foreach t : libdl_tests
      _exe = executable(t + target,
                        t + '.c',
                        objects             : _objs,
                        link_with           : [],
                        c_args              : _c_args,
                        link_args           : _link_args,
                        link_depends        : _link_deps,
                        include_directories : inc)

      test(t + target,
           _exe,
           depends : [_libc_so],
           suite   : 'libdl',
           env     : test_env)
    endforeach

  endforeach

endif
