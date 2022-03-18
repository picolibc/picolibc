if ENABLE_NEWLIB_ICONV

## The following interfaces are EL/IX level 2
if !ELIX_LEVEL_1
libc_a_SOURCES += \
	%D%/aliasesi.c \
	%D%/ucsconv.c \
	%D%/nullconv.c \
	%D%/iconv.c \
	%D%/aliasesbi.c \
	%D%/iconvnls.c
endif

endif

LIBC_CHEWOUT_FILES += %D%/iconv.def
