2009-09-10  J"orn Rennecke  <joern.rennecke@arc.com>

	* libc/machine/configure: Regenerate.
	* libc/sys/arc/Makefile.in: Likewise.

2008-08-29  J"orn Rennecke  <joern.rennecke@arc.com>

	* libc/machine/arc/Makefile.am (lib_a_SOURCES): Add strncpy.S .
	* libc/machine/arc/Makefile.in: Regenerate.
	* libc/machine/arc/strcpy-700.S: Use long branch to avoid stall.
	* libc/machine/arc/memset.S: Provide __strncpy_bzero entry point.
	* libc/machine/arc/strncpy.S: New file.
	* libc/machine/arc/asm.h: Add bclr_l and stb_l .

2008-08-29  J"orn Rennecke  <joern.rennecke@arc.com>

	* libc/sys/arc/crt0.S (__start): Use _init / _fini instead of
	__init / __fini.

2008-08-12  J"orn Rennecke  <joern.rennecke@arc.com>

	* libc/machine/arc/memcmp.S (bytewise): Fix case where there
	the first difference is in the penultimate character, and that
	is at an even index.

2008-06-12  J"orn Rennecke  <joern.rennecke@arc.com>

	* libc/sys/arc/syscalls.c (gettimeofday): Make third parameter void *.

2008-05-07  J"orn Rennecke  <joern.rennecke@arc.com>

	* configure.host: Add mxp support.

2008-04-27  J"orn Rennecke  <joern.rennecke@arc.com>

	* libc/include/machine/ieeefp.h (__IEEE_LITTLE_ENDIAN): Add mxp
	definition.

2008-03-11  J"orn Rennecke  <joern.rennecke@arc.com>

	* libc/machine/arc/memcmp.S (Lbytewise): Fix typo.

2008-02-29  J"orn Rennecke  <joern.rennecke@arc.com>

	* libc/sys/arc/crt0.S: Fix comment on argc / argv.

2008-02-14  J"orn Rennecke  <joern.rennecke@arc.com>

	* libc/machine/arc/asm.h: New file.
	* libc/machine/arc/memcmp.S: Include asm.h.
	Remove asm mnemonic definitions.  Use ENTRY and ENDFUNC.
	* libc/machine/arc/strcpy-600.S: Likewise.
	* libc/machine/arc/strcpy-700.S: Likewise.
	* libc/machine/arc/memcpy.S: Likewise.
	* libc/machine/arc/memcpy-700.S: Likewise.
	* libc/machine/arc/memset.S: Likewise.
	* libc/machine/arc/strchr.S: Likewise.
	* libc/machine/arc/strchr-700.S: Likewise.
	* libc/machine/arc/strlen.S: Likewise.
	* libc/machine/arc/strlen-600.S: Likewise.
	* libc/machine/arc/strcmp.S: Likewise.

	* libc/machine/arc/strcpy-600.S: New file.
	* libc/machine/arc/strlen-a5.c: Rename to:
	* libc/machine/arc/strlen-a4.c: This.  Only use for __A4__.
	* libc/machine/arc/Makefile.am (lib_a_SOURCES): Add strcpy-600.S and
	strlen-a4.c
	* libc/machine/arc/Makefile.in: Regenerate.

2008-02-12  J"orn Rennecke  <joern.rennecke@arc.com>

	* libc/machine/arc/strlen-600.S: New file.
	* libc/machine/arc/Makefile.am: Use it instead of strlen-a5.c .
	* libc/machine/arc/Makefile.in: Regenerate.

2007-11-23  J"orn Rennecke  <joern.rennecke@arc.com>

	* libc/sys/arc/link_r.c: New file.
	* libc/sys/arc/Makefile.am (lib_a_SOURCES): Add unlink_r.c .
	* libc/sys/arc/Makefile.in: Regenerate.

2007-10-23  J"orn Rennecke  <joern.rennecke@arc.com>

	* libc/machine/arc/memcpy-700.S: (memcpy): Remove stall at first
	branch.  Supply size - 1 and zero indicator to .Lcopy_bytewise.
	(.Lcopy_bytewise): Use conditional returnfor 0 size.  Remove now
	redundant calculation of size - 1.
	(.Lcopy_nil): Remove.

2007-10-09  J"orn Rennecke  <joern.rennecke@arc.com>

	* libc/machine/arc/memcmp.S (.Lodd <BIG ENDIAN>): Add missing and to
	get right sign; duplicate a bit of code to speed up this path.

2007-10-09  J"orn Rennecke  <joern.rennecke@arc.com>

	* libc/machine/arc/memcmp.S: New file.
	* libc/machine/arc/Makefile.am (lib_a_SOURCES): Add new file.
	* libc/machine/arc/Makefile.in: Regenerate.

2007-10-08  J"orn Rennecke  <joern.rennecke@arc.com>

	* libc/machine/arc/strlen.S, libc/machine/arc/strlen-a5.c: New files.
	* libc/machine/arc/Makefile.am (lib_a_SOURCES): Add new files.
	* libc/machine/arc/Makefile.in: Regenerate.

2007-10-04  J"orn Rennecke  <joern.rennecke@arc.com>

	* libc/machine/arc/memcpy-700.S: Further optimizations of aligned
	copies.  Enable for ARCtangetnt-A5 / ARC600.
	* libc/machine/arc/memcpy.S: Only use for ARCtangent-a4.

2007-10-04  J"orn Rennecke  <joern.rennecke@arc.com>

	* libc/machine/arc/memset.S (.Ltiny <!ARC700>): Use short stores.
	(SMALL <!ARC700>): Define as 8.

