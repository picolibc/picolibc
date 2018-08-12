libm_machine_dirs = {
		      'aarch64' : 'aarch64',
		      'arm' : 'arm',
		      'i386' : 'i386',
		      'i486' : 'i386',
		      'i586' : 'i386',
		      'i686' : 'i386',
		      'nds32' : 'nds32',
		      'riscv' : 'riscv',
		      'spu' : 'spu',
		    }

libdirs = ['common', 'complex']
if hw_fp
  libdirs += 'mathfp'
else
  libdirs += 'math'
endif

libnames = libdirs

if libm_machine_dirs.has_key(host_cpu)
  libdirs = [join_paths('machine', libm_machine_dirs[host_cpu])] + libdirs
  libnames = ['machine'] + libnames
else
  warning('No libm machine directory for cpu ' + host_cpu)
endif

foreach libdir : libdirs
  subdir(libdir)
endforeach

foreach target : targets
  value = get_variable('target_' + target)
  libobjs = []

  foreach libname : libnames
    libobjs += get_variable('lib_' + libname + target).extract_all_objects(recursive:true)
  endforeach

  instdir = join_paths(get_option('libdir'), value[0])

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
