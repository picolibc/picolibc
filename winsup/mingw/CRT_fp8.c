/*
 * CRT_FP8.c
 *
 * This object file defines __CRT_PC to have a value of 8, which
 * set default floating point precesion to 53-bit mantissa at app startup.
 *
 * To change to 64-bit mantissa, link in CRT_FP10.o before libmningw.a. 
 */

 unsigned int __CRT_PC = 8;
