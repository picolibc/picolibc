#
# SPDX-License-Identifier: BSD-3-Clause
#
# Copyright © 2019-2021 Keith Packard
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
project('picolibc', 'c',
	default_options: [
	  'buildtype=minsize',
	  'debug=true',
	  'c_std=c18',
	  'b_staticpic=false',
          'warning_level=2',
	],
	license : 'BSD',
	meson_version : '>= 0.61',
	version: '1.8.11'
       )

# When picolibc is embedded as a subproject you probably don't expect
# installing of the outter project cause picolibc to get installed.
# However, the user can force install even if picolibc is a subproject
# via the force-install option.
really_install = get_option('force-install') or not meson.is_subproject()

fs = import('fs')

cc = meson.get_compiler('c')

# Find the compiler installation directory by parsing the output of
# cc -print-search-dirs

cc_install_dir = ''
foreach _line : run_command(cc.cmd_array() + ['-print-search-dirs'], check : false).stdout().split('\n')
  if _line.startswith('install: ')
    # trim off the leading 'install: '
    cc_install_dir = _line.substring(9)
  endif
endforeach

# Find the picolibc name for the host cpu. Provide
# for some common aliases

cpu_family_aliases = {
		       # aarch64
		       'arm64' : 'aarch64',
		       # cris
		       'crisv32' : 'cris',
           # loongarch
		       'loongarch64' : 'loongarch',
		       'loongarch32' : 'loongarch',
		       # m68hc11
		       'm6811' : 'm68hc11',
		       'm6812' : 'm68hc11',
		       'm68hc12' : 'm68hc11',
		       # m68k
		       'fido' : 'm68k',
		       # m88k
		       'm88110' : 'm88k',
		       # Microblaze
		       'microblazeel' : 'microblaze',
		       # mips
		       'mips64' : 'mips',
		       # or1k
		       'or1knd': 'or1k',
		       # powerpc
		       'powerpc64' : 'powerpc',
		       'ppc64' : 'powerpc',
		       'ppc64le' : 'powerpc',
		       # riscv
		       'riscv32' : 'riscv',
		       'riscv64' : 'riscv',
		       # sparc
		       'sparc64' : 'sparc',
		       # x86
		       'amd64' : 'x86',
		       'i486' : 'x86',
		       'i586' : 'x86',
		       'i686' : 'x86',
		       'x86_64' : 'x86',
		     }

# Check for an alias, default to provided name
host_cpu_family = cpu_family_aliases.get(host_machine.cpu_family(), host_machine.cpu_family())

# Make sure we have meson build support for the machine-specific files
if not fs.is_file('libc/machine' / host_cpu_family / 'meson.build')
  message = '''

Unsupported architecture: "@0@"

    Read the Supported Architectures section in README.md
    to learn how to add a new architecture.
'''
  error(message.format(host_cpu_family))
endif

enable_multilib = get_option('multilib')
multilib_list = get_option('multilib-list')
multilib_exclude = get_option('multilib-exclude')
enable_picolib = get_option('picolib')
enable_picocrt = get_option('picocrt')
enable_picocrt_lib = get_option('picocrt-lib')
picocrt_enable_mmu = get_option('picocrt-enable-mmu')
test_machine = get_option('test-machine')
enable_semihost = get_option('semihost')
enable_tests = get_option('tests')
if get_option('tests-cdefs') == 'auto'
  enable_cdefs_tests = enable_tests
else
  enable_cdefs_tests = get_option('tests-cdefs') == 'true'
endif
enable_native_tests = get_option('native-tests')
enable_native_math_tests = enable_native_tests and get_option('native-math-tests')
exhaustive_math_tests_value = get_option('exhaustive-math-tests')
if exhaustive_math_tests_value == 'native'
  enable_exhaustive_math_tests = enable_native_tests
else
  enable_exhaustive_math_tests = exhaustive_math_tests_value == 'true'
endif
tests_enable_stack_protector = get_option('tests-enable-stack-protector')
tests_enable_full_malloc_stress = get_option('tests-enable-full-malloc-stress')
tests_enable_posix_io = get_option('tests-enable-posix-io')

# C++ is only used in tests, so only check for it when tests are enabled
if enable_tests
  have_cplusplus = add_languages('cpp', required: false, native: false)
  if have_cplusplus
    cpp = meson.get_compiler('cpp')
  endif
else
  have_cplusplus = false
endif

enable_malloc = get_option('enable-malloc')
malloc_clear_freed = get_option('malloc-clear-freed')
internal_heap = get_option('internal-heap')

# Figure out what flags the linker supports

has_link_defsym = false
has_link_alias = false

if cc.symbols_have_underscore_prefix()
  global_prefix = '_'
else
  global_prefix = ''
endif

# Figure out how to pass aliases on the compiler command line.  Do
# that by finding a suitable alias for the default entry point.

foreach start : ['start', '_start', '__start', '___start']
  _defsym = '-Wl,--defsym=@0@=@1@main'.format(start, global_prefix)
  if cc.has_link_argument(_defsym)
    has_link_defsym = true
    break
  endif
  _alias = '-Wl,-alias,@1@main,@0@'.format(start, global_prefix)
  if cc.has_link_argument(_alias)
    has_link_alias = true
    break
  endif
endforeach

has_link_defsym = meson.get_external_property('has_link_defsym', has_link_defsym)
has_link_alias = meson.get_external_property('has_link_alias', has_link_alias)

# If an empty linker command line doesn't work, then pass the alias
# argument computed above when performing compiler tests involving
# linking

link_test_args = []

if not cc.has_multi_link_arguments([])
  if has_link_defsym
    link_test_args += [_defsym]
  elif has_link_alias
    link_test_args += [_alias]
  endif
endif

# Find the compiler support library

runtime_lib = [meson.get_external_property('librt', meson.get_external_property('libgcc', false))]

if runtime_lib == [false]
  runtime_lib = []
  foreach lrt : ['-lgcc', '-lclang_rt.builtins']
    if cc.has_multi_link_arguments(link_test_args + [lrt])
      runtime_lib = [lrt]
      break
    endif
  endforeach
endif

# Add run-time library for use in future linking tests

link_test_args += runtime_lib

# these options are required for all C compiler operations, including
# detecting many C compiler features, as we cannot expect there
# to be a working C library on the system.

c_args = meson.get_external_property('target_c_args', [])

use_stdlib = get_option('use-stdlib')
if not use_stdlib
  c_args += cc.get_supported_arguments(['-nostdlib'])
endif

native_c_args = ['-DNO_NEWLIB']

profile = get_option('profile')
if profile
  c_args += ['-pg', '-no-pie']
  native_c_args += ['-pg']
endif

if get_option('freestanding')
  c_args += ['-ffreestanding']
endif

# We don't need '-fdata-sections' currently as there aren't any
# files with data used in separate code paths. This works around
# versions of gcc for RISC-V which have a bug that mis-names
# initialized read-only data segments when -fdata-sections
# is defined
c_args += cc.get_supported_arguments(['-ffunction-sections'])

sanitize_flag = get_option('sanitize')
if sanitize_flag == 'none'
  sanitize_flag = ''
endif

if get_option('sanitize-bounds')
  if sanitize_flag == ''
    sanitize_flag = 'bounds'
  else
    sanitize_flag = ','.join(sanitize_flag, 'bounds')
  endif
endif

sanitize_trap_on_error = get_option('sanitize-trap-on-error')

if sanitize_flag == ''
  c_sanitize_flags = []
else
  c_sanitize_flags = cc.get_supported_arguments('-fsanitize=' + sanitize_flag)
  if c_sanitize_flags == [] and not get_option('sanitize-allow-missing')
    error('not supported: -fsanitize=' + sanitize_flag)
  endif

  if sanitize_trap_on_error
    c_sanitize_flags += cc.get_supported_arguments(['-fsanitize-undefined-trap-on-error'])
  endif

  c_args += c_sanitize_flags
  native_c_args += c_sanitize_flags
endif

stack_guard_spec = ''
stack_guard_option = get_option('stack-protector-guard')
prefer_tls_stack_guard = false
if stack_guard_option == 'tls'
  prefer_tls_stack_guard = true
else
  if stack_guard_option == 'auto' and host_cpu_family == 'powerpc'
    prefer_tls_stack_guard = true
  endif
endif
have_tls_stack_guard = fs.is_file('libc/machine' / host_cpu_family / 'machine/_ssp_tls.h')
if have_tls_stack_guard
  c_stack_guard_tls_flags = cc.get_supported_arguments(['-mstack-protector-guard=tls'])
  if c_stack_guard_tls_flags == []
    have_tls_stack_guard = false
  endif
endif
tls_stack_guard = prefer_tls_stack_guard and have_tls_stack_guard
if tls_stack_guard
  stack_guard_spec = '%{!mstack-protector-guard:-mstack-protector-guard=tls}'
  c_args += c_stack_guard_tls_flags
else
  if stack_guard_option == 'tls'
    if not thread_local_storage
      error('stack-protector-guard=tls option selected, but thread local storage disabled')
    else
      error('stack-protector-guard=tls option selected, but not supported for this CPU')
    endif
  endif
  c_stack_guard_global_flags = cc.get_supported_arguments(['-mstack-protector-guard=global'])
  if c_stack_guard_global_flags != []
    stack_guard_spec = '%{!mstack-protector-guard:-mstack-protector-guard=global}'
    c_args += c_stack_guard_global_flags
  endif
endif

tls_model_spec = ''
thread_local_storage = false
thread_local_storage_option = get_option('thread-local-storage')
have_picolibc_tls_api = get_option('thread-local-storage-api') and fs.is_file('libc/machine' / host_cpu_family / 'tls.c')
if thread_local_storage_option == 'auto' or thread_local_storage_option == 'picolibc'
  # We assume that _set_tls() is defined in the arch specific tls.c
  if thread_local_storage_option == 'auto' or have_picolibc_tls_api
    thread_local_storage = not cc.has_function('__emutls_get_address', args: c_args + link_test_args)
  endif
