srcs_ssp = [
    'chk_fail.c',
    'gets_chk.c',
    'memcpy_chk.c',
    'memmove_chk.c',
    'mempcpy_chk.c',
    'memset_chk.c',
    'snprintf_chk.c',
    'sprintf_chk.c',
    'stack_protector.c',
    'stpcpy_chk.c',
    'stpncpy_chk.c',
    'strcat_chk.c',
    'strcpy_chk.c',
    'strncat_chk.c',
    'strncpy_chk.c',
    'vsnprintf_chk.c',
    'vsprintf_chk.c',
]
foreach target : targets
	value = get_variable('target_' + target)
	set_variable('lib_ssp' + target,
		static_library('ssp' + target,
			srcs_ssp,
			pic: false,
			include_directories: inc,
			c_args: value[1]))
endforeach
