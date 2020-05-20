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
project('picolibc', 'c',
	default_options: [
	  'buildtype=minsize',
	  'debug=true',
	  'c_std=gnu99',
	  'b_staticpic=false',
	],
	license : 'BSD',
	meson_version : '>= 0.50',
	version: '1.4.1'
       )

targets = []

enable_multilib = get_option('multilib')
multilib_list = get_option('multilib-list')
enable_picolib = get_option('picolib')
enable_tests = get_option('tests')
enable_native_tests = get_option('native-tests')
have_alias_attribute_option = get_option('have-alias-attribute')
newlib_register_fini = get_option('newlib-register-fini')
newlib_mb = get_option('newlib-mb')
newlib_iconv_encodings = get_option('newlib-iconv-encodings')
newlib_iconv_from_encodings = get_option('newlib-iconv-from-encodings')
newlib_iconv_to_encodings = get_option('newlib-iconv-to-encodings')
newlib_iconv_external_ccs = get_option('newlib-iconv-external-ccs')
newlib_atexit_dynamic_alloc = get_option('newlib-atexit-dynamic-alloc')
newlib_global_atexit = get_option('newlib-global-atexit')
newlib_reent_small = get_option('newlib-reent-small')
newlib_nano_malloc = get_option('newlib-nano-malloc')
newlib_unbuf_stream_opt = get_option('newlib-unbuf-stream-opt')
lite_exit = get_option('lite-exit')
newlib_retargetable_locking = get_option('newlib-retargetable-locking')
newlib_long_time_t = get_option('newlib-long-time_t')
newlib_multithread = get_option('newlib-multithread')
newlib_iconv = get_option('newlib-iconv')

newlib_supplied_syscalls = get_option('newlib-supplied-syscalls')
newlib_elix_level = get_option('newlib-elix-level')
target_optspace = get_option('optimization') == 's'
fast_strcmp = get_option('fast-strcmp')
newlib_locale_info = get_option('newlib-locale-info')
newlib_locale_info_extended = get_option('newlib-locale-info-extended')
newlib_global_errno = get_option('newlib-global-errno')
newlib_initfini_array = get_option('newlib-initfini-array')
newlib_initfini = get_option('newlib-initfini')
thread_local_storage_option = get_option('thread-local-storage')
tls_model = get_option('tls-model')
sysroot_install = get_option('sysroot-install')
newlib_obsolete_math = get_option('newlib-obsolete-math')
prefix = get_option('prefix')

# Select stdio implementation
newlib_tinystdio = get_option('newlib-tinystdio')

# Shared stdio options
io_long_long = get_option('io-long-long') or get_option('newlib-io-long-long')
io_c99_formats = get_option('io-c99-formats') or get_option('newlib-io-c99-formats')

# tinystdio options
posix_io = newlib_tinystdio and get_option('posix-io')
posix_console = posix_io and get_option('posix-console')

# A bunch of newlib-stdio only options
newlib_io_pos_args = get_option('newlib-io-pos-args')
newlib_io_long_double = get_option('newlib-io-long-double')
newlib_global_stdio_streams = get_option('newlib-global-stdio-streams')
newlib_fvwrite_in_streamio = get_option('newlib-fvwrite-in-streamio')
newlib_fseek_optimization = get_option('newlib-fseek-optimization')
newlib_nano_formatted_io = get_option('newlib-nano-formatted-io')
newlib_io_float = get_option('newlib-io-float')
newlib_stdio64 = get_option('newlib-stdio64')
newlib_wide_orient = get_option('newlib-wide-orient')

# options which no longer have any effect
newlib_reentrant_syscalls_provided = get_option('newlib-reentrant-syscalls-provided')
newlib_missing_syscall_names = get_option('newlib-missing-syscall-names')

# Check for a bunch of newlib-stdio only options and complain
# if they are selected while using tinystdio

if newlib_tinystdio
  if newlib_io_pos_args
    error('tinystdio does not support positional arguments (newlib-io-pos-args)')
  endif
  if newlib_io_long_double
    error('tinystdio does not support long double (newlib-io-long-double)')
  endif
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
endif

host_cpu_family=host_machine.cpu_family()
if host_cpu_family == ''
  host_cmds = meson.get_compiler('c').cmd_array() + ['-dumpmachine']
  host_cc_machine=run_command(host_cmds).stdout().strip().split('-')
  host_cpu_family=host_cc_machine[0]
  message('Computed host_cpu_family as ' + host_cpu_family)
