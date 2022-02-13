if HAVE_SYSCALL_DIR

libc_a_SOURCES += \
	%D%/sysclose.c \
	%D%/sysfcntl.c \
	%D%/sysfstat.c \
	%D%/sysgetpid.c \
	%D%/sysgettod.c \
	%D%/sysisatty.c \
	%D%/syskill.c \
	%D%/syslink.c \
	%D%/syslseek.c \
	%D%/sysopen.c \
	%D%/sysread.c \
	%D%/syssbrk.c \
	%D%/sysstat.c \
	%D%/systimes.c \
	%D%/sysunlink.c \
	%D%/syswrite.c

## Weed out EL/IX level 3 interfaces if necessary
if ELIX_LEVEL_1
else
if ELIX_LEVEL_2
else
libc_a_SOURCES += \
	%D%/sysexecve.c \
	%D%/sysfork.c \
	%D%/syswait.c
endif
endif

endif
