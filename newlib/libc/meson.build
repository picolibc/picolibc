subdir('include')

libdirs = ['argz', 'ctype', 'errno', 'iconv', 'misc', 'search',
	   'signal', 'ssp', 'stdlib', 'string', 'time',
	   'xdr']

if newlib_tinystdio
  libdirs += 'tinystdio'
else
  libdirs += 'stdio'
endif

foreach libdir : libdirs
  subdir(libdir)
endforeach

foreach target : targets
  value = get_variable('target_' + target)
  libobjs = []

  foreach libdir : libdirs
    libobjs += get_variable('lib_' + libdir + target).extract_all_objects(recursive:true)
  endforeach

  instdir = join_paths(get_option('libdir'), value[0])

  message(instdir)

  if target == ''
    libc_name = 'c'
  else
    libc_name = join_paths(target, 'libc')
  endif

  set_variable('lib_c' + target,
	       static_library(libc_name,
			      install : true,
			      install_dir : instdir,
			      pic: false,
			      objects : libobjs))
endforeach

