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
	version: '1.8.6'
       )

targets = []

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
		       # arc
		       'arc64' : 'arc',
		       # cris
		       'crisv32' : 'cris',
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

# Find out about float types

long_double_code = '''
#include <float.h>
#ifndef __LDBL_MANT_DIG__
#error No long double support in float.h
#endif
long double test(void)
{
	long double ld = 0.0L;
	return ld;
}
'''

have_long_double = cc.compiles(long_double_code, name : 'long double check', args: core_c_args)

enable_multilib = get_option('multilib')
multilib_list = get_option('multilib-list')
multilib_exclude = get_option('multilib-exclude')
enable_picolib = get_option('picolib')
enable_picocrt = get_option('picocrt')
enable_picocrt_lib = get_option('picocrt-lib')
enable_semihost = get_option('semihost')
enable_tests = get_option('tests')
enable_native_tests = get_option('native-tests')
tests_enable_stack_protector = get_option('tests-enable-stack-protector')
tests_enable_full_malloc_stress = get_option('tests-enable-full-malloc-stress')
tests_enable_posix_io = get_option('tests-enable-posix-io')
have_alias_attribute_option = get_option('have-alias-attribute')
have_format_attribute_option = get_option('have-format-attribute')
have_weak_attribute_option = get_option('have-weak-attribute')

newlib_iconv_encodings = get_option('newlib-iconv-encodings')
newlib_iconv_encodings_exclude = get_option('newlib-iconv-encodings-exclude')
newlib_iconv_from_encodings = get_option('newlib-iconv-from-encodings')
newlib_iconv_to_encodings = get_option('newlib-iconv-to-encodings')
newlib_iconv_external_ccs = get_option('newlib-iconv-external-ccs')

newlib_atexit_dynamic_alloc = get_option('newlib-atexit-dynamic-alloc')
newlib_nano_malloc = get_option('newlib-nano-malloc')
lite_exit = get_option('lite-exit')

newlib_elix_level = get_option('newlib-elix-level')
c_args = ['-include', '@0@/@1@'.format(meson.current_build_dir(), 'picolibc.h')] + core_c_args
native_c_args = ['-DNO_NEWLIB']

if get_option('profile')
  c_args += ['-pg', '-no-pie']
endif

if get_option('freestanding')
  c_args += ['-ffreestanding']
endif

# Disable ssp and fortify source while building picolibc (it's enabled
# by default by the ubuntu native compiler)
c_flags = cc.get_supported_arguments(['-fno-common', '-frounding-math', '-fsignaling-nans',
				      '-Wno-unsupported-floating-point-opt', '-fno-stack-protector',
				      '-fno-builtin-copysignl'])

c_sanitize_bounds_flags = cc.get_supported_arguments(['-fsanitize=bounds'])
c_sanitize_trap_flags = cc.get_supported_arguments(['-fsanitize-undefined-trap-on-error'])
c_sanitize_no_trap_flags = cc.get_supported_arguments(['-fno-sanitize-undefined-trap-on-error', '-static-libubsan'])
have_ubsan = c_sanitize_bounds_flags != []
c_args += c_flags
native_c_args += c_flags

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

newlib_mb = get_option('newlib-mb')
newlib_locale_info = get_option('newlib-locale-info')

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

# Select exit code
picoexit = get_option('picoexit')

# Shared stdio options
io_long_long = get_option('io-long-long')
newlib_io_long_long = get_option('newlib-io-long-long')
if newlib_io_long_long != 'auto'
  io_long_long = newlib_io_long_long == 'true'
endif

io_c99_formats = get_option('io-c99-formats')
newlib_io_c99_formats = get_option('newlib-io-c99-formats')
if newlib_io_c99_formats != 'auto'
  io_c99_formats = newlib_io_c99_formats == 'true'
endif

io_pos_args = get_option('io-pos-args')
newlib_io_pos_args = get_option('newlib-io-pos-args')
if newlib_io_pos_args != 'auto'
  io_pos_args = newlib_io_pos_args == 'true'
endif

# Select stdio implementation
tinystdio = get_option('tinystdio')

has_link_defsym = meson.get_cross_property('has_link_defsym',
                                           cc.has_link_argument('-Wl,--defsym=' + 'start=0') or
                                           cc.has_link_argument('-Wl,--defsym=' + '_start=0') or
                                           cc.has_link_argument('-Wl,--defsym=' + '__start=0') or
                                           cc.has_link_argument('-Wl,--defsym=' + '___start=0') )
has_link_alias = meson.get_cross_property('has_link_alias', cc.has_link_argument('-Wl,-alias,' + global_prefix + 'main,testalias'))
lib_gcc = meson.get_cross_property('libgcc', '-lgcc')

