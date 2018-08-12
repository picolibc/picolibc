srcs_signal = [
    'psignal.c',
    'raise.c',
    'signal.c',
]
foreach target : targets
	value = get_variable('target_' + target)
	set_variable('lib_signal' + target,
		static_library('signal' + target,
			srcs_signal,
			pic: false,
			include_directories: inc,
			c_args: value[1]))
endforeach
