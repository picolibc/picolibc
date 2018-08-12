srcs_machine = [
  'setjmp.S',
  'strcmp.S',
  'strcpy.c',
  'aeabi_memcpy.c',
  'aeabi_memcpy-armv7a.S',
  'aeabi_memmove.c',
  'aeabi_memmove-soft.S',
  'aeabi_memset.c',
  'aeabi_memset-soft.S',
  'aeabi_memclr.c',
  'memchr-stub.c',
  'memchr.S',
  'memcpy-stub.c',
  'memcpy.S',
  'strlen-stub.c',
  'strlen.S',
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
