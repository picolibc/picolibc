srcs_picolib = [
  'picotls.c',
  'picosbrk.c'
]

install_headers(
  'picotls.h'
)

foreach target : targets
	value = get_variable('target_' + target)
	set_variable('lib_picolib' + target,
		     static_library('picolib' + target,
				    srcs_picolib,
				    pic: false,
				    include_directories: inc,
				    c_args: value[1]))
endforeach