# tinystdio options
posix_console = tinystdio and get_option('posix-console')
posix_io = tinystdio and (get_option('posix-io') or posix_console)
io_float_exact = not tinystdio or get_option('io-float-exact')
atomic_ungetc = tinystdio and get_option('atomic-ungetc')
format_default = get_option('format-default')
printf_aliases = get_option('printf-aliases')
io_percent_b = tinystdio and get_option('io-percent-b')
io_long_double = get_option('io-long-double') or get_option('newlib-io-long-double')
printf_small_ultoa = tinystdio and get_option('printf-small-ultoa')
minimal_io_long_long = tinystdio and get_option('minimal-io-long-long')
fast_bufio = tinystdio and get_option('fast-bufio')

if printf_aliases
  double_printf_compile_args=['-DPICOLIBC_DOUBLE_PRINTF_SCANF']
  float_printf_compile_args=['-DPICOLIBC_FLOAT_PRINTF_SCANF']
  llong_printf_compile_args=['-DPICOLIBC_LONG_LONG_PRINTF_SCANF']
  int_printf_compile_args=['-DPICOLIBC_INTEGER_PRINTF_SCANF']
  min_printf_compile_args=['-DPICOLIBC_MINIMAL_PRINTF_SCANF']
else
  double_printf_compile_args=[]
  float_printf_compile_args=[]
  llong_printf_compile_args=[]
  int_printf_compile_args=[]
  min_printf_compile_args=[]
endif
double_printf_link_args=double_printf_compile_args
float_printf_link_args=float_printf_compile_args
llong_printf_link_args=llong_printf_compile_args
int_printf_link_args=int_printf_compile_args
min_printf_link_args=min_printf_compile_args

