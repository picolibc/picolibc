/*
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * (c) Copyright 2019 Joel Sherrill <joel@rtems.org
 */

/*
 * This file is intentionally empty.
 *
 * Newlib's build infrastructure needs a machine specific file to override
 * the generic implementation in the library.  When a target implementation
 * of the fenv.h methods puts all methods in a single file (e.g. fenv.c) or
 * some as inline methods in its <sys/fenv.h>, it will need to override the
 * default implementation found in a file in this directory.
 *
 * For each file that the target's machine directory needs to override,
 * there should be a corresponding stub file in the target directory.
 * To avoid copying this explanation far and wide, #including this
 * fenv_stub.c from the stub files in encouraged.
 */

/* deliberately empty */

