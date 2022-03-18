if ENABLE_NEWLIB_ICONV

## The following interfaces are EL/IX level 2
if !ELIX_LEVEL_1
libc_a_SOURCES += \
	%D%/utf-8.c \
	%D%/utf-16.c \
	%D%/ucs-2.c \
	%D%/us-ascii.c \
	%D%/ucs-4.c \
	%D%/ucs-2-internal.c \
	%D%/ucs-4-internal.c \
	%D%/cesbi.c \
	%D%/table.c \
	%D%/table-pcs.c \
	%D%/euc.c
endif

endif
