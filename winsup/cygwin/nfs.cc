/* nfs.cc

   Copyright 2008 Red Hat, Inc.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

#include "winsup.h"
#include "sys/fcntl.h"
#include "nfs.h"

struct nfs_aol_ffei_t nfs_aol_ffei = { 0, 0, sizeof (NFS_ACT_ON_LINK) - 1, 0,
				       NFS_ACT_ON_LINK };

uint32_t nfs_type_mapping[] = { 0, S_IFREG, S_IFDIR, S_IFBLK,
                                S_IFCHR, S_IFLNK, S_IFSOCK, S_IFIFO };

