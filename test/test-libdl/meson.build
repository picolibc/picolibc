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
# libdl tests: build a small shared object (testmod.so) from source and
# load it at runtime via libdl (dlopen/dlsym/dlclose).
#
# Gated strictly on the 'libdl' option.  For each target the test is only
# emitted when the libdl static library, the OS test library and the test
# crt0 all exist for that target (otherwise there is nothing to link the
# loader against); this keeps configuration from hard-failing on targets
# that have no libdl machine port.
#

# libdl tests that load testmod.so
libdl_tests = [
  'test-libdl-load',
]

# libdl tests that need no shared object (error / defensive paths)
libdl_error_tests = [
  'test-libdl-error',
]

if enable_libdl

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
      get_variable('lib_c' + target),
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

    _objs       = [get_variable(crt0_test + target)]
    _link_args  = target_c_args + _lib_files + get_variable('test_link_args' + target, test_link_args)
    _link_deps  = get_variable('test_link_depends' + target, test_link_depends) + _libs
    _c_args     = target_c_args + get_variable('test_c_args' + target, test_c_args)

    # Build testmod.so as a freestanding ET_DYN.  -fPIC -shared produces
    # the RELA relocations libdl's relocator understands; -nostdlib keeps
    # it dependency-free; --hash-style=sysv is required because libdl's
    # dlsym()/relocation symbol lookup uses the SysV .hash table (DT_HASH)
    # exclusively (GNU-only hash would break resolution).
    testmod_so = custom_target(
      'testmod' + target + '.so',
      input   : 'testmod.c',
      output  : 'testmod' + target + '.so',
      command : cc.cmd_array() + target_c_args + [
        '-fPIC', '-shared', '-nostdlib',
        '-Wl,--hash-style=sysv',
        '-o', '@OUTPUT@', '@INPUT@',
      ],
    )

    # Embed the absolute path to testmod.so so the test binary is
    # self-contained (no --args needed at run time).
    _testmod_def = '-DTESTMOD_SO_PATH="' + testmod_so.full_path() + '"'

    foreach t : libdl_tests
      test(t + target,
           executable(t + target,
                      t + '.c',
                      objects             : _objs,
                      c_args              : _c_args + [_testmod_def],
                      link_args           : _link_args,
                      link_depends        : _link_deps,
                      include_directories : inc),
           depends : [testmod_so] + bios_bin,
           suite   : 'libdl',
           timeout : 60,
           env     : test_env)
    endforeach

    foreach t : libdl_error_tests
      test(t + target,
           executable(t + target,
                      t + '.c',
                      objects             : _objs,
                      c_args              : _c_args,
                      link_args           : _link_args,
                      link_depends        : _link_deps,
                      include_directories : inc),
           depends : bios_bin,
           suite   : 'libdl',
           timeout : 60,
           env     : test_env)
    endforeach

  endforeach

endif
