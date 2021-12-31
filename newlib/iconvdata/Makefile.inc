pkglib_LTLIBRARIES += %D%/EUC-JP.la

%C%_EUC_JP_la_SOURCES = %D%/dummy.c
if !ELIX_LEVEL_1
%C%_EUC_JP_la_SOURCES += \
	%D%/euc-jp.c \
	%D%/jis0201.c \
	%D%/jis0208.c \
	%D%/jis0212.c
endif
%C%_EUC_JP_la_DEPENDENCIES = $(%C%_EUC_JP_la_LIBADD)
%C%_EUC_JP_la_CPPFLAGS = \
	-DNDEBUG \
	-I$(srcdir)/iconvdata \
	-I$(srcdir)/libc/sys/linux \
	$(NEWLIB_CFLAGS) $(CROSS_CFLAGS) $(TARGET_CFLAGS)
%C%_EUC_JP_la_LDFLAGS = -module -no-undefined -Xcompiler -Wl,-rpath-link=. -Xcompiler -nostdlib -L. -lc

gconvdir = $(pkglibdir)
gconv_DATA = %D%/gconv-modules
