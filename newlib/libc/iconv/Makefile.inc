if ENABLE_NEWLIB_ICONV
cctdir = $(datadir)/iconv_data
cct_DATA = %D%/encoding.aliases
endif

LIBC_CHAPTERS += %D%/iconv.tex

include %D%/ces/Makefile.inc
include %D%/ccs/Makefile.inc
include %D%/lib/Makefile.inc
