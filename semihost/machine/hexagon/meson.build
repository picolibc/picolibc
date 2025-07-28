lib_semihost_srcs = [
  'hexagon_close.c',
  'hexagon_exit.c',
  'hexagon_errno.c',
  'hexagon_flen.c', 
  'hexagon_ftell.c', 
  'hexagon_semihost.c',
  'hexagon_lseek.c',
  'hexagon_open.c',
  'hexagon_read.c',
  'hexagon_get_cmdline.c',
  'hexagon_unlink.c', 
  'hexagon_write.c',
]

has_semihost = true

foreach params : targets
  target = params['name']
  target_dir = params['dir']
  target_c_args = params['c_args']
  target_lib_prefix = params['lib_prefix']

  instdir = join_paths(lib_dir, target_dir)

  libsemihost_name = 'semihost'

  local_lib_semihost_target = static_library(join_paths(target_dir, target_lib_prefix + libsemihost_name),
				       lib_semihost_srcs,
				       install : true,
				       install_dir : instdir,
				       include_directories : inc,
				       c_args : target_c_args + c_args,
				       pic: false)

  set_variable('lib_semihost' + target, local_lib_semihost_target)

endforeach
