libc_a_CPPFLAGS_%C% = -I$(srcdir)/libc/stdio -I$(srcdir)/libc/stdlib

libc_a_SOURCES += %D%/setjmp.S

if HAVE_POWERPC_ALTIVEC
libc_a_SOURCES += \
	%D%/vfprintf.c \
	%D%/vfscanf.c \
	%D%/vec_malloc.c \
	%D%/vec_calloc.c \
	%D%/vec_free.c \
	%D%/vec_realloc.c \
	%D%/vec_mallocr.c \
	%D%/vec_callocr.c \
	%D%/vec_reallocr.c
endif
if HAVE_POWERPC_SPE
libc_a_SOURCES += \
	%D%/atosfix16.c \
	%D%/atosfix32.c \
	%D%/atosfix64.c \
	%D%/atoufix16.c \
	%D%/atoufix32.c \
	%D%/atoufix64.c \
	%D%/simdldtoa.c \
	%D%/strtosfix16.c \
	%D%/strtosfix32.c \
	%D%/strtosfix64.c \
	%D%/strtoufix16.c \
	%D%/strtoufix32.c \
	%D%/strtoufix64.c \
	%D%/ufix64toa.c \
	%D%/vfprintf.c \
	%D%/vfscanf.c
endif
