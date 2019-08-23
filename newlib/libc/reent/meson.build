srcs_reent_full = [
    'closer.c',
    'execr.c',
    'fcntlr.c',
    'fstat64r.c',
    'fstatr.c',
    'gettimeofdayr.c',
    'isattyr.c',
    'linkr.c',
    'lseek64r.c',
    'lseekr.c',
    'mkdirr.c',
    'open64r.c',
    'openr.c',
    'readr.c',
    'renamer.c',
    'sbrkr.c',
    'signalr.c',
    'signgam.c',
    'stat64r.c',
    'statr.c',
    'timesr.c',
    'unlinkr.c',
    'writer.c',
]
srcs_reent_common = [
    'getreent.c',
    'impure.c',
    'reent.c',
]

srcs_reent_tiny = [
]
srcs_reent = srcs_reent_common
if newlib_tinystdio
  srcs_reent += srcs_reent_tiny
else
  srcs_reent += srcs_reent_full
endif

foreach target : targets
	value = get_variable('target_' + target)
	set_variable('lib_reent' + target,
		static_library('reent' + target,
			srcs_reent,
			pic: false,
			include_directories: inc,
			c_args: value[1]))
endforeach
