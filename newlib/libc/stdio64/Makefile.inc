if HAVE_STDIO64_DIR

## All interfaces are EL/IX level 2
if !ELIX_LEVEL_1
libc_a_SOURCES += \
	%D%/fdopen64.c \
	%D%/fgetpos64.c \
	%D%/fopen64.c \
	%D%/freopen64.c \
	%D%/fseeko64.c \
	%D%/fsetpos64.c \
	%D%/ftello64.c \
	%D%/stdio64.c \
	%D%/tmpfile64.c
endif

libc_a_CPPFLAGS_%C% = -I$(srcdir)/%D%/../stdio

endif

LIBC_CHEWOUT_FILES += \
	%D%/fdopen64.def \
	%D%/fgetpos64.def \
	%D%/fopen64.def \
	%D%/freopen64.def \
	%D%/fseeko64.def \
	%D%/fsetpos64.def \
	%D%/ftello64.def \
	%D%/tmpfile64.def

LIBC_CHAPTERS += %D%/stdio64.tex
