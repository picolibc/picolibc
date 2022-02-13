libc_a_SOURCES += \
	%D%/closer.c \
	%D%/reent.c \
	%D%/impure.c \
	%D%/fcntlr.c \
	%D%/fstatr.c \
	%D%/getreent.c \
	%D%/gettimeofdayr.c \
	%D%/isattyr.c \
	%D%/linkr.c \
	%D%/lseekr.c \
	%D%/mkdirr.c \
	%D%/openr.c \
	%D%/readr.c \
	%D%/renamer.c \
	%D%/signalr.c \
	%D%/signgam.c \
	%D%/sbrkr.c \
	%D%/statr.c \
	%D%/timesr.c \
	%D%/unlinkr.c \
	%D%/writer.c

%C%_ELIX_2_SOURCES =
if HAVE_STDIO64_DIR
%C%_ELIX_2_SOURCES += \
	%D%/fstat64r.c \
	%D%/lseek64r.c \
	%D%/stat64r.c \
	%D%/open64r.c
endif
%C%_ELIX_3_SOURCES = %D%/execr.c

if ELIX_LEVEL_1
libc_a_SOURCES +=
else
if ELIX_LEVEL_2
libc_a_SOURCES += $(%C%_ELIX_2_SOURCES)
else
libc_a_SOURCES += $(%C%_ELIX_2_SOURCES) $(%C%_ELIX_3_SOURCES)
endif
endif

LIBC_CHEWOUT_FILES += \
	%D%/closer.def \
	%D%/reent.def \
	%D%/execr.def \
	%D%/fcntlr.def \
	%D%/fstatr.def \
	%D%/gettimeofdayr.def \
	%D%/linkr.def \
	%D%/lseekr.def \
	%D%/mkdirr.def \
	%D%/openr.def \
	%D%/readr.def \
	%D%/renamer.def \
	%D%/signalr.def \
	%D%/sbrkr.def \
	%D%/statr.def \
	%D%/timesr.def \
	%D%/unlinkr.def \
	%D%/fstat64r.def \
	%D%/lseek64r.def \
	%D%/stat64r.def \
	%D%/open64r.def \
	%D%/writer.def

LIBC_CHAPTERS += %D%/reent.tex
