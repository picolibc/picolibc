srcs_time = [
    'asctime.c',
    'asctime_r.c',
    'clock.c',
    'ctime.c',
    'ctime_r.c',
    'difftime.c',
    'gettzinfo.c',
    'gmtime.c',
    'gmtime_r.c',
    'lcltime.c',
    'lcltime_r.c',
    'mktime.c',
    'month_lengths.c',
    'strftime.c',
    'strptime.c',
    'time.c',
    'tzcalc_limits.c',
    'tzlock.c',
    'tzset.c',
    'tzset_r.c',
    'tzvars.c',
    'wcsftime.c',
]
hdrs_time = [
    'local.h',
]
foreach target : targets
	value = get_variable('target_' + target)
	set_variable('lib_time' + target,
		static_library('time' + target,
			srcs_time,
			pic: false,
			include_directories: inc,
			c_args: value[1]))
endforeach
