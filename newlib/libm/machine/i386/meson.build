srcs_libm_machine = [
  'f_atan2f.S',
  'f_atan2.S',
  'f_exp.c',
  'f_expf.c',
  'f_frexpf.S',
  'f_frexp.S',
  'f_ldexpf.S',
  'f_ldexp.S',
  'f_llrint.c',
  'f_llrintf.c',
  'f_llrintl.c',
  'f_log10f.S',
  'f_log10.S',
  'f_logf.S',
  'f_log.S',
  'f_lrint.c',
  'f_lrintf.c',
  'f_lrintl.c',
  'f_math.h',
  'f_pow.c',
  'f_powf.c',
  'f_rint.c',
  'f_rintf.c',
  'f_rintl.c',
  'f_tanf.S',
  'f_tan.S',
  'i386mach.h',
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