else
  thread_local_storage = get_option('thread-local-storage') == 'true'
endif

if thread_local_storage
  tls_model_spec = '%{!ftls-model:-ftls-model=' + get_option('tls-model') + '}'

  tls_arg = '-ftls-model=' + get_option('tls-model')
  assert(cc.has_argument(tls_arg), 'Compiler does not support \'-ftls-model\'!')
  c_args += [tls_arg]
endif

if get_option('analyzer')
  c_args += cc.get_supported_arguments(['-fanalyzer'])
endif

common_options = [
  '-Werror=vla',
  '-Warray-bounds',
  '-Werror=double-promotion',
  '-Wno-missing-braces',
  '-Wno-return-type',
  '-Wno-unused-command-line-argument',
  '-Wmissing-prototypes',
  '-Wmissing-declarations',
  '-Werror=implicit-fallthrough=5',
  '-Werror=unreachable-code-fallthrough',
  '-Werror=implicit-function-declaration',
  '-Wold-style-definition',
  '-Wno-implicit-int',
  '-Wflex-array-member-not-at-end',
  '-fno-common',
  '-frounding-math',
  '-fsignaling-nans',
  '-fstrict-flex-arrays=3',
  '-Wno-unsupported-floating-point-opt',
  '-fno-builtin-copysignl',
  '-fno-stack-protector',
]

cpp_options = [
  '-fno-exceptions',
  '-fno-unwind-tables',
  ]

werror_c_flag = cc.get_supported_arguments(['-Werror'])

# Split out C++ args at this point to add C and C++ specific args
if have_cplusplus
  cpp_args = c_args
  native_cpp_args = native_c_args

  # We can't use get_supported_arguments because known flags
  # not supported for the target language generate warnings
  # not errors, so we need -Werror
  cpp_flags = []
  foreach _flag : common_options + cpp_options
    if cpp.compiles('extern int i;',
                    args: ['-Werror', _flag],
                    name: _flag)
      cpp_flags += [_flag]
    endif
  endforeach
  cpp_args += cpp_flags
  native_cpp_args += cpp_flags
endif

c_flags = []
foreach _flag : common_options
  if cc.compiles('extern int i;',
                 args: ['-Werror', _flag],
                 name: _flag)
    c_flags += [_flag]
  endif
endforeach
c_args += c_flags
native_c_args += c_flags

# Make sure _LIBC is not defined when building tests
test_c_args = c_args
c_args += ['-D_LIBC', '-U_FORTIFY_SOURCE']

if have_cplusplus
  test_cpp_args = cpp_args
  cpp_args += ['-D_LIBC', '-U_FORTIFY_SOURCE']
endif

test_flags = []
# Select a fortify source option
fortify_source = get_option('fortify-source')
if fortify_source == 'none'
  test_flags += ['-U_FORTIFY_SOURCE']
else
  test_flags += ['-D_FORTIFY_SOURCE=' + fortify_source]
endif
if tests_enable_posix_io
  test_flags += ['-DTESTS_ENABLE_POSIX_IO']
endif
test_c_args += test_flags
if have_cplusplus
  test_cpp_args += test_flags
endif

# All done computing C compiler arguments
#
# c_args            target C code
# cpp_args          target C++ code
# native_c_args     native C code
# native_cpp_args   native C++ code
# test_c_args       target test C code
# test_cpp_args     target test C++ code

# Get additional flags used for linking tests.

test_link_args_base = runtime_lib

test_link_args_extra = ['-Wl,--gc-sections', '-Wl,--no-warn-rwx-segments']

if profile
  test_link_args_extra += ['-pg']
endif

test_link_args_extra += test_c_args

foreach ld : test_link_args_extra
  if cc.has_multi_link_arguments(link_test_args + [ld])
    test_link_args_base += [ld]
  endif
endforeach

fast_strcmp = get_option('fast-strcmp')

mb_capable = get_option('mb-capable')
mb_extended_charsets = mb_capable and get_option('mb-extended-charsets')
if get_option('mb-ucs-charsets') == 'auto'
  mb_ucs_charsets = mb_extended_charsets
else
  mb_ucs_charsets = mb_capable and get_option('mb-ucs-charsets') == 'true'
endif
if get_option('mb-iso-charsets') == 'auto'
  mb_iso_charsets = mb_extended_charsets
else
  mb_iso_charsets = mb_capable and get_option('mb-iso-charsets') == 'true'
endif
if get_option('mb-windows-charsets') == 'auto'
  mb_windows_charsets = mb_extended_charsets
else
  mb_windows_charsets = mb_capable and get_option('mb-windows-charsets') == 'true'
endif
if get_option('mb-jis-charsets') == 'auto'
  mb_jis_charsets = mb_extended_charsets
else
  mb_jis_charsets = mb_capable and get_option('mb-jis-charsets') == 'true'
endif

obsolete_math = get_option('newlib-obsolete-math')
obsolete_math_float = get_option('newlib-obsolete-math-float')
obsolete_math_double = get_option('newlib-obsolete-math-double')

sysroot_install = get_option('sysroot-install')
system_libc = get_option('system-libc')
prefix = get_option('prefix')

nm = find_program('nm', required : false)

check_duplicate_names = nm.found()

if nm.found()
  duplicate_names = find_program('scripts/duplicate-names', required : true)
endif

if enable_cdefs_tests
  validate_cdefs = find_program ('scripts/validate-cdefs', required : true)
endif

# Shared stdio options
io_long_long = get_option('io-long-long')
io_c99_formats = get_option('io-c99-formats')
io_pos_args = get_option('io-pos-args')
io_long_double = get_option('io-long-double')

stack_symbol = meson.get_external_property('stack_symbol', '__stack')

# stdio options
posix_console = get_option('posix-console')
io_float_exact = get_option('io-float-exact')
atomic_ungetc = get_option('atomic-ungetc')
_line = get_option('format-default')
# Pick off just the leading character
format_default = _line.substring(0, 1)

printf_aliases = get_option('printf-aliases')
io_percent_b = get_option('io-percent-b')
printf_small_ultoa = get_option('printf-small-ultoa')
printf_percent_n = get_option('printf-percent-n')
minimal_io_long_long = get_option('minimal-io-long-long')
fast_bufio = get_option('fast-bufio')
io_wchar = get_option('io-wchar')
stdio_locking = get_option('stdio-locking') and not get_option('single-thread')

foreach format : ['d', 'f', 'l', 'i', 'm']

  compile_variable = 'printf_compile_args_@0@'.format(format)
  if printf_aliases
    compile_value = ['-D_PICOLIBC_PRINTF=\'@0@\''.format(format),
                     '-D_PICOLIBC_SCANF=\'@0@\''.format(format)]
  else
    compile_value = []
  endif
  set_variable(compile_variable, compile_value)

  link_variable = 'printf_link_args_@0@'.format(format)
  link_value = compile_value
  if printf_aliases
    if format_default != format
      if has_link_defsym
        link_value += '-Wl,--defsym=@0@vfprintf=@0@__@1@_vfprintf'.format(global_prefix, format)
        link_value += '-Wl,--defsym=@0@vfscanf=@0@__@1@_vfscanf'.format(global_prefix, format)
      else
        link_value += '-Wl,-alias,@0@__@1@_vfprintf,@0@vfprintf'.format(global_prefix, format)
        link_value += '-Wl,-alias,@0@__@1@_vfscanf,@0@vfscanf'.format(global_prefix, format)
      endif
    endif
  endif
  set_variable(link_variable, link_value)

endforeach

if enable_tests and not (has_link_defsym or has_link_alias)
  # If the alias flag is not supported and we are building tests, emit an
  # error here to avoid surprising test failures.
  error('Symbol alias linker flag not supported - printf tests will fail!')
endif

if host_cpu_family == ''
  host_cc_machine=run_command(cc.cmd_array() + ['-dumpmachine'], check : true).stdout().strip().split('-')
  host_cpu_family=host_cc_machine[0]
  message('Computed host_cpu_family as ' + host_cpu_family)
endif


if sysroot_install
  # Get 'sysroot' or 'GCC_EXEC_PREFIX' from GCC output
  sysroot = run_command(cc.cmd_array() + ['-print-sysroot'], check : true).stdout().split('\n')[0]
  if sysroot != ''
    specs_prefix_format_format = '%R/@0@'
    specs_prefix_format_default = '%R/@0@'
  else
    if not get_option('sysroot-install-skip-checks')
      error('sysroot install requested, but compiler has no sysroot')
    endif
    # The default value of GCC_EXEC_PREFIX is "prefix/lib/gcc/"
    # Since toolchain may be moved and to another directory, let's get actual path using "gcc -print-search-dirs"
    # Note that the "install path" obtained with this command points to the "$GCC_EXEC_PREFIX/$ARCH/$GCC-VERSION"
    # That's why obtained path appended with '../../'
    sysroot = run_command(cc.cmd_array() + ['-print-search-dirs'], check : true).stdout().split('\n')[0].split(' ')[1]
    sysroot += '../../'
    specs_prefix_format_format = '%:getenv(GCC_EXEC_PREFIX @0@)'
    specs_prefix_format_default = '%:getenv(GCC_EXEC_PREFIX ../../@0@)'
  endif

  # Try to calculate relative path from sysroot to prefix
  specs_prefix_format = ''
  if fs.exists(sysroot)
    sysroot_to_prefix_correction = ''
    foreach _ : sysroot.split('/')
      if fs.is_samepath(sysroot + '/' + sysroot_to_prefix_correction, prefix)
        specs_prefix_format = specs_prefix_format_format.format(sysroot_to_prefix_correction + '@0@')
        break
      endif
      sysroot_to_prefix_correction += '../'
    endforeach
  endif

  # Use default 'specs_prefix_format' if can not have relative sysroot path
  if specs_prefix_format == ''
    if not get_option('sysroot-install-skip-checks')
      error('sysroot install requires sysroot(' + sysroot + ') to be a subdirectory of --prefix=<PATH>(' + prefix + ')')
    endif
    specs_prefix_format = specs_prefix_format_default
  endif
