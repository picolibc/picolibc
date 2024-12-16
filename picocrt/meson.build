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

# crt0 variants
picocrt_variants = [
  {
    'name': 'default',
    'suffix': '',
    'c_args': [],
    'src': src_picocrt,
  },
  {
    'name': 'hosted',
    'suffix': '-hosted',
    'c_args': ['-DCRT0_EXIT'],
    'src': src_picocrt,
  },
  {
    'name': 'minimal',
    'suffix': '-minimal',
    'c_args': ['-DCONSTRUCTORS=0'],
    'src': src_picocrt,
  },
  {
    'name': 'none',
    'suffix': '-none',
    'c_args': [],
    'src': src_picocrt_none,
  },
]

if has_semihost
  picocrt_variants += {
                       'name': 'semihost',
                       'suffix': '-semihost',
                       'c_args':  ['-DCRT0_EXIT', '-DCRT0_SEMIHOST'],
                       'src': src_picocrt,
                     }
  crt0_test = 'crt0_semihost'
endif

foreach params : targets
  target = params['name']
  target_dir = params['dir']
  target_c_args = params['c_args']
  target_lib_prefix = params['lib_prefix']

  instdir = join_paths(lib_dir, target_dir)

  foreach machine : picocrt_machines
    machine_suffix = machine['suffix']

    foreach variant : picocrt_variants
      variant_suffix = variant['suffix']
      libcrt_name = 'crt0' + variant_suffix + machine_suffix
      crt_name = libcrt_name + '.o'

      _src = variant['src']
      _c_args = target_c_args + arg_fnobuiltin + ['-ffreestanding', '-DMACHINE_' + machine['name']] + variant['c_args']
      _link_args = target_c_args + ['-r', '-ffreestanding']
      
      # The normal variant does not call 'exit' after return from main (c lingo: freestanding execution environment)
      _crt = executable(join_paths(target_dir, crt_name),
                        _src,
                        include_directories : inc,
                        install : true,
                        install_dir : instdir,
                        c_args : _c_args,
                        link_args : _link_args)

      if machine['name'] == test_machine
        set_variable('crt0' + variant_suffix.underscorify() + target,
                     _crt.extract_objects(_src)
                    )
      endif

      if enable_picocrt_lib
        static_library(join_paths(target_dir, target_lib_prefix + libcrt_name),
                       [],
                       include_directories : inc,
                       install : true,
                       install_dir : instdir,
                       c_args : _c_args,
                       objects: _crt.extract_objects(_src),
                       pic: false)
      endif
    endforeach
  endforeach
endforeach
