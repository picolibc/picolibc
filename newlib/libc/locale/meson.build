srcs_locale = [
    'duplocale.c',
    'freelocale.c',
    'lctype.c',
    'lmessages.c',
    'lmonetary.c',
    'lnumeric.c',
    'locale.c',
    'localeconv.c',
    'newlocale.c',
    'nl_langinfo.c',
    'timelocal.c',
    'uselocale.c',
]
hdrs_locale = [
    'setlocale.h',
]
foreach target : targets
	value = get_variable('target_' + target)
	set_variable('lib_locale' + target,
		static_library('locale' + target,
			srcs_locale,
			pic: false,
			include_directories: inc,
			c_args: value[1]))
endforeach
