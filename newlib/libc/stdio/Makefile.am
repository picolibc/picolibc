## Process this file with automake to generate Makefile.in

AUTOMAKE_OPTIONS = cygnus

INCLUDES = $(NEWLIB_CFLAGS) $(CROSS_CFLAGS) $(TARGET_CFLAGS)

noinst_LIBRARIES = lib.a

lib_a_SOURCES = \
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
	fseek.c 			\
	fsetpos.c 			\
	ftell.c 			\
	fvwrite.c 			\
	fwalk.c 			\
	fwrite.c 			\
	getc.c 				\
	getchar.c 			\
	gets.c 				\
	getw.c				\
	iprintf.c 			\
	makebuf.c			\
	mktemp.c 			\
	perror.c 			\
	printf.c 			\
	putc.c 				\
	putchar.c 			\
	puts.c 				\
	putw.c				\
	refill.c 			\
	remove.c 			\
	rename.c 			\
	rewind.c 			\
	rget.c 				\
	scanf.c 			\
	setbuf.c 			\
	setvbuf.c 			\
	siprintf.c 			\
	snprintf.c			\
	sprintf.c 			\
	sscanf.c 			\
	stdio.c 			\
	tmpfile.c 			\
	tmpnam.c 			\
	ungetc.c 			\
	vfprintf.c 			\
	vfscanf.c 			\
	vprintf.c 			\
	vscanf.c			\
	vsnprintf.c			\
	vsprintf.c 			\
	vsscanf.c			\
	wbuf.c 				\
	wsetup.c 

lib_a_LIBADD = vfiprintf.o

vfiprintf.o: vfprintf.c
	$(COMPILE) -DINTEGER_ONLY -c $(srcdir)/vfprintf.c -o $@

CHEWOUT_FILES = \
	clearerr.def		\
	fclose.def		\
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
	gets.def		\
	getw.def		\
	iprintf.def		\
	mktemp.def		\
	perror.def		\
	putc.def		\
	putchar.def		\
	puts.def		\
	putw.def		\
	remove.def		\
	rename.def		\
	rewind.def		\
	setbuf.def		\
	setvbuf.def		\
	siprintf.def		\
	sprintf.def		\
	sscanf.def		\
	tmpfile.def		\
	tmpnam.def		\
	vfprintf.def		\
	vfscanf.def

SUFFIXES = .def

CHEW = ../../doc/makedoc -f $(srcdir)/../../doc/doc.str

.c.def:
	$(CHEW) < $< > $*.def 2> $*.ref
	touch stmp-def

TARGETDOC = ../tmp.texi

doc: $(CHEWOUT_FILES)
	cat $(srcdir)/stdio.tex >> $(TARGETDOC)

CLEANFILES = $(CHEWOUT_FILES) *.ref

fclose.o: local.h
fdopen.o: local.h
fflush.o: local.h
findfp.o: local.h
fopen.o: local.h
fputs.o: fvwrite.h
fread.o: local.h
freopen.o: local.h
fseek.o: local.h
ftell.o: local.h
fvwrite.o: local.h fvwrite.h
fwalk.o: local.h
fwrite.o: local.h fvwrite.h
makebuf.o: local.h
puts.o: fvwrite.h
refill.o: local.h
scanf.o: local.h
setbuf.o: local.h
setvbuf.o: local.h
siprintf.o: local.h
sprintf.o: local.h
sscanf.o: local.h
stdio.o: local.h
ungetc.o: local.h
vfiprintf.o: local.h
vfprintf.o: local.h
vfscanf.o: local.h floatio.h
vscanf.o: local.h
vsscanf.o: local.h
wbuf.o: local.h fvwrite.h
wsetup.o: local.h
