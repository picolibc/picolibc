# Makefile.am for Cygwin's testsuite.
#
# This file is part of Cygwin.
#
# This software is a copyrighted work licensed under the terms of the
# Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
# details.

# This makefile requires GNU make.

noinst_LIBRARIES = libltp.a

libltp_a_CPPFLAGS=-I$(srcdir)/libltp/include

libltp_a_SOURCES = \
	libltp/lib/dataascii.c \
	libltp/lib/databin.c \
	libltp/lib/datapid.c \
	libltp/lib/forker.c \
	libltp/lib/get_high_address.c \
	libltp/lib/libtestsuite.c \
	libltp/lib/open_flags.c \
	libltp/lib/parse_opts.c \
	libltp/lib/pattern.c \
	libltp/lib/rmobj.c \
	libltp/lib/search_path.c \
	libltp/lib/str_to_bytes.c \
	libltp/lib/string_to_tokens.c \
	libltp/lib/tst_res.c \
	libltp/lib/tst_sig.c \
	libltp/lib/tst_tmpdir.c \
	libltp/lib/write_log.c

DEJATOOL = winsup

# Add '-v' to RUNTESTFLAGS if V=1
RUNTESTFLAGS_1 = -v
RUNTESTFLAGS = $(RUNTESTFLAGS_$(V))

# a temporary directory, to be used for files created by tests
tmpdir = $(abspath $(objdir)/testsuite/tmp/)
# the same temporary directory, as an absolute, /cygdrive path (so it can be
# understood by the test DLL, which will have a different mount table)
testdll_tmpdir = $(shell cygpath -ma $(tmpdir) | sed -e 's#^\([A-Z]\):#/cygdrive/\L\1#')

site-extra.exp: ../config.status Makefile
	@rm -f ./tmp0
	@echo "set runtime_root \"`pwd`/../cygwin\"" >> ./tmp0
	@echo "set CC \"$(CC)\"" >> ./tmp0
	@echo "set CFLAGS \"\"" >> ./tmp0
	@echo "set MINGW_CXX \"$(MINGW_CXX)\"" >> ./tmp0
	@echo "set tmpdir $(tmpdir)" >> ./tmp0
	@echo "set testdll_tmpdir $(testdll_tmpdir)" >> ./tmp0
	@echo "set ltp_includes \"$(srcdir)/libltp/include\"" >> ./tmp0
	@echo "set ltp_libs \"`pwd`/libltp.a\"" >> ./tmp0
	@echo "set cygrun \"`pwd`/cygrun/cygrun\"" >> ./tmp0
	@mv ./tmp0 site-extra.exp

EXTRA_DEJAGNU_SITE_CONFIG = site-extra.exp

clean-local:
	rm -f *.log *.exe *.exp *.bak *.stackdump winsup.sum

SUBDIRS = cygrun
