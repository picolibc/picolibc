#!/bin/sh

aclocal-1.11 -I ../../../ -I ../../../../
/usr/local/bin/autoconf
automake-1.11 --cygnus Makefile