if tinystdio and printf_aliases
  vfprintf_symbol = global_prefix + 'vfprintf'
  __d_vfprintf_symbol = global_prefix + '__d_vfprintf'
  __f_vfprintf_symbol = global_prefix + '__f_vfprintf'
  __l_vfprintf_symbol = global_prefix + '__l_vfprintf'
  __i_vfprintf_symbol = global_prefix + '__i_vfprintf'
  __m_vfprintf_symbol = global_prefix + '__m_vfprintf'
  vfscanf_symbol = global_prefix + 'vfscanf'
  __d_vfscanf_symbol = global_prefix + '__d_vfscanf'
  __f_vfscanf_symbol = global_prefix + '__f_vfscanf'
  __l_vfscanf_symbol = global_prefix + '__l_vfscanf'
  __i_vfscanf_symbol = global_prefix + '__i_vfscanf'
  __m_vfscanf_symbol = global_prefix + '__m_vfscanf'

  if has_link_defsym
    if format_default != 'double'
      double_printf_link_args += '-Wl,--defsym=' + vfprintf_symbol +  '=' + __d_vfprintf_symbol
      double_printf_link_args += '-Wl,--defsym=' + vfscanf_symbol + '=' + __d_vfscanf_symbol
    endif
    if format_default != 'float'
      float_printf_link_args += '-Wl,--defsym=' + vfprintf_symbol +  '=' + __f_vfprintf_symbol
      float_printf_link_args += '-Wl,--defsym=' + vfscanf_symbol + '=' + __f_vfscanf_symbol
    endif
    if format_default != 'long-long'
      llong_printf_link_args += '-Wl,--defsym=' + vfprintf_symbol +  '=' + __l_vfprintf_symbol
      llong_printf_link_args += '-Wl,--defsym=' + vfscanf_symbol + '=' + __l_vfscanf_symbol
    endif
    if format_default != 'integer'
      int_printf_link_args += '-Wl,--defsym=' + vfprintf_symbol +  '=' + __i_vfprintf_symbol
      int_printf_link_args += '-Wl,--defsym=' + vfscanf_symbol + '=' + __i_vfscanf_symbol
    endif
    if format_default != 'minimal'
      min_printf_link_args += '-Wl,--defsym=' + vfprintf_symbol +  '=' + __m_vfprintf_symbol
      min_printf_link_args += '-Wl,--defsym=' + vfscanf_symbol + '=' + __m_vfscanf_symbol
    endif
  elif has_link_alias
    if format_default == 'double'
      float_printf_link_args += '-Wl,-alias,' + vfprintf_symbol +  ',' + __d_vfprintf_symbol
      float_printf_link_args += '-Wl,-alias,' + vfscanf_symbol + ',' + __d_vfscanf_symbol
      llong_printf_link_args += '-Wl,-alias,' + vfprintf_symbol +  ',' + __d_vfprintf_symbol
      llong_printf_link_args += '-Wl,-alias,' + vfscanf_symbol + ',' + __d_vfscanf_symbol
      int_printf_link_args += '-Wl,-alias,' + vfprintf_symbol +  ',' + __d_vfprintf_symbol
      int_printf_link_args += '-Wl,-alias,' + vfscanf_symbol + ',' + __d_vfscanf_symbol
      min_printf_link_args += '-Wl,-alias,' + vfprintf_symbol +  ',' + __d_vfprintf_symbol
      min_printf_link_args += '-Wl,-alias,' + vfscanf_symbol + ',' + __d_vfscanf_symbol
    endif
    if format_default == 'float'
      double_printf_link_args += '-Wl,-alias,' + vfprintf_symbol +  ',' + __f_vfprintf_symbol
      double_printf_link_args += '-Wl,-alias,' + vfscanf_symbol + ',' + __f_vfscanf_symbol
      llong_printf_link_args += '-Wl,-alias,' + vfprintf_symbol +  ',' + __f_vfprintf_symbol
      llong_printf_link_args += '-Wl,-alias,' + vfscanf_symbol + ',' + __f_vfscanf_symbol
      int_printf_link_args += '-Wl,-alias,' + vfprintf_symbol +  ',' + __f_vfprintf_symbol
      int_printf_link_args += '-Wl,-alias,' + vfscanf_symbol + ',' + __f_vfscanf_symbol
      min_printf_link_args += '-Wl,-alias,' + vfprintf_symbol +  ',' + __f_vfprintf_symbol
      min_printf_link_args += '-Wl,-alias,' + vfscanf_symbol + ',' + __f_vfscanf_symbol
    endif
    if format_default == 'long-long'
      double_printf_link_args += '-Wl,-alias,' + vfprintf_symbol +  ',' + __l_vfprintf_symbol
      double_printf_link_args += '-Wl,-alias,' + vfscanf_symbol + ',' + __l_vfscanf_symbol
      float_printf_link_args += '-Wl,-alias,' + vfprintf_symbol +  ',' + __l_vfprintf_symbol
      float_printf_link_args += '-Wl,-alias,' + vfscanf_symbol + ',' + __l_vfscanf_symbol
      int_printf_link_args += '-Wl,-alias,' + vfprintf_symbol +  ',' + __l_vfprintf_symbol
      int_printf_link_args += '-Wl,-alias,' + vfscanf_symbol + ',' + __l_vfscanf_symbol
      min_printf_link_args += '-Wl,-alias,' + vfprintf_symbol +  ',' + __l_vfprintf_symbol
      min_printf_link_args += '-Wl,-alias,' + vfscanf_symbol + ',' + __l_vfscanf_symbol
    endif
    if format_default == 'integer'
      double_printf_link_args += '-Wl,-alias,' + vfprintf_symbol +  ',' + __i_vfprintf_symbol
      double_printf_link_args += '-Wl,-alias,' + vfscanf_symbol + ',' + __i_vfscanf_symbol
      float_printf_link_args += '-Wl,-alias,' + vfprintf_symbol +  ',' + __i_vfprintf_symbol
      float_printf_link_args += '-Wl,-alias,' + vfscanf_symbol + ',' + __i_vfscanf_symbol
      llong_printf_link_args += '-Wl,-alias,' + vfprintf_symbol +  ',' + __i_vfprintf_symbol
      llong_printf_link_args += '-Wl,-alias,' + vfscanf_symbol + ',' + __i_vfscanf_symbol
      min_printf_link_args += '-Wl,-alias,' + vfprintf_symbol +  ',' + __i_vfprintf_symbol
      min_printf_link_args += '-Wl,-alias,' + vfscanf_symbol + ',' + __i_vfscanf_symbol
    endif
    if format_default == 'minimal'
      double_printf_link_args += '-Wl,-alias,' + vfprintf_symbol +  ',' + __m_vfprintf_symbol
      double_printf_link_args += '-Wl,-alias,' + vfscanf_symbol + ',' + __m_vfscanf_symbol
      float_printf_link_args += '-Wl,-alias,' + vfprintf_symbol +  ',' + __m_vfprintf_symbol
      float_printf_link_args += '-Wl,-alias,' + vfscanf_symbol + ',' + __m_vfscanf_symbol
      llong_printf_link_args += '-Wl,-alias,' + vfprintf_symbol +  ',' + __m_vfprintf_symbol
      llong_printf_link_args += '-Wl,-alias,' + vfscanf_symbol + ',' + __m_vfscanf_symbol
      int_printf_link_args += '-Wl,-alias,' + vfprintf_symbol +  ',' + __m_vfprintf_symbol
      int_printf_link_args += '-Wl,-alias,' + vfscanf_symbol + ',' + __m_vfscanf_symbol
    endif
    if format_default != 'double'
      double_printf_link_args += '-Wl,-alias,' + __d_vfprintf_symbol +  ',' + vfprintf_symbol
      double_printf_link_args += '-Wl,-alias,' + __d_vfscanf_symbol + ',' + vfscanf_symbol
    endif
    if format_default != 'float'
      float_printf_link_args += '-Wl,-alias,' + __f_vfprintf_symbol +  ',' + vfprintf_symbol
      float_printf_link_args += '-Wl,-alias,' + __f_vfscanf_symbol + ',' + vfscanf_symbol
    endif
    if format_default != 'long-long'
      llong_printf_link_args += '-Wl,-alias,' + __l_vfprintf_symbol +  ',' + vfprintf_symbol
      llong_printf_link_args += '-Wl,-alias,' + __l_vfscanf_symbol + ',' + vfscanf_symbol
    endif
    if format_default != 'integer'
      int_printf_link_args += '-Wl,-alias,' + __i_vfprintf_symbol +  ',' + vfprintf_symbol
      int_printf_link_args += '-Wl,-alias,' + __i_vfscanf_symbol + ',' + vfscanf_symbol
    endif
    if format_default != 'minimal'
      min_printf_link_args += '-Wl,-alias,' + __m_vfprintf_symbol +  ',' + vfprintf_symbol
      min_printf_link_args += '-Wl,-alias,' + __m_vfscanf_symbol + ',' + vfscanf_symbol
    endif
  else
    if enable_tests
      # If the alias flag is not supported and we are building tests, emit an
      # error here to avoid surprising test failures.
      error('Symbol alias linker flag not supported - printf tests will fail!')
    endif
  endif
