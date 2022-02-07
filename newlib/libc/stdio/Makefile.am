## Process this file with automake to generate Makefile.in

AM_CPPFLAGS = $(NEWLIB_CFLAGS) $(CROSS_CFLAGS) $(TARGET_CFLAGS)

if NEWLIB_NANO_FORMATTED_IO
GENERAL_INT_FORMATTED_IO_SOURCES =
else
GENERAL_INT_FORMATTED_IO_SOURCES = \
	%D%/fiprintf.c			\
	%D%/fiscanf.c			\
	%D%/iprintf.c			\
	%D%/iscanf.c			\
	%D%/siprintf.c			\
	%D%/siscanf.c			\
	%D%/sniprintf.c			\
	%D%/vdiprintf.c			\
	%D%/vfprintf.c			\
	%D%/vfwprintf.c			\
	%D%/viprintf.c			\
	%D%/viscanf.c			\
	%D%/vsiprintf.c			\
	%D%/vsiscanf.c			\
	%D%/vsniprintf.c
endif

GENERAL_SOURCES = \
	%D%/$(GENERAL_INT_FORMATTED_IO_SOURCES) \
	%D%/clearerr.c			\
	%D%/fclose.c			\
	%D%/fdopen.c			\
	%D%/feof.c				\
	%D%/ferror.c			\
	%D%/fflush.c			\
	%D%/fgetc.c			\
	%D%/fgetpos.c			\
	%D%/fgets.c			\
	%D%/fileno.c			\
	%D%/findfp.c			\
	%D%/flags.c			\
	%D%/fopen.c			\
	%D%/fprintf.c			\
	%D%/fputc.c			\
	%D%/fputs.c			\
	%D%/fread.c			\
	%D%/freopen.c			\
	%D%/fscanf.c			\
	%D%/fseek.c			\
	%D%/fsetpos.c			\
	%D%/ftell.c			\
	%D%/fvwrite.c			\
	%D%/fwalk.c			\
	%D%/fwrite.c			\
	%D%/getc.c				\
	%D%/getchar.c			\
	%D%/getc_u.c			\
	%D%/getchar_u.c			\
	%D%/getdelim.c			\
	%D%/getline.c			\
	%D%/gets.c				\
	%D%/makebuf.c			\
	%D%/perror.c			\
	%D%/printf.c			\
	%D%/putc.c				\
	%D%/putchar.c			\
	%D%/putc_u.c			\
	%D%/putchar_u.c			\
	%D%/puts.c				\
	%D%/refill.c			\
	%D%/remove.c			\
	%D%/rename.c			\
	%D%/rewind.c			\
	%D%/rget.c				\
	%D%/scanf.c			\
	%D%/sccl.c				\
	%D%/setbuf.c			\
	%D%/setbuffer.c			\
	%D%/setlinebuf.c			\
	%D%/setvbuf.c			\
	%D%/snprintf.c			\
	%D%/sprintf.c			\
	%D%/sscanf.c			\
	%D%/stdio.c			\
	%D%/tmpfile.c			\
	%D%/tmpnam.c			\
	%D%/ungetc.c			\
	%D%/vdprintf.c			\
	%D%/vprintf.c			\
	%D%/vscanf.c			\
	%D%/vsnprintf.c			\
	%D%/vsprintf.c			\
	%D%/vsscanf.c			\
	%D%/wbuf.c				\
	%D%/wsetup.c

## The following are EL/IX level 2 interfaces
if ELIX_LEVEL_1
ELIX_2_SOURCES =
else
if NEWLIB_NANO_FORMATTED_IO
ELIX_2_INT_FORMATTED_IO_SOURCES =
else
ELIX_2_INT_FORMATTED_IO_SOURCES = \
	%D%/asiprintf.c		\
	%D%/vasiprintf.c

endif !NEWLIB_NANO_FORMATTED_IO
ELIX_2_SOURCES = \
	%D%/$(ELIX_2_INT_FORMATTED_IO_SOURCES) \
	%D%/asprintf.c		\
	%D%/fcloseall.c		\
	%D%/fseeko.c		\
	%D%/ftello.c		\
	%D%/getw.c			\
	%D%/mktemp.c		\
	%D%/putw.c			\
	%D%/vasprintf.c
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
	%D%/asniprintf.c		\
	%D%/diprintf.c		\
	%D%/vasniprintf.c

