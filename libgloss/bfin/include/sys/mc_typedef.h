/*
 * The authors hereby grant permission to use, copy, modify, distribute,
 * and license this software and its documentation for any purpose, provided
 * that existing copyright notices are retained in all copies and that this
 * notice is included verbatim in any distributions. No written agreement,
 * license, or royalty fee is required for any of the authorized uses.
 * Modifications to this software may be copyrighted by their authors
 * and need not follow the licensing terms described here, provided that
 * the new terms are clearly indicated on the first page of each file where
 * they apply.
 */

#pragma once
#ifndef __NO_BUILTIN
#pragma system_header /* sys/mc_typedef.h */
#endif
/************************************************************************
 *
 * sys/mc_typedef.h
 *
 * (c) Copyright 2007-2009 Analog Devices, Inc.  All rights reserved.
 *
 ************************************************************************/

/* Define testset_t. */

#ifndef _SYS_MC_TYPEDEF_H
#define _SYS_MC_TYPEDEF_H

#if !defined(__ADSPLPBLACKFIN__)
  typedef volatile unsigned char testset_t;
#elif defined(__WORKAROUND_TESTSET_ALIGN) || defined(__WORKAROUND_05000412)
  /* these workarounds require 32-bit aligned address */
  typedef volatile unsigned int testset_t;
#else
  typedef volatile unsigned short testset_t;
#endif

#endif /* _SYS_MC_TYPEDEF_H */