endif

# A bunch of newlib-stdio only options
newlib_global_stdio_streams = get_option('newlib-global-stdio-streams')
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
  if newlib_global_stdio_streams
    error('tinystdio always has (reentrant) global stdio streams (newlib-global-stdio-streams)')
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

if have_alias_attribute_option == 'auto'
  have_alias_attribute = cc.has_function_attribute('alias')
else
  have_alias_attribute = have_alias_attribute_option == 'true'
endif

if have_format_attribute_option == 'auto'
  have_format_attribute = cc.has_function_attribute('format')
else
  have_format_attribute = have_format_attribute_option == 'true'
endif

if have_weak_attribute_option == 'auto'
  have_weak_attribute = cc.has_function_attribute('weak')
else
  have_weak_attribute = have_weak_attribute_option == 'true'
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

if sysroot_install
  sysroot = run_command(cc.cmd_array() + ['-print-sysroot'], check : true).stdout().split('\n')[0]
  if sysroot != ''
    specs_prefix_format = '%R/@0@'
  else
    sysroot = cc_install_dir + '../../../..'
    specs_prefix_format = '%:getenv(GCC_EXEC_PREFIX ../../@0@)'
  endif
  if not get_option('sysroot-install-skip-checks')
    if sysroot == ''
      error('sysroot install requested, but compiler has no sysroot')
    endif
    if not fs.is_samepath(sysroot, prefix)
      error('sysroot install requires --prefix=' + sysroot)
    endif
  endif
else
  specs_prefix_format = prefix + '/@0@'
endif

build_type_subdir = get_option('build-type-subdir')

lib_dir = prefix / get_option('libdir') / build_type_subdir

include_dir = prefix / get_option('includedir') / build_type_subdir

newlib_iconv_dir = get_option('newlib-iconv-dir')
if newlib_iconv_dir == ''
  newlib_iconv_dir = join_paths(lib_dir, 'locale')
endif

newlib_iconv_runtime_dir = get_option('newlib-iconv-runtime-dir')
if newlib_iconv_runtime_dir == ''
  newlib_iconv_runtime_dir = newlib_iconv_dir
endif

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

specs_printf = ''
if tinystdio and printf_aliases
  specs_printf=('%{DPICOLIBC_DOUBLE_PRINTF_SCANF:--defsym=vfprintf=' + __d_vfprintf_symbol + '}' +
		' %{DPICOLIBC_DOUBLE_PRINTF_SCANF:--defsym=vfscanf=' + __d_vfscanf_symbol + '}' +
                ' %{DPICOLIBC_FLOAT_PRINTF_SCANF:--defsym=vfprintf=' + __f_vfprintf_symbol + '}' +
		' %{DPICOLIBC_FLOAT_PRINTF_SCANF:--defsym=vfscanf=' + __f_vfscanf_symbol + '}' +
		' %{DPICOLIBC_LONG_LONG_PRINTF_SCANF:--defsym=vfprintf=' + __l_vfprintf_symbol + '}' +
		' %{DPICOLIBC_LONG_LONG_PRINTF_SCANF:--defsym=vfscanf=' + __l_vfscanf_symbol + '}' +
		' %{DPICOLIBC_INTEGER_PRINTF_SCANF:--defsym=vfprintf=' + __i_vfprintf_symbol + '}' +
		' %{DPICOLIBC_INTEGER_PRINTF_SCANF:--defsym=vfscanf=' + __i_vfscanf_symbol + '}' +
		' %{DPICOLIBC_MINIMAL_PRINTF_SCANF:--defsym=vfprintf=' + __m_vfprintf_symbol + '}' +
		' %{DPICOLIBC_MINIMAL_PRINTF_SCANF:--defsym=vfscanf=' + __m_vfscanf_symbol + '}')
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
c_warnings = [
  '-Werror=implicit-function-declaration',
  '-Werror=vla',
  '-Warray-bounds',
  '-Wold-style-definition',
  '-Werror=double-promotion',
]
if get_option('analyzer')
  c_warnings += [
    '-fanalyzer',
    ]
