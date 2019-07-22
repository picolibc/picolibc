subdir('sys')

srcs_tinystdio = [
    'clearerr.c',
    'dtoa_engine.c',
    'dtoa_data.c',
    'fclose.c',
    'fdevopen.c',
    'feof.c',
    'ferror.c',
    'fgetc.c',
    'fgets.c',
    'fiprintf.c',
    'fiscanf.c',
    'fprintf.c',
    'fputc.c',
    'fputs.c',
    'fread.c',
    'fscanf.c',
    'ftoa_engine.c',
    'fwrite.c',
    'getc.c',
    'getchar.c',
    'gets.c',
    'iprintf.c',
    'iscanf.c',
    'printf.c',
    'putc.c',
    'putchar.c',
    'puts.c',
    'scanf.c',
    'siprintf.c',
    'siscanf.c',
    'sniprintf.c',
    'snprintf.c',
    'sprintf.c',
    'sscanf.c',
    'strtof.c',
    'strtod.c',
    'ungetc.c',
    'vfiprintf.c',
    'vfiscanf.c',
    'vfprintf.c',
    'vfscanf.c',
    'viprintf.c',
    'viscanf.c',
    'vprintf.c',
    'vscanf.c',
    'vsiprintf.c',
    'vsniprintf.c',
    'vsnprintf.c',
    'vsprintf.c',
]

srcs_tinystdio_flt = [
    'vfprintf.c',
    'vfscanf.c',
]

hdrs_tinystdio = [
    'dtoa_engine.h',
    'ftoa_engine.h',
    'stdio.h',
    'stdio_private.h',
    'xtoa_fast.h',
]

install_headers(
  'stdio.h'
)

foreach target : targets
	value = get_variable('target_' + target)
	set_variable('lib_tinystdio' + target,
		     static_library('tinystdio' + target,
				    srcs_tinystdio,
				    pic: false,
				    include_directories: inc,
				    c_args: value[1]))
endforeach
