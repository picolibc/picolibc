# Makefile.am for Cygwin the testsuite wrapper cygrun.
#
# This file is part of Cygwin.
#
# This software is a copyrighted work licensed under the terms of the
# Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
# details.

# This makefile requires GNU make.

# This is built with the MinGW compiler, so is in a separate Makefile here
# because it's tricky with Automake to use different compilers for the same
# language in the same Makefile.

override CC = @MINGW_CC@
AM_CPPFLAGS =

noinst_PROGRAMS = cygrun

cygrun_SOURCES = \
	../cygrun.c
