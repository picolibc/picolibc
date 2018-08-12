srcs_locale = [
    'locale.c',
    'localeconv.c',
    'lnumeric.c',
]

srcs_locale_elix2 = [
    'duplocale.c',
    'freelocale.c',
    'lctype.c',
    'lmessages.c',
    'lmonetary.c',
    'newlocale.c',
    'nl_langinfo.c',
    'timelocal.c',
    'uselocale.c',
]

if newlib_elix_level >= 2 and newlib_locale_info
  srcs_locale += srcs_locale_elix2
endif

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
