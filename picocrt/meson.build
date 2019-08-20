src_picocrt_machine = {
			'riscv' : 'riscv/crt0.c',
			'riscv64' : 'riscv/crt0.c',
			'arm' : 'arm/crt0.c'
		      }

if src_picocrt_machine.has_key(host_cpu)
  srcs_picocrt = join_paths('machine', src_picocrt_machine[host_cpu])
else
  srcs_picocrt = 'shared/crt0.c'
endif

foreach target : targets
  value = get_variable('target_' + target)

  instdir = join_paths(get_option('libdir'), value[0])

  if target == ''
    crt_name = 'crt0.o'
  else
    crt_name = join_paths(target, 'crt0.o')
  endif

  executable(crt_name,
	     srcs_picocrt,
	     include_directories : inc,
	     install : true,
	     install_dir : instdir,
	     c_args : value[1] + ['-fno-builtin', '-ffreestanding'],
	     link_args : value[1] + ['-r', '-ffreestanding', '-nostdlib'])
endforeach
