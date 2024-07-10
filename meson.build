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
	meson_version : '>= 0.53',
	version: '1.8.9'
       )

fs = import('fs')

cc = meson.get_compiler('c')

# Find the compiler installation directory by parsing the output of
# cc -print-search-dirs

cc_install_dir = ''
foreach _line : run_command(cc.cmd_array() + ['-print-search-dirs'], check : false).stdout().split('\n')
  if _line.startswith('install: ')
    if meson.version().version_compare('>=0.56')
      # trim off the leading 'install: '
      cc_install_dir = _line.substring(9)
    else
      cc_install_dir = run_command(['expr', _line, ':', 'install: *\(.*\)'], check : false).stdout().split('\n')[0]
    endif
  endif
endforeach

# these options are required for all C compiler operations, including
# detecting many C compiler features, as we cannot expect there
# to be a working C library on the system.

core_c_args = []
if not get_option('use-stdlib')
  core_c_args += cc.get_supported_arguments(['-nostdlib'])
endif

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
if not fs.is_file('newlib/libc/machine' / host_cpu_family / 'meson.build')
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
tests_enable_stack_protector = get_option('tests-enable-stack-protector')
tests_enable_full_malloc_stress = get_option('tests-enable-full-malloc-stress')
tests_enable_posix_io = get_option('tests-enable-posix-io')

# C++ is only used in tests, so only check for it when tests are enabled
if enable_tests
  have_cplusplus = add_languages('cpp', required: false)
  if have_cplusplus
    cpp = meson.get_compiler('cpp')
  endif
else
  have_cplusplus = false
endif

newlib_atexit_dynamic_alloc = get_option('newlib-atexit-dynamic-alloc')
newlib_nano_malloc = get_option('newlib-nano-malloc')
nano_malloc_clear_freed = get_option('nano-malloc-clear-freed')

newlib_elix_level = get_option('newlib-elix-level')
c_args = core_c_args
native_common_args = ['-DNO_NEWLIB']

if get_option('profile')
  c_args += ['-pg', '-no-pie']
endif

if get_option('freestanding')
  c_args += ['-ffreestanding']
endif

if have_cplusplus
  cpp_args = c_args
  cpp_flags = cpp.get_supported_arguments(['-fno-common', '-frounding-math', '-fsignaling-nans',
				           '-Wno-unsupported-floating-point-opt',
				           '-fno-builtin-copysignl'])
  cpp_args += cpp_flags
  native_cpp_args = native_common_args + cpp_flags
endif

# Disable ssp and fortify source while building picolibc (it's enabled
# by default by the ubuntu native compiler)
c_flags = cc.get_supported_arguments(['-fno-common', '-frounding-math', '-fsignaling-nans',
				      '-Wno-unsupported-floating-point-opt',
				      '-fno-builtin-copysignl'])

c_sanitize_bounds_flags = cc.get_supported_arguments(['-fsanitize=bounds'])
c_sanitize_trap_flags = cc.get_supported_arguments(['-fsanitize-undefined-trap-on-error'])
c_sanitize_no_trap_flags = cc.get_supported_arguments(['-fno-sanitize-undefined-trap-on-error', '-static-libubsan'])
have_ubsan = c_sanitize_bounds_flags != []
c_args += c_flags
native_c_args = native_common_args + c_flags

sanitize_bounds = get_option('sanitize-bounds')
sanitize_trap_on_error = get_option('sanitize-trap-on-error')

if sanitize_bounds
  if not have_ubsan
    error('sanitize-bounds option selected but -fsanitize=bounds is not supported')
  endif
  c_args += c_sanitize_bounds_flags
  native_c_args += c_sanitize_bounds_flags
  if sanitize_trap_on_error
    c_args += c_sanitize_trap_flags
  else
    c_args += c_sanitize_no_trap_flags
  endif
endif

if cc.symbols_have_underscore_prefix()
  global_prefix = '_'
else
  global_prefix = ''
endif
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

newlib_obsolete_math = get_option('newlib-obsolete-math')
newlib_obsolete_math_float = get_option('newlib-obsolete-math-float')
newlib_obsolete_math_double = get_option('newlib-obsolete-math-double')

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

# Select exit code
picoexit = get_option('picoexit')

# Shared stdio options
io_long_long = get_option('io-long-long')
io_c99_formats = get_option('io-c99-formats')
io_pos_args = get_option('io-pos-args')
io_long_double = get_option('io-long-double')

# Select stdio implementation
tinystdio = get_option('tinystdio')