endif
disabled_warnings = [
  '-Wno-missing-braces',
  '-Wno-implicit-int',
  '-Wno-return-type',
  '-Wno-unused-command-line-argument',
]
c_flags = cc.get_supported_arguments(c_warnings, disabled_warnings)
c_args += c_flags
native_c_args += c_flags

# Make sure _LIBC is not defined when building tests
test_c_args = c_args
c_args += ['-D_LIBC', '-U_FORTIFY_SOURCE']

# Select a fortify source option
fortify_source = get_option('fortify-source')
if fortify_source == 'none'
  test_c_args += '-U_FORTIFY_SOURCE'
else
  test_c_args += '-D_FORTIFY_SOURCE=' + fortify_source
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
    message('target ' + target)
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
	  name = ''
	else
	  name = tmp[0].underscorify()
	endif
      else
	name = ''
      endif
      targets += name

      # rv64 needs to use a non-default mcmodel so that variables can
      # live in a broader range of memory addresses
      if not has_mcmodel and name.startswith('rv64')
	flags += [ '-mcmodel=medany' ]
      endif

      # Add any extra flags for this target from the cross file
      flags += meson.get_cross_property('c_args_' + name, [])

      value = [tmp[0], flags]
      set_variable('target_' + name, value)
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
  targets = ['']
  target_ = ['.', []]
endif

foreach target : ['default-target'] + targets
  
  message('generate linker script target ' + target)

  custom_mem_config = meson.get_cross_property('custom_mem_config_' + target, '')

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
        if not cc.has_link_argument('--package-metadata=1')
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
      picolibc_ld_config_variable = 'picolibc_' + target + 'ld_config'
      picolibcpp_ld_file = 'picolibcpp_' + custom_mem_config + '.ld'
      picolibcpp_ld_variable = 'picolibcpp_' + target + '_ld'
      picolibcpp_ld_config_variable = 'picolibcpp_' + target + 'ld_config'
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

# The supported builtins vary depending on compiler and target.
# If you want to check for a given builtin, add an array
#   ['some_builtin_name', '__call_to_builtin(1,2,3);']
# The below loop will then add the define HAVE_SOME_BUILTIN_NAME if the code snippet
#   > int main (void) { __call_to_builtin(1,2,3); return 0; }
# can be compiled + linked.
# The name should match the builtin, but technically it's not necessary
builtins = [
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
]
foreach builtin : builtins
  builtin_template='''
static int foo(@1@ i __attribute__((unused))) { return 0; }
int main(void) { return foo(@0@); }
'''
  builtin_code = builtin_template.format(builtin[1], builtin[2])
  have_current_builtin = cc.links(builtin_code, name : 'test for __' + builtin[0], args: core_c_args)
  conf_data.set('_HAVE_' + builtin[0].to_upper(), have_current_builtin, description: 'The compiler supports __' + builtin[0])
endforeach

NEWLIB_VERSION='4.1.0'
NEWLIB_MAJOR_VERSION=4
NEWLIB_MINOR_VERSION=1
NEWLIB_PATCHLEVEL_VERSION=0

if get_option('newlib-retargetable-locking') != get_option('newlib-multithread')
  error('newlib-retargetable-locking and newlib-multithread must be set to the same value')
endif

conf_data.set('_HAVE_CC_INHIBIT_LOOP_TO_LIBCALL',
	      cc.has_argument('-fno-tree-loop-distribute-patterns'),
	      description: 'Compiler flag to prevent detecting memcpy/memset patterns')

conf_data.set('_HAVE_NO_BUILTIN_ATTRIBUTE',
	      cc.compiles('int __attribute__((no_builtin)) foo(int x) { return x + 1; }',
			  name : 'no_builtin attribute',
			  args : werror_c_args),
	      description: 'Compiler attribute to prevent the optimizer from adding new builtin calls')

conf_data.set('_HAVE_LONG_DOUBLE', have_long_double,
	      description: 'Compiler has long double type')
