srcs_machine = [
  'setjmp.S',
  'longjmp.S',
]

foreach params : targets
  target = params['name']
  target_dir = params['dir']
  target_c_args = params['c_args']
  target_lib_prefix = params['lib_prefix']
	set_variable('lib_machine' + target,
		static_library('machine' + target,
			srcs_machine,
			pic: false,
			include_directories: inc,
			c_args: target_c_args + c_args + arg_fnobuiltin))
endforeach