else
  specs_prefix_format = prefix + '/@0@'
endif

build_type_subdir = get_option('build-type-subdir')

lib_dir = prefix / get_option('libdir') / build_type_subdir

include_dir = prefix / get_option('includedir') / build_type_subdir

specs_dir_option = get_option('specsdir')
if build_type_subdir != ''
  specs_dir = ''
  specs_install = false
elif specs_dir_option == ''
  specs_dir = cc_install_dir
  specs_install = specs_dir != ''
elif specs_dir_option == 'none'
  specs_dir = ''
  specs_install = false
else
  specs_dir = join_paths(prefix, specs_dir_option)
  specs_install = true
endif

# Let targets add more support libraries
additional_libs_list = meson.get_external_property('additional_libs', [])

compiler_id = cc.get_id()
if compiler_id == 'ccomp'
  # When passing the specs file to CompCert, the libcompcert needs to be included there as well
  additional_libs_list += '-lcompcert'
endif

additional_libs = ' '.join(additional_libs_list)

if compiler_id == 'gcc' and target_machine.cpu_family() == 'msp430'
  # Extract -mhwmult-selection-snippet from GCC.
  dumped_specs = run_command(cc.cmd_array() + ['-dumpspecs']).stdout()
  hwmult_start = dumped_specs.split('%{mhwmult=auto')[1]
  hwmult_snippet = '%{mhwmult=auto' + hwmult_start.split('-lc')[0]
  additional_libs += hwmult_snippet
endif

specs_extra = ''
specs_extra_list = meson.get_external_property('specs_extra', [])
if specs_extra_list != []
  specs_extra = '\n' + '\n'.join(specs_extra_list)
endif

# Handle old-style printf/scanf selection mechanism in the specs file

specs_printf = ''
if printf_aliases
  specs_printf=('%{DPICOLIBC_DOUBLE_PRINTF_SCANF:--defsym=@0@vfprintf=@0@__d_vfprintf}' +
		' %{DPICOLIBC_DOUBLE_PRINTF_SCANF:--defsym=@0@vfscanf=@0@__d_vfscanf}' +
                ' %{DPICOLIBC_FLOAT_PRINTF_SCANF:--defsym=@0@vfprintf=@0@__f_vfprintf}' +
		' %{DPICOLIBC_FLOAT_PRINTF_SCANF:--defsym=@0@vfscanf=@0@__f_vfscanf}' +
		' %{DPICOLIBC_LONG_LONG_PRINTF_SCANF:--defsym=@0@vfprintf=@0@__l_vfprintf}' +
		' %{DPICOLIBC_LONG_LONG_PRINTF_SCANF:--defsym=@0@vfscanf=@0@__l_vfscanf}' +
		' %{DPICOLIBC_INTEGER_PRINTF_SCANF:--defsym=@0@vfprintf=@0@__i_vfprintf}' +
		' %{DPICOLIBC_INTEGER_PRINTF_SCANF:--defsym=@0@vfscanf=@0@__i_vfscanf}' +
		' %{DPICOLIBC_MINIMAL_PRINTF_SCANF:--defsym=@0@vfprintf=@0@__m_vfprintf}' +
		' %{DPICOLIBC_MINIMAL_PRINTF_SCANF:--defsym=@0@vfscanf=@0@__m_vfscanf}').format(global_prefix)
endif

if system_libc
  specs_isystem = ''
  specs_libpath = ''
  # The '%s' here asks gcc to search for this file in the 'usual' library path
  specs_startfile = '%{-crt0=*:crt0-%*%O%s; :crt0%O%s}'
else

# Note that this value does *not* include the trailing %s which
# means we have to compute the full path to the file
crt0_expr = '%{-crt0=*:crt0-%*%O; :crt0%O}'

#
# Construct path values for specs file
#
# Each of these needs to handle --picolibc-prefix and
# --picolibc-buildtype options, system-root vs absolute paths and
# multilib stuff. That makes this all unreasonably complicated.
#
# Each option is computed in three parts, the 'prefix' value
# (corresponding to --picolibc-prefix), the 'buildtype' value
# (corresponding to --picolibc-buildtype) and the 'gen' value (for
# when neither of these options is specifed).
#
# Because 'getenv' appends a space afterwards in GCC spec files, the
# entire final path elements must be specified inside the parens;
# e.g. %:getenv(FOO a/b/c) instead of %:getenv(FOO)/a/b/c. That means
# we use the specs_prefix_format value computed above to build paths
# instead of simple concatenation
#

#
# How to format each of the three option-selected
# values
#
picolibc_prefix_format = '-picolibc-prefix=*:@0@'
picolibc_buildtype_format = '-picolibc-buildtype=*:@0@'
gen_format = '@0@'

#
# How to glue the three options together
#
specs_option_format = '%{@0@; @1@; :@2@}'

#
# Build the -isystem value
#

prefix_include_dir = '%*/@0@/'.format(get_option('includedir'))
isystem_prefix = picolibc_prefix_format.format(prefix_include_dir)

buildtype_include_dir = specs_prefix_format.format(get_option('includedir') / '%*')
isystem_buildtype = picolibc_buildtype_format.format(buildtype_include_dir)

gen_include_dir = specs_prefix_format.format(get_option('includedir'))
isystem_gen = gen_format.format(gen_include_dir)

specs_isystem = '-isystem ' + specs_option_format.format(isystem_prefix, isystem_buildtype, isystem_gen)

gcc_dumpmachine = run_command(cc.cmd_array() + ['-dumpmachine'], check : true).stdout().split('\n')[0]

cxxconfig_path_postfix = '..' / gcc_dumpmachine / 'include' / 'c++' / '%(version)' / gcc_dumpmachine / '%M'
cxxconfig_isystem_prefix = isystem_prefix / cxxconfig_path_postfix
cxxconfig_isystem_buildtype = isystem_buildtype / cxxconfig_path_postfix
cxxconfig_isystem_gen = isystem_gen / cxxconfig_path_postfix

specs_isystem = specs_isystem + ' -isystem ' + specs_option_format.format(cxxconfig_isystem_prefix, cxxconfig_isystem_buildtype, cxxconfig_isystem_gen)

#
# Build the non-multilib -L value
#

prefix_lib_dir = '%*/@0@'.format(get_option('libdir'))
lib_prefix = picolibc_prefix_format.format(prefix_lib_dir)

buildtype_lib_dir = specs_prefix_format.format(get_option('libdir') / '%*')
lib_buildtype = picolibc_buildtype_format.format(buildtype_lib_dir)

gen_lib_dir = specs_prefix_format.format(get_option('libdir'))
lib_gen = gen_format.format(gen_lib_dir)

specs_libpath = '-L' + specs_option_format.format(lib_prefix, lib_buildtype, lib_gen)

#
# Build the non-multilib libgcc -L value
#

libgcc_dir = get_option('libgccdir') / '%(version)'
prefix_libgcc_dir = '%*/@0@'.format(libgcc_dir)
libgcc_prefix = picolibc_prefix_format.format(prefix_libgcc_dir)

buildtype_libgcc_dir = specs_prefix_format.format(libgcc_dir / '%*')
libgcc_buildtype = picolibc_buildtype_format.format(buildtype_libgcc_dir)

gen_libgcc_dir = specs_prefix_format.format(libgcc_dir)
libgcc_gen = gen_format.format(gen_libgcc_dir)

specs_libgccpath = '-L' + specs_option_format.format(libgcc_prefix, libgcc_buildtype, libgcc_gen)

#
# Build the non-multilib *startfile options
#

prefix_crt0_path = '%*/@0@'.format(get_option('libdir')) / crt0_expr
crt0_prefix = picolibc_prefix_format.format(prefix_crt0_path)

buildtype_crt0_path = specs_prefix_format.format(get_option('libdir') / '%*' / crt0_expr)
crt0_buildtype = picolibc_buildtype_format.format(buildtype_crt0_path)

gen_crt0_path = specs_prefix_format.format(get_option('libdir') / crt0_expr)
crt0_gen = gen_format.format(gen_crt0_path)

#
# Now build multilib versions of the -L and *startfile values
#
if enable_multilib

  #
  # Build the multilib -L value
  #
  prefix_multilib_dir = '%*/@0@'.format(get_option('libdir') / '%M')
  multilib_prefix = picolibc_prefix_format.format(prefix_multilib_dir)

  buildtype_multilib_dir = specs_prefix_format.format(get_option('libdir') / '%*/%M')
  multilib_buildtype = picolibc_buildtype_format.format(buildtype_multilib_dir)

  gen_multilib_dir = specs_prefix_format.format(get_option('libdir') / '%M')
  multilib_gen = gen_format.format(gen_multilib_dir)

  specs_multilibpath = '-L' + specs_option_format.format(multilib_prefix, multilib_buildtype, multilib_gen)

  #
  # Build the multilib libgcc -L value
  #
  prefix_multilibgcc_dir = '%*/@0@'.format(libgcc_dir / '%M')
  multilibgcc_prefix = picolibc_prefix_format.format(prefix_multilibgcc_dir)

  buildtype_multilibgcc_dir = specs_prefix_format.format(libgcc_dir / '%*/%M')
  multilibgcc_buildtype = picolibc_buildtype_format.format(buildtype_multilibgcc_dir)

  gen_multilibgcc_dir = specs_prefix_format.format(libgcc_dir / '%M')
  multilibgcc_gen = gen_format.format(gen_multilibgcc_dir)

  specs_multilibgccpath = '-L' + specs_option_format.format(multilibgcc_prefix, multilibgcc_buildtype, multilibgcc_gen)

  #
  # Prepend the multilib -L option to the non-multilib option
  #
  specs_libpath = specs_multilibpath + ' ' + specs_libpath
  specs_libgccpath = specs_multilibgccpath + ' ' + specs_libgccpath

  #
  # Build the multilib *startfile options
  #
  prefix_multilib_crt0_path = '%*/@0@'.format(get_option('libdir')) / '%M' / crt0_expr
  crt0_prefix = picolibc_prefix_format.format(prefix_multilib_crt0_path)

  buildtype_multilib_crt0_path = specs_prefix_format.format(get_option('libdir') / '%*/%M' / crt0_expr)
  crt0_buildtype = picolibc_buildtype_format.format(buildtype_multilib_crt0_path)

  gen_multilib_crt0_path = specs_prefix_format.format(get_option('libdir') / '%M' / crt0_expr)
  crt0_gen = gen_format.format(gen_multilib_crt0_path)
