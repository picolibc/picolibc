# Makefile.am for winsup stuff
#
# This file is part of Cygwin.
#
# This software is a copyrighted work licensed under the terms of the
# Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
# details.

# This makefile requires GNU make.

cygdocdir = $(datarootdir)/doc/Cygwin

cygdoc_DATA = \
	CYGWIN_LICENSE \
	COPYING

SUBDIRS = cygwin cygserver doc utils testsuite

cygserver utils testsuite: cygwin
