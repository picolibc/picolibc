/*
 * CRT_noglob.c
 * This file has no copyright is assigned and is placed in the Public Domain.
 * This file is a part of the mingw-runtime package.
 * No warranty is given; refer to the file DISCLAIMER within the package.
 *
 * Include this object file to set _CRT_glob to a state that will turn off 
 * command line globbing by default.  NOTE: _CRT_glob has a default state of on.
 *
 * To use this object include the object file in your link command:
 * gcc -o foo.exe foo.o CRT_noglob.o
 *
 */

int _CRT_glob = 0;