endif

#
# Construct the *startfile value from the options computed
# above. As there's only one value, it's either the
# multilib path or the non-multilib path
#
specs_startfile = specs_option_format.format(crt0_prefix, crt0_buildtype, crt0_gen)
endif

specs_data = configuration_data()
specs_data.set('SPECS_ISYSTEM', specs_isystem)
specs_data.set('SPECS_LIBGCC_PATH', specs_libgccpath)
specs_data.set('SPECS_LIBPATH', specs_libpath)
specs_data.set('SPECS_STARTFILE', specs_startfile)
specs_data.set('TLSMODEL', tls_model_spec)
specs_data.set('STACKGUARD', stack_guard_spec)
specs_data.set('LINK_SPEC', meson.get_external_property('link_spec', ''))
specs_data.set('CC1_SPEC', meson.get_external_property('cc1_spec', ''))
specs_data.set('CC1PLUS_SPEC', meson.get_external_property('cc1plus_spec', ''))
specs_data.set('ADDITIONAL_LIBS', additional_libs)
specs_data.set('SPECS_EXTRA', specs_extra)
specs_data.set('SPECS_PRINTF', specs_printf)
specs_data.set('PREFIX', global_prefix)

# Create C and C++ specific specs data,
# that includes setting the correct linker script
# and adding the C++ startup/shutdown files

specs_c_data = specs_data

specs_c_data.set('PICOLIBC_LD', 'picolibc.ld')
specs_c_data.set('CRTBEGIN', '')
specs_c_data.set('CRTEND', '')

specs_cpp_data = specs_data

specs_cpp_data.set('PICOLIBC_LD', 'picolibcpp.ld')
specs_cpp_data.set('CRTBEGIN', 'crtbegin%O%s')
specs_cpp_data.set('CRTEND', 'crtend%O%s')

if specs_install and really_install

  compiler_picolibc_native = run_command(cc.cmd_array() + ['-v'], check : false).stderr().contains('with-picolibc')

  if compiler_picolibc_native
    picolibc_specs_in = 'picolibc-native.specs.in'
  else
    picolibc_specs_in = 'picolibc.specs.in'
  endif

  configure_file(input: picolibc_specs_in,
	         output: 'picolibc.specs',
	         configuration: specs_c_data,
	         install_dir: specs_dir,
	         install: specs_install and really_install)

  configure_file(input: picolibc_specs_in,
	         output: 'picolibcpp.specs',
	         configuration: specs_cpp_data,
	         install_dir: specs_dir,
	         install: specs_install and really_install)
endif

# CompCert does not support bitfields in packed structs, so avoid using this optimization
bitfields_in_packed_structs_code = '''
struct test { int part: 24; } __attribute__((packed));
unsigned int foobar (const struct test *p) { return p->part; }
'''
have_bitfields_in_packed_structs = cc.compiles(bitfields_in_packed_structs_code, name : 'packed structs may contain bitfields', args: c_args)

# CompCert does not support _Complex
complex_code = '''
float _Complex test(float _Complex z) { return z; }
'''
have_complex = cc.compiles(complex_code, name : 'supports _Complex', args: c_args)

werror_c_args = c_args + cc.get_supported_arguments('-Werror')

# CompCert uses the GCC preprocessor, which causes to
#  > #if __has_attribute(__alloc_size__)
# produce a wrong result. So test if the compiler has that attribute
alloc_size_code = '''
void *foobar(int) __attribute__((__alloc_size__(1)));
void *foobar2(int, int) __attribute__((__alloc_size__(1, 2)));
'''
have_alloc_size = cc.compiles(alloc_size_code, name : 'attribute __alloc_size__', args : werror_c_args)

# attributes constructor/destructor are a GNU extension - if the compiler doesn't have them, don't test them.
attr_ctor_dtor_code = '''
void __attribute__((constructor(101))) ctor (void) {}
void __attribute__((destructor(101))) dtor (void) {}
'''
have_attr_ctor_dtor = cc.compiles(attr_ctor_dtor_code, name : 'attributes constructor/destructor', args : werror_c_args)

targets = []

if enable_multilib
  used_libs = []

  # Ask the compiler for the set of available multilib configurations,
  # set up the build system to compile for all desired ones

  target_list = run_command(cc.cmd_array() + get_option('c_args') + ['--print-multi-lib'], check : true).stdout().strip().split('\n')

  has_mcmodel = false
  foreach target : target_list
    if target.contains('mcmodel=')
      has_mcmodel = true
    endif
  endforeach

  foreach target : target_list
    tmp = target.split(';')
    matches_exclude = false
    foreach exclude : multilib_exclude
      if tmp[0].contains(exclude)
        matches_exclude = true
        message('skipping target ' + tmp[0])
        break
      endif
    endforeach
    if matches_exclude
      continue
    endif

    _c_flags = []

    # Let the user specify a subset of the possible multilib
    # configurations to build for
    if multilib_list == [] or tmp[0] in multilib_list
      used_libs += tmp[0]
      if tmp.length() > 1
	foreach flag : tmp[1].strip('@').split('@')
	  if flag != ''
	    if host_cpu_family == 'nios2'
	      # Hacks for NIOS II to get rid of -fsingle-precision-constant
	      # mode (which breaks libm). We don't need fph2 as that
	      # doesn't set the single precision constant flag
	      if flag == 'mcustom-fpu-cfg=60-1'
		_c_flags += ['-mcustom-fmuls=252', '-mcustom-fadds=253',
			  '-mcustom-fsubs=254']
	      elif flag == 'mcustom-fpu-cfg=60-2'
		_c_flags += ['-mcustom-fmuls=252', '-mcustom-fadds=253',
			  '-mcustom-fsubs=254', '-mcustom-fdivs=255']
	      elif flag == 'mcustom-fpu-cfg=72-3'
		_c_flags += ['-mcustom-floatus=243', '-mcustom-fixsi=244',
			  '-mcustom-floatis=245','-mcustom-fcmpgts=246',
			  '-mcustom-fcmples=249', '-mcustom-fcmpeqs=250',
			  '-mcustom-fcmpnes=251', '-mcustom-fmuls=252',
			  '-mcustom-fadds=253', '-mcustom-fsubs=254',
			  '-mcustom-fdivs=255']
	      else
		_c_flags += '-' + flag
	      endif
	    else
	      _c_flags += '-' + flag
	    endif
	  endif
	endforeach
	if tmp[0] == '.'
          dir = ''
	  name = ''
          lib_prefix = ''
	else
          dir = tmp[0]
	  name = '_' + tmp[0].underscorify()
          lib_prefix = 'lib'
	endif
      else
        dir = ''
	name = ''
        lib_prefix = ''
      endif

      # rv64 needs to use a non-default mcmodel so that variables can
      # live in a broader range of memory addresses
      if not has_mcmodel and name.strip('_').startswith('rv64')
	_c_flags += [ '-mcmodel=medany' ]
      endif

      # Add any extra flags for this target from the cross file
      _c_flags += meson.get_external_property('c_args_' + name.strip('_'), [])

      targets += {
                  'name': name,
                  'dir': dir,
                  'c_args': _c_flags,
                  'lib_prefix': lib_prefix,
                }
    endif
  endforeach

  # Make sure all requested multilib configurations
  # are actually available
  if multilib_list != []
    foreach lib : multilib_list
      if lib not in used_libs
	error('Unavailable multilib: ' + lib)
      endif
    endforeach
  endif
else
  targets += {
              'name': '',
              'dir': '',
              'c_args': [],
              'lib_prefix': '',
            }
endif

default_target = {
              'name': 'default-target',
              'dir': '',
              'c_args': [],
              'lib_prefix': '',
            }

ld_comment = '#'

# Templates for various linker script fragments

init_memory_template = '''
	@0@ (rx!w) :
		ORIGIN = DEFINED(__@0@) ? __@0@ : @1@,
		LENGTH = DEFINED(__@0@_size) ? __@0@_size : @2@'''
init_phdr_template = '''
	text_@0@ PT_LOAD;'''
init_section_template = '''
	.@0@ : {@1@
	} >@2@ AT>@2@ :@3@
'''

