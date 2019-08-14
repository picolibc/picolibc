project('picolibc', 'c',
	default_options: [
		'buildtype=debugoptimized',
		'c_std=gnu99',
	],
	version: '3.1.0.2019.08.14'
)

targets = []

enable_multilib = get_option('multilib')
enable_tests = get_option('tests')
newlib_tinystdio = get_option('newlib-tinystdio')
have_alias_attribute_option = get_option('have-alias-attribute')
newlib_io_pos_args = get_option('newlib-io-pos-args')
newlib_io_c99_formats = get_option('newlib-io-c99-formats')
newlib_register_fini = get_option('newlib-register-fini')
newlib_io_long_long = get_option('newlib-io-long-long')
newlib_io_long_double = get_option('newlib-io-long-double')
newlib_mb = get_option('newlib-mb')
newlib_iconv_encodings = get_option('newlib-iconv-encodings')
newlib_iconv_from_encodings = get_option('newlib-iconv-from-encodings')
newlib_iconv_to_encodings = get_option('newlib-iconv-to-encodings')
newlib_iconv_external_ccs = get_option('newlib-iconv-external-ccs')
newlib_atexit_dynamic_alloc = get_option('newlib-atexit-dynamic-alloc')
newlib_global_atexit = get_option('newlib-global-atexit')
newlib_reent_small = get_option('newlib-reent-small')
newlib_global_stdio_streams = get_option('newlib-global-stdio-streams')
newlib_fvwrite_in_streamio = get_option('newlib-fvwrite-in-streamio')
newlib_fseek_optimization = get_option('newlib-fseek-optimization')
newlib_wide_orient = get_option('newlib-wide-orient')
newlib_nano_malloc = get_option('newlib-nano-malloc')
newlib_unbuf_stream_opt = get_option('newlib-unbuf-stream-opt')
lite_exit = get_option('lite-exit')
newlib_nano_formatted_io = get_option('newlib-nano-formatted-io')
newlib_retargetable_locking = get_option('newlib-retargetable-locking')
newlib_long_time_t = get_option('newlib-long-time_t')
newlib_multithread = get_option('newlib-multithread')
newlib_iconv = get_option('newlib-iconv')
newlib_io_float = get_option('newlib-io-float')
newlib_supplied_syscalls = get_option('newlib-supplied-syscalls')
newlib_elix_level = get_option('newlib-elix-level')
target_optspace = get_option('target-optspace')
newlib_reentrant_syscalls_provided = get_option('newlib-reentrant-syscalls-provided')
newlib_missing_syscall_names = get_option('newlib-missing-syscall-names')
newlib_locale_info = get_option('newlib-locale-info')
newlib_locale_info_extended = get_option('newlib-locale-info-extended')
newlib_global_errno = get_option('newlib-global-errno')
hw_fp = get_option('hw-fp')

host_cpu=host_machine.cpu()
if host_cpu == ''
  host_cmds = meson.get_compiler('c').cmd_array() + ['-dumpmachine']
  host_cc_machine=run_command(host_cmds).stdout().strip().split('-')
  host_cpu=host_cc_machine[0]
  message('Computed host_cpu as ' + host_cpu)
endif

specs_dir = get_option('specsdir')
if specs_dir == ''
  search_cmds = meson.get_compiler('c').cmd_array() + ['-print-search-dirs']
  install_dir=run_command(search_cmds).stdout().split('\n')[0]
  specs_dir=install_dir.split(':')[1].strip()
endif

message('specs dir is ' + specs_dir)

if have_alias_attribute_option == 'auto'
  have_alias_attribute = meson.get_compiler('c').has_function_attribute('alias')
else
  have_alias_attribute = have_alias_attribute_option == 'true'
endif

specs_data = configuration_data()
specs_data.set(
  'INCLUDEDIR',
  join_paths(get_option('prefix'), get_option('includedir'))
)

specs_data.set(
  'LIBDIR',
  join_paths(get_option('prefix'), get_option('libdir'))
)

configure_file(input: 'picolibc.specs.in',
	       output: '@BASENAME@',
	       configuration: specs_data,
	       install_dir: specs_dir,
	       install: true)
				     
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

