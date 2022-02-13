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
PRE_DLL_NAME=cygwin0.dll
DBG_DLL_NAME=cygwin1.dbg
NEW_DLL_NAME=new-cygwin1.dll
DIN_FILE=@DIN_FILE@
DEF_FILE=cygwin.def
TLSOFFSETS_H=@TLSOFFSETS_H@
LIB_NAME=libcygwin.a
TEST_LIB_NAME=libcygwin0.a

#
# sources
#

# These objects are included directly into the import library
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

REGEX_FILES = \
	regex/regcomp.c \
	regex/regerror.c \
	regex/regexec.c \
	regex/regfree.c

TZCODE_FILES= \
	tzcode/localtime_wrapper.c

DLL_FILES= \
	advapi32.cc \
	aio.cc \
	assert.cc \
	autoload.cc \
	clock.cc \
	ctype.cc \
	cxx.cc \
	cygheap.cc \
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
	fhandler.cc \
	fhandler_clipboard.cc \
	fhandler_console.cc \
	fhandler_cygdrive.cc \
	fhandler_dev.cc \
	fhandler_dev_fd.cc \
	fhandler_disk_file.cc \
	fhandler_dsp.cc \
	fhandler_fifo.cc \
	fhandler_floppy.cc \
	fhandler_mqueue.cc \
	fhandler_netdrive.cc \
	fhandler_nodevice.cc \
	fhandler_pipe.cc \
	fhandler_proc.cc \
	fhandler_process.cc \
	fhandler_process_fd.cc \
	fhandler_procnet.cc \
	fhandler_procsys.cc \
	fhandler_procsysvipc.cc \
	fhandler_random.cc \
	fhandler_raw.cc \
	fhandler_registry.cc \
	fhandler_serial.cc \
	fhandler_signalfd.cc \
	fhandler_socket.cc \
	fhandler_socket_inet.cc \
	fhandler_socket_local.cc \
	fhandler_socket_unix.cc \
	fhandler_tape.cc \
	fhandler_termios.cc \
	fhandler_timerfd.cc \
	fhandler_tty.cc \
	fhandler_virtual.cc \
	fhandler_windows.cc \
	fhandler_zero.cc \
	flock.cc \
	fork.cc \
	forkable.cc \
	glob.cc \
	glob_pattern_p.cc \
	globals.cc \
	grp.cc \
	heap.cc \
	hookapi.cc \
	init.cc \
	ioctl.cc \
	ipc.cc \
	kernel32.cc \
	ldap.cc \
	libstdcxx_wrapper.cc \
	loadavg.cc \
	lsearch.cc \
	malloc_wrapper.cc \
	miscfuncs.cc \
	mktemp.cc \
	mmap.cc \
	mmap_alloc.cc \
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
	pthread.cc \
	quotactl.cc \
	random.cc \
	registry.cc \
	resource.cc \
	scandir.cc \
	sched.cc \
	sec_acl.cc \
	sec_auth.cc \
	sec_helper.cc \
	sec_posixacl.cc \
	security.cc \
	select.cc \
	sem.cc \
	setlsapwd.cc \
	shared.cc \
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
	thread.cc \
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

MALLOC_FILES= \
	malloc.cc

GMON_FILES= \
	gmon.c \
	mcount.c \
	profil.c \
	mcountFunc.S

GENERATED_FILES= \
	sigfe.s

liblib_a_SOURCES= \
	$(LIB_FILES)

libdll_a_SOURCES= \
	$(DLL_FILES) \
	$(REGEX_FILES) \
	$(MALLOC_FILES) \
	$(LIBC_FILES) \
	$(MATH_FILES) \
	$(TZCODE_FILES) \
	$(GENERATED_FILES)

#
# generated sources
#

shared_info_magic.h: cygmagic shared_info.h
	$(AM_V_GEN)$(srcdir)/cygmagic $@ "$(CC) $(INCLUDES) $(CPPFLAGS) -E -x c++" $(word 2,$^) SHARED_MAGIC 'class shared_info' USER_MAGIC 'class user_info'

child_info_magic.h: cygmagic child_info.h
	$(AM_V_GEN)$(srcdir)/cygmagic $@ "$(CC) $(INCLUDES) $(CPPFLAGS) -E -x c++" $(word 2,$^) CHILD_INFO_MAGIC 'class child_info'

globals.h: mkglobals_h globals.cc
	$(AM_V_GEN)$^ > $@

localtime.patched.c: tzcode/localtime.c tzcode/localtime.c.patch
	$(AM_V_GEN)patch -u -o localtime.patched.c \
		    $(srcdir)/tzcode/localtime.c \
		    $(srcdir)/tzcode/localtime.c.patch

$(srcdir)/devices.cc: gendevices devices.in devices.h
	$(wordlist 1,2,$^) $@

$(srcdir)/$(TLSOFFSETS_H): gentls_offsets cygtls.h
	$^ $@ $(target_cpu) $(CC) $(AM_CFLAGS) -c || rm $@

BUILT_SOURCES = \
	child_info_magic.h \
	shared_info_magic.h \
	globals.h \
	localtime.patched.c

