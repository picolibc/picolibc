srcs_ces = [
    'cesbi.c',
    'euc.c',
    'table.c',
    'table-pcs.c',
    'ucs-2.c',
    'ucs-2-internal.c',
    'ucs-4.c',
    'ucs-4-internal.c',
    'us-ascii.c',
    'utf-16.c',
    'utf-8.c',
]
hdrs_ces = [
    'cesbi.h',
    'cesdeps.h',
]
lib_ces = static_library('ces',
    srcs_ces,
    pic: false,
    include_directories: inc,
)
