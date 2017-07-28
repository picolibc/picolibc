## Process this file with automake to generate Makefile.in

AUTOMAKE_OPTIONS = cygnus

INCLUDES = $(NEWLIB_CFLAGS) $(CROSS_CFLAGS) $(TARGET_CFLAGS)

if NEWLIB_NANO_FORMATTED_IO
GENERAL_INT_FORMATTED_IO_SOURCES =
else
GENERAL_INT_FORMATTED_IO_SOURCES = \
	fiprintf.c			\
	fiscanf.c			\
	iprintf.c			\
	iscanf.c			\
	siprintf.c			\
	siscanf.c			\
	sniprintf.c			\
	vdiprintf.c			\
	vfprintf.c			\
	vfwprintf.c			\
	viprintf.c			\
	viscanf.c			\
	vsiprintf.c			\
	vsiscanf.c			\
	vsniprintf.c
endif

GENERAL_SOURCES = \
	$(GENERAL_INT_FORMATTED_IO_SOURCES) \
	clearerr.c			\
	fclose.c			\
	fdopen.c			\
	feof.c				\
	ferror.c			\
	fflush.c			\
	fgetc.c			\
	fgetpos.c			\
	fgets.c			\
	fileno.c			\
	findfp.c			\
	flags.c			\
	fopen.c			\
	fprintf.c			\
	fputc.c			\
	fputs.c			\
	fread.c			\
	freopen.c			\
	fscanf.c			\
	fseek.c			\
	fsetpos.c			\
	ftell.c			\
	fvwrite.c			\
	fwalk.c			\
	fwrite.c			\
	getc.c				\
	getchar.c			\
	getc_u.c			\
	getchar_u.c			\
	getdelim.c			\
	getline.c			\
	gets.c				\
	makebuf.c			\
	perror.c			\
	printf.c			\
	putc.c				\
	putchar.c			\
	putc_u.c			\
	putchar_u.c			\
	puts.c				\
	refill.c			\
	remove.c			\
	rename.c			\
	rewind.c			\
	rget.c				\
	scanf.c			\
	sccl.c				\
	setbuf.c			\
	setbuffer.c			\
	setlinebuf.c			\
	setvbuf.c			\
	snprintf.c			\
	sprintf.c			\
	sscanf.c			\
	stdio.c			\
	tmpfile.c			\
	tmpnam.c			\
	ungetc.c			\
	vdprintf.c			\
	vprintf.c			\
	vscanf.c			\
	vsnprintf.c			\
	vsprintf.c			\
	vsscanf.c			\
	wbuf.c				\
	wsetup.c

## The following are EL/IX level 2 interfaces
if ELIX_LEVEL_1
ELIX_2_SOURCES =
else
if NEWLIB_NANO_FORMATTED_IO
ELIX_2_INT_FORMATTED_IO_SOURCES =
else
ELIX_2_INT_FORMATTED_IO_SOURCES = \
	asiprintf.c		\
	vasiprintf.c

endif !NEWLIB_NANO_FORMATTED_IO
ELIX_2_SOURCES = \
	$(ELIX_2_INT_FORMATTED_IO_SOURCES) \
	asprintf.c		\
	fcloseall.c		\
	fseeko.c		\
	ftello.c		\
	getw.c			\
	mktemp.c		\
	putw.c			\
	vasprintf.c
endif !ELIX_LEVEL_1

## The following are EL/IX level 2 interfaces
if ELIX_LEVEL_1
ELIX_4_SOURCES =
else
if ELIX_LEVEL_2
ELIX_4_SOURCES =
else
if ELIX_LEVEL_3
ELIX_4_SOURCES =
else
if NEWLIB_NANO_FORMATTED_IO
ELIX_4_INT_FORMATTED_IO_SOURCES =
else
ELIX_4_INT_FORMATTED_IO_SOURCES = \
	asniprintf.c		\
	diprintf.c		\
	vasniprintf.c