# Every time we touch a source file, the version info has to be rebuilt
# to maintain a correct build date, especially in uname release output
dirs = $(srcdir) $(srcdir)/regex $(srcdir)/lib $(srcdir)/libc $(srcdir)/math $(srcdir)/tzcode
find_src_files = $(wildcard $(dir)/*.[chS]) $(wildcard $(dir)/*.cc)
src_files := $(foreach dir,$(dirs),$(find_src_files))

# mkvers.sh creates version.cc in the first place, winver.o always
# second, so version.cc is always older than winver.o
version.cc: mkvers.sh include/cygwin/version.h winver.rc $(src_files)
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

if !TARGET_X86_64
NEW_FUNCTIONS+=$(addprefix --replace=,\
	acl=_acl32 \
	aclcheck=_aclcheck32 \
	aclfrommode=_aclfrommode32 \
	aclfrompbits=_aclfrompbits32 \
	aclfromtext=_aclfromtext32 \
	aclsort=_aclsort32 \
	acltomode=_acltomode32 \
	acltopbits=_acltopbits32 \
	acltotext=_acltotext32 \
	chown=_chown32 \
	facl=_facl32 \
	fchown=_fchown32 \
	fcntl=_fcntl64 \
	fdopen=_fdopen64 \
	fgetpos=_fgetpos64 \
	fopen=_fopen64 \
	freopen=_freopen64 \
	fseeko=_fseeko64 \
	fsetpos=_fsetpos64 \
	fstat=_fstat64 \
	ftello=_ftello64 \
	ftruncate=_ftruncate64 \
	getegid=_getegid32 \
	geteuid=_geteuid32 \
	getgid=_getgid32 \
	getgrent=_getgrent32 \
	getgrgid=_getgrgid32 \
	getgrnam=_getgrnam32 \
	getgroups=_getgroups32 \
	getpwuid=_getpwuid32 \
	getpwuid_r=_getpwuid_r32 \
	getuid=_getuid32 \
	initgroups=_initgroups32 \
	lchown=_lchown32 \
	lseek=_lseek64 \
	lstat=_lstat64 \
	mknod=_mknod32 \
	mmap=_mmap64 \
	open=_open64 \
	setegid=_setegid32 \
	seteuid=_seteuid32 \
	setgid=_setgid32 \
	setgroups=_setgroups32 \
	setregid=_setregid32 \
	setreuid=_setreuid32 \
	setuid=_setuid32 \
	stat=_stat64 \
	tmpfile=_tmpfile64 \
	truncate=_truncate64 \
)
endif

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

fhandler_proc_CFLAGS=-DUSERNAME="\"$(USER)\"" -DHOSTNAME="\"$(HOSTNAME)\"" \
		     -DGCC_VERSION="\"`$(CC) -v 2>&1 | tail -n 1`\""

if !TARGET_X86_64
# on x86, exceptions.cc must be compiled with a frame-pointer as it uses RtlCaptureContext()
exceptions_CFLAGS=-fno-omit-frame-pointer
endif

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

EXTRALIBS = \
	libautomode.a \
	libbinmode.a \
	libtextmode.a \
	libtextreadmode.a

noinst_LIBRARIES = \
	libdll.a \
	liblib.a

toollib_LIBRARIES = \
	libgmon.a \
	$(EXTRALIBS)

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

libautomode_a_SOURCES =
libautomode_a_LIBADD = automode.o

libbinmode_a_SOURCES =
libbinmode_a_LIBADD = binmode.o

libtextmode_a_SOURCES =
libtextmode_a_LIBADD = textmode.o

libtextreadmode_a_SOURCES =
libtextreadmode_a_LIBADD = textreadmode.o

# cygserver library
cygserver_blddir = ${target_builddir}/winsup/cygserver
LIBSERVER = $(cygserver_blddir)/libcygserver.a

$(LIBSERVER):
	$(MAKE) -C $(cygserver_blddir) libcygserver.a

# We build as cygwin0.dll and rename at install time to overcome native
# rebuilding issues (we don't want the build tools to see a partially built
# cygwin.dll and attempt to use it instead of the old one).

# linker script
LDSCRIPT=cygwin.sc
$(LDSCRIPT): $(LDSCRIPT).in
	$(AM_V_GEN)$(CC) -E - -P < $^ -o $@

# cygwin dll
$(PRE_DLL_NAME): $(LDSCRIPT) libdll.a $(VERSION_OFILES) $(LIBSERVER)\
		  $(newlib_build)/libm.a $(newlib_build)/libc.a
	$(AM_V_CXXLD)$(CXX) $(CXXFLAGS) \
	-mno-use-libstdc-wrappers \
	-Wl,--gc-sections -nostdlib -Wl,-T$(LDSCRIPT) -static \
	-Wl,--heap=0 -Wl,--out-implib,cygdll.a -shared -o $@ \
	-e @DLL_ENTRY@ $(DEF_FILE) \
	-Wl,-whole-archive libdll.a -Wl,-no-whole-archive \
	$(VERSION_OFILES) \
	$(LIBSERVER) \
	$(newlib_build)/libm.a \
	$(newlib_build)/libc.a \
	-lgcc -lkernel32 -lntdll -Wl,-Map,cygwin.map

# create cygwin1.dbg file
$(DBG_DLL_NAME): $(PRE_DLL_NAME)
	$(AM_V_GEN)$(OBJCOPY) -R .gnu_debuglink_overlay \
			      --add-gnu-debuglink=/dev/null \
			      --only-keep-debug \
			      $(PRE_DLL_NAME) \
			      $(DBG_DLL_NAME)

# create stripped, temporary DLL, append .gnu_debuglink section, move
# .gnu_debuglink section in place of .gnu_debuglink_overlay placeholder
# section and store result in new-cygwin1.dll
$(NEW_DLL_NAME): $(PRE_DLL_NAME) $(DBG_DLL_NAME)
	$(AM_V_GEN)TMP_DLL_NAME=$$( mktemp --suffix=.dll ) && \
	$(OBJCOPY) -g \
		   --keep-section=.gnu_debuglink_overlay \
		   --add-gnu-debuglink=$(DBG_DLL_NAME) \
		   $(PRE_DLL_NAME) $${TMP_DLL_NAME} && \
	vma=$$( objdump --headers $${TMP_DLL_NAME} | \
	       awk '/.gnu_debuglink_overlay/{print $$4;}' ) && \
	$(OBJCOPY) -R .gnu_debuglink_overlay \
		   --change-section-address .gnu_debuglink=0x$${vma} \
		   --set-section-flag .gnu_debuglink=contents,readonly,debug,noload \
		   $${TMP_DLL_NAME} $(NEW_DLL_NAME) && \
	rm $${TMP_DLL_NAME}

# cygwin import library
toolopts=--cpu=@target_cpu@ --ar=@AR@ --as=@AS@ --nm=@NM@ --objcopy=@OBJCOPY@

$(DEF_FILE): gendef $(srcdir)/$(TLSOFFSETS_H) $(DIN_FILE) common.din
	$(AM_V_GEN)$(srcdir)/gendef --cpu=@target_cpu@ --output-def=$(DEF_FILE) --tlsoffsets=$(srcdir)/$(TLSOFFSETS_H) $(srcdir)/$(DIN_FILE) $(srcdir)/common.din

sigfe.s: $(DEF_FILE)
	@[ -s $@ ] || \
	{ rm -f $(DEF_FILE); $(MAKE) -s -j1 $(DEF_FILE); }; \
	[ -s $@ ] && touch $@

LIBCOS=$(addsuffix .o,$(basename $(LIB_FILES)))
$(LIB_NAME): $(DEF_FILE) $(LIBCOS) | $(NEW_DLL_NAME)
	$(AM_V_GEN)$(srcdir)/mkimport $(toolopts) $(NEW_FUNCTIONS) $@ cygdll.a $(wordlist 2,99,$^)

# cygwin import library used by testsuite
$(TEST_LIB_NAME): $(LIB_NAME)
	$(AM_V_GEN)perl -p -e 'BEGIN{binmode(STDIN); binmode(STDOUT);}; s/cygwin1/cygwin0/g' < $? > $@

# sublibs
# import libraries for some subset of symbols indicated by given objects
speclib=\
	$(srcdir)/speclib $(toolopts) \
	--exclude='cygwin' \
	--exclude='(?i:dll)' \
	--exclude='reloc' \
	--exclude='^main$$' \
	--exclude='^_main$$'

libc.a: $(LIB_NAME) libm.a libpthread.a libutil.a
	$(AM_V_GEN)$(speclib) $^ -v $(@F)

libm.a: $(LIB_NAME) $(newlib_build)/libm.a $(addsuffix .o,$(basename $(MATH_FILES)))
	$(AM_V_GEN)$(speclib) $^ $(@F)

libpthread.a: $(LIB_NAME) pthread.o thread.o libc/call_once.o libc/cnd.o \
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

libacl.a: $(LIB_NAME) sec_posixacl.o
	$(AM_V_GEN)$(speclib) $^ $(@F)

libssp.a: $(LIB_NAME) $(wildcard $(newlib_build)/libc/ssp/*.o)
	$(AM_V_GEN)$(speclib) $^ $(@F)

#
# all
#

all-local: $(LIB_NAME) $(TEST_LIB_NAME) $(SUBLIBS)

#
# clean
#

clean-local:
	-rm -f $(BUILT_SOURCES)
	-rm -f $(DEF_FILE) sigfe.s
	-rm -f cygwin.sc cygdll.a cygwin.map
	-rm -f $(PRE_DLL_NAME) $(DBG_DLL_NAME) $(NEW_DLL_NAME)
	-rm -f $(LIB_NAME) $(TEST_LIB_NAME) $(SUBLIBS)
	-rm -f version.cc

maintainer-clean-local:
	-rm -f $(srcdir)/$(TLSOFFSETS_H) $(srcdir)/devices.cc

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
	rm -f $(DESTDIR)$(bindir)/cygwin1.dll
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
