srcs_iconv = [
]
foreach target : targets
	value = get_variable('target_' + target)
	set_variable('lib_iconv' + target,
		static_library('iconv' + target,
			srcs_iconv,
			pic: false,
			include_directories: inc,
			c_args: value[1]))
endforeach
