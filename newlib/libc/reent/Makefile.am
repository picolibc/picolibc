## Process this file with automake to generate Makefile.in

AM_CPPFLAGS = $(NEWLIB_CFLAGS) $(TARGET_CFLAGS)

if HAVE_STDIO64_DIR
STDIO64_SOURCES = \
	fstat64r.c \
	lseek64r.c \
	stat64r.c \
	open64r.c
endif

ELIX_2_SOURCES = $(STDIO64_SOURCES)
ELIX_3_SOURCES = execr.c

if ELIX_LEVEL_1
ELIX_SOURCES =
else
if ELIX_LEVEL_2
ELIX_SOURCES = $(ELIX_2_SOURCES)
else
ELIX_SOURCES = $(ELIX_2_SOURCES) $(ELIX_3_SOURCES)
endif
endif

GENERAL_SOURCES = \
	closer.c \
	reent.c \
	impure.c \
	fcntlr.c \
	fstatr.c \
	getreent.c \
	gettimeofdayr.c \
	isattyr.c \
	linkr.c \
	lseekr.c \
	mkdirr.c \
	openr.c \
	readr.c \
	renamer.c \
	signalr.c \
	signgam.c \
	sbrkr.c \
	statr.c \
	timesr.c \
	unlinkr.c \
	writer.c

noinst_LIBRARIES = lib.a
lib_a_SOURCES = $(GENERAL_SOURCES) $(ELIX_SOURCES)
lib_a_CFLAGS = $(AM_CFLAGS)

$(lpfx)impure.o: $(srcdir)/impure.c $(srcdir)/../include/sys/reent.h
