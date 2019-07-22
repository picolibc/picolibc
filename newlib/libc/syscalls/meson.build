srcs_syscalls = [
  'sysclose.c',
  'sysexecve.c',
  'sysfcntl.c',
  'sysfork.c',
  'sysfstat.c',
  'sysgetpid.c',
  'sysgettod.c',
  'sysisatty.c',
  'syskill.c',
  'syslink.c',
  'syslseek.c',
  'sysopen.c',
  'sysread.c',
  'syssbrk.c',
  'sysstat.c',
  'systimes.c',
  'sysunlink.c',
  'syswait.c',
  'syswrite.c',
]

foreach target : targets
	value = get_variable('target_' + target)
	set_variable('lib_syscalls' + target,
		static_library('syscalls' + target,
			srcs_syscalls,
			pic: false,
			include_directories: inc,
			c_args: value[1]))
endforeach
