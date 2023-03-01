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

tests = [ 'iconvnm' ]

tests_ccs = ['iconvjp', 'iconvru']

# Add CCS tests if the data are internal or we're not using semihosting
# or we're using arm semihosting

if not newlib_iconv_external_ccs or has_arm_semihost or not has_semihost
  tests += tests_ccs
endif

iconv_test_c_args = ['-DTEST_NLSPATH="' + newlib_iconv_ccs_dir + '"']

foreach target : targets
  value = get_variable('target_' + target)

  _libs = [get_variable('lib_c' + target)]
  if is_variable('lib_semihost' + target)
    _libs += [get_variable('lib_semihost' + target)]
  endif
  if is_variable('crt0_hosted' + target)
    _objs = [get_variable('crt0_hosted'+ target)]
  else
    _objs = []
  endif
  
  _c_args = value[1] + test_c_args
  _link_args = value[1] + test_link_args

  foreach test : tests
    if target == ''
      test_name = test
    else
      test_name = test + '_' + target
    endif

    src = test + '.c'

    test(test_name,
	 executable(test_name, src,
		    c_args: ['-D_XOPEN_SOURCE=700', '-D_GNU_SOURCE'] + _c_args + iconv_test_c_args,
		    link_args: _link_args,
		    objects: _objs,
		    link_with: _libs,
		    include_directories: test_inc),
	 depends: [bios_bin],
	 env: test_env)
  endforeach
endforeach
