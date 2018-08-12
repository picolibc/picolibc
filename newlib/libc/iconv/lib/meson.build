srcs_lib = [
    'aliasesbi.c',
    'aliasesi.c',
    'iconv.c',
    'iconvnls.c',
    'nullconv.c',
    'ucsconv.c',
]
hdrs_lib = [
    'conv.h',
    'encnames.h',
    'endian.h',
    'iconvnls.h',
    'local.h',
    'ucsconv.h',
]
lib_lib = static_library('lib',
    srcs_lib,
    pic: false,
    include_directories: inc,
)
