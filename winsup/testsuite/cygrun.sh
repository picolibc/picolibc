#!/bin/dash
#
# test driver to run $1 in the appropriate environment
#

# $1 = test executable to run
exe=$1

export PATH="$runtime_root:${PATH}"

if [ "$1" = "./mingw/cygload" ]
then
    windows_runtime_root=$(cygpath -m $runtime_root)
    $cygrun "$exe -v -cygwin $windows_runtime_root/cygwin1.dll"
else
    cygdrop $cygrun $exe
fi
