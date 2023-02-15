# Makefile.am for Cygwin.
#
# This file is part of Cygwin.
#
# This software is a copyrighted work licensed under the terms of the
# Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
# details.

# This makefile requires GNU make.

AUTOMAKE_OPTIONS=-Wno-override

include $(top_srcdir)/Makefile.am.common

COMMON_CFLAGS=-Wimplicit-fallthrough=5 -Werror -fmerge-constants -ftracer
if TARGET_X86_64
COMMON_CFLAGS+=-mcmodel=small
endif

AM_CFLAGS=$(cflags_common) $(COMMON_CFLAGS)
AM_CXXFLAGS=$(cxxflags_common) $(COMMON_CFLAGS) -fno-threadsafe-statics

if TARGET_X86_64
# Needed by mcountFunc.S to choose the right code path and symbol names
AM_CCASFLAGS=-D_WIN64
endif

target_builddir=@target_builddir@
newlib_build=$(target_builddir)/newlib

#
# Specialize libdir and includedir to use tooldir prefixed paths (containing
# target_alias) as used by newlib, because we overwrite library and header files
# installed there by newlib.
#

toollibdir=$(tooldir)/lib
toolincludedir=$(tooldir)/include

# Parameters used in building the cygwin.dll.

DLL_NAME=cygwin1.dll
NEW_DLL_NAME=new-cygwin1.dll
DEF_FILE=cygwin.def
LIB_NAME=libcygwin.a

#
# sources
#

# These objects are included directly into the import library
if TARGET_X86_64
TARGET_FILES= \
	x86_64/bcopy.S \
	x86_64/memchr.S \
	x86_64/memcpy.S \
	x86_64/memmove.S \
	x86_64/mempcpy.S \
	x86_64/memset.S \
	x86_64/swab.S \
	x86_64/wmemcpy.S \
	x86_64/wmemmove.S \
	x86_64/wmempcpy.S
endif

LIB_FILES= \
	lib/_cygwin_crt0_common.cc \
	lib/atexit.c \
	lib/cygwin_attach_dll.c \
	lib/cygwin_crt0.c \
	lib/dll_entry.c \
	lib/dll_main.cc \
	lib/dso_handle.c \
	lib/libcmain.c \
	lib/premain0.c \
	lib/premain1.c \
	lib/premain2.c \
	lib/premain3.c \
	lib/pseudo-reloc-dummy.c

FHANDLER_FILES= \
	fhandler/base.cc \
	fhandler/clipboard.cc \
	fhandler/console.cc \
	fhandler/cygdrive.cc \
	fhandler/dev.cc \
	fhandler/dev_fd.cc \
	fhandler/disk_file.cc \
	fhandler/dsp.cc \
	fhandler/fifo.cc \
	fhandler/floppy.cc \
	fhandler/mqueue.cc \
	fhandler/netdrive.cc \
	fhandler/nodevice.cc \
	fhandler/null.cc \
	fhandler/pipe.cc \
	fhandler/proc.cc \
	fhandler/process.cc \
	fhandler/process_fd.cc \
	fhandler/procnet.cc \
	fhandler/procsys.cc \
	fhandler/procsysvipc.cc \
	fhandler/random.cc \
	fhandler/raw.cc \
	fhandler/registry.cc \
	fhandler/serial.cc \
	fhandler/signalfd.cc \
	fhandler/socket.cc \
	fhandler/socket_inet.cc \
	fhandler/socket_local.cc \
	fhandler/socket_unix.cc \
	fhandler/tape.cc \
	fhandler/termios.cc \
	fhandler/timerfd.cc \
	fhandler/pty.cc \
	fhandler/virtual.cc \
	fhandler/windows.cc \
	fhandler/zero.cc

LIBC_FILES= \
	libc/arc4random_stir.c \
	libc/base64.c \
	libc/bsdlib.cc \
	libc/call_once.c \
	libc/cnd.c \
	libc/fnmatch.c \
	libc/fts.c \
	libc/ftw.c \
	libc/getentropy.cc \
	libc/getopt.c \
	libc/inet_addr.c \
	libc/inet_network.c \
	libc/minires-os-if.c \
	libc/minires.c \
	libc/msgcat.c \
	libc/mtx.c \
	libc/nftw.c \
	libc/rcmd.cc \
	libc/rexec.cc \
	libc/strfmon.c \
	libc/strptime.cc \
	libc/thrd.c \
	libc/tss.c \
	libc/xsique.cc

