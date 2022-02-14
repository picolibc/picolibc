## Process this file with automake to generate Makefile.in

AM_CPPFLAGS = $(NEWLIB_CFLAGS) $(TARGET_CFLAGS)

GENERAL_SOURCES = \
	closedir.c collate.c collcmp.c creat.c dirfd.c \
	fnmatch.c glob.c _isatty.c isatty.c \
	opendir.c readdir.c readdir_r.c \
	regcomp.c regerror.c regexec.c regfree.c \
	rewinddir.c sleep.c usleep.c \
	telldir.c ftw.c nftw.c

ELIX_2_SOURCES = \
        scandir.c seekdir.c

ELIX_3_SOURCES = \
	execl.c execle.c execlp.c execv.c \
	execve.c execvp.c wordexp.c wordfree.c

ELIX_4_SOURCES = \
	popen.c posix_spawn.c

if ELIX_LEVEL_1
ELIX_SOURCES =
else 
if ELIX_LEVEL_2
ELIX_SOURCES = $(ELIX_2_SOURCES)
else 
if ELIX_LEVEL_3
ELIX_SOURCES = $(ELIX_2_SOURCES) $(ELIX_3_SOURCES)
else
ELIX_SOURCES = $(ELIX_2_SOURCES) $(ELIX_3_SOURCES) $(ELIX_4_SOURCES)
endif
endif
endif

noinst_LIBRARIES = lib.a
lib_a_SOURCES = $(GENERAL_SOURCES) $(ELIX_SOURCES)
lib_a_CFLAGS = $(AM_CFLAGS)

AM_CFLAGS = -D_GNU_SOURCE
