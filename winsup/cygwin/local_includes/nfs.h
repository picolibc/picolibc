/* nfs.h

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

#define NFS_ACT_ON_LINK "NfsActOnLink"
#define NFS_SYML_TARGET "NfsSymlinkTargetName"
#define NFS_V3_ATTR     "NfsV3Attributes"

/* NFS datastructures per RFC 1813, as returned by SFU NFS. */

enum ftype3 {
  NF3REG    = 1,
  NF3DIR    = 2,
  NF3BLK    = 3,
  NF3CHR    = 4,
  NF3LNK    = 5,
  NF3SOCK   = 6,
  NF3FIFO   = 7
};

#pragma pack (push, 4)
struct nfs_timestruc_t
{
  int32_t  tv_sec;
  uint32_t tv_nsec;
};

struct fattr3 {
  uint32_t type;
  uint32_t mode;
  uint32_t nlink;
  uint32_t uid;
  uint32_t gid;
  uint32_t filler1;
  uint64_t size;
  uint64_t used;
  struct
    {
      uint32_t specdata1;
      uint32_t specdata2;
    } rdev;
  uint64_t fsid;
  uint64_t fileid;
  struct nfs_timestruc_t atime;
  struct nfs_timestruc_t mtime;
  struct nfs_timestruc_t ctime;
};
#pragma pack (pop)

struct nfs_aol_ffei_t {
  ULONG NextEntryOffset;
  UCHAR Flags;
  UCHAR EaNameLength;
  USHORT EaValueLength;
  CHAR EaName[sizeof (NFS_ACT_ON_LINK)];
};
extern struct nfs_aol_ffei_t nfs_aol_ffei;

extern uint32_t nfs_type_mapping[];

extern NTSTATUS nfs_fetch_fattr3 (HANDLE h, fattr3 *fattr_buf);
