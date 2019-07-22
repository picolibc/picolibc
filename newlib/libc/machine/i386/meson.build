srcs_machine = [
  'memchr.S',
  'memcmp.S',
  'memcpy.S',
  'memmove.S',
  'memset.S',
  'setjmp.S',
  'strchr.S',
  'strlen.S',
  'i386mach.h'
]

foreach target : targets
	value = get_variable('target_' + target)
	set_variable('lib_machine' + target,
		static_library('machine' + target,
			srcs_machine,
			pic: false,
			include_directories: inc,
			c_args: value[1]))
endforeach
