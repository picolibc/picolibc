libc_a_SOURCES += \
	%D%/setlocale.h \
	%D%/locale.c \
	%D%/localeconv.c

## The following interfaces are EL/IX level 2
if !ELIX_LEVEL_1
libc_a_SOURCES += \
	%D%/duplocale.c \
	%D%/freelocale.c \
	%D%/lctype.c \
	%D%/lmessages.c \
	%D%/lnumeric.c \
	%D%/lmonetary.c \
	%D%/newlocale.c \
	%D%/nl_langinfo.c \
	%D%/timelocal.c \
	%D%/uselocale.c
endif

LIBC_CHEWOUT_FILES += \
	%D%/duplocale.def \
	%D%/freelocale.def \
	%D%/locale.def \
	%D%/newlocale.def \
	%D%/uselocale.def

LIBC_CHAPTERS += %D%/locale.tex
