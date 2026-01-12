#
# SPDX-License-Identifier: BSD-3-Clause
#
# Copyright © 2025 Keith Packard
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

tests_iconv = [
  'test-iconv',
  ]

foreach params : targets
  target = params['name']
  target_dir = params['dir']
  target_c_args = params['c_args']

  _libs = [get_variable('lib_c' + target)]
  if is_variable('lib_semihost' + target)
    _libs += [get_variable('lib_semihost' + target)]
  endif

  _lib_files=[]
  foreach _lib : _libs
    _lib_files += _lib.full_path()
  endforeach

  if is_variable(crt0_test + target)
    _objs = [get_variable(crt0_test + target)]
  else
    _objs = []
  endif

  _c_args = target_c_args + get_variable('test_c_args' + target, test_c_args)
  _link_args = target_c_args + _lib_files + get_variable('test_link_args' + target, test_link_args)
  _link_depends = get_variable('test_link_depends' + target, test_link_depends) + _libs
  foreach t1 : tests_iconv
    t1_src = t1 + '.c'
    test_file_name_arg=['-DTEST_FILE_NAME="' + t1 + target + '.txt"']

    test(t1 + target,
	 executable(t1 + target, [t1_src],
		    c_args: printf_compile_args_d + test_file_name_arg + _c_args,
		    link_args: printf_link_args_d + _link_args,
		    objects: _objs,
		    link_depends:  _link_depends,
		    include_directories: inc),
         depends: bios_bin,
	 timeout: 600,
         suite: 'test-iconv',
	 env: test_env)
  endforeach

endforeach

if enable_native_tests

  localedef = find_program('localedef', required: true)

  ctype_c_args = []

  locales=[]
  if mb_capable
    locale = 'fr_FR.UTF-8'
    locales += [custom_target(locale, output: locale,
                              command: [localedef, '--no-warnings=ascii',
                                        '-f', 'UTF-8', '-i', 'fr_FR', '.' / '@OUTPUT@'])]
    locale = 'C.UTF-8'
    locales += [custom_target(locale, output: locale,
                              command: [localedef, '--no-warnings=ascii',
                                        '-f', 'UTF-8', '-i', 'C', '.' / '@OUTPUT@'])]
    ctype_c_args += ['-DHAVE_UTF_CHARSETS']
  endif
  if mb_extended_charsets
    # CP720 doesn't have any Linux support.
    # JIS doesn't make any sense to me.
    # CP1255 and CP1258 seem to be broken in Linux
    foreach ctype : ['ISO-8859-1', 'ISO-8859-2', 'ISO-8859-3', 'ISO-8859-4',
                     'ISO-8859-5', 'ISO-8859-6', 'ISO-8859-7', 'ISO-8859-8',
                     'ISO-8859-9', 'ISO-8859-10', 'ISO-8859-11', 'ISO-8859-13',
                     'ISO-8859-14', 'ISO-8859-15', 'ISO-8859-16',

                     'GEORGIAN-PS', 'PT154', 'KOI8-T', 'CP437', 'CP737', 'CP775',
                     'CP850', 'CP852', 'CP855', 'CP857', 'CP858', 'CP862', 'CP866',
                     'CP874', 'CP1125', 'CP1250', 'CP1251', 'CP1252', 'CP1253',
                     'CP1254', 'CP1256', 'CP1257', 'KOI8-R', 'KOI8-U',

                     'SHIFT-JIS', 'EUC-JP']
      locale = 'C.' + ctype
      if ctype == 'SHIFT-JIS'
        ctype = 'SHIFT_JIS'
      elif ctype == 'CP437'
        ctype = 'IBM437'
      elif ctype == 'CP850'
        ctype = 'IBM850'
      elif ctype == 'CP852'
        ctype = 'IBM852'
      elif ctype == 'CP855'
        ctype = 'IBM855'
      elif ctype == 'CP857'
        ctype = 'IBM857'
      elif ctype == 'CP858'
        ctype = 'IBM858'
      elif ctype == 'CP862'
        ctype = 'IBM862'
      elif ctype == 'CP866'
        ctype = 'IBM866'
      elif ctype == 'CP874'
        ctype = 'IBM874'
      endif
      locales += [custom_target(locale, output: locale,
                                command: [localedef, '--no-warnings=ascii',
                                          '-f', ctype, '-i', 'C', '.' / '@OUTPUT@'])]
    endforeach
    ctype_c_args += ['-DHAVE_ISO_CHARSETS',
                     '-DHAVE_WINDOWS_CHARSETS',
                     '-DHAVE_JIS_CHARSETS'
                    ]
  endif

  foreach t1 : tests_iconv
    t1_src = t1 + '.c'
    t1_name = t1 + '-native'
    test_file_name_arg=['-DTEST_FILE_NAME="' + t1_name + '.txt"']

    test(t1_name,
	 executable(t1_name, t1_src,
		    c_args: native_c_args + test_file_name_arg + ctype_c_args,
		    link_args: native_c_args),
         depends: locales,
         suite: 'test-iconv',
         env: ['LOCPATH=' + meson.current_build_dir()],
        )
  endforeach

endif
