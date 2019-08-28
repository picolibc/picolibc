srcs_reent = [
    'getreent.c',
    'impure.c',
    'reent.c',
]

foreach target : targets
	value = get_variable('target_' + target)
	set_variable('lib_reent' + target,
		static_library('reent' + target,
			srcs_reent,
			pic: false,
			include_directories: inc,
			c_args: value[1]))
endforeach