endif

specs_dir = get_option('specsdir')
if specs_dir == ''
  search_cmds = meson.get_compiler('c').cmd_array() + ['-print-search-dirs']
  install_dir=run_command(search_cmds).stdout().split('\n')[0]
  specs_dir=install_dir.split(':')[1].strip()
endif

if have_alias_attribute_option == 'auto'
  have_alias_attribute = meson.get_compiler('c').has_function_attribute('alias')
else
  have_alias_attribute = have_alias_attribute_option == 'true'
endif

if thread_local_storage_option == 'auto'
  thread_local_storage = not meson.get_compiler('c').has_function('__emutls_get_address', args: ['-nostdlib', '-lgcc'])
else
  thread_local_storage = thread_local_storage_option == 'true'
endif

if sysroot_install
  sysroot = run_command(meson.get_compiler('c').cmd_array() + ['-print-sysroot']).stdout().split('\n')[0]
  if sysroot == ''
    error('sysroot install requested, but compiler has no sysroot')
  endif
  if sysroot != prefix
    error('sysroot install requires --prefix=' + sysroot)
  endif
endif

lib_dir = join_paths(prefix, get_option('libdir'))

specs_data = configuration_data()
specs_data.set(
  'INCLUDEDIR',
  join_paths(sysroot_install? '%R' : prefix, get_option('includedir'))
)

specs_data.set(
  'LIBDIR',
  sysroot_install? join_paths('%R', get_option('libdir')) : lib_dir
)

specs_data.set(
  'TLSMODEL',
  tls_model
)

picolibc_specs = configure_file(input: 'picolibc.specs.in',
				output: '@BASENAME@',
				configuration: specs_data,
				install_dir: specs_dir,
				install: true)
				     
install_data('picolibc.ld',
	     install_dir: lib_dir)

long_double_code = '''
#include <float.h>
#ifndef LDBL_MANT_DIG
#error No long double support in float.h
#endif
long double test()
{
	long double ld = 0.0L;
	 return ld;
}
'''
have_long_double = meson.get_compiler('c').compiles(long_double_code, name : 'long double check')

long_double_size_code = '''
#include <float.h>
char size_test[sizeof(double) == sizeof(long double) ? 1 : -1];
'''

if have_long_double
  long_double_equals_double = meson.get_compiler('c').compiles(long_double_size_code, name : 'long double same as double')
endif

if enable_multilib
  used_libs = []

  # Ask the compiler for the set of available multilib configurations,
  # set up the build system to compile for all desired ones
  foreach target : run_command(meson.get_compiler('c'), '--print-multi-lib').stdout().strip().split('\n')
    tmp = target.split(';')
    flags = []

    # Let the user specify a subset of the possible multilib
    # configurations to build for
    if multilib_list == [] or tmp[0] in multilib_list
      used_libs += tmp[0]
      if tmp.length() > 1
	foreach flag : tmp[1].strip('@').split('@')
	  if flag != ''
	    flags += '-' + flag
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
      if name.startswith('rv64')
	flags += [ '-mcmodel=medany' ]
      endif

      # Add any extra flags for this target from the cross file
      target_extra = meson.get_cross_property('c_args_' + name, [])
      if target_extra != []
	flags += target_extra
      endif

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

conf_data = configuration_data()

NEWLIB_VERSION='3.3.0'
NEWLIB_MAJOR_VERSION=3
NEWLIB_MINOR_VERSION=3
NEWLIB_PATCHLEVEL_VERSION=0

have_cc_inhibit_loop_to_libcall=meson.get_compiler('c').has_argument('-fno-tree-loop-distribute-patterns')
conf_data.set('_HAVE_CC_INHIBIT_LOOP_TO_LIBCALL', have_cc_inhibit_loop_to_libcall,
	      description: 'GCC flag to prevent detecting memcpy/memset patterns')
conf_data.set('_HAVE_LONG_DOUBLE', have_long_double,
	      description: 'Compiler has long double type')
if have_long_double
  conf_data.set('_LDBL_EQ_DBL', long_double_equals_double,
		description: 'long double is the same as double')