MATH_FILES= \
	math/acoshl.c \
	math/acosl.c \
	math/asinhl.c \
	math/asinl.c \
	math/atan2l.c \
	math/atanhl.c \
	math/atanl.c \
	math/cabsl.c \
	math/cacosl.c \
	math/cargl.c \
	math/casinl.c \
	math/catanl.c \
	math/cbrtl.c \
	math/ccosl.c \
	math/ceill.S \
	math/cephes_emath.c \
	math/cexpl.c \
	math/cimagl.c \
	math/clog10l.c \
	math/clogl.c \
	math/conjl.c \
	math/copysignl.S \
	math/coshl.c \
	math/cosl.c \
	math/cosl_internal.S \
	math/cossin.c \
	math/cpowl.c \
	math/cprojl.c \
	math/creall.c \
	math/csinl.c \
	math/csqrtl.c \
	math/ctanl.c \
	math/erfl.c \
	math/exp10l.c \
	math/exp2l.S \
	math/expl.c \
	math/expm1l.c \
	math/fabsl.c \
	math/fdiml.c \
	math/finite.c \
	math/floorl.S \
	math/fmal.c \
	math/fmaxl.c \
	math/fminl.c \
	math/fmodl.c \
	math/frexpl.S \
	math/hypotl.c \
	math/ilogbl.S \
	math/internal_logl.S \
	math/isinf.c \
	math/isnan.c \
	math/ldexpl.c \
	math/lgammal.c \
	math/llrint.c \
	math/llrintf.c \
	math/llrintl.c \
	math/llroundl.c \
	math/log10l.S \
	math/log1pl.S \
	math/log2l.S \
	math/logbl.c \
	math/logl.c \
	math/lrint.c \
	math/lrintf.c \
	math/lrintl.c \
	math/lroundl.c \
	math/modfl.c \
	math/nearbyint.S \
	math/nearbyintf.S \
	math/nearbyintl.S \
	math/nextafterl.c \
	math/nexttoward.c \
	math/nexttowardf.c \
	math/pow10l.c \
	math/powil.c \
	math/powl.c \
	math/remainder.S \
	math/remainderf.S \
	math/remainderl.S \
	math/remquol.S \
	math/rint.c \
	math/rintf.c \
	math/rintl.c \
	math/roundl.c \
	math/scalbl.S \
	math/scalbnl.S \
	math/sinhl.c \
	math/sinl.c \
	math/sinl_internal.S \
	math/sqrtl.c \
	math/tanhl.c \
	math/tanl.S \
	math/tgammal.c \
	math/truncl.c

MM_FILES = \
	mm/cygheap.cc \
	mm/heap.cc \
	mm/malloc.cc \
	mm/malloc_wrapper.cc \
	mm/mmap.cc \
	mm/mmap_alloc.cc \
	mm/shared.cc

REGEX_FILES = \
	regex/regcomp.c \
	regex/regerror.c \
	regex/regexec.c \
	regex/regfree.c

SEC_FILES= \
	sec/acl.cc \
	sec/auth.cc \
	sec/base.cc \
	sec/helper.cc \
	sec/posixacl.cc

TZCODE_FILES= \
	tzcode/localtime_wrapper.c