foreach params : [default_target] + targets

  target = params['name']
  target_dir = params['dir']
  target_c_args = params['c_args']

  custom_mem_config = ''

  if target != 'default-target'
    target_part = ''
    foreach part : target.split('_')
      if part != ''
        target_part += '_' + part
        custom_mem_config = meson.get_external_property('custom_mem_config' + target_part, custom_mem_config)
      endif
    endforeach

    target_head = target_dir.split('_')[0].underscorify()
    custom_mem_config = meson.get_external_property('custom_mem_config_' + target_head, custom_mem_config)
  endif

  custom_mem_config = meson.get_external_property('custom_mem_config' + target, custom_mem_config)

  if target == 'default-target' or custom_mem_config != ''

    # Figure out all of the file and variable names

    if target == 'default-target'
      picolibc_ld_file = 'picolibc.ld'
      picolibc_ld_variable = 'picolibc_ld'
      picolibc_ld_config_variable = 'picolibc_ld_config'

      picolibc_noflash_ld_file = 'picolibc_noflash.ld'
      picolibc_noflash_ld_variable = 'picolibc_noflash_ld'
      picolibc_noflash_ld_config_variable = 'picolibc_noflash_ld_config'

      picolibcpp_ld_file = 'picolibcpp.ld'
      picolibcpp_ld_variable = 'picolibcpp_ld'
      picolibcpp_ld_config_variable = 'picolibcpp_ld_config'

      picolibcpp_noflash_ld_file = 'picolibcpp_noflash.ld'
      picolibcpp_noflash_ld_variable = 'picolibcpp_noflash_ld'
      picolibcpp_noflash_ld_config_variable = 'picolibcpp_noflash_ld_config'

      picolibc_ld_install = true

      if thread_local_storage
        picolibc_inittls_ld_file = 'picolibc_inittls.ld'
        picolibc_inittls_ld_variable = 'picolibc_inittls_ld'
        picolibc_inittls_ld_config_variable = 'picolibc_inittls_ld_config'
      endif
    else
      picolibc_ld_file = 'picolibc_' + custom_mem_config + '.ld'
      picolibc_ld_variable = 'picolibc' + target + '_ld'
      picolibc_ld_config_variable = 'picolibc_' + custom_mem_config + '_ld_config'

      picolibc_noflash_ld_file = 'picolibc_' + custom_mem_config + '_noflash.ld'
      picolibc_noflash_ld_variable = 'picolibc' + target + '_noflash_ld'
      picolibc_noflash_ld_config_variable = 'picolibc_' + custom_mem_config + '_noflash_ld_config'

      picolibcpp_ld_file = 'picolibcpp_' + custom_mem_config + '.ld'
      picolibcpp_ld_variable = 'picolibcpp' + target + '_ld'
      picolibcpp_ld_config_variable = 'picolibcpp_' + custom_mem_config + '_ld_config'

      picolibcpp_noflash_ld_file = 'picolibcpp_' + custom_mem_config + '_noflash.ld'
      picolibcpp_noflash_ld_variable = 'picolibcpp_' + target.underscorify() + '_noflash_ld'
      picolibcpp_noflash_ld_config_variable = 'picolibcpp_' + custom_mem_config + '_noflash_ld_config'

      if thread_local_storage
        picolibc_inittls_ld_file = 'picolibc_inittls_' + custom_mem_config + '.ld'
        picolibc_inittls_ld_variable = 'picolibc_inittls_' + target + '_ld'
        picolibc_inittls_ld_config_variable = 'picolibc_inittls_' + custom_mem_config + '_ld_config'
      endif

      picolibc_ld_install = false
    endif

    # Compute settings for the default linker script

    picolibc_ld_data = configuration_data()

    picolibc_ld_data.set('BFD', '')
    picolibc_ld_data.set('LLD', ld_comment)
    picolibc_ld_data.set('TLS_PHDRS', '''tls_init PT_TLS;
        tls PT_TLS;''')
    picolibc_ld_data.set('TLS_INIT_PHDR', 'tls_init')

    # place some defaults here to make other sections a bit shorter
    picolibc_ld_data.set('CPP', ld_comment)
    picolibc_ld_data.set('C', '')
    picolibc_ld_data.set('PREFIX', global_prefix)
    picolibc_ld_data.set('INITTLS', ld_comment)
    picolibc_ld_data.set('DATATLS', '')

    # non-exec mode for ram PHDR
    picolibc_ld_data.set('RAM_MODE', 'w!rx')

    # attributes for regular executable sections
    picolibc_ld_data.set('TEXT_MEM', 'flash')
    picolibc_ld_data.set('TEXT_AT', 'flash')
    picolibc_ld_data.set('TEXT_PHDR', 'text')

    # attributes for read-only data sections
    picolibc_ld_data.set('RODATA_MEM', 'flash')
    picolibc_ld_data.set('RODATA_AT', 'flash')
    picolibc_ld_data.set('RODATA_PHDR', 'rodata')

    # attributes for data-initialized read-write data sections
    picolibc_ld_data.set('DATA_MEM', 'ram')
    picolibc_ld_data.set('DATA_AT', 'flash')
    picolibc_ld_data.set('DATA_PHDR', 'ram')
    picolibc_ld_data.set('DATA_INIT_PHDR', 'ram_init')

    # attributes for zero-initialized read-write data sections
    picolibc_ld_data.set('BSS_MEM', 'ram')
    picolibc_ld_data.set('BSS_AT', 'ram')
    picolibc_ld_data.set('BSS_PHDR', 'ram')

    # phdrs for tls
    picolibc_ld_data.set('TLS_PHDR', 'tls')
    picolibc_ld_data.set('TLS_INIT_PHDR', 'tls_init')

    picolibc_ld_data.set('STACK', stack_symbol)

    picolibc_ld_data.set('EXTRA_TEXT_SECTIONS',
                                  meson.get_external_property('extra_text_sections', ''))
    picolibc_ld_data.set('EXTRA_RODATA_SECTIONS',
                                  meson.get_external_property('extra_rodata_sections', ''))
    picolibc_ld_data.set('EXTRA_DATA_SECTIONS',
                                  meson.get_external_property('extra_data_sections', ''))
    picolibc_ld_data.set('EXTRA_SDATA_SECTIONS',
                                  meson.get_external_property('extra_sdata_sections', ''))
    picolibc_ld_data.set('EXTRA_BSS_SECTIONS',
                                  meson.get_external_property('extra_bss_sections', ''))
    picolibc_ld_data.set('EXTRA_SBSS_SECTIONS',
                                  meson.get_external_property('extra_sbss_sections', ''))

    # We need to use different alignment flags for .tdata/.tbss for ld.bfd
    # (ALIGN_WITH_INPUT, which is not supported by ld.lld) and ld.lld
    # (ALIGN(__tls_align), which is rejected as non-constant by ld.bfd), so we use
    # the {BFD,LLD}_{START,END} templates to comment out the incompatible flag.

    if cc.get_linker_id() == 'ld.lld'
      picolibc_ld_data.set('BFD', ld_comment)
      picolibc_ld_data.set('LLD', '')
      picolibc_ld_data.set('TLS_PHDRS', 'tls PT_TLS;')
      picolibc_ld_data.set('TLS_INIT_PHDR', 'tls')
      if host_cpu_family == 'riscv'
        # ld.lld before version 15 did not support linker relaxations, disable
        # them if we are using an older version.
        # There is no `cc.get_linker_version()` function, so we detect ld.lld
        # version 15 by checking for a newly added linker flag.
        # Note: --version still checks for valid arguments so this works.
        if not cc.has_link_argument('-Wl,--package-metadata=1,--version')
          message('Linking for RISCV with ld.lld < 15, forcing -mno-relax')
          c_args += ['-mno-relax']
        endif
      endif
    endif

    if cc.get_linker_id() == 'ld.eld'
      picolibc_ld_data.set('TLS_PHDRS', 'tls PT_TLS;')
      picolibc_ld_data.set('TLS_INIT_PHDR', 'tls')
    endif

    default_init_section_template = init_section_template.format('@0@', '''
		KEEP (*(.text.init.enter))
		KEEP (*(.data.init.enter))
		KEEP (*(SORT_BY_NAME(.init) SORT_BY_NAME(.init.*)))''',
                                                                 '@1@',
                                                                 '@2@')

    additional_sections = []

    fallback_flash_addr = '0x10000000'
    fallback_flash_size = '0x00010000'
    fallback_ram_addr = '0x20000000'
    fallback_ram_size = '0x00008000'
    fallback_stack_size = '0x00001000'
    fallback_alignment = '8'

    if meson.get_external_property('separate_boot_flash_' + custom_mem_config,
                                meson.get_external_property('separate_boot_flash', false))
      boot_flash_addr = meson.get_external_property(
        'default_boot_flash_addr_' + custom_mem_config,
        meson.get_external_property(
          'default_boot_flash_addr',
          '0x10000000'))
      boot_flash_size = meson.get_external_property(
        'default_boot_flash_size_' + custom_mem_config,
        meson.get_external_property(
          'default_boot_flash_size',
          '0x00000400'))
      additional_sections = [
        {
          'name' : 'boot_flash',
          'addr' : boot_flash_addr,
          'size' : boot_flash_size,
          'contents' : default_init_section_template.format('boot_flash', 'boot_flash', 'text_boot_flash')
        }]
      fallback_flash_addr = '0x10000400'
      fallback_flash_size = '0x0000fc00'
    else
      additional_section_names = meson.get_external_property('additional_sections_' + custom_mem_config,
                                                     meson.get_external_property('additional_sections', []))
      if additional_section_names != []
        foreach section_name : additional_section_names
          default_content_list =  ['*.(.' + section_name + ')', '*.(.' + section_name + '.*)']
          content_list = meson.get_external_property('default_' + section_name + '_contents', default_content_list)
          contents = ''
          foreach content : content_list
            contents += '''
		@0@'''.format(content)
          endforeach
          additional_sections += [
            {
              'name' : section_name,
              'addr' : meson.get_external_property('default_' + section_name + '_addr'),
              'size' : meson.get_external_property('default_' + section_name + '_size'),
              'contents' : init_section_template.format(section_name, contents, section_name, 'text_' + section_name)
            }]
        endforeach
      endif
    endif

    init_memory = ''
    init_phdrs = ''
    init_sections = ''

    if additional_sections != []
      foreach section : additional_sections
        section_name = section['name']
        init_memory += init_memory_template.format(section_name, section['addr'], section['size'])
        init_phdrs += init_phdr_template.format(section_name)
        init_sections += section['contents']
      endforeach
    else
      # Default init sections map to flash in the primary script; no option gating
      init_section_name = meson.get_external_property('init_section_name', 'init')
      init_sections = default_init_section_template.format(init_section_name, 'flash', 'text')
    endif

    picolibc_ld_data.set('INIT_MEMORY', init_memory)
    picolibc_ld_data.set('INIT_PHDRS', init_phdrs)
    picolibc_ld_data.set('INIT_SECTIONS', init_sections)

    # Get default memory configuration values

    _default_flash_addr =  meson.get_external_property('default_flash_addr_' + custom_mem_config,
                                                    meson.get_external_property(
                                                      'default_flash_addr',
                                                      fallback_flash_addr))

    _default_flash_size =  meson.get_external_property('default_flash_size_' + custom_mem_config,
                                                    meson.get_external_property(
                                                      'default_flash_size',
                                                      fallback_flash_size))

    _default_ram_addr =  meson.get_external_property('default_ram_addr_' + custom_mem_config,
                                                  meson.get_external_property(
                                                    'default_ram_addr',
                                                    fallback_ram_addr))

    _default_ram_size =  meson.get_external_property('default_ram_size_' + custom_mem_config,
                                                  meson.get_external_property(
                                                    'default_ram_size', fallback_ram_size))

    _default_stack_size = meson.get_external_property('default_stack_size_' + custom_mem_config,
                                                   meson.get_external_property(
                                                     'default_stack_size', fallback_stack_size))

    _default_alignment = meson.get_external_property('default_alignment_' + custom_mem_config,
                                                  meson.get_external_property(
                                                    'default_alignment', fallback_alignment))

    _default_noflash_flash_addr = meson.get_external_property(
      'default_noflash_flash_addr_' + custom_mem_config,
      meson.get_external_property('default_noflash_flash_addr', _default_flash_addr))

    _default_noflash_flash_size = meson.get_external_property(
      'default_noflash_flash_size_' + custom_mem_config,
      meson.get_external_property('default_noflash_flash_size', _default_flash_size))

    _default_noflash_ram_addr = meson.get_external_property(
      'default_noflash_ram_addr_' + custom_mem_config,
      meson.get_external_property('default_noflash_ram_addr', _default_ram_addr))

    _default_noflash_ram_size = meson.get_external_property(
      'default_noflash_ram_size_' + custom_mem_config,
      meson.get_external_property('default_noflash_ram_size', _default_ram_size))

    _default_noflash_stack_size = meson.get_external_property(
      'default_noflash_stack_size_' + custom_mem_config,
      meson.get_external_property('default_noflash_stack_size', _default_stack_size))

    _default_noflash_alignment = meson.get_external_property(
      'default_noflash_alignment_' + custom_mem_config,
      meson.get_external_property('default_noflash_alignment', _default_alignment))

    picolibc_ld_data.set('DEFAULT_FLASH_ADDR', _default_flash_addr)
    picolibc_ld_data.set('DEFAULT_FLASH_SIZE', _default_flash_size)
    picolibc_ld_data.set('DEFAULT_RAM_ADDR', _default_ram_addr)
    picolibc_ld_data.set('DEFAULT_RAM_SIZE', _default_ram_size)
    picolibc_ld_data.set('DEFAULT_STACK_SIZE', _default_stack_size)
    picolibc_ld_data.set('DEFAULT_ALIGNMENT', _default_alignment)

    # Generate the default linker script

    if not is_variable(picolibc_ld_config_variable)
      set_variable(picolibc_ld_config_variable,
                   configure_file(input: 'picolibc.ld.in',
                                  output: picolibc_ld_file,
                                  configuration: picolibc_ld_data,
                                  install: picolibc_ld_install and really_install,
                                  install_dir: lib_dir))
    endif

    set_variable(picolibc_ld_variable, get_variable(picolibc_ld_config_variable))

    # Generate C++ linker script
    picolibcpp_ld_data = configuration_data()
    picolibcpp_ld_data.merge_from(picolibc_ld_data)

    # Almost the same as C, just flip CPP and C bits
    picolibcpp_ld_data.set('CPP', '')
    picolibcpp_ld_data.set('C', ld_comment)

    if not is_variable(picolibcpp_ld_config_variable)
      set_variable(picolibcpp_ld_config_variable,
                 configure_file(input: 'picolibc.ld.in',
                                output: picolibcpp_ld_file,
                                configuration: picolibcpp_ld_data,
                                install: picolibc_ld_install and really_install,
                                install_dir: lib_dir))
    endif

    set_variable(picolibcpp_ld_variable, get_variable(picolibcpp_ld_config_variable))

    # Generate a linker script with separate TLS data

    if thread_local_storage
      picolibc_inittls_ld_data = configuration_data()
      picolibc_inittls_ld_data.merge_from(picolibc_ld_data)

      # Take the C data and flip DATATLS for INITTLS bits
      picolibc_inittls_ld_data.set('INITTLS', '')
      picolibc_inittls_ld_data.set('DATATLS', ld_comment)

      if not is_variable(picolibc_inittls_ld_config_variable)
        set_variable(picolibc_inittls_ld_config_variable,
                     configure_file(input: 'picolibc.ld.in',
                                    output: picolibc_inittls_ld_file,
                                    configuration: picolibc_inittls_ld_data,
                                    install: picolibc_ld_install and really_install,
                                    install_dir: lib_dir))
      endif

      set_variable(picolibc_inittls_ld_variable, get_variable(picolibc_inittls_ld_config_variable))
    endif

    # Generate and install noflash C and C++ linker scripts

    picolibc_noflash_ld_data = configuration_data()
    # start with the regular C bits
    picolibc_noflash_ld_data.merge_from(picolibc_ld_data)
    # noflash also needs inittls
    picolibc_noflash_ld_data.set('INITTLS', '')
    picolibc_noflash_ld_data.set('DATATLS', ld_comment)

    # place all remaining sections in ram

    # attributes for regular executable sections
    picolibc_noflash_ld_data.set('TEXT_MEM', 'ram')
    picolibc_noflash_ld_data.set('TEXT_AT', 'ram')
    picolibc_noflash_ld_data.set('TEXT_PHDR', 'ram')

    # attributes for read-only data sections
    picolibc_noflash_ld_data.set('RODATA_MEM', 'ram')
    picolibc_noflash_ld_data.set('RODATA_AT', 'ram')
    picolibc_noflash_ld_data.set('RODATA_PHDR', 'ram')

    picolibc_noflash_ld_data.set('DATA_AT', 'ram')
    picolibc_noflash_ld_data.set('DATA_INIT_PHDR', 'ram')

    # mark the ram section rwx
    picolibc_noflash_ld_data.set('RAM_MODE', 'rwx')

    # set up default memory layout
    picolibc_noflash_ld_data.set('DEFAULT_FLASH_ADDR', _default_noflash_flash_addr)
    picolibc_noflash_ld_data.set('DEFAULT_FLASH_SIZE', _default_noflash_flash_size)
    picolibc_noflash_ld_data.set('DEFAULT_RAM_ADDR', _default_noflash_ram_addr)
    picolibc_noflash_ld_data.set('DEFAULT_RAM_SIZE', _default_noflash_ram_size)
    picolibc_noflash_ld_data.set('DEFAULT_STACK_SIZE', _default_noflash_stack_size)
    picolibc_noflash_ld_data.set('DEFAULT_ALIGNMENT', _default_noflash_alignment)

    if not is_variable(picolibc_noflash_ld_config_variable)
      set_variable(picolibc_noflash_ld_config_variable,
                   configure_file(input: 'picolibc.ld.in',
                                  output: picolibc_noflash_ld_file,
                                  configuration: picolibc_noflash_ld_data,
                                  install: picolibc_ld_install and really_install,
                                  install_dir: lib_dir))
    endif

    set_variable(picolibc_noflash_ld_variable, get_variable(picolibc_noflash_ld_config_variable))

    # no flash C++ version is just the noflash version with CPP and C swapped
    picolibcpp_noflash_ld_data = configuration_data()
    picolibcpp_noflash_ld_data.merge_from(picolibc_noflash_ld_data)
    picolibcpp_noflash_ld_data.set('CPP', '')
    picolibcpp_noflash_ld_data.set('C', ld_comment)

    if not is_variable(picolibcpp_noflash_ld_config_variable)
      set_variable(picolibcpp_noflash_ld_config_variable,
                   configure_file(input: 'picolibc.ld.in',
                                  output: picolibcpp_noflash_ld_file,
                                  configuration: picolibcpp_noflash_ld_data,
                                  install: picolibc_ld_install and really_install,
                                  install_dir: lib_dir))
    endif

    set_variable(picolibcpp_noflash_ld_variable, get_variable(picolibcpp_noflash_ld_config_variable))

  endif
