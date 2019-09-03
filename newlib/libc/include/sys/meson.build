inc_sys_headers = [
  'cdefs.h',
  'config.h',
  'custom_file.h',
  '_default_fcntl.h',
  'dirent.h',
  'dir.h',
  'errno.h',
  'fcntl.h',
  'features.h',
  'file.h',
  'iconvnls.h',
  '_intsup.h',
  '_locale.h',
  'lock.h',
  'param.h',
  '_pthreadtypes.h',
  'queue.h',
  'resource.h',
  'sched.h',
  'select.h',
  'signal.h',
  '_sigset.h',
  'stat.h',
  '_stdint.h',
  'string.h',
  'syslimits.h',
  'timeb.h',
  'time.h',
  'times.h',
  '_timespec.h',
  'timespec.h',
  '_timeval.h',
  'tree.h',
  '_types.h',
  'types.h',
  'unistd.h',
  'utime.h',
  'wait.h'
]
if not newlib_tinystdio
  inc_sys_headers += ['reent.h']
endif

install_headers(inc_sys_headers,
		subdir:'sys'
	       )