DLL_FILES= \
	advapi32.cc \
	aio.cc \
	assert.cc \
	autoload.cc \
	clock.cc \
	create_posix_thread.cc \
	ctype.cc \
	cxx.cc \
	cygthread.cc \
	cygtls.cc \
	cygwait.cc \
	cygxdr.cc \
	dcrt0.cc \
	debug.cc \
	devices.cc \
	dir.cc \
	dlfcn.cc \
	dll_init.cc \
	dtable.cc \
	environ.cc \
	errno.cc \
	exceptions.cc \
	exec.cc \
	external.cc \
	fcntl.cc \
	fenv.c \
	flock.cc \
	fork.cc \
	forkable.cc \
	glob.cc \
	glob_pattern_p.cc \
	globals.cc \
	grp.cc \
	hookapi.cc \
	init.cc \
	ioctl.cc \
	ipc.cc \
	kernel32.cc \
	ldap.cc \
	libstdcxx_wrapper.cc \
	loadavg.cc \
	lsearch.cc \
	miscfuncs.cc \
	mktemp.cc \
	msg.cc \
	mount.cc \
	net.cc \
	netdb.cc \
	nfs.cc \
	nlsfuncs.cc \
	ntea.cc \
	passwd.cc \
	path.cc \
	pinfo.cc \
	poll.cc \
	posix_ipc.cc \
	posix_timer.cc \
	pseudo-reloc.cc \
	thread.cc \
	quotactl.cc \
	random.cc \
	registry.cc \
	resource.cc \
	scandir.cc \
	sched.cc \
	select.cc \
	sem.cc \
	setlsapwd.cc \
	shm.cc \
	signal.cc \
	sigproc.cc \
	smallprint.cc \
	spawn.cc \
	strace.cc \
	strfuncs.cc \
	strsep.cc \
	strsig.cc \
	sync.cc \
	syscalls.cc \
	sysconf.cc \
	syslog.cc \
	termios.cc \
	timerfd.cc \
	times.cc \
	tls_pbuf.cc \
	tty.cc \
	uinfo.cc \
	uname.cc \
	wait.cc \
	wincap.cc \
	window.cc \
	winf.cc

if TARGET_X86_64
GMON_TARGET_FILES = \
	x86_64/mcountFunc.S
endif

GMON_FILES= \
	$(GMON_TARGET_FILES) \
	gmon.c \
	mcount.c \
	profil.c

GENERATED_FILES= \
	sigfe.s \
	uname_version.c

liblib_a_SOURCES= \
	$(LIB_FILES)

libdll_a_SOURCES= \
	$(TARGET_FILES) \
	$(DLL_FILES) \
	$(REGEX_FILES) \
	$(FHANDLER_FILES) \
	$(LIBC_FILES) \
	$(MATH_FILES) \
	$(MM_FILES) \
	$(SEC_FILES) \
	$(TZCODE_FILES) \
	$(GENERATED_FILES)

#
# generated sources
#

shared_info_magic.h: scripts/cygmagic local_includes/shared_info.h
	$(AM_V_GEN)$(srcdir)/scripts/cygmagic $@ "$(CC) $(INCLUDES) $(CPPFLAGS) -E -x c++" $(word 2,$^) SHARED_MAGIC 'class shared_info' USER_MAGIC 'class user_info'

child_info_magic.h: scripts/cygmagic local_includes/child_info.h
	$(AM_V_GEN)$(srcdir)/scripts/cygmagic $@ "$(CC) $(INCLUDES) $(CPPFLAGS) -E -x c++" $(word 2,$^) CHILD_INFO_MAGIC 'class child_info'

globals.h: scripts/mkglobals_h globals.cc
	$(AM_V_GEN)$^ > $@

localtime.patched.c: tzcode/localtime.c tzcode/localtime.c.patch
	$(AM_V_GEN)patch -u -o localtime.patched.c \
		    $(srcdir)/tzcode/localtime.c \
		    $(srcdir)/tzcode/localtime.c.patch

$(srcdir)/devices.cc: scripts/gendevices devices.in local_includes/devices.h
	$(wordlist 1,2,$^) $@

tlsoffsets: scripts/gentls_offsets local_includes/cygtls.h
	$(AM_V_GEN)CXXCOMPILE="$(CXXCOMPILE)" $^ $@

BUILT_SOURCES = \
	child_info_magic.h \
	shared_info_magic.h \
	globals.h \
	localtime.patched.c