endif
conf_data.set('HAVE_ALIAS_ATTRIBUTE', have_alias_attribute)
conf_data.set('_WANT_REGISTER_FINI', newlib_register_fini)
conf_data.set('_WANT_IO_LONG_LONG', io_long_long)
conf_data.set('_WANT_IO_C99_FORMATS', io_c99_formats)
if not newlib_tinystdio
  conf_data.set('_WANT_IO_LONG_DOUBLE', newlib_io_long_double)
  conf_data.set('_WANT_IO_POS_ARGS', newlib_io_pos_args)
  conf_data.set('NO_FLOATING_POINT', not newlib_io_float)
  conf_data.set('FLOATING_POINT', newlib_io_float)
  conf_data.set('_WANT_REENT_GLOBAL_STDIO_STREAMS', newlib_global_stdio_streams)
  conf_data.set('__LARGE64_FILES', newlib_stdio64)
endif
conf_data.set('_WANT_REENT_SMALL', newlib_reent_small)
conf_data.set('_MB_CAPABLE', newlib_mb)
conf_data.set('_MB_LEN_MAX', newlib_mb ? '8' : '1')
conf_data.set('__SINGLE_THREAD__', newlib_multithread == false)
conf_data.set('_ICONV_ENABLE_EXTERNAL_CCS', newlib_iconv_external_ccs)
conf_data.set('_ICONV_ENABLED', newlib_iconv)
conf_data.set('_ICONV_ENABLE_EXTERNAL_CCS', newlib_iconv_external_ccs)
conf_data.set('_ATEXIT_DYNAMIC_ALLOC', newlib_atexit_dynamic_alloc)
conf_data.set('_REENT_GLOBAL_ATEXIT', newlib_global_atexit)
conf_data.set('_FVWRITE_IN_STREAMIO', newlib_fvwrite_in_streamio)
conf_data.set('_FSEEK_OPTIMIZATION', newlib_fseek_optimization)
conf_data.set('_WIDE_ORIENT', newlib_wide_orient)
conf_data.set('_NANO_MALLOC', newlib_nano_malloc)
conf_data.set('_UNBUF_STREAM_OPT', newlib_unbuf_stream_opt)
conf_data.set('_LITE_EXIT', lite_exit)
conf_data.set('_NANO_FORMATTED_IO', newlib_nano_formatted_io)
conf_data.set('_RETARGETABLE_LOCKING', newlib_retargetable_locking)
conf_data.set('_WANT_USE_LONG_TIME_T', newlib_long_time_t)
conf_data.set('TINY_STDIO', newlib_tinystdio, description: 'Use tiny stdio from gcc avr')
conf_data.set('_IEEE_LIBM', true, description: 'math library only offers ieee semantics')
conf_data.set('PREFER_SIZE_OVER_SPEED', target_optspace, description: 'Optimize for space over speed')
conf_data.set('FAST_STRCMP', fast_strcmp, description: 'Always optimize strcmp for performance')
conf_data.set('REENTRANT_SYSCALLS_PROVIDED', newlib_reentrant_syscalls_provided, description: 'Reentrant syscalls provided for us')
conf_data.set('MISSING_SYSCALL_NAMES', newlib_missing_syscall_names, description: 'use regular syscalls')
conf_data.set('__HAVE_LOCALE_INFO__', newlib_locale_info, description: 'locale support')
conf_data.set('__HAVE_LOCALE_INFO_EXTENDED__', newlib_locale_info_extended, description: 'extended locale support')
conf_data.set('NEWLIB_GLOBAL_ERRNO', newlib_global_errno, description: 'use global errno variable')
conf_data.set('HAVE_INITFINI_ARRAY', newlib_initfini_array, description: 'compiler supports INIT_ARRAY sections')
conf_data.set('HAVE_INIT_FINI', newlib_initfini, description: 'Support _init() and _fini() functions')
conf_data.set('NEWLIB_TLS', thread_local_storage, description: 'use thread local storage')
conf_data.set('PICOLIBC_TLS', thread_local_storage, description: 'use thread local storage')
conf_data.set('POSIX_IO', posix_io, description: 'Use open/close/read/write in tinystdio')
conf_data.set('POSIX_CONSOLE', posix_console, description: 'Use POSIX I/O for stdin, stdout and stderr')

if newlib_obsolete_math == 'auto'
  obsolete_math_value = false
elif newlib_obsolete_math == 'true'
  obsolete_math_value = 1