endif !NEWLIB_NANO_FORMATTED_IO
ELIX_4_SOURCES = \
	$(ELIX_4_INT_FORMATTED_IO_SOURCES) \
	asnprintf.c		\
	clearerr_u.c		\
	dprintf.c		\
	feof_u.c		\
	ferror_u.c		\
	fflush_u.c		\
	fgetc_u.c		\
	fgets_u.c		\
	fgetwc.c		\
	fgetwc_u.c		\
	fgetws.c		\
	fgetws_u.c		\
	fileno_u.c		\
	fmemopen.c		\
	fopencookie.c		\
	fpurge.c		\
	fputc_u.c		\
	fputs_u.c		\
	fputwc.c		\
	fputwc_u.c		\
	fputws.c		\
	fputws_u.c		\
	fread_u.c		\
	fsetlocking.c		\
	funopen.c		\
	fwide.c			\
	fwprintf.c		\
	fwrite_u.c		\
	fwscanf.c		\
	getwc.c			\
	getwc_u.c		\
	getwchar.c		\
	getwchar_u.c		\
	open_memstream.c	\
	putwc.c			\
	putwc_u.c		\
	putwchar.c		\
	putwchar_u.c		\
	stdio_ext.c		\
	swprintf.c		\
	swscanf.c		\
	ungetwc.c		\
	vasnprintf.c		\
	vswprintf.c		\
	vswscanf.c		\
	vwprintf.c		\
	vwscanf.c		\
	wprintf.c		\
	wscanf.c

endif !ELIX_LEVEL_3
endif !ELIX_LEVEL_2
endif !ELIX_LEVEL_1

if NEWLIB_NANO_FORMATTED_IO
LIBADD_OBJS = \
	$(lpfx)nano-vfprintf_float.$(oext)	\
	$(lpfx)nano-svfprintf.$(oext)		\
	$(lpfx)nano-svfscanf.$(oext)		\
	$(lpfx)nano-vfprintf.$(oext)		\
	$(lpfx)nano-vfprintf_i.$(oext)		\
	$(lpfx)nano-vfscanf.$(oext)		\
	$(lpfx)nano-vfscanf_i.$(oext)		\
	$(lpfx)nano-vfscanf_float.$(oext)	\
	$(lpfx)svfiwprintf.$(oext)		\
	$(lpfx)svfwprintf.$(oext)		\
	$(lpfx)vfiwprintf.$(oext)		\
	$(lpfx)svfiwscanf.$(oext)		\
	$(lpfx)svfwscanf.$(oext)		\
	$(lpfx)vfiwscanf.$(oext)		\
	$(lpfx)vfwscanf.$(oext)
else
LIBADD_OBJS = \
	$(lpfx)svfiprintf.$(oext) $(lpfx)svfprintf.$(oext) \
	$(lpfx)svfiscanf.$(oext) $(lpfx)svfscanf.$(oext) \
	$(lpfx)vfiprintf.$(oext) \
	$(lpfx)vfscanf.$(oext) $(lpfx)vfiscanf.$(oext) \
	$(lpfx)svfiwprintf.$(oext) $(lpfx)svfwprintf.$(oext) \
	$(lpfx)vfiwprintf.$(oext) \
	$(lpfx)svfiwscanf.$(oext) $(lpfx)svfwscanf.$(oext) \
	$(lpfx)vfiwscanf.$(oext) $(lpfx)vfwscanf.$(oext)
endif

libstdio_la_LDFLAGS = -Xcompiler -nostdlib

if USE_LIBTOOL
noinst_LTLIBRARIES = libstdio.la
libstdio_la_SOURCES = $(GENERAL_SOURCES) $(ELIX_2_SOURCES) $(ELIX_4_SOURCES)
libstdio_la_LIBADD = $(LIBADD_OBJS)
libstdio_la_DEPENDENCIES = $(LIBADD_OBJS)
LIB_COMPILE = $(LTCOMPILE)
noinst_DATA = objectlist.awk.in
else
noinst_LIBRARIES = lib.a
lib_a_SOURCES = $(GENERAL_SOURCES) $(ELIX_2_SOURCES) $(ELIX_4_SOURCES)
lib_a_LIBADD = $(LIBADD_OBJS)
lib_a_CFLAGS = $(AM_CFLAGS)
lib_a_DEPENDENCIES = $(LIBADD_OBJS)
LIB_COMPILE = $(COMPILE)
noinst_DATA =
endif # USE_LIBTOOL

include $(srcdir)/../../Makefile.shared


# Though small footprint nano-formatted-IO implementation is used
# when NEWLIB_NANO_FORMATTED_IO is enabled, we keep all rules for
# the other implementation of formatted IO including all i-family
# functions.  The object files in !NEWLIB_NANO_FORMATTED_IO version
# implementation will be neither compiled nor archived into final
# library, because they are not depended on by final makefile target.

