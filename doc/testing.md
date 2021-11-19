## Building for the local processor

If you want to compile the library for your local processor to test
changes in the library, the meson configuration is happy to do that
for you. You won't need a meson cross compilation configuration file,
so all you need is the right compile options. They're mostly the same
as the embedded version, but you don't want the multi-architecture
stuff and I prefer plain debug to an -Os, as that makes debugging the
library easier.

The do-native-configure script has an example:

    #!/bin/sh
    DIR=`dirname $0`
    meson $DIR \
	    -Dmultilib=false \
	    -Dnewlib-wide-orient=false\
	    -Dnewlib-nano-malloc=true\
	    -Dlite-exit=true\
	    -Dnewlib-global-atexit=true\
	    -Dincludedir=lib/newlib-nano/include \
	    -Dlibdir=lib/newlib-nano/lib \
	    -Dtests=true \
	    --buildtype debug

Again, create a directory and build there:

    $ mkdir build-native
    $ cd build-native
    $ ../do-native-configure
    $ ninja

This will also build a test case for printf and scanf in the
'test' directory, which I used to fix up the floating point input and
output code.
