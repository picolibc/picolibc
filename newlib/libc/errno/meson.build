srcs_errno = [
    'errno.c',
]
foreach target : targets
	value = get_variable('target_' + target)
	set_variable('lib_errno' + target,
		static_library('errno' + target,
			srcs_errno,
			pic: false,
			include_directories: inc,
			c_args: value[1]))
endforeach
