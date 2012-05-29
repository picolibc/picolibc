#! /bin/sh
# This script shows how to generate a 64 bit version of cyglsa.dll.
# The 32 bit version will not work on 64 bit systems.
#
# Note that you need MinGW-w64 GCC, headers, and import libs.  On Cygwin,
# the required packages are: mingw64-x86_64-binutils, mingw64-x86_64-gcc-core,
# mingw64-x86_64-headers, and mingw64-x86_64-runtime.
#
# Note that this is for building inside the source dir as not to interfere
# with the "official" 32 bit build in the build directory.
#
# Install the dll into /bin and use the cyglsa-config script to register it.
# Don't forget to reboot afterwards.
#
# Add "-DDEBUGGING" to CFLAGS below to create debugging output to
# C:\cyglsa.dbgout at runtime.
#
set -e

CC="x86_64-w64-mingw32-gcc"
CFLAGS="-fno-exceptions -O0 -Wall -Werror"
LDFLAGS="-s -nostdlib -Wl,--entry,DllMain,--major-os-version,5,--minor-os-version,2"
# Never again try to remove advapi32.  It does not matter if the DLL calls
# advapi32 functions or the equivalent ntdll functions.
# But if the LSA authentication DLL is not linked against advapi32, it's
# not recognized by LSA.
LIBS="-ladvapi32 -lkernel32 -lntdll"

$CC $CFLAGS $LDFLAGS -shared -o cyglsa64.dll cyglsa.c cyglsa64.def $LIBS
