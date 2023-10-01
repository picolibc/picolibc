#
# SPDX-License-Identifier: BSD-3-Clause
#
# Copyright © 2020 Keith Packard
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

semihost_tests = [
  'semihost-argv',
  'semihost-exit',
  'semihost-exit-extended',
  'semihost-clock',
  'semihost-errno',
  'semihost-flen',
  'semihost-get-cmdline',
  'semihost-gettimeofday',
  'semihost-istty',
  'semihost-open',
  'semihost-read',
  'semihost-remove',
  'semihost-rename',
  'semihost-seek',
  'semihost-system',
  'semihost-time',
  'semihost-times',
  'semihost-writec',
  'semihost-write0',
]

# These tests should pass, but are currently broken under qemu
semihost_broken = [
  'semihost-elapsed',
  'semihost-heapinfo',
  'semihost-readc',
  'semihost-tickfreq',
  'semihost-tmpnam',
  ]

semihost_fail_tests = [
  'semihost-exit-failure',
  'semihost-exit-extended-failure',
  'semihost-system-failure',
]

semihost_no_args_tests = [
  'semihost-no-argv',
]

test_args = ['hello', 'world']

foreach target : targets
  value = get_variable('target_' + target)

  _libs = [get_variable('lib_c' + target)]
  _libs += [get_variable('lib_semihost' + target)]
  if is_variable('crt0_semihost' + target)
    _objs = [get_variable('crt0_semihost'+ target)]
  elif is_variable('crt0_hosted' + target)
    _objs = [get_variable('crt0_hosted'+ target)]
  else
    _objs = []
  endif
  
  _c_args = value[1] + get_variable('test_c_args_' + target, test_c_args)
  _link_args = value[1] + get_variable('test_link_args_' + target, test_link_args)
  _link_depends = get_variable('test_link_depends_' + target, test_link_depends)

  foreach semihost_test : semihost_tests
    semihost_test_src = semihost_test + '.c'
    if target == ''
      semihost_test_name = semihost_test
    else
      semihost_test_name = semihost_test + '_' + target
    endif

    _cmd_arg = ['-DCOMMAND_LINE="' + 'program-name' + ' ' + ' '.join(test_args) + '"']

    test(semihost_test_name,
	 executable(semihost_test_name, [semihost_test_src],
		    c_args: _c_args + _cmd_arg,
		    link_args: double_printf_link_args + _link_args,
		    objects: _objs,
		    link_with: _libs,
		    link_depends:  _link_depends,
		    include_directories: inc),
	 args: test_args,
         depends: bios_bin,
         suite: 'semihost',
	 env: test_env)
  endforeach

  foreach semihost_fail_test : semihost_fail_tests
    semihost_fail_test_src = semihost_fail_test + '.c'
    if target == ''
      semihost_fail_test_name = semihost_fail_test
    else
      semihost_fail_test_name = semihost_fail_test + '_' + target
    endif

    test(semihost_fail_test_name,
	 executable(semihost_fail_test_name, [semihost_fail_test_src],
		    c_args: _c_args + _cmd_arg,
		    link_args: _link_args,
		    objects: _objs,
		    link_with: _libs,
		    link_depends:  _link_depends,
		    include_directories: inc),
	 args: test_args,
         depends: bios_bin,
         suite: 'semihost',
	 env: test_env,
	 should_fail: true)
  endforeach
  
  if has_semihost_args
    foreach semihost_test : semihost_no_args_tests
      semihost_test_src = semihost_test + '.c'
      if target == ''
	semihost_test_name = semihost_test
      else
	semihost_test_name = semihost_test + '_' + target
      endif

      test(semihost_test_name,
	   executable(semihost_test_name, [semihost_test_src],
		      c_args: _c_args + _cmd_arg,
		      link_args: double_printf_link_args + _link_args,
		      objects: _objs,
		      link_with: _libs,
		      link_depends:  test_link_depends,
		      include_directories: inc),
           depends: bios_bin,
           suite: 'semihost',
	   env: test_env)
    endforeach
  endif

endforeach
