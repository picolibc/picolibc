srcs_machine = [
  'ffs.c',
  'ieeefp.c',
  'memcpy-asm.S',
  'memcpy.c',
  'memmove.S',
  'memmove-stub.c',
  'memset.S',
  'setjmp.S',
  'strcmp.S',
  'strcpy.c',
  'strlen.c',
]

foreach target : targets
	value = get_variable('target_' + target)
	set_variable('lib_machine' + target,
		static_library('machine' + target,
			srcs_machine,
			pic: false,
			include_directories: inc,
			c_args: value[1] + ['-fno-builtin']))
endforeach