2007-10-04  J"orn Rennecke  <joern.rennecke@arc.com>

	* libc/machine/arc/memset.S, libc/machine/arc/memset-a4.c: New files.
	* libc/machine/arc/Makefile.am (lib_a_SOURCES): Add new files.
	* libc/machine/arc/Makefile.in: Regenerate.

2007-10-01  J"orn Rennecke  <joern.rennecke@arc.com>

	* libc/machine/arc/strcmp.S, libc/machine/arc/strcmp-a4.c: New files.
	* libc/machine/arc/strchr.S, libc/machine/arc/strchr-700.S: Likewise.
	* libc/machine/arc/strchr-a4.c: Likewise.
	* libc/machine/arc/Makefile.am (lib_a_SOURCES): Add new files.
	* libc/machine/arc/Makefile.in: Regenerate.

2007-09-17  J"orn Rennecke  <joern.rennecke@arc.com>

	* libc/sys/arc/sys/fcntl.h: New file.

2007-05-14  J"orn Rennecke  <joern.rennecke@arc.com>

	* libc/stdlib/strtod.c (strtof): Check for ERANGE.

2007-05-02  J"orn Rennecke  <joern.rennecke@arc.com>

	* libc/include/machine/types.h (TIME_T): long long for arc.
	* libc/include/sys/types.h (ino_t): unsigned long for arc too.
	(dev_t): long for arc.
	(mode_t): unsigned sort for arc.
	* libc/include/sys/stat.h (struct stat):
	Arc has dst_blksize / st_blocks before a/m/c times; the latter
	have following padding.
	* libc/sys/arc/syscalls.c (fstat): Define using _syscall2.
	(gettimeofday): Likewise.

2007-04-25  J"orn Rennecke  <joern.rennecke@arc.com>

	* libc/sys/arc/crt0.S: initialize _GLOBAL_REENT->atexit .

2007-04-25  J"orn Rennecke  <joern.rennecke@arc.com>

	* libc/sys/arc/unlink_r.c: New file.
	* libc/sys/arc/Makefile.am (lib_a_SOURCES): Add unlink_r.c .
	* libc/sys/arc/Makefile.in: Regenerate.

2007-04-25  J"orn Rennecke  <joern.rennecke@arc.com>

	* libc/machine/arc/strcpy-arccompact.S: Rename to ..
	* libc/machine/arc/strcpy-700.S .  Fix assembly errors.
	* libc/machine/arc/strcpy: New file.
	* libc/machine/arc/Makefile.am (lib_a_SOURCES): Add strcpy.c and
	strcpy-700.S .
	* libc/machine/arc/Makefile.in: Regenerate.

2007-04-24  J"orn Rennecke  <joern.rennecke@arc.com>

	* libc/machine/arc/Makefile.am (lib_a_SOURCES): Add memcpy.S and
	memcpy-700.S .
	* libc/machine/arc/Makefile.in: Regenerate.
	* libc/machine/arc/memcpy.S: Guard with #ifndef __ARC700__ .
	* libc/machine/arc/memcpy-700.S: Guard with #ifdef __ARC700__ .
	Debug little-endian code.
	* libc/machine/arc/strcpy-arccompact.S: New file (WIP).

2007-04-18  J"orn Rennecke  <joern.rennecke@arc.com>

	* libc/sys/Makefile.am (.S.o): New rule.
	(SUFFIXES): Remove explicit .S inclusion.
	(lib_a_CCASFLAGS, lib_a_CFLAGS, CCASCOMPILE): Define.

2007-04-18  J"orn Rennecke  <joern.rennecke@arc.com>

	* libc/machine/configure.in (machine_dir): Handle arc.
	* libc/machine/configure: Regenerate.

2007-04-18  J"orn Rennecke  <joern.rennecke@arc.com>

	* libc/sys/arc/Makefile.am: Add .S SUFFIXES rule.
	* libc/sys/arc/Makefile.in: Regenerate.
	* libc/machine/arc/Makefile.am: Update for new autoconf / automake.
	* libc/machine/arc/configure.in: Likewise.
	* libc/machine/arc/aclocal.m4: Likewise.
	* libc/machine/arc/configure: Regenerate.
	* libc/machine/arc/Makefile.in: Likewise.

2007-04-17  J"orn Rennecke  <joern.rennecke@arc.com>

	* configure.host (arc-*-*): Add -DREENTRANT_SYSCALLS_PROVIDED to
	newlib_cflags.
	* libc/stdlib/_Exit.c: For REENTRANT_SYSCALLS_PROVIDED, use exit_r.
	* libc/stdlib/exit.c, libc/stdlib/abort.c: Likewise.
	* libc/machine/arc/memcpy-700.S: New file.
	* libc/sys/arc/sbrk.c: Use implementation from stubs.c, except that
	a struct reent parameter is used.
	* libc/sys/arc/Makefile.am (lib_a_SOURCES): Replace stubs.c with
	isatty.c, sbrk.c, syscalls.c and _exit.c.
	* libc/sys/arc/Makefile.in: Regenerate.
	* src/newlib/libc/sys/arc/crt0.S (__start): Pass r1 to main.
	Don't use lp_count for temporary value.
	Use single-instruction loop to zero bss.
	Avoid calling atexit.
	* src/newlib/libc/sys/arc/_exit.c: New file.
	* src/newlib/libc/sys/arc/sys/syscall.h: Use linux syscall interface.
	* src/newlib/libc/sys/arc/syscalls.c: Likewise.