has_link_defsym = meson.get_cross_property('has_link_defsym',
                                           cc.has_link_argument('-Wl,--defsym=' + 'start=0') or
                                           cc.has_link_argument('-Wl,--defsym=' + '_start=0') or
                                           cc.has_link_argument('-Wl,--defsym=' + '__start=0') or
                                           cc.has_link_argument('-Wl,--defsym=' + '___start=0') )
has_link_alias = meson.get_cross_property('has_link_alias', cc.has_link_argument('-Wl,-alias,' + global_prefix + 'main,testalias'))
lib_gcc = meson.get_cross_property('libgcc', '-lgcc')
stack_symbol = meson.get_cross_property('stack_symbol', '__stack')

# tinystdio options
posix_console = tinystdio and get_option('posix-console')
io_float_exact = not tinystdio or get_option('io-float-exact')
atomic_ungetc = tinystdio and get_option('atomic-ungetc')
_line = get_option('format-default')
# Pick off just the leading character
if meson.version().version_compare('>=0.56')
  format_default = _line.substring(0, 1)
else
  format_default = run_command(['expr', 'substr', _line, '1', '1'], check : false).stdout().split('\n')[0]
endif

printf_aliases = get_option('printf-aliases')
io_percent_b = tinystdio and get_option('io-percent-b')
printf_small_ultoa = tinystdio and get_option('printf-small-ultoa')
printf_percent_n = tinystdio and get_option('printf-percent-n')
minimal_io_long_long = tinystdio and get_option('minimal-io-long-long')
fast_bufio = tinystdio and get_option('fast-bufio')
io_wchar = tinystdio and get_option('io-wchar')
stdio_locking = tinystdio and get_option('stdio-locking') and not get_option('single-thread')

foreach format : ['d', 'f', 'l', 'i', 'm']

  compile_variable = 'printf_compile_args_@0@'.format(format)
  if tinystdio and printf_aliases
    compile_value = ['-D_PICOLIBC_PRINTF=\'@0@\''.format(format),
                     '-D_PICOLIBC_SCANF=\'@0@\''.format(format)]
  else
    compile_value = []
  endif
  set_variable(compile_variable, compile_value)

  link_variable = 'printf_link_args_@0@'.format(format)
  link_value = compile_value
  if tinystdio and printf_aliases
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

if tinystdio and enable_tests and not (has_link_defsym or has_link_alias)
  # If the alias flag is not supported and we are building tests, emit an
  # error here to avoid surprising test failures.
  error('Symbol alias linker flag not supported - printf tests will fail!')
endif

# A bunch of newlib-stdio only options
newlib_fvwrite_in_streamio = get_option('newlib-fvwrite-in-streamio')
newlib_fseek_optimization = get_option('newlib-fseek-optimization')
newlib_nano_formatted_io = get_option('newlib-nano-formatted-io')
newlib_io_float = get_option('newlib-io-float')
newlib_stdio64 = get_option('newlib-stdio64')
newlib_wide_orient = get_option('newlib-wide-orient')
newlib_have_fcntl = get_option('newlib-have-fcntl')

# Check for a bunch of newlib-stdio only options and complain
# if they are selected while using tinystdio

if tinystdio
  if newlib_io_float
    error('tinystdio uses a run-time mechanism to select floating point io (newlib-io-float)')
  endif
  if newlib_fvwrite_in_streamio
    error('tinystdio has no fvwrite support (newlib-fvwrite-in-streamio)')
  endif
  if newlib_fseek_optimization
    error('tinystdio has no fseek support (newlib-fseek-optimization)')
  endif
  if newlib_nano_formatted_io
    error('tinystdio uses a run-time mechanism to select smaller printf code (newlib-nano-formatted-io)')
  endif
  if newlib_wide_orient
    error('tinystdio does not support the wide-orient option (newlib-wide-orient)')
  endif
else
  if posix_console
    error('newlib stdio does not support the posix-console option (posix-console)')
  endif
  if io_pos_args
    error('newlib stdio has bugs with positional arguments (io-pos-args)')
  endif	
endif

if host_cpu_family == ''
  host_cc_machine=run_command(cc.cmd_array() + ['-dumpmachine'], check : true).stdout().strip().split('-')
  host_cpu_family=host_cc_machine[0]
  message('Computed host_cpu_family as ' + host_cpu_family)
endif

