## Process this file with automake to generate Makefile.in

AUTOMAKE_OPTIONS = cygnus

INCLUDES = $(NEWLIB_CFLAGS) $(CROSS_CFLAGS) $(TARGET_CFLAGS)

GENERAL_SOURCES = \
	clearerr.c 			\
	fclose.c 			\
	fdopen.c 			\
	feof.c 				\
	ferror.c 			\
	fflush.c 			\
	fgetc.c 			\
	fgetpos.c 			\
	fgets.c 			\
	fileno.c			\
	findfp.c 			\
	fiprintf.c 			\
	flags.c 			\
	fopen.c 			\
	fprintf.c 			\
	fputc.c 			\
	fputs.c 			\
	fread.c 			\
	freopen.c 			\
	fscanf.c 			\
	fiscanf.c 			\
	fseek.c 			\
	fsetpos.c 			\
	ftell.c 			\
	fvwrite.c 			\
	fwalk.c 			\
	fwrite.c 			\
	getc.c 				\
	getchar.c 			\
	getc_u.c			\
	getchar_u.c			\
	getdelim.c			\
	getline.c			\
	gets.c 				\
	iprintf.c 			\
	iscanf.c 			\
	makebuf.c			\
	perror.c 			\
	printf.c 			\
	putc.c 				\
	putchar.c 			\
	putc_u.c			\
	putchar_u.c			\
	puts.c 				\
	refill.c 			\
	remove.c 			\
	rename.c 			\
	rewind.c 			\
	rget.c 				\
	scanf.c 			\
	sccl.c	 			\
	setbuf.c 			\
	setbuffer.c 			\
	setlinebuf.c 			\
	setvbuf.c 			\
	siprintf.c 			\
	siscanf.c 			\
	sniprintf.c			\
	snprintf.c			\
	sprintf.c 			\
	sscanf.c 			\
	stdio.c 			\
	tmpfile.c 			\
	tmpnam.c 			\
	ungetc.c 			\
	viprintf.c 			\
	viscanf.c			\
	vprintf.c 			\
	vscanf.c			\
	vsiprintf.c 			\
	vsiscanf.c			\
	vsnprintf.c			\
	vsniprintf.c			\
	vsprintf.c 			\
	vsscanf.c			\
	wbuf.c 				\
	wsetup.c 

## The following are EL/IX level 2 interfaces
if ELIX_LEVEL_1
LIB_OBJS =
else
LIB_OBJS = \
	asiprintf.$(oext)		\
	asprintf.$(oext)		\
	fcloseall.$(oext)		\
	fseeko.$(oext) 			\
	ftello.$(oext) 			\
	getw.$(oext)			\
	mktemp.$(oext) 			\
	putw.$(oext) 			\
	vasiprintf.$(oext)		\
	vasprintf.$(oext)
endif

LIBADD_OBJS = vfiprintf.$(oext) vfprintf.$(oext) vfscanf.$(oext) vfiscanf.$(oext)

libstdio_la_LDFLAGS = -Xcompiler -nostdlib

if USE_LIBTOOL
noinst_LTLIBRARIES = libstdio.la
libstdio_la_SOURCES = $(GENERAL_SOURCES)
libstdio_la_LIBADD = $(LIBADD_OBJS) $(LIB_OBJS)
libstdio_la_DEPENDENCIES = $(LIBADD_OBJS) $(LIB_OBJS)
LIB_COMPILE = $(LTCOMPILE)
noinst_DATA = objectlist.awk.in
else
noinst_LIBRARIES = lib.a
lib_a_SOURCES = $(GENERAL_SOURCES)
lib_a_LIBADD = $(LIBADD_OBJS) $(LIB_OBJS)
lib_a_DEPENDENCIES = $(LIBADD_OBJS) $(LIB_OBJS)
LIB_COMPILE = $(COMPILE)
noinst_DATA =
endif # USE_LIBTOOL

include $(srcdir)/../../Makefile.shared

