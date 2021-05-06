#!/bin/sh
set -e
cd $(dirname $0)
/usr/bin/aclocal --force
/usr/bin/autoconf -f
/usr/bin/automake -ac
/bin/rm -rf autom4te.cache