conf_data.set('_HAVE_ALIAS_ATTRIBUTE', have_alias_attribute)
conf_data.set('_HAVE_FORMAT_ATTRIBUTE', have_format_attribute)
conf_data.set('_HAVE_WEAK_ATTRIBUTE', have_weak_attribute)
conf_data.set('_WANT_REGISTER_FINI', get_option('newlib-register-fini'))
conf_data.set('_WANT_IO_LONG_LONG', io_long_long)
conf_data.set('_WANT_MINIMAL_IO_LONG_LONG', minimal_io_long_long)
conf_data.set('_WANT_FAST_BUFIO', fast_bufio)
conf_data.set('_WANT_IO_POS_ARGS', io_pos_args)
conf_data.set('_WANT_IO_C99_FORMATS', io_c99_formats)
conf_data.set('_IO_FLOAT_EXACT', io_float_exact)
conf_data.set('_WANT_IO_PERCENT_B', io_percent_b)
conf_data.set('_WANT_IO_LONG_DOUBLE', io_long_double)
conf_data.set('_ASSERT_VERBOSE', get_option('assert-verbose'))

if not tinystdio
  conf_data.set('NO_FLOATING_POINT', not newlib_io_float)
  conf_data.set('FLOATING_POINT', newlib_io_float)
  conf_data.set('_WANT_REENT_GLOBAL_STDIO_STREAMS', newlib_global_stdio_streams)
  conf_data.set('__LARGE64_FILES', newlib_stdio64)
endif
conf_data.set('_WANT_REENT_SMALL', get_option('newlib-reent-small'))
conf_data.set('_MB_CAPABLE', newlib_mb)
conf_data.set('_MB_LEN_MAX', newlib_mb ? '8' : '1')
conf_data.set('__SINGLE_THREAD__', get_option('newlib-multithread') == false)
conf_data.set('_ICONV_ENABLE_EXTERNAL_CCS', newlib_iconv_external_ccs)
conf_data.set('_ELIX_LEVEL', newlib_elix_level)
if newlib_iconv_external_ccs
  conf_data.set_quoted('ICONV_DEFAULT_NLSPATH', newlib_iconv_runtime_dir)
endif
conf_data.set('_ATEXIT_DYNAMIC_ALLOC', newlib_atexit_dynamic_alloc)
conf_data.set('_REENT_GLOBAL_ATEXIT', get_option('newlib-global-atexit'))
conf_data.set('_FVWRITE_IN_STREAMIO', newlib_fvwrite_in_streamio)
conf_data.set('_FSEEK_OPTIMIZATION', newlib_fseek_optimization)
conf_data.set('_WIDE_ORIENT', newlib_wide_orient)
conf_data.set('_HAVE_FCNTL', newlib_have_fcntl)
conf_data.set('_NANO_MALLOC', newlib_nano_malloc)
conf_data.set('_UNBUF_STREAM_OPT', get_option('newlib-unbuf-stream-opt'))
conf_data.set('_LITE_EXIT', lite_exit)
conf_data.set('_PICO_EXIT', picoexit)
conf_data.set('_NANO_FORMATTED_IO', newlib_nano_formatted_io)
conf_data.set('_RETARGETABLE_LOCKING', get_option('newlib-retargetable-locking'))
conf_data.set('TINY_STDIO', tinystdio, description: 'Use tiny stdio from gcc avr')
conf_data.set('_IEEE_LIBM', not get_option('want-math-errno'), description: 'math library does not set errno (offering only ieee semantics)')
conf_data.set('_WANT_MATH_ERRNO', get_option('want-math-errno'), description: 'math library sets errno')
conf_data.set('PREFER_SIZE_OVER_SPEED', get_option('optimization') == 's', description: 'Optimize for space over speed')
conf_data.set('FAST_STRCMP', fast_strcmp, description: 'Always optimize strcmp for performance')
conf_data.set('__HAVE_LOCALE_INFO__', newlib_locale_info, description: 'locale support')
conf_data.set('__HAVE_LOCALE_INFO_EXTENDED__', get_option('newlib-locale-info-extended'), description: 'extended locale support')
conf_data.set('NEWLIB_GLOBAL_ERRNO', get_option('newlib-global-errno'), description: 'use global errno variable')
conf_data.set('_HAVE_INITFINI_ARRAY', get_option('newlib-initfini-array'), description: 'compiler supports INIT_ARRAY sections')
conf_data.set('_HAVE_INIT_FINI', get_option('newlib-initfini'), description: 'Support _init() and _fini() functions')
conf_data.set('NEWLIB_TLS', thread_local_storage, description: 'use thread local storage')
conf_data.set('PICOLIBC_TLS', thread_local_storage, description: 'use thread local storage')
conf_data.set('_HAVE_PICOLIBC_TLS_API', thread_local_storage and have_picolibc_tls_api, description: '_set_tls and _init_tls functions available')
conf_data.set('_HAVE_PICOLIBC_TLS_RP2040', get_option('tls-rp2040'),
              description: 'Use Raspberry Pi RP2040 CPUID register to index thread local storage value')