tls_model_spec = ''
thread_local_storage = false
thread_local_storage_option = get_option('thread-local-storage')
have_picolibc_tls_api = fs.is_file('newlib/libc/picolib/machine' / host_cpu_family / 'tls.c')
if thread_local_storage_option == 'auto' or thread_local_storage_option == 'picolibc'
  # We assume that _set_tls() is defined in the arch specific tls.c
  if thread_local_storage_option == 'auto' or have_picolibc_tls_api
    thread_local_storage = not cc.has_function('__emutls_get_address', args: core_c_args + [lib_gcc])
  endif
else
  thread_local_storage = get_option('thread-local-storage') == 'true'
endif
if thread_local_storage
  tls_model_spec = '%{!ftls-model:-ftls-model=' + get_option('tls-model') + '}'
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
have_tls_stack_guard = fs.is_file('newlib/libc/machine' / host_cpu_family / 'machine/_ssp_tls.h')
if have_tls_stack_guard
  c_stack_guard_tls_flags = cc.get_supported_arguments(['-mstack-protector-guard=tls'])
  if c_stack_guard_tls_flags == []
    have_tls_stack_guard = false
  endif
endif
tls_stack_guard = prefer_tls_stack_guard and have_tls_stack_guard
if tls_stack_guard
  stack_guard_spec = '%{!mstack-protector-guard:-mstack-protector-guard=tls}'
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
  endif
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
additional_libs_list = meson.get_cross_property('additional_libs', [])

compiler_id = cc.get_id()
if compiler_id == 'ccomp'
  # When passing the specs file to CompCert, the libcompcert needs to be included there as well
  additional_libs_list += '-lcompcert'
endif

additional_libs = ' '.join(additional_libs_list)

if compiler_id == 'gcc' and target_machine.cpu_family() == 'msp430'
  # Extract -mhwmult-selection-snippet from GCC.
  dumped_specs = run_command(cc.cmd_array() + '-dumpspecs').stdout()
  hwmult_start = dumped_specs.split('%{mhwmult=auto')[1]
  hwmult_snippet = '%{mhwmult=auto' + hwmult_start.split('-lc')[0]
  additional_libs += hwmult_snippet
endif

specs_extra = ''
specs_extra_list = meson.get_cross_property('specs_extra', [])
if specs_extra_list != []
  specs_extra = '\n' + '\n'.join(specs_extra_list)
endif

# Handle old-style printf/scanf selection mechanism in the specs file

specs_printf = ''
if tinystdio and printf_aliases
  specs_printf=('%{DPICOLIBC_DOUBLE_PRINTF_SCANF:--defsym=vfprintf=@0@__d_vfprintf}' +
		' %{DPICOLIBC_DOUBLE_PRINTF_SCANF:--defsym=vfscanf=@0@__d_vfscanf}' +
                ' %{DPICOLIBC_FLOAT_PRINTF_SCANF:--defsym=vfprintf=@0@__f_vfprintf}' +
		' %{DPICOLIBC_FLOAT_PRINTF_SCANF:--defsym=vfscanf=@0@__f_vfscanf}' +
		' %{DPICOLIBC_LONG_LONG_PRINTF_SCANF:--defsym=vfprintf=@0@__l_vfprintf}' +
		' %{DPICOLIBC_LONG_LONG_PRINTF_SCANF:--defsym=vfscanf=@0@__l_vfscanf}' +
		' %{DPICOLIBC_INTEGER_PRINTF_SCANF:--defsym=vfprintf=@0@__i_vfprintf}' +
		' %{DPICOLIBC_INTEGER_PRINTF_SCANF:--defsym=vfscanf=@0@__i_vfscanf}' +
		' %{DPICOLIBC_MINIMAL_PRINTF_SCANF:--defsym=vfprintf=@0@__m_vfprintf}' +
		' %{DPICOLIBC_MINIMAL_PRINTF_SCANF:--defsym=vfscanf=@0@__m_vfscanf}').format(global_prefix)
endif

crt0_expr = '%{-crt0=*:crt0-%*%O%s; :crt0%O%s}'

if system_libc
  specs_isystem = ''
  specs_libpath = ''
  specs_startfile = crt0_expr
else

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
  # Prepend the multilib -L option to the non-multilib option
  #
  specs_libpath = specs_multilibpath + ' ' + specs_libpath

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
specs_data.set('SPECS_LIBPATH', specs_libpath)
specs_data.set('SPECS_STARTFILE', specs_startfile)
specs_data.set('TLSMODEL', tls_model_spec)
specs_data.set('STACKGUARD', stack_guard_spec)
specs_data.set('LINK_SPEC', meson.get_cross_property('link_spec', ''))
specs_data.set('CC1_SPEC', meson.get_cross_property('cc1_spec', ''))
specs_data.set('CC1PLUS_SPEC', meson.get_cross_property('cc1plus_spec', ''))
specs_data.set('ADDITIONAL_LIBS', additional_libs)
specs_data.set('SPECS_EXTRA', specs_extra)
specs_data.set('SPECS_PRINTF', specs_printf)

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

