/*
 * CRT_FP10.c
 *
 * This object file defines __CRT_PC to have a value of 10,
 * which will set default floating point precesion to 64-bit mantissa
 * at app startup.
 *
 * Linking in CRT_FP10.o before libmingw.a will override the value
 * set by CRT_FP8.o. 
 */

unsigned int __CRT_PC = 10;