conf_data.set('POSIX_IO', posix_io, description: 'Use open/close/read/write in tinystdio')
conf_data.set('_PRINTF_SMALL_ULTOA', printf_small_ultoa, description: 'avoid software division in decimal conversion')
conf_data.set('ATOMIC_UNGETC', atomic_ungetc, description: 'Use atomics for fgetc/ungetc for re-entrancy')
conf_data.set('_PICOLIBC_ATOMIC_SIGNAL', get_option('atomic-signal'), description: 'Use atomics for signal/raise for re-entrancy')
conf_data.set('_HAVE_BITFIELDS_IN_PACKED_STRUCTS', have_bitfields_in_packed_structs, description: 'Use bitfields in packed structs')
conf_data.set('_HAVE_BUILTIN_MUL_OVERFLOW', have_builtin_mul_overflow, description: 'Compiler has __builtin_mul_overflow')
conf_data.set('_HAVE_BUILTIN_ADD_OVERFLOW', have_builtin_add_overflow, description: 'Compiler has __builtin_add_overflow')
conf_data.set('_HAVE_COMPLEX', have_complex, description: 'Compiler supports _Complex')
conf_data.set('_HAVE_BUILTIN_COMPLEX', have_builtin_complex, description: 'Compiler has __builtin_complex')
conf_data.set('_HAVE_BUILTIN_EXPECT', have_builtin_expect, description: 'Compiler has __builtin_expect')
conf_data.set('_HAVE_ALLOC_SIZE', have_alloc_size, description: 'The compiler REALLY has the attribute __alloc_size__')
conf_data.set('_HAVE_ATTRIBUTE_ALWAYS_INLINE',
	      cc.has_function_attribute('always_inline'),
	      description: 'The compiler supports the always_inline function attribute')
conf_data.set('_HAVE_ATTRIBUTE_GNU_INLINE',
	      cc.has_function_attribute('gnu_inline'),
	      description: 'The compiler supports the gnu_inline function attribute')
conf_data.set('__PICOLIBC_CRT_RUNTIME_SIZE',
	      get_option('crt-runtime-size'),
	      description: 'Compute static memory area sizes at runtime instead of link time')
if tinystdio
  conf_data.set('_FORMAT_DEFAULT_DOUBLE',
                format_default == 'double',
                description: 'The default printf functions is the double variant')
  conf_data.set('_FORMAT_DEFAULT_FLOAT',
                format_default == 'float',
                description: 'The default printf functions is the float variant')
  conf_data.set('_FORMAT_DEFAULT_INTEGER',
                format_default == 'integer',
                description: 'The default printf functions is the integer variant')
  conf_data.set('_FORMAT_DEFAULT_MINIMAL',
                format_default == 'minimal',
                description: 'The default printf functions is the minimal variant')
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

# Obsolete newlib options
conf_data.set('_WANT_USE_LONG_TIME_T', get_option('newlib-long-time_t'), description: 'Obsoleted. Define time_t to long instead of using a 64-bit type')
conf_data.set('REENTRANT_SYSCALLS_PROVIDED', get_option('newlib-reentrant-syscalls-provided'), description: 'Obsoleted. Reentrant syscalls provided for us')
conf_data.set('MISSING_SYSCALL_NAMES', get_option('newlib-missing-syscall-names'), description: 'Obsoleted. Use regular syscalls')

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

# Compute iconv encodings to support in the library

# Dig out the list of available encodings from the encoding.aliases file. Only
# accept the first entry from each line

available_encodings = []
# fs.read was added in meson version 0.57.0. The sed invocation can be
# removed once picolibc requires a version of meson newer than that.
if meson.version().version_compare('>=0.57')
  foreach line : fs.read('newlib/libc/iconv/encoding.aliases').split('\n')
    if line != '' and not line.startswith('#')
      available_encodings += [line.split()[0]]
    endif
  endforeach
else
  _available_encodings = run_command(['sed', '-e', '/^#/d', '-e', '/^$/d', '-e', 's/ .*$//', files('newlib/libc/iconv/encoding.aliases')[0]], check : true).stdout().split('\n')
  foreach _encoding : _available_encodings
    if _encoding != ''
      available_encodings += _encoding
    endif
  endforeach
endif

# Include all available encodings if none were specified on the command line

if newlib_iconv_encodings.length() == 0
  newlib_iconv_encodings = available_encodings
elif newlib_iconv_encodings.length() == 1 and newlib_iconv_encodings[0] == 'none'
  newlib_iconv_encodings = []
endif

# Use newlib_iconv_encodings for from/to by default

if newlib_iconv_from_encodings.length() == 0
  newlib_iconv_from_encodings = newlib_iconv_encodings
endif
if newlib_iconv_to_encodings.length() == 0
  newlib_iconv_to_encodings = newlib_iconv_encodings
endif

# Set config variables for each requested 'from' encoding

foreach from : newlib_iconv_from_encodings
  message('from ' + from)
  if not available_encodings.contains(from)
    error('Requested from encoding ' + from + ' is not available. Look in newlib/libc/iconv/encoding.aliases')
  endif
  encoding_include = true
  if from not in newlib_iconv_encodings_exclude
    conf_data.set('_ICONV_FROM_ENCODING_' + from.to_upper(), true)
  endif