elif newlib_obsolete_math == 'false'
  obsolete_math_value = 0
endif
  
conf_data.set('__OBSOLETE_MATH', obsolete_math_value, description: 'Use old math code (undef auto, 0 no, 1 yes)')

version_array = meson.project_version().split('.')

if version_array.length() > 2
  picolibc_patch_level = version_array[2]
else
  picolibc_patch_level = 0
endif

conf_data.set('_PICOLIBC_VERSION', '"@0@"'.format(meson.project_version()), description: 'The Picolibc version in string format.')
conf_data.set('_PICOLIBC__', version_array[0], description: 'The Picolibc major version number.')
conf_data.set('_PICOLIBC_MINOR__', version_array[1], description: 'The Picolibc minor version number.')
conf_data.set('__PICOLIBC_PATCHLEVEL__', picolibc_patch_level, description: 'The Picolibc patch level.')

conf_data.set('_NEWLIB_VERSION', '"@0@"'.format(NEWLIB_VERSION), description: 'The newlib version in string format.')
conf_data.set('_NEWLIB__', NEWLIB_MAJOR_VERSION, description: 'The newlib major version number.')
conf_data.set('_NEWLIB_MINOR__', NEWLIB_MINOR_VERSION, description: 'The newlib minor version number.')
conf_data.set('__NEWLIB_PATCHLEVEL__', NEWLIB_PATCHLEVEL_VERSION, description: 'The newlib patch level.')

if newlib_tinystdio
  stdio_inc_dir = 'newlib/libc/tinystdio'
else
  stdio_inc_dir = 'newlib/libc/stdio'
endif

inc_dirs = [stdio_inc_dir, '.', 'newlib/libc/include']

inc = include_directories(inc_dirs)

includedir = join_paths(prefix, get_option('includedir'))

# We don't need '-fdata-sections' currently as there aren't any
# files with data used in separate code paths. This works around
# versions of gcc for RISC-V which have a bug that mis-names
# initialized read-only data segments when -fdata-sections
# is defined
arguments = ['-ffunction-sections']

if tls_model != ''
  arguments += ['-ftls-model=' + tls_model]
endif

add_project_arguments(arguments, language: 'c')

# libc will adjust this if supported
has_ieeefp_funcs = false

# semihost will adjust this if supported
has_semihost = false

subdir('semihost')
if newlib_tinystdio
  subdir('dummyhost')
endif

conf_data.set('HAVE_SEMIHOST', has_semihost, description: 'Semihost APIs supported')

# By default, tests don't require any special arguments

test_c_args = []
test_link_args = []
test_link_depends = []

if has_semihost

  # If we're using semihosting, we assume that there's a
  # custom linker script that includes picolibc.specs

  test_link_name = 'test-' + host_cpu_family + '.ld'
  test_link_dep = files(test_link_name)

  # Linker args must be strings (seems like a bug), so compute the
  # full path by hand

  test_link_path = '@0@/@1@'.format(meson.current_source_dir(), test_link_name)

  picolibc_link_path = '@0@/@1@'.format(meson.current_source_dir(), 'picolibc.ld')

  test_c_args += ['--specs', '@0@/@1@'.format(meson.current_build_dir(), picolibc_specs)]

  # Undefine _exit as that's in libsemihost and needs to replace the weak _exit

  test_link_args += test_c_args + ['-nostartfiles', '-nostdlib', '-T', test_link_path, '-T', picolibc_link_path, '-Wl,--undefined=_exit', '-lgcc']

  # Make sure all of the tests get re-linked if the linker scripts change

  test_link_depends += test_link_dep + files('picolibc.ld') + [picolibc_specs]
endif

subdir('picocrt')
subdir('newlib')

if enable_tests
  subdir('test')
endif

conf_data.set('HAVE_IEEEFP_FUNCS', has_ieeefp_funcs, description: 'IEEE fp funcs available')

configure_file(output : 'picolibc.h',
	       configuration: conf_data,
	       install_dir: includedir)

# Usage as an embedded subproject:
# If picolibc is embedded into the source as a subproject, 
# provide a dependency to be used by the main project:
#   dependency('libc', fallback: ['picolibc', 'libc_dep'])
if not enable_multilib and meson.is_subproject()
  picolibc_dep = declare_dependency(include_directories: inc, link_with: [lib_c, lib_m])
endif
