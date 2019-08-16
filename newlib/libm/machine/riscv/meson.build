srcs_libm_machine = [
  'feclearexcept.c',
  'fe_dfl_env.c',
  'fegetenv.c',
  'fegetexceptflag.c',
  'fegetround.c',
  'feholdexcept.c',
  'feraiseexcept.c',
  'fesetenv.c',
  'fesetexceptflag.c',
  'fesetround.c',
  'fetestexcept.c',
  'feupdateenv.c',
]

foreach target : targets
	value = get_variable('target_' + target)
	set_variable('lib_machine' + target,
		static_library('machine' + target,
			srcs_libm_machine,
			pic: false,
			include_directories: [ inc,
					       include_directories('../../common'),
					       include_directories('../../../libc/machine/riscv/include'),
					       include_directories('../../../libc/machine/riscv') ],
			c_args: value[1] + ['-fno-builtin']))
endforeach