endforeach

conf_data = configuration_data()

has_builtin_code = '''
int main(void) {
#if __has_builtin(__builtin_object_size)
return 0;
#else
return 1;
#endif
}
'''

have_has_builtin = cc.compiles(has_builtin_code,
                               name : '__has_builtin builtin',
                               args : werror_c_args)

#
# Some builtins cannot be detected with has_builtin as they are
# keywords instead of functions
#
builtins = [
    ['builtin_complex', '__builtin_complex(1.0, 2.0)', 'double _Complex'],
]

if not have_has_builtin

  # For compilers without __has_builtin, detect the available builtins
  # at meson time. This list should match that in include/sys/config.h
  #
  # The supported builtins vary depending on compiler and target.
  # If you want to check for a given builtin, add an array
  #   ['some_builtin_name', '__call_to_builtin(1,2,3);']
  # The below loop will then add the define HAVE_SOME_BUILTIN_NAME if the code snippet
  #   > int main (void) { __call_to_builtin(1,2,3); return 0; }
  # can be compiled + linked.

  builtins += [
    ['builtin_alloca', '__builtin_alloca(1)', 'void *'],
    ['builtin_ffs', '__builtin_ffs(42)', 'int'],
    ['builtin_ffsl', '__builtin_ffsl((long)42)', 'long'],
    ['builtin_ffsll', '__builtin_ffsll((long long)42)', 'long long'],
    ['builtin_ctz', '__builtin_ctz((unsigned int)42)', 'int'],
    ['builtin_ctzl', '__builtin_ctzl((unsigned long)42)', 'int'],
    ['builtin_ctzll', '__builtin_ctzll((unsigned long long)42)', 'int'],
    ['builtin_copysignl', '__builtin_copysignl((long double)42, (long double) 42)', 'long double'],
    ['builtin_copysign', '__builtin_copysign(42, 42)', 'double'],
    ['builtin_isinfl', '__builtin_isinfl((long double)42)', 'int'],
    ['builtin_isinf', '__builtin_isinf((long double)42)', 'int'],
    ['builtin_isnanl', '__builtin_isnanl((long double)42)', 'int'],
    ['builtin_isnan', '__builtin_isnan((long double)42)', 'int'],
    ['builtin_finitel', '__builtin_finitel((long double)42)', 'int'],
    ['builtin_isfinite', '__builtin_isfinite((long double)42)', 'int'],
    ['builtin_issignalingl', '__builtin_issignalingl((long double)42)', 'int'],
    ['builtin_mul_overflow', '__builtin_mul_overflow(1, 2, &u)', 'int'],
    ['builtin_add_overflow', '__builtin_add_overflow(1, 2, &u)', 'int'],
    ['builtin_expect', '__builtin_expect(1 > 0)', 'int'],
  ]
