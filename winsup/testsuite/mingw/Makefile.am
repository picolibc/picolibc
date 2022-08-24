# Makefile.am for Cygwin testsuite MINGW executables
#
# This file is part of Cygwin.
#
# This software is a copyrighted work licensed under the terms of the
# Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
# details.

# This makefile requires GNU make.

# These are built with the MinGW compiler, so are in a separate Makefile here
# because it's tricky with Automake to use different compilers for the same
# language in the same Makefile.

override CC = @MINGW_CC@
override CXX = @MINGW_CXX@
AM_CPPFLAGS =

noinst_PROGRAMS = cygrun cygload

cygrun_SOURCES = \
	../cygrun.c

cygload_SOURCES = \
	../winsup.api/cygload.cc
cygload_LDFLAGS=-static -Wl,-e,cygloadCRTStartup