if enable_multilib
  foreach target : run_command(meson.get_compiler('c'), '--print-multi-lib').stdout().strip().split('\n')
    tmp = target.split(';')
    flags = []
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
    value = [tmp[0], flags]
    set_variable('target_' + name, value)
  endforeach
else
  targets = ['']
  target_ = ['.', []]
endif

conf_data = configuration_data()

NEWLIB_MAJOR_VERSION=2
NEWLIB_MINOR_VERSION=10
NEWLIB_PATCHLEVEL_VERSION=0

have_cc_inhibit_loop_to_libcall=meson.get_compiler('c').has_argument('-fno-tree-loop-distribute-patterns')
conf_data.set('_HAVE_CC_INHIBIT_LOOP_TO_LIBCALL', have_cc_inhibit_loop_to_libcall,
	      description: 'GCC flag to prevent detecting memcpy/memset patterns')
conf_data.set('HAVE_LONG_DOUBLE', have_long_double,
	      description: 'Compiler has long double type')
conf_data.set('HAVE_ALIAS_ATTRIBUTE', have_alias_attribute)
conf_data.set('_WANT_IO_C99_FORMATS', newlib_io_c99_formats)
conf_data.set('_WANT_REGISTER_FINI', newlib_register_fini)
conf_data.set('_WANT_IO_LONG_LONG', newlib_io_long_long)
conf_data.set('_WANT_IO_LONG_DOUBLE', newlib_io_long_double)
conf_data.set('_WANT_IO_POS_ARGS', newlib_io_pos_args)
conf_data.set('_WANT_REENT_SMALL', newlib_reent_small)
conf_data.set('_WANT_REENT_GLOBAL_STDIO_STREAMS', newlib_global_stdio_streams)
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
conf_data.set('__CUSTOM_FILE_IO__', newlib_tinystdio, description: 'Use tiny stdio FILE')
conf_data.set('_IEEE_LIBM', true, description: 'math library only offers ieee semantics')
conf_data.set('PREFER_SIZE_OVER_SPEED', target_optspace, description: 'Optimize for space over speed')
conf_data.set('REENTRANT_SYSCALLS_PROVIDED', newlib_reentrant_syscalls_provided, description: 'Reentrant syscalls provided for us')
conf_data.set('MISSING_SYSCALL_NAMES', newlib_missing_syscall_names, description: 'use regular syscalls')
conf_data.set('__HAVE_LOCALE_INFO__', newlib_locale_info, description: 'locale support')
conf_data.set('__HAVE_LOCALE_INFO_EXTENDED__', newlib_locale_info_extended, description: 'extended locale support')
conf_data.set('NEWLIB_GLOBAL_ERRNO', newlib_global_errno, description: 'use global errno variable')

version_data = configuration_data()

version_array = meson.project_version().split('.')

version_data.set('_NEWLIB_VERSION', '"@0@"'.format(meson.project_version()), description: 'The newlib version in string format.')
version_data.set('_NEWLIB__', version_array[0], description: 'The newlib major version number.')
version_data.set('_NEWLIB_MINOR__', version_array[1], description: 'The newlib minor version number.')
version_data.set('__NEWLIB_PATCHLEVEL__', version_array[2], description: 'The newlib patch level.')

if newlib_tinystdio
  inc = include_directories(
    '.',
    'newlib/libc/tinystdio',
    'newlib/libc/include'
  )
else
  inc = include_directories(
    '.',
    'newlib/libc/include'
  )
endif

includedir = join_paths(get_option('prefix'), get_option('includedir'))

configure_file(output : 'newlib.h',
	       configuration: conf_data,
	       install_dir: includedir)

configure_file(output : '_newlib_version.h',
	       configuration: version_data,
	       install_dir: includedir)

arguments = ['-g', '-ffunction-sections', '-fdata-sections']
if target_optspace
  arguments += '-Os'
endif

add_project_arguments(arguments, language: 'c')

subdir('newlib')

if enable_tests
  subdir('test')
endif
