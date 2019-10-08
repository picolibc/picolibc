/*
 * SPDX-License-Identifier: BSD-2-Clause
 * 
 * (c) Copyright 2019 Joel Sherrill <joel@rtems.org
 */

/*
 * This file is intentionally empty. 
 *
 * Newlib's build infrastructure needs a machine specific fiel to override
 * the generic implementation in the library.  When a target
 * implementation of the fenv.h methods puts all methods in a single file
 * (e.g. fenv.c) or some as inline methods in its <sys/fenv.h>, it will need
 * to override the default implementation found in a file in this directory.
 *
 * For each file that the target's machine directory needs to override,
 * this file should be symbolically linked to that specific file name
 * in the target directory. For example, the target may use fe_dfl_env.c
 * from the default implementation but need to override all others.
 */

/* deliberately empty */

