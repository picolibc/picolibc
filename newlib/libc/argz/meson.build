srcs_argz = [
    'argz_add.c',
    'argz_add_sep.c',
    'argz_append.c',
    'argz_count.c',
    'argz_create.c',
    'argz_create_sep.c',
    'argz_delete.c',
    'argz_extract.c',
    'argz_insert.c',
    'argz_next.c',
    'argz_replace.c',
    'argz_stringify.c',
    'buf_findstr.c',
    'dummy.c',
    'envz_add.c',
    'envz_entry.c',
    'envz_get.c',
    'envz_merge.c',
    'envz_remove.c',
    'envz_strip.c',
]
hdrs_argz = [
    'buf_findstr.h',
]
foreach target : targets
	value = get_variable('target_' + target)
	set_variable('lib_argz' + target,
		static_library('argz' + target,
			srcs_argz,
			pic: false,
			include_directories: inc,
			c_args: value[1]))
endforeach