if NEWLIB_NANO_FORMATTED_IO
# Rules compiling small-footprint nano-formatted-io implementation.
$(lpfx)nano-vfprintf.$(oext): nano-vfprintf.c
	$(LIB_COMPILE) -c $(srcdir)/nano-vfprintf.c -o $@

$(lpfx)nano-vfprintf_i.$(oext): nano-vfprintf_i.c
	$(LIB_COMPILE) -c $(srcdir)/nano-vfprintf_i.c -o $@

$(lpfx)nano-vfprintf_float.$(oext): nano-vfprintf_float.c
	$(LIB_COMPILE) -c $(srcdir)/nano-vfprintf_float.c -o $@

$(lpfx)nano-svfprintf.$(oext): nano-vfprintf.c
	$(LIB_COMPILE) -DSTRING_ONLY -c $(srcdir)/nano-vfprintf.c -o $@
endif

# This rule is needed so that libtool compiles vfiprintf before vfprintf.
# Otherwise libtool moves vfprintf.o and subsequently can't find it.

$(lpfx)vfiprintf.$(oext): vfprintf.c
	$(LIB_COMPILE) -DINTEGER_ONLY -c $(srcdir)/vfprintf.c -o $@

$(lpfx)svfprintf.$(oext): vfprintf.c
	$(LIB_COMPILE) -DSTRING_ONLY -c $(srcdir)/vfprintf.c -o $@

$(lpfx)svfiprintf.$(oext): vfprintf.c
	$(LIB_COMPILE) -DINTEGER_ONLY -DSTRING_ONLY -c $(srcdir)/vfprintf.c -o $@

$(lpfx)vfiwprintf.$(oext): vfwprintf.c
	$(LIB_COMPILE) -DINTEGER_ONLY -c $(srcdir)/vfwprintf.c -o $@

$(lpfx)svfwprintf.$(oext): vfwprintf.c
	$(LIB_COMPILE) -DSTRING_ONLY -c $(srcdir)/vfwprintf.c -o $@

$(lpfx)svfiwprintf.$(oext): vfwprintf.c
	$(LIB_COMPILE) -DINTEGER_ONLY -DSTRING_ONLY -c $(srcdir)/vfwprintf.c -o $@

if NEWLIB_NANO_FORMATTED_IO
# Rules compiling small-footprint nano-formatted-io implementation.
$(lpfx)nano-vfscanf.$(oext): nano-vfscanf.c
	$(LIB_COMPILE) -c $(srcdir)/nano-vfscanf.c -o $@

$(lpfx)nano-vfscanf_i.$(oext): nano-vfscanf_i.c
	$(LIB_COMPILE) -c $(srcdir)/nano-vfscanf_i.c -o $@

$(lpfx)nano-vfscanf_float.$(oext): nano-vfscanf_float.c
	$(LIB_COMPILE) -c $(srcdir)/nano-vfscanf_float.c -o $@

$(lpfx)nano-svfscanf.$(oext): nano-vfscanf.c
	$(LIB_COMPILE) -DSTRING_ONLY -c $(srcdir)/nano-vfscanf.c -o $@
endif

$(lpfx)vfscanf.$(oext): vfscanf.c
	$(LIB_COMPILE) -c $(srcdir)/vfscanf.c -o $@

$(lpfx)vfiscanf.$(oext): vfscanf.c
	$(LIB_COMPILE) -DINTEGER_ONLY -c $(srcdir)/vfscanf.c -o $@

$(lpfx)svfscanf.$(oext): vfscanf.c
	$(LIB_COMPILE) -DSTRING_ONLY -c $(srcdir)/vfscanf.c -o $@

$(lpfx)svfiscanf.$(oext): vfscanf.c
	$(LIB_COMPILE) -DINTEGER_ONLY -DSTRING_ONLY -c $(srcdir)/vfscanf.c -o $@

$(lpfx)vfwscanf.$(oext): vfwscanf.c
	$(LIB_COMPILE) -c $(srcdir)/vfwscanf.c -o $@

$(lpfx)vfiwscanf.$(oext): vfwscanf.c
	$(LIB_COMPILE) -DINTEGER_ONLY -c $(srcdir)/vfwscanf.c -o $@

$(lpfx)svfwscanf.$(oext): vfwscanf.c
	$(LIB_COMPILE) -DSTRING_ONLY -c $(srcdir)/vfwscanf.c -o $@

