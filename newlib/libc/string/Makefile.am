## Process this file with automake to generate Makefile.in

AUTOMAKE_OPTIONS = cygnus

INCLUDES = $(NEWLIB_CFLAGS) $(CROSS_CFLAGS) $(TARGET_CFLAGS)

noinst_LIBRARIES = lib.a

lib_a_SOURCES = \
	bcmp.c \
	bcopy.c \
	bzero.c \
	index.c \
	memchr.c \
	memcmp.c \
	memcpy.c \
	memmove.c \
	memset.c \
	rindex.c \
	strcat.c \
	strchr.c \
	strcmp.c \
	strcasecmp.c \
	strcoll.c \
	strcpy.c \
	strcspn.c \
	strerror.c \
	strlen.c  \
	strlwr.c \
	strncat.c \
	strncmp.c \
	strncasecmp.c \
	strncpy.c \
	strpbrk.c \
	strrchr.c \
	strspn.c \
	strtok.c \
	strtok_r.c \
	strupr.c \
	strxfrm.c  \
	strstr.c \
	swab.c \
	u_strerr.c

CHEWOUT_FILES=\
bcmp.def	memcpy.def	strcmp.def	strncat.def strstr.def \
bcopy.def	memmove.def	strcoll.def	strncmp.def strtok.def \
bzero.def	memset.def	strcpy.def	strncpy.def strxfrm.def \
index.def	rindex.def	strcspn.def	strpbrk.def swab.def \
memchr.def	strcat.def	strerror.def	strrchr.def \
memcmp.def	strchr.def	strlen.def	strspn.def \
strcasecmp.def	strncasecmp.def strlwr.def	strupr.def

SUFFIXES = .def

CHEW = ../../doc/makedoc -f $(srcdir)/../../doc/doc.str

.c.def:
	$(CHEW) < $< > $*.def 2> $*.ref
	touch stmp-def

TARGETDOC = ../tmp.texi

doc: $(CHEWOUT_FILES)
	cat $(srcdir)/strings.tex >> $(TARGETDOC)

CLEANFILES = $(CHEWOUT_FILES) *.ref