endif

foreach builtin : builtins
  builtin_template='''
static int foo(@1@ i __attribute__((unused))) { return 0; }
unsigned u;
int main(void) { return foo(@0@); }
'''
  builtin_code = builtin_template.format(builtin[1], builtin[2])
  have_current_builtin = cc.links(builtin_code, name : 'test for __' + builtin[0], args: c_args)
  if have_current_builtin
    conf_data.set10('__HAVE_' + builtin[0].to_upper(), have_current_builtin, description: 'The compiler supports __' + builtin[0])
  else
    conf_data.set('__HAVE_' + builtin[0].to_upper(), have_current_builtin, description: 'The compiler supports __' + builtin[0])
  endif
endforeach

NEWLIB_VERSION='4.3.0'
NEWLIB_MAJOR_VERSION=4
NEWLIB_MINOR_VERSION=3
NEWLIB_PATCHLEVEL_VERSION=0

conf_data.set('__HAVE_CC_INHIBIT_LOOP_TO_LIBCALL',
	      cc.has_argument('-fno-tree-loop-distribute-patterns'),
	      description: 'Compiler flag to prevent detecting memcpy/memset patterns')

conf_data.set('__IO_LONG_LONG', io_long_long)
conf_data.set('__IO_MINIMAL_LONG_LONG', minimal_io_long_long)
conf_data.set('__FAST_BUFIO', fast_bufio)
conf_data.set('__FSTAT_BUFSIZ', get_option('fstat-bufsiz'))
conf_data.set('__IO_POS_ARGS', io_pos_args)
conf_data.set('__IO_C99_FORMATS', io_c99_formats)
conf_data.set('__IO_FLOAT_EXACT', io_float_exact)
conf_data.set('__IO_PERCENT_B', io_percent_b)
conf_data.set('__IO_LONG_DOUBLE', io_long_double)
conf_data.set('__IO_WCHAR', io_wchar)
conf_data.set('__ASSERT_VERBOSE', get_option('assert-verbose'), description: 'assert() is verbose by default')
conf_data.set('__SINGLE_THREAD', get_option('single-thread'), description: 'Disable multi-thread support')
conf_data.set('__HAVE_FCNTL', get_option('have-fcntl'), description: 'System provides fcntl function')
conf_data.set('__MALLOC_CLEAR_FREED', malloc_clear_freed)
if internal_heap != 0
  conf_data.set('__INTERNAL_HEAP', internal_heap)
endif
conf_data.set('__IEEE_LIBM', not get_option('want-math-errno'), description: 'math library does not set errno (offering only ieee semantics)')
conf_data.set('__MATH_ERRNO', get_option('want-math-errno'), description: 'math library sets errno')
conf_data.set('__PREFER_SIZE_OVER_SPEED', get_option('optimization') == 's', description: 'Optimize for space over speed')
conf_data.set('__FAST_STRCMP', fast_strcmp, description: 'Always optimize strcmp for performance')
conf_data.set('__GLOBAL_ERRNO', get_option('newlib-global-errno'), description: 'use global errno variable')
conf_data.set('__INIT_FINI_ARRAY', get_option('initfini-array'), description: 'Support INIT_ARRAY linker sections')
conf_data.set('__INIT_FINI_FUNCS', get_option('initfini'), description: 'Support _init() and _fini() functions')
conf_data.set('__THREAD_LOCAL_STORAGE', thread_local_storage, description: 'use thread local storage')
conf_data.set('__THREAD_LOCAL_STORAGE_API', thread_local_storage and have_picolibc_tls_api, description: '_set_tls and _init_tls functions available')
conf_data.set('__THREAD_LOCAL_STORAGE_RP2040', get_option('tls-rp2040'),
              description: 'Use Raspberry Pi RP2040 CPUID register to index thread local storage value')
conf_data.set('__THREAD_LOCAL_STORAGE_STACK_GUARD', tls_stack_guard, description: 'use thread local storage for stack protection canary')
conf_data.set('__HAVE_BITFIELDS_IN_PACKED_STRUCTS', have_bitfields_in_packed_structs, description: 'Use bitfields in packed structs')
conf_data.set('__HAVE_COMPLEX', have_complex, description: 'Compiler supports _Complex')

conf_data.set('__MB_CAPABLE', mb_capable)
if mb_capable
  conf_data.set('__MB_EXTENDED_CHARSETS_UCS', mb_ucs_charsets)
  conf_data.set('__MB_EXTENDED_CHARSETS_ISO', mb_iso_charsets)
  conf_data.set('__MB_EXTENDED_CHARSETS_WINDOWS', mb_windows_charsets)
  conf_data.set('__MB_EXTENDED_CHARSETS_JIS', mb_jis_charsets)
endif

conf_data.set('__TINY_STDIO', true, description: 'Picolibc stdio comes from the avr C library')
conf_data.set('__IO_DEFAULT', '\'@0@\''.format(format_default),
              description: 'The default printf and scanf variants')
conf_data.set('__IO_SMALL_ULTOA',
              printf_small_ultoa,
              description: 'avoid software division in decimal conversion')
conf_data.set('__IO_PERCENT_N',
              printf_percent_n,
              description: 'support %n in printf format strings')
conf_data.set('__ATOMIC_UNGETC',
              atomic_ungetc,
              description: 'Use atomics for fgetc/ungetc for re-entrancy')
conf_data.set('__STDIO_LOCKING',
              stdio_locking,
              description: 'Perform POSIX-conforming file locking for all stdio operations')

if enable_picocrt
  conf_data.set('__PICOCRT_ENABLE_MMU', picocrt_enable_mmu,
                description: 'Turn on mmu in picocrt startup code')
  conf_data.set('__PICOCRT_RUNTIME_SIZE',
	        get_option('crt-runtime-size'),
	        description: 'Compute static memory area sizes at runtime instead of link time')
endif

if use_stdlib
  conf_data.set('__USE_SYSTEM_LIBC', true)
endif
if profile
  conf_data.set('__ENABLE_PROFILING', true)
endif

errno_function=get_option('errno-function')
if errno_function == 'auto'
  errno_function = 'false'
  # these symbols are from the glibc and os x system C libraries,
  # it's useful to access them when doing testing
  foreach e : ['__errno_location', '__error']
    code = '''extern int @0@();
int main(void) {
	return @0@();
}
'''.format(e)
    if cc.links(code, name : 'detect errno function')
      errno_function = e
      break
    endif
  endforeach
elif errno_function == 'zephyr'
  if thread_local_storage
    errno_function = 'false'
  else
    errno_function = 'z_errno_wrap'
  endif
endif

if errno_function != 'false'
  conf_data.set('__PICOLIBC_ERRNO_FUNCTION', errno_function)
endif

if obsolete_math == 'auto'
  obsolete_math_value = false
elif obsolete_math == 'true'
  obsolete_math_value = 1
elif obsolete_math == 'false'
  obsolete_math_value = 0
endif

if obsolete_math_float == 'auto'
  obsolete_math_float_value = false
elif obsolete_math_float == 'true'
  obsolete_math_float_value = 1
elif obsolete_math_float == 'false'
  obsolete_math_float_value = 0
endif

if obsolete_math_double == 'auto'
  obsolete_math_double_value = false
elif obsolete_math_double == 'true'
  obsolete_math_double_value = 1
elif obsolete_math_double == 'false'
  obsolete_math_double_value = 0
endif

conf_data.set('__OBSOLETE_MATH', obsolete_math_value, description: 'Use old math code (undef auto, 0 no, 1 yes)')
conf_data.set('__OBSOLETE_MATH_FLOAT', obsolete_math_float_value, description: 'Use old math code for float funcs (undef auto, 0 no, 1 yes)')
conf_data.set('__OBSOLETE_MATH_DOUBLE', obsolete_math_double_value, description: 'Use old math code for double funcs (undef auto, 0 no, 1 yes)')

# Check if compiler has -fno-builtin

arg_fnobuiltin = []
if cc.has_argument('-fno-builtin')
  arg_fnobuiltin = ['-fno-builtin']
endif

arg_fnolto = []
if cc.has_argument('-fno-lto')
  arg_fnolto = ['-fno-lto']
endif

version_array = meson.project_version().split('.')

if version_array.length() > 2
  picolibc_patch_level = version_array[2]
