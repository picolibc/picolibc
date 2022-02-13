if HAVE_XDR_DIR

%C%_ELIX_SOURCES = \
	%D%/xdr_private.c \
	%D%/xdr.c \
	%D%/xdr_float.c \
	%D%/xdr_array.c \
	%D%/xdr_mem.c \
	%D%/xdr_rec.c \
	%D%/xdr_reference.c \
	%D%/xdr_sizeof.c

if HAVE_STDIO_DIR
%C%_ELIX_SOURCES += \
	%D%/xdr_stdio.c
endif

%C%_EXTRA_SRC = \
	%D%/xdr_float_vax.c

## None of these functions are specified by EL/IX
if ELIX_LEVEL_1
else
if ELIX_LEVEL_2
else
if ELIX_LEVEL_3
else
if ELIX_LEVEL_4
else
libc_a_SOURCES += $(%C%_ELIX_SOURCES)
endif
endif
endif
endif

EXTRA_DIST += %D%/README $(%C%_EXTRA_SRC)

endif
