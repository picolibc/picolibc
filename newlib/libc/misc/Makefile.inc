libc_a_SOURCES += \
	%D%/__dprintf.c \
	%D%/unctrl.c \
	%D%/ffs.c \
	%D%/init.c \
	%D%/fini.c

if NEWLIB_RETARGETABLE_LOCKING
libc_a_SOURCES += \
	%D%/lock.c
endif

LIBC_CHEWOUT_FILES += \
	%D%/ffs.def \
	%D%/lock.def \
	%D%/unctrl.def

LIBC_CHAPTERS += %D%/misc.tex