endif !NEWLIB_NANO_FORMATTED_IO
ELIX_4_SOURCES = \
	%D%/$(ELIX_4_INT_FORMATTED_IO_SOURCES) \
	%D%/asnprintf.c		\
	%D%/clearerr_u.c		\
	%D%/dprintf.c		\
	%D%/feof_u.c		\
	%D%/ferror_u.c		\
	%D%/fflush_u.c		\
	%D%/fgetc_u.c		\
	%D%/fgets_u.c		\
	%D%/fgetwc.c		\
	%D%/fgetwc_u.c		\
	%D%/fgetws.c		\
	%D%/fgetws_u.c		\
	%D%/fileno_u.c		\
	%D%/fmemopen.c		\
	%D%/fopencookie.c		\
	%D%/fpurge.c		\
	%D%/fputc_u.c		\
	%D%/fputs_u.c		\
	%D%/fputwc.c		\
	%D%/fputwc_u.c		\
	%D%/fputws.c		\
	%D%/fputws_u.c		\
	%D%/fread_u.c		\
	%D%/fsetlocking.c		\
	%D%/funopen.c		\
	%D%/fwide.c			\
	%D%/fwprintf.c		\
	%D%/fwrite_u.c		\
	%D%/fwscanf.c		\
	%D%/getwc.c			\
	%D%/getwc_u.c		\
	%D%/getwchar.c		\
	%D%/getwchar_u.c		\
	%D%/open_memstream.c	\
	%D%/putwc.c			\
	%D%/putwc_u.c		\
	%D%/putwchar.c		\
	%D%/putwchar_u.c		\
	%D%/stdio_ext.c		\
	%D%/swprintf.c		\
	%D%/swscanf.c		\
	%D%/ungetwc.c		\
	%D%/vasnprintf.c		\
	%D%/vswprintf.c		\
	%D%/vswscanf.c		\
	%D%/vwprintf.c		\
	%D%/vwscanf.c		\
	%D%/wprintf.c		\
	%D%/wscanf.c

endif !ELIX_LEVEL_3
endif !ELIX_LEVEL_2
endif !ELIX_LEVEL_1

if NEWLIB_NANO_FORMATTED_IO
LIBADD_OBJS = \
	%D%/$(lpfx)nano-vfprintf_float.o	\
	%D%/$(lpfx)nano-svfprintf.o		\
	%D%/$(lpfx)nano-svfscanf.o		\
	%D%/$(lpfx)nano-vfprintf.o		\
	%D%/$(lpfx)nano-vfprintf_i.o		\
	%D%/$(lpfx)nano-vfscanf.o		\
	%D%/$(lpfx)nano-vfscanf_i.o		\
	%D%/$(lpfx)nano-vfscanf_float.o	\
	%D%/$(lpfx)svfiwprintf.o		\
	%D%/$(lpfx)svfwprintf.o		\
	%D%/$(lpfx)vfiwprintf.o		\
	%D%/$(lpfx)svfiwscanf.o		\
	%D%/$(lpfx)svfwscanf.o		\
	%D%/$(lpfx)vfiwscanf.o		\
	%D%/$(lpfx)vfwscanf.o
else
LIBADD_OBJS = \
	%D%/$(lpfx)svfiprintf.o $(lpfx)svfprintf.o \
	%D%/$(lpfx)svfiscanf.o $(lpfx)svfscanf.o \
	%D%/$(lpfx)vfiprintf.o \
	%D%/$(lpfx)vfscanf.o $(lpfx)vfiscanf.o \
	%D%/$(lpfx)svfiwprintf.o $(lpfx)svfwprintf.o \
	%D%/$(lpfx)vfiwprintf.o \
	%D%/$(lpfx)svfiwscanf.o $(lpfx)svfwscanf.o \
	%D%/$(lpfx)vfiwscanf.o $(lpfx)vfwscanf.o
endif

noinst_LIBRARIES = lib.a
lib_a_SOURCES = $(GENERAL_SOURCES) $(ELIX_2_SOURCES) $(ELIX_4_SOURCES)
lib_a_LIBADD = $(LIBADD_OBJS)
lib_a_CFLAGS = $(AM_CFLAGS)
lib_a_DEPENDENCIES = $(LIBADD_OBJS)
LIB_COMPILE = $(AM_V_CC)$(COMPILE)

# Though small footprint nano-formatted-IO implementation is used
# when NEWLIB_NANO_FORMATTED_IO is enabled, we keep all rules for
# the other implementation of formatted IO including all i-family
# functions.  The object files in !NEWLIB_NANO_FORMATTED_IO version
# implementation will be neither compiled nor archived into final
# library, because they are not depended on by final makefile target.

if NEWLIB_NANO_FORMATTED_IO
# Rules compiling small-footprint nano-formatted-io implementation.
$(lpfx)nano-vfprintf.o: nano-vfprintf.c
	%D%/$(LIB_COMPILE) -c $(srcdir)/nano-vfprintf.c -o $@

$(lpfx)nano-vfprintf_i.o: nano-vfprintf_i.c
	%D%/$(LIB_COMPILE) -c $(srcdir)/nano-vfprintf_i.c -o $@

