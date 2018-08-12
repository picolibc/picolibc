srcs_libm_machine = [
  'e_sqrt.c',
  'ef_sqrt.c',
  's_ceil.c',
  's_floor.c',
  's_nearbyint.c',
  's_rint.c',
  's_round.c',
  's_trunc.c',
  'sf_ceil.c',
  'sf_floor.c',
  'sf_nearbyint.c',
  'sf_rint.c',
  'sf_round.c',
  'sf_trunc.c',
]

foreach target : targets
	value = get_variable('target_' + target)
	set_variable('lib_machine' + target,
		static_library('machine' + target,
			srcs_libm_machine,
			pic: false,
			include_directories: [ inc, include_directories('../../common') ],
			c_args: value[1] + ['-fno-builtin']))
endforeach
