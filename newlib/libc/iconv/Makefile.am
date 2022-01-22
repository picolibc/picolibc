## Process this file with automake to generate Makefile.in
SUBDIRS = ces ccs lib .

cctdir=$(datadir)/iconv_data
cct_DATA=encoding.aliases

AM_CPPFLAGS = $(NEWLIB_CFLAGS) $(CROSS_CFLAGS) $(TARGET_CFLAGS)

include $(srcdir)/../../Makefile.shared

LIBC_CHEWOUT_FILES =
LIBC_CHAPTERS =
include ./Makefile.inc
CHEWOUT_FILES = $(LIBC_CHEWOUT_FILES)
CHAPTERS = $(LIBC_CHAPTERS)