$(lpfx)nano-vfprintf_float.o: nano-vfprintf_float.c
	%D%/$(LIB_COMPILE) -c $(srcdir)/nano-vfprintf_float.c -o $@

$(lpfx)nano-svfprintf.o: nano-vfprintf.c
	%D%/$(LIB_COMPILE) -DSTRING_ONLY -c $(srcdir)/nano-vfprintf.c -o $@
endif

# This rule is needed so that libtool compiles vfiprintf before vfprintf.
# Otherwise libtool moves vfprintf.o and subsequently can't find it.

$(lpfx)vfiprintf.o: vfprintf.c
	%D%/$(LIB_COMPILE) -DINTEGER_ONLY -c $(srcdir)/vfprintf.c -o $@

$(lpfx)svfprintf.o: vfprintf.c
	%D%/$(LIB_COMPILE) -DSTRING_ONLY -c $(srcdir)/vfprintf.c -o $@

$(lpfx)svfiprintf.o: vfprintf.c
	%D%/$(LIB_COMPILE) -DINTEGER_ONLY -DSTRING_ONLY -c $(srcdir)/vfprintf.c -o $@

$(lpfx)vfiwprintf.o: vfwprintf.c
	%D%/$(LIB_COMPILE) -DINTEGER_ONLY -c $(srcdir)/vfwprintf.c -o $@

$(lpfx)svfwprintf.o: vfwprintf.c
	%D%/$(LIB_COMPILE) -DSTRING_ONLY -c $(srcdir)/vfwprintf.c -o $@

$(lpfx)svfiwprintf.o: vfwprintf.c
	%D%/$(LIB_COMPILE) -DINTEGER_ONLY -DSTRING_ONLY -c $(srcdir)/vfwprintf.c -o $@

if NEWLIB_NANO_FORMATTED_IO
# Rules compiling small-footprint nano-formatted-io implementation.
$(lpfx)nano-vfscanf.o: nano-vfscanf.c
	%D%/$(LIB_COMPILE) -c $(srcdir)/nano-vfscanf.c -o $@

$(lpfx)nano-vfscanf_i.o: nano-vfscanf_i.c
	%D%/$(LIB_COMPILE) -c $(srcdir)/nano-vfscanf_i.c -o $@

$(lpfx)nano-vfscanf_float.o: nano-vfscanf_float.c
	%D%/$(LIB_COMPILE) -c $(srcdir)/nano-vfscanf_float.c -o $@

$(lpfx)nano-svfscanf.o: nano-vfscanf.c
	%D%/$(LIB_COMPILE) -DSTRING_ONLY -c $(srcdir)/nano-vfscanf.c -o $@
endif

$(lpfx)vfscanf.o: vfscanf.c
	%D%/$(LIB_COMPILE) -c $(srcdir)/vfscanf.c -o $@

$(lpfx)vfiscanf.o: vfscanf.c
	%D%/$(LIB_COMPILE) -DINTEGER_ONLY -c $(srcdir)/vfscanf.c -o $@

$(lpfx)svfscanf.o: vfscanf.c
	%D%/$(LIB_COMPILE) -DSTRING_ONLY -c $(srcdir)/vfscanf.c -o $@

$(lpfx)svfiscanf.o: vfscanf.c
	%D%/$(LIB_COMPILE) -DINTEGER_ONLY -DSTRING_ONLY -c $(srcdir)/vfscanf.c -o $@

$(lpfx)vfwscanf.o: vfwscanf.c
	%D%/$(LIB_COMPILE) -c $(srcdir)/vfwscanf.c -o $@

$(lpfx)vfiwscanf.o: vfwscanf.c
	%D%/$(LIB_COMPILE) -DINTEGER_ONLY -c $(srcdir)/vfwscanf.c -o $@

$(lpfx)svfwscanf.o: vfwscanf.c
	%D%/$(LIB_COMPILE) -DSTRING_ONLY -c $(srcdir)/vfwscanf.c -o $@

$(lpfx)svfiwscanf.o: vfwscanf.c
	%D%/$(LIB_COMPILE) -DINTEGER_ONLY -DSTRING_ONLY -c $(srcdir)/vfwscanf.c -o $@

