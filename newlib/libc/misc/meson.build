srcs_misc = [
    '__dprintf.c',
    'ffs.c',
    'fini.c',
    'init.c',
    'lock.c',
    'unctrl.c',
]
foreach target : targets
	value = get_variable('target_' + target)
	set_variable('lib_misc' + target,
		static_library('misc' + target,
			srcs_misc,
			pic: false,
			include_directories: inc,
			c_args: value[1]))
endforeach
