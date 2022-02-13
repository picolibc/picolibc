libc_a_SOURCES += %D%/setjmp.S

## tiny-printf.c and tiny-puts.c are derived from the nano printf/puts
## functions, so other supporting nano functions are required, and the tiny
## printf/puts will not work without them.
if NEWLIB_NANO_FORMATTED_IO
libc_a_SOURCES += %D%/tiny-puts.c %D%/tiny-printf.c
endif
