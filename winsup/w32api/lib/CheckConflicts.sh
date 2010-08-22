#!/bin/bash

if [ $# -ne 2 ]
then
    echo "Usage: $0 pattern_file check_file"
    exit 1
fi

for pattern in `cat $1 | cut -f1 -d'@'`
do
    grep -Hwrn --color --exclude=$1 $pattern $2
done
