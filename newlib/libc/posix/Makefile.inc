if HAVE_POSIX_DIR

libc_a_SOURCES += \
	%D%/closedir.c %D%/collate.c %D%/collcmp.c %D%/creat.c %D%/dirfd.c \
	%D%/fnmatch.c %D%/glob.c %D%/_isatty.c %D%/isatty.c \
	%D%/opendir.c %D%/readdir.c %D%/readdir_r.c \
	%D%/regcomp.c %D%/regerror.c %D%/regexec.c %D%/regfree.c \
	%D%/rewinddir.c %D%/sleep.c %D%/usleep.c \
	%D%/telldir.c %D%/ftw.c %D%/nftw.c

%C%_ELIX_2_SOURCES = \
	%D%/scandir.c %D%/seekdir.c

%C%_ELIX_3_SOURCES = \
	%D%/execl.c %D%/execle.c %D%/execlp.c %D%/execv.c \
	%D%/execve.c %D%/execvp.c %D%/wordexp.c %D%/wordfree.c

%C%_ELIX_4_SOURCES = \
	%D%/popen.c %D%/posix_spawn.c

if ELIX_LEVEL_1
libc_a_SOURCES +=
else
if ELIX_LEVEL_2
libc_a_SOURCES += $(%C%_ELIX_2_SOURCES)
else
if ELIX_LEVEL_3
libc_a_SOURCES += $(%C%_ELIX_2_SOURCES) $(%C%_ELIX_3_SOURCES)
else
libc_a_SOURCES += $(%C%_ELIX_2_SOURCES) $(%C%_ELIX_3_SOURCES) $(%C%_ELIX_4_SOURCES)
endif
endif
endif

endif

libc_a_CPPFLAGS_%C% = -D_GNU_SOURCE

LIBC_CHEWOUT_FILES += \
	%D%/popen.def \
	%D%/posix_spawn.def

LIBC_CHAPTERS += %D%/posix.tex
