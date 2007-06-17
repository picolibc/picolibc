/*
 * CRTfmode.c
 * This file has no copyright assigned and is placed in the Public Domain.
 * This file is a part of the mingw-runtime package.
 * No warranty is given; refer to the file DISCLAIMER within the package.
 *
 * This libmingw.a object sets  _CRT_fmode to a state that will cause 
 * _mingw32_init_fmode to leave all file modes in their default state 
 * (basically text mode).
 * 	
 * To override the default, add, eg:
 *
 * #include <fcntl.h>
 * int _CRT_fmode = _O_BINARY;
 *
 * to your app.	
 */

int _CRT_fmode = 0;