# Every time we touch a source file, the version info has to be rebuilt
# to maintain a correct build date, especially in uname release output
dirs = $(srcdir) $(srcdir)/fhandler $(srcdir)/lib $(srcdir)/libc $(srcdir)/math $(srcdir)/mm $(srcdir)/regex $(srcdir)/sec $(srcdir)/tzcode
find_src_files = $(wildcard $(dir)/*.[chS]) $(wildcard $(dir)/*.cc)
src_files := $(foreach dir,$(dirs),$(find_src_files))

# Regenerate uname_version.c only if it doesn't exist or if it differs
# from the former uname_version.c
uname_version.c: .FORCE
	$(AM_V_GEN)cd $(srcdir) && \
	echo "const char *uname_dev_version = \
	\"$$(git rev-parse --git-dir >/dev/null 2>&1 && \
	     git describe --abbrev=12 --long --match 'cygwin*' --dirty | \
	     sed -e 's/cygwin-//')\";" \
	     > $(abs_builddir)/uname_version-pre.c && \
	if [ -f $(abs_builddir)/uname_version.c ] && \
	   cmp $(abs_builddir)/uname_version.c \
	       $(abs_builddir)/uname_version-pre.c >/dev/null 2>&1 ; \
	then :; else \
	       mv $(abs_builddir)/uname_version-pre.c \
		  $(abs_builddir)/uname_version.c; fi
.FORCE:

# mkvers.sh creates version.cc in the first place, winver.o always
# second, so version.cc is always older than winver.o
version.cc: scripts/mkvers.sh include/cygwin/version.h winver.rc $(src_files)
	@echo "Making version.cc and winver.o";\
	export CC="$(CC)";\
	/bin/sh $(word 1,$^) $(word 2,$^) $(word 3,$^) $(WINDRES) $(CFLAGS)

winver.o: version.cc

VERSION_OFILES = version.o winver.o

#
# export renames for mkimport
#

NEW_FUNCTIONS=$(addprefix --replace=,\
	atexit= \
	timezone= \
	uname=uname_x \
	__xdrrec_getrec= \
	__xdrrec_setnonblock= \
	xdr_array= \
	xdr_bool= \
	xdr_bytes= \
	xdr_char= \
	xdr_double= \
	xdr_enum= \
	xdr_float= \
	xdr_free= \
	xdr_hyper= \
	xdr_int= \
	xdr_int16_t= \
	xdr_int32_t= \
	xdr_int64_t= \
	xdr_int8_t= \
	xdr_long= \
	xdr_longlong_t= \
	xdr_netobj= \
	xdr_opaque= \
	xdr_pointer= \
	xdr_reference= \
	xdr_short= \
	xdr_sizeof= \
	xdr_string= \
	xdr_u_char= \
	xdr_u_hyper= \
	xdr_u_int= \
	xdr_u_int16_t= \
	xdr_u_int32_t= \
	xdr_u_int64_t= \
	xdr_u_int8_t= \
	xdr_u_long= \
	xdr_u_longlong_t= \
	xdr_u_short= \
	xdr_uint16_t= \
	xdr_uint32_t= \
	xdr_uint64_t= \
	xdr_uint8_t= \
	xdr_union= \
	xdr_vector= \
	xdr_void= \
	xdr_wrapstring= \
	xdrmem_create= \
	xdrrec_create= \
	xdrrec_endofrecord= \
	xdrrec_eof= \
	xdrrec_skiprecord= \
	xdrstdio_create= \
)

#
# per-file compilation flags
#
# (these come after, and thus override, the user-supplied CXXFLAGS, as they need
# to be able to fiddle with the optimization level)
#

override CFLAGS += $($(*F)_CFLAGS)
override CXXFLAGS += $($(*F)_CFLAGS)
AM_CXXFLAGS += $(CFLAGS)

localtime_wrapper_CFLAGS=-I$(srcdir)/tzcode -fwrapv

# required since gcc 9.x
exec_CFLAGS=-fno-builtin-execve

proc_CFLAGS=-DUSERNAME="\"$(USER)\"" -DHOSTNAME="\"$(HOSTNAME)\"" \
		     -DGCC_VERSION="\"`$(CC) -v 2>&1 | tail -n 1`\""

dtable_CFLAGS=-fcheck-new

#
# libraries and installed objects
#
# (Don't ever try to use automake's shared library support via libtool to build
# Cygwin. Instead we have explicit rules to build it.)
#

SUBLIBS = \
	libpthread.a \
	libutil.a \
	libm.a \
	libc.a \
	libdl.a \
	libresolv.a \
	librt.a \
	libacl.a \
	libssp.a

noinst_LIBRARIES = \
	libdll.a \
	liblib.a

toollib_LIBRARIES = \
	libgmon.a

CYGWIN_START=crt0.o
GMON_START=gcrt0.o
INSTOBJS=automode.o binmode.o textmode.o textreadmode.o

toollib_DATA = \
	$(CYGWIN_START) \
	$(GMON_START) \
	$(INSTOBJS) \
	$(LIB_NAME) \
	$(SUBLIBS)

libgmon_a_SOURCES = $(GMON_FILES)
libgmon_a_LIBADD =

# cygserver library
cygserver_blddir = ${target_builddir}/winsup/cygserver
LIBSERVER = $(cygserver_blddir)/libcygserver.a

$(LIBSERVER):
	$(MAKE) -C $(cygserver_blddir) libcygserver.a

# We build as new-cygwin1.dll and rename at install time to overcome native
# rebuilding issues (we don't want the build tools to see a partially built
# cygwin1.dll and attempt to use it instead of the old one).

# linker script
LDSCRIPT=cygwin.sc
$(LDSCRIPT): $(LDSCRIPT).in
	$(AM_V_GEN)$(CC) -E - -P < $^ -o $@

# cygwin dll
$(NEW_DLL_NAME): $(LDSCRIPT) libdll.a $(VERSION_OFILES) $(LIBSERVER)\
		  $(newlib_build)/libm.a $(newlib_build)/libc.a
	$(AM_V_CXXLD)$(CXX) $(CXXFLAGS) \
	-mno-use-libstdc-wrappers \
	-Wl,--gc-sections -nostdlib -Wl,-T$(LDSCRIPT) \
	-Wl,--dynamicbase -static \
	-Wl,--heap=0 -Wl,--out-implib,cygdll.a -shared -o $@ \
	-e @DLL_ENTRY@ $(DEF_FILE) \
	-Wl,-whole-archive libdll.a -Wl,-no-whole-archive \
	$(VERSION_OFILES) \
	$(LIBSERVER) \
	$(newlib_build)/libm.a \
	$(newlib_build)/libc.a \
	-lgcc -lkernel32 -lntdll -Wl,-Map,cygwin.map
	@$(MKDIR_P) ${target_builddir}/winsup/testsuite/runtime/
	$(AM_V_at)$(INSTALL_PROGRAM) $(NEW_DLL_NAME) ${target_builddir}/winsup/testsuite/runtime/$(DLL_NAME)

# cygwin import library
toolopts=--cpu=@target_cpu@ --ar=@AR@ --as=@AS@ --nm=@NM@ --objcopy=@OBJCOPY@

$(DEF_FILE): scripts/gendef cygwin.din
	$(AM_V_GEN)$(srcdir)/scripts/gendef --cpu=@target_cpu@ --output-def=$(DEF_FILE) $(srcdir)/cygwin.din

sigfe.s: $(DEF_FILE) tlsoffsets
	@[ -s $@ ] || \
	{ rm -f $(DEF_FILE); $(MAKE) -s -j1 $(DEF_FILE); }; \
	[ -s $@ ] && touch $@

LIBCOS=$(addsuffix .o,$(basename $(LIB_FILES)))
$(LIB_NAME): $(DEF_FILE) $(LIBCOS) | $(NEW_DLL_NAME)
	$(AM_V_GEN)$(srcdir)/scripts/mkimport $(toolopts) $(NEW_FUNCTIONS) $@ cygdll.a $(wordlist 2,99,$^)

# sublibs
# import libraries for some subset of symbols indicated by given objects
speclib=\
	$(srcdir)/scripts/speclib $(toolopts) \
	--exclude='cygwin' \
	--exclude='(?i:dll)' \
	--exclude='reloc' \
	--exclude='^main$$' \
	--exclude='^_main$$'

libc.a: $(LIB_NAME) libm.a libpthread.a libutil.a
	$(AM_V_GEN)$(speclib) $^ -v $(@F)

libm.a: $(LIB_NAME) $(newlib_build)/libm.a $(addsuffix .o,$(basename $(MATH_FILES)))
	$(AM_V_GEN)$(speclib) $^ $(@F)

libpthread.a: $(LIB_NAME) thread.o libc/call_once.o libc/cnd.o \
	      libc/mtx.o libc/thrd.o libc/tss.o
	$(AM_V_GEN)$(speclib) $^ $(@F)

libutil.a: $(LIB_NAME) libc/bsdlib.o
	$(AM_V_GEN)$(speclib) $^ $(@F)

libdl.a: $(LIB_NAME) dlfcn.o
	$(AM_V_GEN)$(speclib) $^ $(@F)

libresolv.a: $(LIB_NAME) libc/minires.o
	$(AM_V_GEN)$(speclib) $^ $(@F)

librt.a: $(LIB_NAME) posix_ipc.o
	$(AM_V_GEN)$(speclib) $^ $(@F)

libacl.a: $(LIB_NAME) sec/posixacl.o
	$(AM_V_GEN)$(speclib) $^ $(@F)

libssp.a: $(LIB_NAME) $(wildcard $(newlib_build)/libc/ssp/*.o)
	$(AM_V_GEN)$(speclib) $^ $(@F)

#
# all
#

all-local: $(LIB_NAME) $(SUBLIBS)

#
# clean
#

clean-local:
	-rm -f $(BUILT_SOURCES)
	-rm -f $(DEF_FILE) sigfe.s
	-rm -f cygwin.sc cygdll.a cygwin.map
	-rm -f $(NEW_DLL_NAME)
	-rm -f $(LIB_NAME) $(SUBLIBS)
	-rm -f version.cc
	-rm -f tlsoffsets

maintainer-clean-local:
	-rm -f $(srcdir)/devices.cc

#
# install
#

man_MANS = regex/regex.3 regex/regex.7

install-exec-hook: install-libs
install-data-local: install-headers install-ldif

install-libs:
	@$(MKDIR_P) $(DESTDIR)$(bindir)
	$(INSTALL_PROGRAM) $(NEW_DLL_NAME) $(DESTDIR)$(bindir)/$(DLL_NAME)
	@$(MKDIR_P) $(DESTDIR)$(toollibdir)
	(cd $(DESTDIR)$(toollibdir) && ln -sf $(LIB_NAME) libg.a)

install-headers:
	cd $(srcdir)/include; \
	for sub in `find . -type d -print | sort`; do \
	    $(MKDIR_P) $(DESTDIR)$(toolincludedir)/$$sub; \
	    for i in $$sub/*.h ; do \
	      $(INSTALL_DATA) $$i $(DESTDIR)$(toolincludedir)/$$sub/`basename $$i` ; \
	    done ; \
	done ;

install-ldif:
	@$(MKDIR_P) $(DESTDIR)$(datarootdir)/cygwin
	$(INSTALL_DATA) $(srcdir)/cygwin.ldif $(DESTDIR)$(datarootdir)/cygwin

#
# uninstall
#

uninstall-hook: uninstall-headers uninstall-ldif uninstall-libs

uninstall-libs:
	rm -f $(DESTDIR)$(bindir)/$(DLL_NAME)
	rm -f $(DESTDIR)$(toollibdir)/libg.a

uninstall-headers:
	cd $(srcdir)/include; \
	for sub in `find . -type d -print | sort`; do \
	    for i in $$sub/*.h ; do \
	      rm -f $(DESTDIR)$(toolincludedir)/$$sub/`basename $$i` ; \
	    done ; \
	done ;

uninstall-ldif:
	rm -f $(DESTDIR)$(datarootdir)/cygwin/cygwin.ldif

ctags: CTAGS
tags:  CTAGS
CTAGS:
	-cd $(srcdir) && \
	ctags -R --c++-kinds=+p --fields=+iaS --extras=+q \
	--regex-C++='/EXPORT_ALIAS *\([a-zA-Z0-9_]*, *([a-zA-Z0-9_]*)\)/\1/' \
	--regex-C++='/__ASMNAME *\("([a-zA-Z0-9_]+)"\)/\1/' \
	.
