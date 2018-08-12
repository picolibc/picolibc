srcs_unix = [
    'basename.c',
    'dirname.c',
    'getcwd.c',
    'getlogin.c',
    'getpass.c',
    'getpwent.c',
    'getut.c',
    'pread.c',
    'pwrite.c',
    'sigset.c',
    'ttyname.c',
    'ttyname_r.c',
]
hdrs_unix = [
    'ttyname.h',
]
lib_unix = static_library('unix',
    srcs_unix,
    pic: false,
    include_directories: inc,
)