$(lpfx)clearerr.o: local.h
$(lpfx)clearerr_u.o: local.h
$(lpfx)fclose.o: local.h
$(lpfx)fdopen.o: local.h
$(lpfx)feof.o: local.h
$(lpfx)feof_u.o: local.h
$(lpfx)ferror.o: local.h
$(lpfx)ferror_u.o: local.h
$(lpfx)fflush.o: local.h
$(lpfx)fflush_u.o: fflush.c
$(lpfx)fgetc.o: local.h
$(lpfx)fgetc_u.o: local.h
$(lpfx)fgets.o: local.h
$(lpfx)fgets_u.o: fgets.c
$(lpfx)fgetwc.o: local.h
$(lpfx)fgetwc_u.o: local.h
$(lpfx)fgetws.o: local.h
$(lpfx)fgetws_u.o: fgetws.c
$(lpfx)fileno.o: local.h
$(lpfx)fileno_u.o: local.h
$(lpfx)findfp.o: local.h
$(lpfx)fmemopen.o: local.h
$(lpfx)fopen.o: local.h
$(lpfx)fopencookie.o: local.h
$(lpfx)fpurge.o: local.h
$(lpfx)fputc.o: local.h
$(lpfx)fputc_u.o: local.h
$(lpfx)fputs.o: fvwrite.h
$(lpfx)fputs_u.o: fputs.c
$(lpfx)fputwc.o: local.h
$(lpfx)fputwc_u.o: local.h
$(lpfx)fputws.o: local.h fvwrite.h
$(lpfx)fputws_u.o: fputws.c
$(lpfx)fread.o: local.h
$(lpfx)fread_u.o: fread.c
$(lpfx)freopen.o: local.h
$(lpfx)fseek.o: local.h
$(lpfx)fsetlocking.o: local.h
$(lpfx)ftell.o: local.h
$(lpfx)funopen.o: local.h
$(lpfx)fvwrite.o: local.h fvwrite.h
$(lpfx)fwalk.o: local.h
$(lpfx)fwide.o: local.h
$(lpfx)fwprintf.o: local.h
$(lpfx)fwrite.o: local.h fvwrite.h
$(lpfx)fwrite_u.o: fwrite.c
$(lpfx)fwscanf.o: local.h
$(lpfx)getwc.o: local.h
$(lpfx)getwc_u.o: local.h
$(lpfx)getwchar.o: local.h
$(lpfx)getwchar_u.o: local.h
$(lpfx)iscanf.o: local.h
$(lpfx)makebuf.o: local.h
$(lpfx)open_memstream.o: local.h
$(lpfx)puts.o: fvwrite.h
$(lpfx)putwc.o: local.h
$(lpfx)putwc_u.o: local.h
$(lpfx)putwchar.o: local.h
$(lpfx)putwchar_u.o: local.h
$(lpfx)refill.o: local.h
$(lpfx)scanf.o: local.h
$(lpfx)setbuf.o: local.h
$(lpfx)setvbuf.o: local.h
$(lpfx)siprintf.o: local.h
$(lpfx)siscanf.o: local.h
$(lpfx)sniprintf.o: local.h
$(lpfx)sprintf.o: local.h
$(lpfx)sscanf.o: local.h
$(lpfx)stdio.o: local.h
if NEWLIB_NANO_FORMATTED_IO
$(lpfx)nano-svfprintf.o: local.h nano-vfprintf_local.h
$(lpfx)nano-svfscanf.o: local.h nano-vfscanf_local.h
endif
$(lpfx)svfiprintf.o: local.h
$(lpfx)svfiscanf.o: local.h floatio.h
$(lpfx)svfprintf.o: local.h
$(lpfx)svfscanf.o: local.h floatio.h
$(lpfx)swprintf.o: local.h
$(lpfx)swscanf.o: local.h
$(lpfx)ungetc.o: local.h
$(lpfx)ungetwc.o: local.h
if NEWLIB_NANO_FORMATTED_IO
$(lpfx)nano-vfprintf.o: local.h nano-vfprintf_local.h
$(lpfx)nano-vfprintf_i.o: local.h nano-vfprintf_local.h
$(lpfx)nano-vfprintf_float.o: local.h floatio.h nano-vfprintf_local.h
$(lpfx)nano-vfscanf.o: local.h nano-vfscanf_local.h
$(lpfx)nano-vfscanf_i.o: local.h nano-vfscanf_local.h
$(lpfx)nano-vfscanf_float.o: local.h floatio.h nano-vfscanf_local.h
endif
$(lpfx)vfiprintf.o: local.h
$(lpfx)vfiscanf.o: local.h floatio.h
$(lpfx)vfprintf.o: local.h
$(lpfx)vfscanf.o: local.h floatio.h
$(lpfx)vfwprintf.o: local.h
$(lpfx)vfwscanf.o: local.h
$(lpfx)viscanf.o: local.h
$(lpfx)vscanf.o: local.h
$(lpfx)vsiscanf.o: local.h
$(lpfx)vsniprintf.o: local.h
$(lpfx)vsscanf.o: local.h
$(lpfx)vswprintf.o: local.h
$(lpfx)vswscanf.o: local.h
$(lpfx)vwprintf.o: local.h
$(lpfx)vwscanf.o: local.h
$(lpfx)wbuf.o: local.h fvwrite.h
$(lpfx)wprintf.o: local.h
$(lpfx)wscanf.o: local.h
$(lpfx)wsetup.o: local.h
