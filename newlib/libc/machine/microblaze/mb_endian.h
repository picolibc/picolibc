/* Copyright (c) 2013 Corinna Vinschen <corinna@vinschen.de> */
#ifndef __MB_ENDIAN_H
#define __MB_ENDIAN_H

/* Convenience macros for loading and store 4 bytes in a byte invariant way with
 * a singe instruction. Endianess affects this and we rely on MicroBlaze
 * load/store reverse instructions to do the trick on little-endian systems.
 */
#ifdef __LITTLE_ENDIAN__
#define LOAD4BYTES(rD,rA,rB)   "\tlwr\t" rD ", " rA ", " rB "\n"
#define STORE4BYTES(rD,rA,rB)  "\tswr\t" rD ", " rA ", " rB "\n"
#else
#define LOAD4BYTES(rD,rA,rB)   "\tlw\t" rD ", " rA ", " rB "\n"
#define STORE4BYTES(rD,rA,rB)  "\tsw\t" rD ", " rA ", " rB "\n"
#endif
#endif