endforeach

# Set config variables for each requested 'to' encoding

foreach to : newlib_iconv_to_encodings
  if not available_encodings.contains(to)
    error('Requested to encoding ' + to + ' is not available. Look in newlib/libc/iconv/encoding.aliases')
  endif
  if to not in newlib_iconv_encodings_exclude
    conf_data.set('_ICONV_TO_ENCODING_' + to.to_upper(), true)
  endif
endforeach

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

# Old version macros. These have just a single leading underscore,
# which isn't consistent with the newlib macros; they were created
# by mistake.
conf_data.set('_PICOLIBC_VERSION', '"@0@"'.format(meson.project_version()), description: 'The Picolibc version in string format.')
conf_data.set('_PICOLIBC__', version_array[0], description: 'The Picolibc major version number.')
conf_data.set('_PICOLIBC_MINOR__', version_array[1], description: 'The Picolibc minor version number.')

conf_data.set('_NEWLIB_VERSION', '"@0@"'.format(NEWLIB_VERSION), description: 'The newlib version in string format.')
conf_data.set('__NEWLIB__', NEWLIB_MAJOR_VERSION, description: 'The newlib major version number.')
conf_data.set('__NEWLIB_MINOR__', NEWLIB_MINOR_VERSION, description: 'The newlib minor version number.')
conf_data.set('__NEWLIB_PATCHLEVEL__', NEWLIB_PATCHLEVEL_VERSION, description: 'The newlib patch level.')

if tinystdio
  stdio_inc_dir = 'newlib/libc/tinystdio'
else
  stdio_inc_dir = 'newlib/libc/stdio'
endif

inc_dirs = [stdio_inc_dir, '.', 'newlib/libc/include']

inc = include_directories(inc_dirs)

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
has_ieeefp_funcs = false

# semihost will adjust these if supported

# any kind of semihosting support, enough to run general
has_semihost = false

# arm compatible semihosting, enough to run arm semihost tests
has_arm_semihost = false

# some semihost needs bios to run the tests with qemu
bios_bin = []

# make sure to include semihost BEFORE picocrt!
if enable_semihost
  subdir('semihost')
endif
if tinystdio
  subdir('dummyhost')
endif

conf_data.set('_HAVE_SEMIHOST', has_semihost, description: 'Semihost APIs supported')

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

if have_long_double and get_option('test-long-double')
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

# CompCert needs the '-WUl,' prefix to correctly pass the --spec parameters to the linker
specs_prefix = ''
if cc.get_id() == 'ccomp'
  specs_prefix = '-WUl,'
endif

if has_semihost
  specs = picolibc_specs
  specs_name = picolibc_specs_name
else
  specs = configure_file(input: 'test.specs.in',
			 output: '@BASENAME@',
			 configuration: specs_data,
			 install: false)
  specs_name = 'test.specs'
endif

# Clang does not support spec files
specs_line = []
if cc.get_id() != 'clang'
  specs_line += [specs_prefix + '--specs', specs_prefix + meson.current_build_dir() / specs_name]
endif
test_c_args += specs_line

if has_semihost
  foreach target : ['default-target'] + targets
    if target == 'default-target'
      test_link_args_variable = 'test_link_args'
      test_linker_files_variable = 'test_linker_files'
      test_link_depends_variable = 'test_link_depends'
      picolibc_ld_value = picolibc_ld
    else
      test_link_args_variable = 'test_link_args_' + target
      test_linker_files_variable = 'test_linker_files_' + target
      test_link_depends_variable = 'test_link_depends_' + target
      picolibc_ld_value = get_variable('picolibc_' + target + '_ld', picolibc_ld)
    endif

    if meson.version().version_compare('>=1.4.0')
      picolibc_ld_string = picolibc_ld_value.full_path()
    else
      picolibc_ld_string = '@0@'.format(picolibc_ld_value)
    endif

    set_variable(test_link_args_variable,
		 ['-nostartfiles', '-T', picolibc_ld_string,
		  lib_gcc] + additional_libs_list + specs_line)

    set_variable(test_linker_files_variable, [picolibc_ld_value])

    # Make sure all of the tests get re-linked if the linker scripts change.
    set_variable(test_link_depends_variable, [picolibc_ld_value, specs])
  endforeach
else
  test_link_args = specs_line
  test_linker_files = [specs]
  test_link_depends = [specs]
endif

# make sure to include semihost BEFORE picocrt!
if enable_picocrt
  subdir('picocrt')
endif
subdir('newlib')

if enable_tests
  subdir('test')
endif

conf_data.set('_HAVE_IEEEFP_FUNCS', has_ieeefp_funcs, description: 'IEEE fp funcs available')

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
