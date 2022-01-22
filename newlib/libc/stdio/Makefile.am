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
	%D%/$(lpfx)nano-vfprintf_float.$(oext)	\
	%D%/$(lpfx)nano-svfprintf.$(oext)		\
	%D%/$(lpfx)nano-svfscanf.$(oext)		\
	%D%/$(lpfx)nano-vfprintf.$(oext)		\
	%D%/$(lpfx)nano-vfprintf_i.$(oext)		\
	%D%/$(lpfx)nano-vfscanf.$(oext)		\
	%D%/$(lpfx)nano-vfscanf_i.$(oext)		\
	%D%/$(lpfx)nano-vfscanf_float.$(oext)	\
	%D%/$(lpfx)svfiwprintf.$(oext)		\
	%D%/$(lpfx)svfwprintf.$(oext)		\
	%D%/$(lpfx)vfiwprintf.$(oext)		\
	%D%/$(lpfx)svfiwscanf.$(oext)		\
	%D%/$(lpfx)svfwscanf.$(oext)		\
	%D%/$(lpfx)vfiwscanf.$(oext)		\
	%D%/$(lpfx)vfwscanf.$(oext)
else
LIBADD_OBJS = \
	%D%/$(lpfx)svfiprintf.$(oext) $(lpfx)svfprintf.$(oext) \
	%D%/$(lpfx)svfiscanf.$(oext) $(lpfx)svfscanf.$(oext) \
	%D%/$(lpfx)vfiprintf.$(oext) \
	%D%/$(lpfx)vfscanf.$(oext) $(lpfx)vfiscanf.$(oext) \
	%D%/$(lpfx)svfiwprintf.$(oext) $(lpfx)svfwprintf.$(oext) \
	%D%/$(lpfx)vfiwprintf.$(oext) \
	%D%/$(lpfx)svfiwscanf.$(oext) $(lpfx)svfwscanf.$(oext) \
	%D%/$(lpfx)vfiwscanf.$(oext) $(lpfx)vfwscanf.$(oext)
endif

libstdio_la_LDFLAGS = -Xcompiler -nostdlib

if USE_LIBTOOL
noinst_LTLIBRARIES = libstdio.la
libstdio_la_SOURCES = $(GENERAL_SOURCES) $(ELIX_2_SOURCES) $(ELIX_4_SOURCES)
libstdio_la_LIBADD = $(LIBADD_OBJS)
libstdio_la_DEPENDENCIES = $(LIBADD_OBJS)
LIB_COMPILE = $(AM_V_CC)$(LTCOMPILE)
noinst_DATA = objectlist.awk.in
else
noinst_LIBRARIES = lib.a
lib_a_SOURCES = $(GENERAL_SOURCES) $(ELIX_2_SOURCES) $(ELIX_4_SOURCES)
lib_a_LIBADD = $(LIBADD_OBJS)
lib_a_CFLAGS = $(AM_CFLAGS)
lib_a_DEPENDENCIES = $(LIBADD_OBJS)
LIB_COMPILE = $(AM_V_CC)$(COMPILE)
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
	%D%/$(LIB_COMPILE) -c $(srcdir)/nano-vfprintf.c -o $@

$(lpfx)nano-vfprintf_i.$(oext): nano-vfprintf_i.c
	%D%/$(LIB_COMPILE) -c $(srcdir)/nano-vfprintf_i.c -o $@

$(lpfx)nano-vfprintf_float.$(oext): nano-vfprintf_float.c
	%D%/$(LIB_COMPILE) -c $(srcdir)/nano-vfprintf_float.c -o $@

$(lpfx)nano-svfprintf.$(oext): nano-vfprintf.c
	%D%/$(LIB_COMPILE) -DSTRING_ONLY -c $(srcdir)/nano-vfprintf.c -o $@
endif

# This rule is needed so that libtool compiles vfiprintf before vfprintf.
# Otherwise libtool moves vfprintf.o and subsequently can't find it.

$(lpfx)vfiprintf.$(oext): vfprintf.c
	%D%/$(LIB_COMPILE) -DINTEGER_ONLY -c $(srcdir)/vfprintf.c -o $@

$(lpfx)svfprintf.$(oext): vfprintf.c
	%D%/$(LIB_COMPILE) -DSTRING_ONLY -c $(srcdir)/vfprintf.c -o $@

$(lpfx)svfiprintf.$(oext): vfprintf.c
	%D%/$(LIB_COMPILE) -DINTEGER_ONLY -DSTRING_ONLY -c $(srcdir)/vfprintf.c -o $@

$(lpfx)vfiwprintf.$(oext): vfwprintf.c
	%D%/$(LIB_COMPILE) -DINTEGER_ONLY -c $(srcdir)/vfwprintf.c -o $@

$(lpfx)svfwprintf.$(oext): vfwprintf.c
	%D%/$(LIB_COMPILE) -DSTRING_ONLY -c $(srcdir)/vfwprintf.c -o $@

$(lpfx)svfiwprintf.$(oext): vfwprintf.c
	%D%/$(LIB_COMPILE) -DINTEGER_ONLY -DSTRING_ONLY -c $(srcdir)/vfwprintf.c -o $@

if NEWLIB_NANO_FORMATTED_IO
# Rules compiling small-footprint nano-formatted-io implementation.
$(lpfx)nano-vfscanf.$(oext): nano-vfscanf.c
	%D%/$(LIB_COMPILE) -c $(srcdir)/nano-vfscanf.c -o $@

$(lpfx)nano-vfscanf_i.$(oext): nano-vfscanf_i.c
	%D%/$(LIB_COMPILE) -c $(srcdir)/nano-vfscanf_i.c -o $@

$(lpfx)nano-vfscanf_float.$(oext): nano-vfscanf_float.c
	%D%/$(LIB_COMPILE) -c $(srcdir)/nano-vfscanf_float.c -o $@

$(lpfx)nano-svfscanf.$(oext): nano-vfscanf.c
	%D%/$(LIB_COMPILE) -DSTRING_ONLY -c $(srcdir)/nano-vfscanf.c -o $@
endif

$(lpfx)vfscanf.$(oext): vfscanf.c
	%D%/$(LIB_COMPILE) -c $(srcdir)/vfscanf.c -o $@

$(lpfx)vfiscanf.$(oext): vfscanf.c
	%D%/$(LIB_COMPILE) -DINTEGER_ONLY -c $(srcdir)/vfscanf.c -o $@

$(lpfx)svfscanf.$(oext): vfscanf.c
	%D%/$(LIB_COMPILE) -DSTRING_ONLY -c $(srcdir)/vfscanf.c -o $@

$(lpfx)svfiscanf.$(oext): vfscanf.c
	%D%/$(LIB_COMPILE) -DINTEGER_ONLY -DSTRING_ONLY -c $(srcdir)/vfscanf.c -o $@

$(lpfx)vfwscanf.$(oext): vfwscanf.c
	%D%/$(LIB_COMPILE) -c $(srcdir)/vfwscanf.c -o $@

$(lpfx)vfiwscanf.$(oext): vfwscanf.c
	%D%/$(LIB_COMPILE) -DINTEGER_ONLY -c $(srcdir)/vfwscanf.c -o $@

$(lpfx)svfwscanf.$(oext): vfwscanf.c
	%D%/$(LIB_COMPILE) -DSTRING_ONLY -c $(srcdir)/vfwscanf.c -o $@

$(lpfx)svfiwscanf.$(oext): vfwscanf.c
	%D%/$(LIB_COMPILE) -DINTEGER_ONLY -DSTRING_ONLY -c $(srcdir)/vfwscanf.c -o $@

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
