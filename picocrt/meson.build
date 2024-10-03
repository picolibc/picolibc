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

src_picocrt = []
src_picocrt_none = files('crt0-none.c')

# Machine-specific crt0 variants to build.
picocrt_machines = [
  {
    # Must match a valid value of the 'test-machine' option, and will cause the
    # MACHINE_<name> preprocessor macro to be defined when compiling crt0.
    'name': 'qemu',
    # Suffix used on file names, QEMU's is empty because it is the default.
    'suffix': '',
  },
]

machine_dir = 'machine' / host_cpu_family
picocrt_march_add=''
if fs.is_dir(machine_dir)
  subdir(machine_dir)
else
  src_picocrt = files('shared/crt0.c')
endif

machine_found = false
foreach machine : picocrt_machines
  if machine['name'] == test_machine
    machine_found = true
  endif
endforeach
if not machine_found
  error(test_machine + ': requested test machine not found')
endif

foreach target : targets
  value = get_variable('target_' + target)

  instdir = join_paths(lib_dir, value[0])

  if picocrt_march_add != ''
    new_cflags = []
    foreach cflag : value[1]
      if cflag.startswith('-march') and not cflag.contains(picocrt_march_add)
	cflag = cflag + picocrt_march_add
      endif
      new_cflags += cflag
    endforeach
    value = [value[0], new_cflags]
  endif

  foreach machine : picocrt_machines
    suffix = machine['suffix']
    if target == ''
      crt_name = 'crt0' + suffix + '.o'
      crt_hosted_name = 'crt0-hosted' + suffix + '.o'
      crt_minimal_name = 'crt0-minimal' + suffix + '.o'
      crt_semihost_name = 'crt0-semihost' + suffix + '.o'
      crt_none_name = 'crt0-none' + suffix + '.o'
      libcrt_name = 'crt0' + suffix
      libcrt_hosted_name = 'crt0-hosted' + suffix
      libcrt_minimal_name = 'crt0-minimal' + suffix
      libcrt_semihost_name = 'crt0-semihost' + suffix
      libcrt_none_name = 'crt0-none' + suffix
    else
      crt_name = join_paths(target, 'crt0' + suffix + '.o')
      crt_hosted_name = join_paths(target, 'crt0-hosted' + suffix + '.o')
      crt_minimal_name = join_paths(target, 'crt0-minimal' + suffix + '.o')
      crt_semihost_name = join_paths(target, 'crt0-semihost' + suffix + '.o')
      crt_none_name = join_paths(target, 'crt0-none' + suffix + '.o')
      libcrt_name = join_paths(target, 'libcrt0' + suffix)
      libcrt_hosted_name = join_paths(target, 'libcrt0-hosted' + suffix)
      libcrt_minimal_name = join_paths(target, 'libcrt0-minimal' + suffix)
      libcrt_semihost_name = join_paths(target, 'libcrt0-semihost' + suffix)
      libcrt_none_name = join_paths(target, 'libcrt0-none' + suffix)
    endif

    crt0_name = 'crt0' + target
    crt0_hosted_name = 'crt0_hosted' + target
    crt0_minimal_name = 'crt0_minimal' + target
    crt0_semihost_name = 'crt0_semihost' + target
    crt0_none_name = 'crt0_none' + target

    _c_args = value[1] + arg_fnobuiltin + ['-ffreestanding', '-DMACHINE_' + machine['name']]
    _link_args = value[1] + ['-r', '-ffreestanding']

    # The normal variant does not call 'exit' after return from main (c lingo: freestanding execution environment)
    _crt = executable(crt_name,
                      src_picocrt,
                      include_directories : inc,
                      install : true,
                      install_dir : instdir,
                      c_args : _c_args,
                      link_args : _link_args)

    if machine['name'] == test_machine
      set_variable(crt0_name,
                   _crt.extract_objects(src_picocrt)
                  )
    endif

    if enable_picocrt_lib
      static_library(libcrt_name,
                     [],
                     include_directories : inc,
                     install : true,
                     install_dir : instdir,
                     c_args : _c_args,
                     objects: _crt.extract_objects(src_picocrt),
                     pic: false)
    endif

    # The 'hosted' variant calls 'exit' after return from main (c lingo: hosted execution environment)
    _crt = executable(crt_hosted_name,
                      src_picocrt,
                      include_directories : inc,
                      install : true,
                      install_dir : instdir,
                      c_args : _c_args + ['-DCRT0_EXIT'],
                      link_args : _link_args)

    if machine['name'] == test_machine
    set_variable(crt0_hosted_name,
                 _crt.extract_objects(src_picocrt)
                )
    endif

    if enable_picocrt_lib
      static_library(libcrt_hosted_name,
                     [],
                     include_directories : inc,
                     install : true,
                     install_dir : instdir,
                     pic: false,
                     objects: _crt.extract_objects(src_picocrt),
                     c_args : value[1] + ['-DCRT0_EXIT'])
    endif

    # The 'minimal' variant doesn't call exit, nor does it invoke any constructors
    _crt = executable(crt_minimal_name,
                      src_picocrt,
                      include_directories : inc,
                      install : true,
                      install_dir : instdir,
                      c_args : _c_args + ['-DCONSTRUCTORS=0'],
                      link_args : _link_args)

    if machine['name'] == test_machine
    set_variable(crt0_minimal_name,
                 _crt.extract_objects(src_picocrt)
                )
    endif

    if enable_picocrt_lib
      static_library(libcrt_minimal_name,
                     [],
                     include_directories : inc,
                     install : true,
                     install_dir : instdir,
                     pic: false,
                     objects: _crt.extract_objects(src_picocrt),
                     c_args : _c_args + ['-DCONSTRUCTORS=0'])
    endif

    if has_arm_semihost
      # The 'semihost' variant calls sys_semihost_get_cmdline to build argv
      # and calls exit when main returns
      _crt = executable(crt_semihost_name,
                        src_picocrt,
                        include_directories : inc,
                        install : true,
                        install_dir : instdir,
                        c_args : _c_args + ['-DCRT0_EXIT', '-DCRT0_SEMIHOST'],
                        link_args : _link_args)

      if machine['name'] == test_machine
        set_variable(crt0_semihost_name,
                     _crt.extract_objects(src_picocrt)
                    )
      endif

      if enable_picocrt_lib
        static_library(libcrt_semihost_name,
                       [],
                       include_directories : inc,
                       install : true,
                       install_dir : instdir,
                       pic: false,
                       objects: _crt.extract_objects(src_picocrt),
                       c_args : value[1] + ['-DCRT0_EXIT', '-DCRT0_SEMIHOST'])
      endif
    endif

    # The 'none' variant is completely empty
    _crt = executable(crt_none_name,
                      src_picocrt_none,
                      include_directories : inc,
                      install : true,
                      install_dir : instdir,
                      c_args : _c_args,
                      link_args : _link_args)

    if machine['name'] == test_machine
    set_variable(crt0_none_name,
                 _crt.extract_objects(src_picocrt_none)
                )
    endif

  endforeach
  
endforeach