$(lpfx)svfiwscanf.$(oext): vfwscanf.c
	$(LIB_COMPILE) -DINTEGER_ONLY -DSTRING_ONLY -c $(srcdir)/vfwscanf.c -o $@

if NEWLIB_NANO_FORMATTED_IO
CHEWOUT_INT_FORMATTED_IO_FILES =\
	nano-vfprintf.def	\
	nano-vfprintf_i.def	\
	nano-vfprintf_float.def	\
	nano-vfscanf.def	\
	nano-vfscanf_i.def	\
	nano-vfscanf_float.def
else
CHEWOUT_INT_FORMATTED_IO_FILES =\
	diprintf.def		\
	siprintf.def		\
	siscanf.def		\
	vfprintf.def		\
	vfscanf.def		\
	viprintf.def		\
	viscanf.def
endif
CHEWOUT_FILES = \
	$(CHEWOUT_INT_FORMATTED_IO_FILES)	\
	clearerr.def		\
	dprintf.def		\
	fclose.def		\
	fcloseall.def		\
	fdopen.def		\
	feof.def		\
	ferror.def		\
	fflush.def		\
	fgetc.def		\
	fgetpos.def		\
	fgets.def		\
	fgetwc.def		\
	fgetws.def		\
	fileno.def		\
	fmemopen.def		\
	fopen.def		\
	fopencookie.def		\
	fpurge.def		\
	fputc.def		\
	fputs.def		\
	fputwc.def		\
	fputws.def		\
	fread.def		\
	freopen.def		\
	fseek.def		\
	fsetlocking.def		\
	fsetpos.def		\
	ftell.def		\
	funopen.def		\
	fwide.def		\
	fwrite.def		\
	getc.def		\
	getc_u.def		\
	getchar.def		\
	getchar_u.def		\
	getdelim.def		\
	getline.def		\
	gets.def		\
	getw.def		\
	getwchar.def		\
	mktemp.def		\
	open_memstream.def	\
	perror.def		\
	putc.def		\
	putc_u.def		\
	putchar.def		\
	putchar_u.def		\
	puts.def		\
	putw.def		\
	putwchar.def		\
	remove.def		\
	rename.def		\
	rewind.def		\
	setbuf.def		\
	setbuffer.def		\
	setlinebuf.def		\
	setvbuf.def		\
	sprintf.def		\
	sscanf.def		\
	stdio_ext.def		\
	swprintf.def		\
	swscanf.def		\
	tmpfile.def		\
	tmpnam.def		\
	ungetc.def		\
	ungetwc.def		\
	vfwprintf.def		\
	vfwscanf.def

CHAPTERS = stdio.tex

