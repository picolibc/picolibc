/* Dummy crt0 code.  */

/* Copyright (c) 2003 by Tensilica Inc.  ALL RIGHTS RESERVED.
   These coded instructions, statements, and computer programs are the
   copyrighted works and confidential proprietary information of Tensilica Inc.
   They may not be modified, copied, reproduced, distributed, or disclosed to
   third parties in any manner, medium, or form, in whole or in part, without
   the prior written consent of Tensilica Inc.  */

/* Xtensa systems normally use a crt1 file associated with a particular
   linker support package (LSP).  There is no need for this crt0 file,
   except that the newlib makefiles require it to exist if there is a
   sys/xtensa directory.  The directory exists only to hold the header
   files for the Xtensa ISS semihosting "platform".  */

void crt0_unused (void) {}
