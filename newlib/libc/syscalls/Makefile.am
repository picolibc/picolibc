## Process this file with automake to generate Makefile.in

AM_CPPFLAGS = $(NEWLIB_CFLAGS) $(CROSS_CFLAGS) $(TARGET_CFLAGS)

GENERAL_SOURCES = \
	sysclose.c	\
	sysfcntl.c	\
	sysfstat.c	\
	sysgetpid.c	\
	sysgettod.c	\
	sysisatty.c	\
	syskill.c	\
	syslink.c	\
	syslseek.c	\
	sysopen.c	\
	sysread.c	\
	syssbrk.c	\
	sysstat.c	\
	systimes.c	\
	sysunlink.c	\
	syswrite.c


## Weed out EL/IX level 3 interfaces if necessary
if ELIX_LEVEL_1
ELIX_SOURCES =
else
if ELIX_LEVEL_2
ELIX_SOURCES =
else
ELIX_SOURCES = \
	sysexecve.c	\
	sysfork.c	\
	syswait.c
endif
endif

noinst_LIBRARIES = lib.a
lib_a_SOURCES = $(GENERAL_SOURCES) $(ELIX_SOURCES)
lib_a_CFLAGS = $(AM_CFLAGS)
