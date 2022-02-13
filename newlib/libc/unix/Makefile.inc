if HAVE_UNIX_DIR

libc_a_SOURCES += \
	%D%/getcwd.c \
	%D%/pread.c \
	%D%/pwrite.c \
	%D%/sigset.c

## Distinguish different EL/IX level interfaces
%C%_ELIX_2_SOURCES = \
	%D%/ttyname.c \
	%D%/ttyname_r.c

%C%_ELIX_4_SOURCES = \
	%D%/basename.c \
	%D%/dirname.c \
	%D%/getlogin.c \
	%D%/getpass.c \
	%D%/getpwent.c \
	%D%/getut.c

if ELIX_LEVEL_1
else
if ELIX_LEVEL_2
libc_a_SOURCES += $(%C%_ELIX_2_SOURCES)
else
if ELIX_LEVEL_3
libc_a_SOURCES += $(%C%_ELIX_2_SOURCES)
else
libc_a_SOURCES += $(%C%_ELIX_2_SOURCES) $(%C%_ELIX_4_SOURCES)
endif
endif
endif

endif
