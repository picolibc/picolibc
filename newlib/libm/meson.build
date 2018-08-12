libdirs = ['common', 'complex']
if hw_fp
  libdirs += 'mathfp'
else
  libdirs += 'math'
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
    libm_name = 'm'
  else
    libm_name = join_paths(target, 'libm')
  endif

  set_variable('lib_m' + target,
	       static_library(libm_name,
			      install : true,
			      install_dir : instdir,
			      pic: false,
			      objects : libobjs))
endforeach
