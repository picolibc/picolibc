srcs_search = [
    'bsd_qsort_r.c',
    'bsearch.c',
    'hash_bigkey.c',
    'hash_buf.c',
    'hash.c',
    'hash_func.c',
    'hash_log2.c',
    'hash_page.c',
    'hcreate.c',
    'hcreate_r.c',
    'qsort.c',
    'qsort_r.c',
    'tdelete.c',
    'tdestroy.c',
    'tfind.c',
    'tsearch.c',
    'twalk.c',
]
hdrs_search = [
    'db_local.h',
    'extern.h',
    'hash.h',
    'page.h',
]
foreach target : targets
	value = get_variable('target_' + target)
	set_variable('lib_search' + target,
		static_library('search' + target,
			srcs_search,
			pic: false,
			include_directories: inc,
			c_args: value[1]))
endforeach