# This rule is needed so that libtool compiles vfiprintf before vfprintf. Otherwise
# libtool moves vfprintf.o and subsequently can't find it.

vfprintf.$(oext): vfprintf.c
	$(LIB_COMPILE) -fshort-enums -c $(srcdir)/vfprintf.c

vfiprintf.$(oext): vfprintf.c
	$(LIB_COMPILE) -fshort-enums -DINTEGER_ONLY -c $(srcdir)/vfprintf.c -o $@

vfscanf.$(oext): vfscanf.c
	$(LIB_COMPILE) -c $(srcdir)/vfscanf.c

vfiscanf.$(oext): vfscanf.c
	$(LIB_COMPILE) -DINTEGER_ONLY -c $(srcdir)/vfscanf.c -o $@

CHEWOUT_FILES = \
	clearerr.def		\
	fclose.def		\
	fcloseall.def		\
	fdopen.def		\
	feof.def		\
	ferror.def		\
	fflush.def		\
	fgetc.def		\
	fgetpos.def		\
	fgets.def		\
	fileno.def		\
	fiprintf.def		\
	fopen.def		\
	fputc.def		\
	fputs.def		\
	fread.def		\
	freopen.def		\
	fseek.def		\
	fsetpos.def		\
	ftell.def		\
	fwrite.def		\
	getc.def		\
	getchar.def		\
	getc_u.def		\
	getchar_u.def		\
	getdelim.def		\
	getline.def		\
	gets.def		\
	getw.def		\
	mktemp.def		\
	perror.def		\
	putc.def		\
	putchar.def		\
	putc_u.def		\
	putchar_u.def		\
	puts.def		\
	putw.def		\
	remove.def		\
	rename.def		\
	rewind.def		\
	setbuf.def		\
	setbuffer.def		\
	setlinebuf.def		\
	setvbuf.def		\
	siprintf.def		\
	siscanf.def		\
	sprintf.def		\
	sscanf.def		\
	tmpfile.def		\
	tmpnam.def		\
	vfprintf.def		\
	vfscanf.def		\
	viprintf.def		\
	viscanf.def

SUFFIXES = .def

CHEW = ../../doc/makedoc -f $(srcdir)/../../doc/doc.str

.c.def:
	$(CHEW) < $< > $*.def 2> $*.ref
	touch stmp-def

TARGETDOC = ../tmp.texi

doc: $(CHEWOUT_FILES)
	cat $(srcdir)/stdio.tex >> $(TARGETDOC)

CLEANFILES = $(CHEWOUT_FILES) *.ref

fclose.$(oext): local.h
fdopen.$(oext): local.h
fflush.$(oext): local.h
findfp.$(oext): local.h
fopen.$(oext): local.h
fputs.$(oext): fvwrite.h
fread.$(oext): local.h
freopen.$(oext): local.h
fseek.$(oext): local.h
ftell.$(oext): local.h
fvwrite.$(oext): local.h fvwrite.h
fwalk.$(oext): local.h
fwrite.$(oext): local.h fvwrite.h
iscanf.$(oext): local.h
makebuf.$(oext): local.h
puts.$(oext): fvwrite.h
refill.$(oext): local.h
scanf.$(oext): local.h
setbuf.$(oext): local.h
setvbuf.$(oext): local.h
siprintf.$(oext): local.h
sniprintf.$(oext): local.h
sprintf.$(oext): local.h
siscanf.$(oext): local.h
sscanf.$(oext): local.h
stdio.$(oext): local.h
ungetc.$(oext): local.h
vfiprintf.$(oext): local.h
vfprintf.$(oext): local.h
vfiscanf.$(oext): local.h floatio.h
vfscanf.$(oext): local.h floatio.h
viscanf.$(oext): local.h
vscanf.$(oext): local.h
vsniprintf.$(oext): local.h
vsiscanf.$(oext): local.h
vsscanf.$(oext): local.h
wbuf.$(oext): local.h fvwrite.h
wsetup.$(oext): local.h