picolibc_specs = configure_file(input: 'picolibc.specs.in',
				output: 'picolibc.specs',
				configuration: specs_c_data,
				install_dir: specs_dir,
				install: specs_install)

picolibc_specs_name = 'picolibc.specs'

picolibcpp_specs = configure_file(input: 'picolibc.specs.in',
				  output: 'picolibcpp.specs',
				  configuration: specs_cpp_data,
				  install_dir: specs_dir,
				  install: specs_install)

# Not all compilers necessarily support all warnings; only use these which are:
common_warnings = [
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
]
if get_option('analyzer')
  common_warnings += [
    '-fanalyzer',
    ]
endif
c_warnings = common_warnings + [
  '-Werror=implicit-function-declaration',
  '-Wold-style-definition',
  '-Wno-implicit-int',
]
c_flags = cc.get_supported_arguments(c_warnings)
c_args += c_flags
native_c_args += c_flags

# Make sure _LIBC is not defined when building tests
test_c_args = c_args
c_args += ['-D_LIBC', '-U_FORTIFY_SOURCE']

# Select a fortify source option
fortify_source = get_option('fortify-source')
if fortify_source == 'none'
  test_fortify_arg = '-U_FORTIFY_SOURCE'
else
  test_fortify_arg = '-D_FORTIFY_SOURCE=' + fortify_source
endif
test_c_args += [test_fortify_arg]

c_args += cc.get_supported_arguments(['-fno-stack-protector'])

if have_cplusplus
  cpp_warnings = common_warnings
  cpp_flags = cpp.get_supported_arguments(cpp_warnings)
  cpp_args += cpp_flags
  cpp_flags = cpp.get_supported_arguments(['-fno-exceptions', '-fno-unwind-tables', '-fno-stack-protector'])
  cpp_args += cpp_flags
  test_cpp_args = cpp_args + [test_fortify_arg]
endif

# CompCert does not support bitfields in packed structs, so avoid using this optimization
bitfields_in_packed_structs_code = '''
struct test { int part: 24; } __attribute__((packed));
unsigned int foobar (const struct test *p) { return p->part; }
'''
have_bitfields_in_packed_structs = cc.compiles(bitfields_in_packed_structs_code, name : 'packed structs may contain bitfields', args: core_c_args)

# CompCert does not have __builtin_mul_overflow
builtin_mul_overflow_code = '''
#include <stddef.h>
int overflows (size_t a, size_t b) { size_t x; return __builtin_mul_overflow(a, b, &x); }
volatile size_t aa = 42;
int main (void) { return overflows(aa, aa); }
'''
have_builtin_mul_overflow = cc.links(builtin_mul_overflow_code, name : 'has __builtin_mul_overflow', args: core_c_args)

# CompCert does not have __builtin_add_overflow
builtin_add_overflow_code = '''
#include <stddef.h>
int overflows (size_t a, size_t b) { size_t x; return __builtin_add_overflow(a, b, &x); }
volatile size_t aa = 42;
int main (void) { return overflows(aa, aa); }
'''
have_builtin_add_overflow = cc.links(builtin_add_overflow_code, name : 'has __builtin_add_overflow', args: core_c_args)

# CompCert does not support _Complex
complex_code = '''
float _Complex test(float _Complex z) { return z; }
'''
have_complex = cc.compiles(complex_code, name : 'supports _Complex', args: core_c_args)

builtin_complex_code = '''
#include <stddef.h>
double _Complex test(double r, double i) { return __builtin_complex(r, i); }
'''

have_builtin_complex = have_complex and cc.compiles(builtin_complex_code, name : 'supports __builtin_complex', args: core_c_args)

# CompCert does not have __builtin_expect
builtin_expect_code = '''
volatile int a = 42;
int main (void) {
  return __builtin_expect(a, 1);
}
'''
have_builtin_expect = cc.links(builtin_expect_code, name : 'has __builtin_expect', args: core_c_args)

