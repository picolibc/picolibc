## Process this file with automake to generate Makefile.in
SUBDIRS = ces ccs lib .

cctdir=$(datadir)/iconv_data
cct_DATA=encoding.aliases

AM_CPPFLAGS = $(NEWLIB_CFLAGS) $(CROSS_CFLAGS) $(TARGET_CFLAGS)

include $(srcdir)/../../Makefile.shared
