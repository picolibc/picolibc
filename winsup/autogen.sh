#!/bin/sh -e
if ! /usr/bin/test -e config.guess; then
    /usr/bin/wget -q -O config.guess 'http://git.savannah.gnu.org/gitweb/?p=config.git;a=blob_plain;f=config.guess;hb=HEAD'
    /bin/chmod a+x config.guess
fi
if ! /usr/bin/test -e config.sub; then
    /usr/bin/wget -q -O config.sub 'http://git.savannah.gnu.org/gitweb/?p=config.git;a=blob_plain;f=config.sub;hb=HEAD'
    /bin/chmod a+x config.sub
fi
/usr/bin/aclocal --force
/usr/bin/autoconf -f
/bin/rm -rf autom4te.cache
res=0
for d in cygwin utils cygserver; do
    (cd $d && exec ./autogen.sh) || res=1
done
exit $res