werror_c_args = core_c_args + cc.get_supported_arguments('-Werror')

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
    flags = []

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
		flags += ['-mcustom-fmuls=252', '-mcustom-fadds=253',
			  '-mcustom-fsubs=254']
	      elif flag == 'mcustom-fpu-cfg=60-2'
		flags += ['-mcustom-fmuls=252', '-mcustom-fadds=253',
			  '-mcustom-fsubs=254', '-mcustom-fdivs=255']
	      elif flag == 'mcustom-fpu-cfg=72-3'
		flags += ['-mcustom-floatus=243', '-mcustom-fixsi=244',
			  '-mcustom-floatis=245','-mcustom-fcmpgts=246',
			  '-mcustom-fcmples=249', '-mcustom-fcmpeqs=250',
			  '-mcustom-fcmpnes=251', '-mcustom-fmuls=252',
			  '-mcustom-fadds=253', '-mcustom-fsubs=254',
			  '-mcustom-fdivs=255']
	      else
		flags += '-' + flag
	      endif
	    else
	      flags += '-' + flag
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
	flags += [ '-mcmodel=medany' ]
      endif

      # Add any extra flags for this target from the cross file
      flags += meson.get_cross_property('c_args_' + name.strip('_'), [])

      targets += {
                  'name': name,
                  'dir': dir,
                  'c_args': flags,
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

foreach params : [default_target] + targets
  
  target = params['name']
  target_dir = params['dir']
  target_c_args = params['c_args']
  target_lib_prefix = params['lib_prefix']

  custom_mem_config = ''

  if target != 'default-target'
    target_part = ''
    foreach part : target.split('_')
      if part != ''
        target_part += '_' + part
        custom_mem_config = meson.get_cross_property('custom_mem_config' + target_part, custom_mem_config)
      endif
    endforeach

    target_head = target_dir.split('_')[0].underscorify()
    custom_mem_config = meson.get_cross_property('custom_mem_config_' + target_head, custom_mem_config)
  endif

  custom_mem_config = meson.get_cross_property('custom_mem_config' + target, custom_mem_config)

  if target == 'default-target' or custom_mem_config != ''

    picolibc_linker_type_data = configuration_data()
    # We need to use different alignment flags for .tdata/.tbss for ld.bfd
    # (ALIGN_WITH_INPUT, which is not supported by ld.lld) and ld.lld
    # (ALIGN(__tls_align), which is rejected as non-constant by ld.bfd), so we use
    # the {BFD,LLD}_{START,END} templates to comment out the incompatible flag.
    if cc.get_linker_id() == 'ld.lld'
      picolibc_linker_type_data.set('BFD_START', '/* For ld.bfd: ')
      picolibc_linker_type_data.set('BFD_END', '*/')
      picolibc_linker_type_data.set('LLD_START', '')
      picolibc_linker_type_data.set('LLD_END', '')
      picolibc_linker_type_data.set('TLS_PHDRS', 'tls PT_TLS;')
      picolibc_linker_type_data.set('TLS_INIT_SEG', 'tls')
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
    else
      picolibc_linker_type_data.set('BFD_START', '')
      picolibc_linker_type_data.set('BFD_END', '')
      picolibc_linker_type_data.set('LLD_START', '/* For ld.lld: ')
      picolibc_linker_type_data.set('LLD_END', '*/')
      picolibc_linker_type_data.set('TLS_PHDRS', '''tls_init PT_TLS;
        tls PT_TLS;''')
      picolibc_linker_type_data.set('TLS_INIT_SEG', 'tls_init')
    endif

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
    default_init_section_template = init_section_template.format('@0@', '''
		KEEP (*(.text.init.enter))
		KEEP (*(.data.init.enter))
		KEEP (*(SORT_BY_NAME(.init) SORT_BY_NAME(.init.*)))''',
                                                                 '@1@',
                                                                 '@2@')

    additional_sections = []

    fallback_flash_addr = '0x10000000'
    fallback_flash_size = '0x00010000'

    if meson.get_cross_property('separate_boot_flash_' + custom_mem_config,
                                meson.get_cross_property('separate_boot_flash', false))
      boot_flash_addr = meson.get_cross_property(
        'default_boot_flash_addr_' + custom_mem_config,
        meson.get_cross_property(
          'default_boot_flash_addr',
          '0x10000000'))
      boot_flash_size = meson.get_cross_property(
        'default_boot_flash_size_' + custom_mem_config,
        meson.get_cross_property(
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
      additional_section_names = meson.get_cross_property('additional_sections_' + custom_mem_config,
                                                     meson.get_cross_property('additional_sections', []))
      if additional_section_names != []
        foreach section_name : additional_section_names
          default_content_list =  ['*.(.' + section_name + ')', '*.(.' + section_name + '.*)']
          content_list = meson.get_cross_property('default_' + section_name + '_contents', default_content_list)
          contents = ''
          foreach content : content_list
            contents += '''
		@0@'''.format(content)
          endforeach
          additional_sections += [
            {
              'name' : section_name,
              'addr' : meson.get_cross_property('default_' + section_name + '_addr'),
              'size' : meson.get_cross_property('default_' + section_name + '_size'),
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
      init_memory = ''
      init_sections = default_init_section_template.format('init', 'flash', 'text')
    endif

    picolibc_linker_type_data.set('INIT_MEMORY', init_memory)
    picolibc_linker_type_data.set('INIT_PHDRS', init_phdrs)
    picolibc_linker_type_data.set('INIT_SECTIONS', init_sections)

    picolibc_linker_type_data.set(
      'DEFAULT_FLASH_ADDR', meson.get_cross_property(
        'default_flash_addr_' + custom_mem_config,
        meson.get_cross_property(
          'default_flash_addr',
          fallback_flash_addr)))

    picolibc_linker_type_data.set(
      'DEFAULT_FLASH_SIZE', meson.get_cross_property(
        'default_flash_size_' + custom_mem_config,
        meson.get_cross_property(
          'default_flash_size',
          fallback_flash_size)))

    picolibc_linker_type_data.set(
      'DEFAULT_RAM_ADDR',
      meson.get_cross_property('default_ram_addr_' + custom_mem_config,
                               meson.get_cross_property('default_ram_addr',
                                                        '0x20000000')))

    picolibc_linker_type_data.set(
      'DEFAULT_RAM_SIZE',
      meson.get_cross_property('default_ram_size_' + custom_mem_config,
                               meson.get_cross_property('default_ram_size',
                                                        '0x00008000')))

    picolibc_linker_type_data.set(
      'DEFAULT_STACK_SIZE',
      meson.get_cross_property('default_stack_size_' + custom_mem_config,
                               meson.get_cross_property('default_stack_size',
                                                        '0x00001000')))

    picolibc_linker_type_data.set(
      'DEFAULT_ALIGNMENT',
      meson.get_cross_property('default_alignment_' + custom_mem_config,
                               meson.get_cross_property('default_alignment',
                                                        '8')))
    picolibc_linker_type_data.set('STACK', stack_symbol)

    picolibc_linker_type_data.set('EXTRA_TEXT_SECTIONS',
                                  meson.get_cross_property('extra_text_sections', ''))
    picolibc_linker_type_data.set('EXTRA_RODATA_SECTIONS',
                                  meson.get_cross_property('extra_rodata_sections', ''))
    picolibc_linker_type_data.set('EXTRA_DATA_SECTIONS',
                                  meson.get_cross_property('extra_data_sections', ''))
    picolibc_linker_type_data.set('EXTRA_SDATA_SECTIONS',
                                  meson.get_cross_property('extra_sdata_sections', ''))
    picolibc_linker_type_data.set('EXTRA_BSS_SECTIONS',
                                  meson.get_cross_property('extra_bss_sections', ''))
    picolibc_linker_type_data.set('EXTRA_SBSS_SECTIONS',
                                  meson.get_cross_property('extra_sbss_sections', ''))

    picolibc_ld_data = configuration_data()
    picolibc_ld_data.merge_from(picolibc_linker_type_data)
    picolibc_ld_data.set('CPP_START', '/*')
    picolibc_ld_data.set('CPP_END', '*/')
    picolibc_ld_data.set('C_START', '')
    picolibc_ld_data.set('C_END', '')
    picolibc_ld_data.set('PREFIX', global_prefix)

    if target == 'default-target'
      picolibc_ld_file = 'picolibc.ld'
      picolibc_ld_variable = 'picolibc_ld'
      picolibc_ld_config_variable = 'picolibc_ld_config'
      picolibcpp_ld_file = 'picolibcpp.ld'
      picolibcpp_ld_variable = 'picolibcpp_ld'
      picolibcpp_ld_config_variable = 'picolibcpp_ld_config'
      picolibc_ld_install = true
    else
      picolibc_ld_file = 'picolibc_' + custom_mem_config + '.ld'
      picolibc_ld_variable = 'picolibc_' + target + '_ld'
      picolibc_ld_config_variable = 'picolibc_' + custom_mem_config + 'ld_config'
      picolibcpp_ld_file = 'picolibcpp_' + custom_mem_config + '.ld'
      picolibcpp_ld_variable = 'picolibcpp_' + target + '_ld'
      picolibcpp_ld_config_variable = 'picolibcpp_' + custom_mem_config + 'ld_config'
      picolibc_ld_install = false
    endif

    if not is_variable(picolibc_ld_config_variable)
      set_variable(picolibc_ld_config_variable,
                   configure_file(input: 'picolibc.ld.in',
                                  output: picolibc_ld_file,
                                  configuration: picolibc_ld_data,
                                  install: picolibc_ld_install,
                                  install_dir: lib_dir))
    endif

    set_variable(picolibc_ld_variable, get_variable(picolibc_ld_config_variable))

    picolibcpp_ld_data = configuration_data()
    picolibcpp_ld_data.merge_from(picolibc_linker_type_data)
    picolibcpp_ld_data.set('CPP_START', '')
    picolibcpp_ld_data.set('CPP_END', '')
    picolibcpp_ld_data.set('C_START', '/*')
    picolibcpp_ld_data.set('C_END', '*/')
    picolibcpp_ld_data.set('PREFIX', global_prefix)

    if not is_variable(picolibcpp_ld_config_variable)
      set_variable(picolibcpp_ld_config_variable,
                 configure_file(input: 'picolibc.ld.in',
                                output: picolibcpp_ld_file,
                                configuration: picolibcpp_ld_data,
                                install: picolibc_ld_install,
                                install_dir: lib_dir))
    endif
                   
    set_variable(picolibcpp_ld_variable, get_variable(picolibcpp_ld_config_variable))

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
  have_current_builtin = cc.links(builtin_code, name : 'test for __' + builtin[0], args: core_c_args)
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
conf_data.set('__IO_POS_ARGS', io_pos_args)
conf_data.set('__IO_C99_FORMATS', io_c99_formats)
conf_data.set('__IO_FLOAT_EXACT', io_float_exact)
conf_data.set('__IO_PERCENT_B', io_percent_b)
conf_data.set('__IO_LONG_DOUBLE', io_long_double)
conf_data.set('__IO_WCHAR', io_wchar)
conf_data.set('__ASSERT_VERBOSE', get_option('assert-verbose'), description: 'assert() is verbose by default')
conf_data.set('__SINGLE_THREAD', get_option('single-thread'), description: 'Disable multi-thread support')
conf_data.set('__HAVE_FCNTL', newlib_have_fcntl, description: 'System provides fcntl function')
conf_data.set('__NANO_MALLOC', newlib_nano_malloc, description: 'Provide smaller malloc implementation')
conf_data.set('__NANO_MALLOC_CLEAR_FREED', nano_malloc_clear_freed and newlib_nano_malloc)
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

conf_data.set('__PICO_EXIT', picoexit)
if not picoexit
  conf_data.set('_ATEXIT_DYNAMIC_ALLOC', newlib_atexit_dynamic_alloc)
  conf_data.set('_GLOBAL_ATEXIT', get_option('newlib-global-atexit'))
  conf_data.set('_WANT_REGISTER_FINI', get_option('newlib-register-fini'))
endif

conf_data.set('__TINY_STDIO', tinystdio, description: 'Use tiny stdio from gcc avr')
if tinystdio
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
else
  conf_data.set('__IO_NO_FLOATING_POINT', not newlib_io_float)
  conf_data.set('__IO_FLOATING_POINT', newlib_io_float)
  conf_data.set('__LARGE64_FILES', newlib_stdio64)
  conf_data.set('__ELIX_LEVEL', newlib_elix_level)
  conf_data.set('__FVWRITE_IN_STREAMIO', newlib_fvwrite_in_streamio)
  conf_data.set('__FSEEK_OPTIMIZATION', newlib_fseek_optimization)
  conf_data.set('__WIDE_ORIENT', newlib_wide_orient)
  conf_data.set('__UNBUF_STREAM_OPT', get_option('newlib-unbuf-stream-opt'))
  conf_data.set('__NANO_FORMATTED_IO', newlib_nano_formatted_io)
endif

if enable_picocrt
  conf_data.set('__PICOCRT_ENABLE_MMU', picocrt_enable_mmu,
                description: 'Turn on mmu in picocrt startup code')
  conf_data.set('__PICOCRT_RUNTIME_SIZE',
	        get_option('crt-runtime-size'),
	        description: 'Compute static memory area sizes at runtime instead of link time')
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

if newlib_obsolete_math == 'auto'
  obsolete_math_value = false
elif newlib_obsolete_math == 'true'
  obsolete_math_value = 1
elif newlib_obsolete_math == 'false'
  obsolete_math_value = 0
endif

if newlib_obsolete_math_float == 'auto'
  obsolete_math_float_value = false
elif newlib_obsolete_math_float == 'true'
  obsolete_math_float_value = 1
elif newlib_obsolete_math_float == 'false'
  obsolete_math_float_value = 0
endif

if newlib_obsolete_math_double == 'auto'
  obsolete_math_double_value = false
elif newlib_obsolete_math_double == 'true'
  obsolete_math_double_value = 1
elif newlib_obsolete_math_double == 'false'
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

if tinystdio
  stdio_inc_dir = 'newlib/libc/tinystdio'
else
  stdio_inc_dir = 'newlib/libc/stdio'
endif

inc_dirs = [stdio_inc_dir, 'newlib/libc/locale', '.', 'newlib/libc/include']

machine_dir = 'newlib/libc/machine' / host_cpu_family
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

# We don't need '-fdata-sections' currently as there aren't any
# files with data used in separate code paths. This works around
# versions of gcc for RISC-V which have a bug that mis-names
# initialized read-only data segments when -fdata-sections
# is defined
arguments = []
if cc.has_argument('-ffunction-sections')
  arguments += ['-ffunction-sections']
endif

if thread_local_storage
  tls_arg = '-ftls-model=' + get_option('tls-model')
  assert(cc.has_argument(tls_arg), 'Compiler does not support \'-ftls-model\'!')
  arguments += [tls_arg]
endif

add_project_arguments(arguments, language: 'c')

# libc will adjust this if supported
ieeefp_funcs = false

# semihost will adjust these if supported

# any kind of semihosting support, enough to run general
has_semihost = false

# arm compatible semihosting, enough to run arm semihost tests
has_arm_semihost = false

crt0_test = 'crt0_hosted'

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

# Meson version 0.53.2 doesn't check for the skip_sanity_check value
# before attempting to run test fragments during configuration. That
# means the exe_wrapper value needs to handle being run at that
# point. To support this, the exe_wrapper becomes a script which
# checks for a environment variable (PICOLIBC_TEST) and exits
# indicating success if that is not present.
#
# This was fixed in meson version 0.54.2, so this can be removed from
# here and the sample cross scripts once picolibc requires a version
# of meson newer than that.

test_env = environment({'PICOLIBC_TEST' : '1'})
test_env.prepend('PATH', meson.current_source_dir() / 'scripts')

test_link_defaults = cc.get_supported_link_arguments(['-Wl,--gc-sections', '-Wl,--no-warn-rwx-segments'])

if has_semihost
  foreach params : [default_target] + targets
    
    target = params['name']
    target_dir = params['dir']
    target_c_args = params['c_args']
    target_lib_prefix = params['lib_prefix']
    if target == 'default-target'
      test_link_args_variable = 'test_link_args'
      test_linker_files_variable = 'test_linker_files'
      test_link_depends_variable = 'test_link_depends'
      picolibc_ld_value = picolibc_ld
    else
      test_link_args_variable = 'test_link_args' + target
      test_linker_files_variable = 'test_linker_files_' + target
      test_link_depends_variable = 'test_link_depends' + target
      picolibc_ld_value = get_variable('picolibc_' + target + '_ld', picolibc_ld)
    endif

    if meson.version().version_compare('>=1.4.0')
      picolibc_ld_string = picolibc_ld_value.full_path()
    else
      picolibc_ld_string = '@0@'.format(picolibc_ld_value)
    endif

    set_variable(test_link_args_variable,
		 test_link_defaults + ['-nostdlib', '-T', picolibc_ld_string]
       + additional_libs_list
       + [lib_gcc])

    set_variable(test_linker_files_variable, [picolibc_ld_value])

    # Make sure all of the tests get re-linked if the linker scripts change.
    set_variable(test_link_depends_variable, [picolibc_ld_value])
  endforeach
else
  test_link_args = test_link_defaults + cc.get_supported_link_arguments(['-lgcc'])
  test_link_depends = []
endif

# make sure to include semihost BEFORE picocrt!
if enable_picocrt
  subdir('picocrt')
endif
subdir('newlib')

if enable_tests
  subdir('test')
endif

conf_data.set('__IEEEFP_FUNCS', ieeefp_funcs, description: 'IEEE fp funcs available')

configure_file(output : 'picolibc.h',
	       configuration: conf_data,
	       install_dir: include_dir)

# Usage as an embedded subproject:
# If picolibc is embedded into the source as a subproject,
# provide a dependency to be used by the main project:
#   dependency('libc', fallback: ['picolibc', 'libc_dep'])
if not enable_multilib and meson.is_subproject()
  picolibc_dep = declare_dependency(include_directories: inc, link_with: [lib_c])
endif