$(lpfx)clearerr.$(oext): local.h
$(lpfx)clearerr_u.$(oext): local.h
$(lpfx)fclose.$(oext): local.h
$(lpfx)fdopen.$(oext): local.h
$(lpfx)feof.$(oext): local.h
$(lpfx)feof_u.$(oext): local.h
$(lpfx)ferror.$(oext): local.h
$(lpfx)ferror_u.$(oext): local.h
$(lpfx)fflush.$(oext): local.h
$(lpfx)fflush_u.$(oext): fflush.c
$(lpfx)fgetc.$(oext): local.h
$(lpfx)fgetc_u.$(oext): local.h
$(lpfx)fgets.$(oext): local.h
$(lpfx)fgets_u.$(oext): fgets.c
$(lpfx)fgetwc.$(oext): local.h
$(lpfx)fgetwc_u.$(oext): local.h
$(lpfx)fgetws.$(oext): local.h
$(lpfx)fgetws_u.$(oext): fgetws.c
$(lpfx)fileno.$(oext): local.h
$(lpfx)fileno_u.$(oext): local.h
$(lpfx)findfp.$(oext): local.h
$(lpfx)fmemopen.$(oext): local.h
$(lpfx)fopen.$(oext): local.h
$(lpfx)fopencookie.$(oext): local.h
$(lpfx)fpurge.$(oext): local.h
$(lpfx)fputc.$(oext): local.h
$(lpfx)fputc_u.$(oext): local.h
$(lpfx)fputs.$(oext): fvwrite.h
$(lpfx)fputs_u.$(oext): fputs.c
$(lpfx)fputwc.$(oext): local.h
$(lpfx)fputwc_u.$(oext): local.h
$(lpfx)fputws.$(oext): local.h fvwrite.h
$(lpfx)fputws_u.$(oext): fputws.c
$(lpfx)fread.$(oext): local.h
$(lpfx)fread_u.$(oext): fread.c
$(lpfx)freopen.$(oext): local.h
$(lpfx)fseek.$(oext): local.h
$(lpfx)fsetlocking.$(oext): local.h
$(lpfx)ftell.$(oext): local.h
$(lpfx)funopen.$(oext): local.h
$(lpfx)fvwrite.$(oext): local.h fvwrite.h
$(lpfx)fwalk.$(oext): local.h
$(lpfx)fwide.$(oext): local.h
$(lpfx)fwprintf.$(oext): local.h
$(lpfx)fwrite.$(oext): local.h fvwrite.h
$(lpfx)fwrite_u.$(oext): fwrite.c
$(lpfx)fwscanf.$(oext): local.h
$(lpfx)getwc.$(oext): local.h
$(lpfx)getwc_u.$(oext): local.h
$(lpfx)getwchar.$(oext): local.h
$(lpfx)getwchar_u.$(oext): local.h
$(lpfx)iscanf.$(oext): local.h
$(lpfx)makebuf.$(oext): local.h
$(lpfx)open_memstream.$(oext): local.h
$(lpfx)puts.$(oext): fvwrite.h
$(lpfx)putwc.$(oext): local.h
$(lpfx)putwc_u.$(oext): local.h
$(lpfx)putwchar.$(oext): local.h
$(lpfx)putwchar_u.$(oext): local.h
$(lpfx)refill.$(oext): local.h
$(lpfx)scanf.$(oext): local.h
$(lpfx)setbuf.$(oext): local.h
$(lpfx)setvbuf.$(oext): local.h
$(lpfx)siprintf.$(oext): local.h
$(lpfx)siscanf.$(oext): local.h
$(lpfx)sniprintf.$(oext): local.h
$(lpfx)sprintf.$(oext): local.h
$(lpfx)sscanf.$(oext): local.h
$(lpfx)stdio.$(oext): local.h
if NEWLIB_NANO_FORMATTED_IO
$(lpfx)nano-svfprintf.$(oext): local.h nano-vfprintf_local.h
$(lpfx)nano-svfscanf.$(oext): local.h nano-vfscanf_local.h
endif
$(lpfx)svfiprintf.$(oext): local.h
$(lpfx)svfiscanf.$(oext): local.h floatio.h
$(lpfx)svfprintf.$(oext): local.h
$(lpfx)svfscanf.$(oext): local.h floatio.h
$(lpfx)swprintf.$(oext): local.h
$(lpfx)swscanf.$(oext): local.h
$(lpfx)ungetc.$(oext): local.h
$(lpfx)ungetwc.$(oext): local.h
if NEWLIB_NANO_FORMATTED_IO
$(lpfx)nano-vfprintf.$(oext): local.h nano-vfprintf_local.h
$(lpfx)nano-vfprintf_i.$(oext): local.h nano-vfprintf_local.h
$(lpfx)nano-vfprintf_float.$(oext): local.h floatio.h nano-vfprintf_local.h
$(lpfx)nano-vfscanf.$(oext): local.h nano-vfscanf_local.h
$(lpfx)nano-vfscanf_i.$(oext): local.h nano-vfscanf_local.h
$(lpfx)nano-vfscanf_float.$(oext): local.h floatio.h nano-vfscanf_local.h
endif
$(lpfx)vfiprintf.$(oext): local.h
$(lpfx)vfiscanf.$(oext): local.h floatio.h
$(lpfx)vfprintf.$(oext): local.h
$(lpfx)vfscanf.$(oext): local.h floatio.h
$(lpfx)vfwprintf.$(oext): local.h
$(lpfx)vfwscanf.$(oext): local.h
$(lpfx)viscanf.$(oext): local.h
$(lpfx)vscanf.$(oext): local.h
$(lpfx)vsiscanf.$(oext): local.h
$(lpfx)vsniprintf.$(oext): local.h
$(lpfx)vsscanf.$(oext): local.h
$(lpfx)vswprintf.$(oext): local.h
$(lpfx)vswscanf.$(oext): local.h
$(lpfx)vwprintf.$(oext): local.h
$(lpfx)vwscanf.$(oext): local.h
$(lpfx)wbuf.$(oext): local.h fvwrite.h
$(lpfx)wprintf.$(oext): local.h
$(lpfx)wscanf.$(oext): local.h
$(lpfx)wsetup.$(oext): local.h
