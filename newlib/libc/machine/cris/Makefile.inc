libc_a_SOURCES += %D%/setjmp.c %D%/memcpy.c %D%/memset.c %D%/memmove.c %D%/libcdtor.c

# We also make a library with just the useful
# machine-but-not-system-specific functions, usable as an add-on
# by itself together with e.g. uclibc.
toollib_LIBRARIES += %D%/libic.a
%C%_libic_a_SOURCES =
%C%_libic_a_LIBADD = \
	%D%/libc_a-setjmp.o \
	%D%/libc_a-memcpy.o \
	%D%/libc_a-memset.o \
	%D%/libc_a-memmove.o \
	%D%/libc_a-libcdtor.o
