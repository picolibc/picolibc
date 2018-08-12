srcs_xdr = [
    'dummy.c',
    'xdr_array.c',
    'xdr.c',
    'xdr_float.c',
    'xdr_mem.c',
    'xdr_private.c',
    'xdr_rec.c',
    'xdr_reference.c',
    'xdr_sizeof.c',
    'xdr_stdio.c',
]
hdrs_xdr = [
    'xdr_private.h',
]
foreach target : targets
	value = get_variable('target_' + target)
	set_variable('lib_xdr' + target,
		static_library('xdr' + target,
			srcs_xdr,
			pic: false,
			include_directories: inc,
			c_args: value[1]))
endforeach