else
  picolibc_patch_level = 0
endif

conf_data.set('__PICOLIBC_VERSION__', '"@0@"'.format(meson.project_version()), description: 'The Picolibc version in string format.')
conf_data.set('__PICOLIBC__', version_array[0], description: 'The Picolibc major version number.')
conf_data.set('__PICOLIBC_MINOR__', version_array[1], description: 'The Picolibc minor version number.')
conf_data.set('__PICOLIBC_PATCHLEVEL__', picolibc_patch_level, description: 'The Picolibc patch level.')

conf_data.set('__NEWLIB_VERSION__', '"@0@"'.format(NEWLIB_VERSION), description: 'The newlib version in string format.')
# The macro below is used in the LLVM's libcxx code base, so we define it for compatibility
conf_data.set('_NEWLIB_VERSION', '"@0@"'.format(NEWLIB_VERSION), description: 'The newlib version in string format.')
conf_data.set('__NEWLIB__', NEWLIB_MAJOR_VERSION, description: 'The newlib major version number.')
conf_data.set('__NEWLIB_MINOR__', NEWLIB_MINOR_VERSION, description: 'The newlib minor version number.')
conf_data.set('__NEWLIB_PATCHLEVEL__', NEWLIB_PATCHLEVEL_VERSION, description: 'The newlib patch level.')

inc_dirs = ['libc/stdio', 'libc/locale', '.', 'libc/include']

machine_dir = 'libc/machine' / host_cpu_family
if fs.is_dir(machine_dir)
  machine_inc_dir = machine_dir / 'include'
  if fs.is_dir(machine_inc_dir)
    inc_dirs = [machine_inc_dir] + inc_dirs
  endif
  inc_dirs = [machine_dir] + inc_dirs
endif

inc = include_directories(inc_dirs)

inc_args = []
foreach inc_dir : inc_dirs + [meson.current_build_dir()]
  inc_args += '-I' + meson.current_source_dir() / inc_dir
endforeach

# libc will adjust this if supported
ieeefp_funcs = false

# semihost will adjust these if supported

# any kind of semihosting support, enough to run general
has_semihost = false

# arm compatible semihosting, enough to run arm semihost tests
has_arm_semihost = false

crt0_test = 'crt0_hosted'
oslib_test = 'lib_semihost'

# some semihost needs bios to run the tests with qemu
bios_bin = []

# make sure to include semihost BEFORE picocrt!
if enable_semihost
  subdir('semihost')
endif

conf_data.set('__SEMIHOST', has_semihost, description: 'Semihost APIs supported')
conf_data.set('__ARM_SEMIHOST', has_arm_semihost, description: 'ARM Semihost APIs supported')

# By default, tests don't require any special arguments

# disable compiler built-ins so we don't use native equivalents
native_c_args += arg_fnobuiltin
if tests_enable_stack_protector
  if cc.has_argument('-fstack-protector-all') and cc.has_argument('-fstack-protector-strong')
    test_c_args += ['-fstack-protector-all', '-fstack-protector-strong', '-DTESTS_ENABLE_STACK_PROTECTOR']
  else
    tests_enable_stack_protector = false
  endif
endif

if not tests_enable_stack_protector
  test_c_args += cc.get_supported_arguments(['-fno-stack-protector'])
endif

if get_option('test-long-double')
  test_c_args += ['-D_TEST_LONG_DOUBLE']
  native_c_args += ['-D_TEST_LONG_DOUBLE']
endif

test_env = environment()
test_env.prepend('PATH', meson.current_source_dir() / 'scripts')

if has_semihost
  foreach params : [default_target] + targets

    target = params['name']
    target_dir = params['dir']
    target_c_args = params['c_args']
    if target == 'default-target'
      test_link_args_variable = 'test_link_args'
      test_link_args_noflash_variable = 'test_link_args_noflash'
      test_linker_files_variable = 'test_linker_files'
      test_link_depends_variable = 'test_link_depends'
      test_link_depends_noflash_variable = 'test_link_depends_noflash'
      picolibc_ld_value = picolibc_ld

      if thread_local_storage
        test_inittls_link_args_variable = 'test_inittls_link_args'
        test_inittls_linker_files_variable = 'test_inittls_linker_files'
        test_inittls_link_depends_variable = 'test_inittls_link_depends'
        picolibc_inittls_ld_value = picolibc_inittls_ld
      endif

    else
      test_link_args_variable = 'test_link_args' + target
      test_link_args_noflash_variable = 'test_link_args_noflash' + target
      test_linker_files_variable = 'test_linker_files_' + target
      test_link_depends_variable = 'test_link_depends' + target
      test_link_depends_noflash_variable = 'test_link_depends_noflash' + target
      picolibc_ld_value = get_variable('picolibc' + target + '_ld', picolibc_ld)

      if thread_local_storage
        test_inittls_link_args_variable = 'test_inittls_link_args' + target
        test_inittls_linker_files_variable = 'test_inittls_linker_files_' + target
        test_inittls_link_depends_variable = 'test_inittls_link_depends' + target
        picolibc_inittls_ld_value = get_variable('picolibc_inittls_' + target + '_ld', picolibc_inittls_ld)
      endif

    endif

    if meson.version().version_compare('>=1.4.0')
      picolibc_ld_string = picolibc_ld_value.full_path()
      if thread_local_storage
        picolibc_inittls_ld_string = picolibc_inittls_ld_value.full_path()
      endif
    else
      picolibc_ld_string = '@0@'.format(picolibc_ld_value)
      if thread_local_storage
        picolibc_inittls_ld_string = '@0@'.format(picolibc_inittls_ld_value)
      endif
    endif

    set_variable(test_link_args_variable,
                 additional_libs_list +
		 test_link_args_base +
                 ['-nostdlib', '-T', picolibc_ld_string])

    set_variable(test_linker_files_variable, [picolibc_ld_value])

    # Make sure all of the tests get re-linked if the linker scripts change.
    set_variable(test_link_depends_variable, [picolibc_ld_value])

    if thread_local_storage
      set_variable(test_inittls_link_args_variable,
                   additional_libs_list +
		   test_link_args_base +
                   ['-nostdlib', '-T', picolibc_inittls_ld_string])

      set_variable(test_inittls_linker_files_variable, [picolibc_ld_value])

      # Make sure all of the tests get re-linked if the linker scripts change.
      set_variable(test_inittls_link_depends_variable, [picolibc_inittls_ld_value])
    endif
    # Prepare no-flash link args and depends using the noflash script
    if target == 'default-target'
      picolibc_noflash_ld_value = picolibc_noflash_ld
    else
      picolibc_noflash_ld_value = get_variable('picolibc' + target + '_noflash_ld', picolibc_noflash_ld)
    endif
    if meson.version().version_compare('>=1.4.0')
      picolibc_noflash_ld_string = picolibc_noflash_ld_value.full_path()
    else
      picolibc_noflash_ld_string = '@0@'.format(picolibc_noflash_ld_value)
    endif
    set_variable(test_link_args_noflash_variable,
                 additional_libs_list +
                 test_link_args_base +
                 ['-nostdlib', '-T', picolibc_noflash_ld_string])
    set_variable(test_link_depends_noflash_variable, [picolibc_noflash_ld_value])
  endforeach
  test_noflash = true
else
  test_link_args = test_link_args_base
  test_link_depends = []
  test_noflash = false
endif

# make sure to include semihost BEFORE picocrt!
if enable_picocrt
  subdir('picocrt')
endif

# Build the C library

subdir('libc')
subdir('libm')

libnames = ['cpart', 'mpart']

libsrcs = []

foreach libname : libnames
  libsrcs += get_variable('src_' + libname, [])
endforeach

foreach params : targets
  target = params['name']
  target_dir = params['dir']
  target_c_args = params['c_args']

  libobjs = []
  libsrcs_target = libsrcs

  foreach libname : libnames
    if is_variable('lib_' + libname + target)
      libobjs += get_variable('lib_' + libname + target).extract_all_objects(recursive:true)
    endif
    libsrcs_target += get_variable('src_' + libname + target, [])
  endforeach

  instdir = join_paths(lib_dir, target_dir)

  libc_name = 'c'

  if meson.version().version_compare('>=1.10')
    local_lib_c_target = static_library(libc_name,
				        libsrcs_target,
                                        build_subdir : target_dir,
				        install : really_install,
				        install_dir : instdir,
				        pic: false,
				        objects : libobjs,
				        include_directories: inc,
				        c_args: target_c_args + c_args)

  else
    target_lib_prefix = params['lib_prefix']

    local_lib_c_target = static_library(join_paths(target_dir, target_lib_prefix + libc_name),
				        libsrcs_target,
				        install : really_install,
				        install_dir : instdir,
				        pic: false,
				        objects : libobjs,
				        include_directories: inc,
				        c_args: target_c_args + c_args)
  endif

  set_variable('lib_c' + target, local_lib_c_target)

  if check_duplicate_names
    custom_target('libc_duplicates' + target,
		  build_by_default: true,
		  input: local_lib_c_target,
		  output: 'libc_duplicates' + target,
		  command: [duplicate_names, nm, '@INPUT@', '@OUTPUT@'],
		  install: false)
  endif
endforeach

if enable_tests
  subdir('test')
  subdir('libm/test')
endif

subdir('empty-libs')

conf_data.set('__IEEEFP_FUNCS', ieeefp_funcs, description: 'IEEE fp funcs available')

configure_file(output : 'picolibc.h',
	       configuration: conf_data,
	       install_dir: include_dir,
	       install: really_install)

# Usage as an embedded subproject:
# If picolibc is embedded into the source as a subproject,
# provide a dependency to be used by the main project:
#   dependency('libc', fallback: ['picolibc', 'libc_dep'])
if not enable_multilib and meson.is_subproject()
  picolibc_dep = declare_dependency(include_directories: inc, link_with: [lib_c])
endif
