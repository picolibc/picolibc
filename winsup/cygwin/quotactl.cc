/* quotactl.cc: code for manipulating disk quotas

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

#include "winsup.h"
#include "cygtls.h"
#include "security.h"
#include "path.h"
#include "fhandler.h"
#include "dtable.h"
#include "cygheap.h"
#include "ntdll.h"
#include "tls_pbuf.h"
#include <sys/mount.h>
#include <sys/quota.h>

#define PGQI_SIZE (sizeof (FILE_GET_QUOTA_INFORMATION) + SECURITY_MAX_SID_SIZE)
#define PFQI_SIZE (sizeof (FILE_QUOTA_INFORMATION) + SECURITY_MAX_SID_SIZE)

/* Modelled after the Linux quotactl function. */
extern "C" int
quotactl (int cmd, const char *special, int id, caddr_t addr)
{
  ACCESS_MASK access = FILE_READ_DATA;
  PSID sid = NO_SID;
  path_conv pc;
  tmp_pathbuf tp;
  UNICODE_STRING path;
  OBJECT_ATTRIBUTES attr;
  NTSTATUS status;
  HANDLE fh;
  IO_STATUS_BLOCK io;
  FILE_FS_CONTROL_INFORMATION ffci;
  int ret = 0;

  uint32_t subcmd = (uint32_t) cmd >> SUBCMDSHIFT;
  uint32_t type = (uint32_t) cmd & SUBCMDMASK;

  if (type != USRQUOTA && type != GRPQUOTA)
    {
      set_errno (EINVAL);
      return -1;
    }
  switch (subcmd)
    {
    case Q_SYNC:
      if (!special)
	return 0;
      access |= FILE_WRITE_DATA;
      break;
    case Q_QUOTAON:
      if (id < QFMT_VFS_OLD || id > QFMT_VFS_V1)
	{
	  set_errno (EINVAL);
	  return -1;
	}
      /*FALLTHRU*/
    case Q_QUOTAOFF:
    case Q_SETINFO:
      access |= FILE_WRITE_DATA;
      break;
    case Q_GETFMT:
    case Q_GETINFO:
      break;
    case Q_SETQUOTA:
      access |= FILE_WRITE_DATA;
      /*FALLTHRU*/
    case Q_GETQUOTA:
      /* Windows feature: Default limits.  Get or set them with id == -1. */
      if (id != -1)
	{
	  if (type == USRQUOTA)
	    sid = sidfromuid (id, NULL);
	  else
	    sid = sidfromgid (id, NULL);
	  if (sid == NO_SID)
	    {
	      set_errno (EINVAL);
	      return -1;
	    }
	}
      break;
    default:
      set_errno (EINVAL);
      return -1;
    }
  /* Check path */
  pc.check (special, PC_SYM_FOLLOW | PC_NOWARN, stat_suffixes);
  if (pc.error)
    {
      set_errno (pc.error);
      return -1;
    }
  if (!pc.exists ())
    {
      set_errno (ENOENT);
      return -1;
    }
  if (!S_ISBLK (pc.dev.mode ()))
    {
      set_errno (ENOTBLK);
      return -1;
    }
  pc.get_object_attr (attr, sec_none_nih);
  /* For the following functions to work, we must attach the virtual path to 
     the quota file to the device path.
     
     FIXME: Note that this is NTFS-specific.  Adding ReFS in another step. */
  tp.u_get (&path);
  RtlCopyUnicodeString (&path, attr.ObjectName);
  RtlAppendUnicodeToString (&path, L"\\$Extend\\$Quota:$Q:$INDEX_ALLOCATION");
  attr.ObjectName = &path;

  /* Open filesystem */
  status = NtOpenFile (&fh, access, &attr, &io, FILE_SHARE_VALID_FLAGS, 0);
  if (NT_SUCCESS (status))
    switch (subcmd)
      {
      case Q_SYNC:
	/* No sync, just report success. */
	status = STATUS_SUCCESS;
      	break;
      case Q_QUOTAON:
      case Q_QUOTAOFF:
	/* Ignore filename in addr. */
	status = NtQueryVolumeInformationFile (fh, &io, &ffci, sizeof ffci,
					       FileFsControlInformation);
	if (!NT_SUCCESS (status))
	  break;
	ffci.FileSystemControlFlags &= ~FILE_VC_QUOTA_ENFORCE
				       & ~FILE_VC_QUOTA_TRACK
				       & ~FILE_VC_QUOTAS_INCOMPLETE
				       & ~FILE_VC_QUOTAS_REBUILDING;
	if (subcmd == Q_QUOTAON)
	  ffci.FileSystemControlFlags |= FILE_VC_QUOTA_ENFORCE;
	status = NtSetVolumeInformationFile (fh, &io, &ffci, sizeof ffci,
					     FileFsControlInformation);
	break;
      case Q_GETFMT:
      	__try
	  {
	    uint32_t *retval = (uint32_t *) addr;

	    /* Always fake the latest format. */
	    *retval = QFMT_VFS_V1;
	  }
	__except (EFAULT)
	  {
	    ret = -1;
	    break;
	  }
	__endtry
	status = STATUS_SUCCESS;
	break;
      case Q_GETINFO:
	__try
	  {
	    struct dqinfo *dqi = (struct dqinfo *) addr;

	    dqi->dqi_bgrace = dqi->dqi_igrace = UINT64_MAX;
	    dqi->dqi_flags = 0;
	    dqi->dqi_valid = IIF_BGRACE | IIF_IGRACE;
	  }
	__except (EFAULT)
	  {
	    ret = -1;
	    break;
	  }
	__endtry
	status = STATUS_SUCCESS;
	break;
      case Q_SETINFO:
	/* No settings possible, just report success. */
	status = STATUS_SUCCESS;
      	break;
      case Q_GETQUOTA:
	/* Windows feature: Default limits.  Get or set them with id == -1. */
	if (id == -1)
	  {
	    status = NtQueryVolumeInformationFile (fh, &io, &ffci, sizeof ffci,
						   FileFsControlInformation);
	    if (!NT_SUCCESS (status))
	      break;
	    __try
	      {
		struct dqblk *dq = (struct dqblk *) addr;

		dq->dqb_bhardlimit = (uint64_t) ffci.DefaultQuotaLimit.QuadPart;
		if (dq->dqb_bhardlimit != UINT64_MAX)
		  dq->dqb_bhardlimit /= BLOCK_SIZE;
		dq->dqb_bsoftlimit =
				(uint64_t) ffci.DefaultQuotaThreshold.QuadPart;
		if (dq->dqb_bsoftlimit != UINT64_MAX)
		  dq->dqb_bsoftlimit /= BLOCK_SIZE;
		dq->dqb_curspace = 0;
		dq->dqb_ihardlimit = UINT64_MAX;
		dq->dqb_isoftlimit = UINT64_MAX;
		dq->dqb_curinodes = 0;
		dq->dqb_btime = UINT64_MAX;
		dq->dqb_itime = UINT64_MAX;
		dq->dqb_valid = QIF_BLIMITS;
	      }
	    __except (EFAULT)
	      {
		ret = -1;
		break;
	      }
	    __endtry
	  }
	else
	  {
	    PFILE_GET_QUOTA_INFORMATION pgqi = (PFILE_GET_QUOTA_INFORMATION)
					       alloca (PGQI_SIZE);
	    PFILE_QUOTA_INFORMATION pfqi = (PFILE_QUOTA_INFORMATION)
					   alloca (PFQI_SIZE);

	    pgqi->NextEntryOffset = 0;
	    pgqi->SidLength = RtlLengthSid (sid);
	    RtlCopySid (RtlLengthSid (sid), &pgqi->Sid, sid);
	    status = NtQueryQuotaInformationFile (fh, &io, pfqi, PFQI_SIZE,
						  TRUE, pgqi, PGQI_SIZE,
						  NULL, TRUE);
	    if (!NT_SUCCESS (status))
	      break;
	    __try
	      {
		struct dqblk *dq = (struct dqblk *) addr;

		dq->dqb_bhardlimit = (uint64_t) pfqi->QuotaLimit.QuadPart;
		if (dq->dqb_bhardlimit != UINT64_MAX)
		  dq->dqb_bhardlimit /= BLOCK_SIZE;
		dq->dqb_bsoftlimit = (uint64_t) pfqi->QuotaThreshold.QuadPart;
		if (dq->dqb_bsoftlimit != UINT64_MAX)
		  dq->dqb_bsoftlimit /= BLOCK_SIZE;
		dq->dqb_curspace = (uint64_t) pfqi->QuotaUsed.QuadPart;
		if (dq->dqb_curspace != UINT64_MAX)
		  dq->dqb_curspace /= BLOCK_SIZE;
		dq->dqb_ihardlimit = UINT64_MAX;
		dq->dqb_isoftlimit = UINT64_MAX;
		dq->dqb_curinodes = 0;
		dq->dqb_btime = UINT64_MAX;
		dq->dqb_itime = UINT64_MAX;
		dq->dqb_valid = QIF_BLIMITS | QIF_SPACE;
	      }
	    __except (EFAULT)
	      {
		ret = -1;
		break;
	      }
	    __endtry
	  }
	break;
      case Q_SETQUOTA:
	/* Windows feature: Default limits.  Get or set them with id == -1. */
	if (id == -1)
	  {
	    status = NtQueryVolumeInformationFile (fh, &io, &ffci, sizeof ffci,
						   FileFsControlInformation);
	    if (!NT_SUCCESS (status))
	      break;
	    __try
	      {
		struct dqblk *dq = (struct dqblk *) addr;

		if (!(dq->dqb_valid & QIF_BLIMITS))
		  break;
		ffci.DefaultQuotaLimit.QuadPart = dq->dqb_bhardlimit;
		if (ffci.DefaultQuotaLimit.QuadPart != -1)
		  ffci.DefaultQuotaLimit.QuadPart *= BLOCK_SIZE;
		ffci.DefaultQuotaThreshold.QuadPart = dq->dqb_bsoftlimit;
		if (ffci.DefaultQuotaThreshold.QuadPart != -1)
		  ffci.DefaultQuotaThreshold.QuadPart *= BLOCK_SIZE;
	      }
	    __except (EFAULT)
	      {
	        ret = -1;
		break;
	      }
	    __endtry
	    status = NtSetVolumeInformationFile (fh, &io, &ffci, sizeof ffci,
						 FileFsControlInformation);
	  }
	else
	  {
	    PFILE_GET_QUOTA_INFORMATION pgqi = (PFILE_GET_QUOTA_INFORMATION)
					       alloca (PGQI_SIZE);
	    PFILE_QUOTA_INFORMATION pfqi = (PFILE_QUOTA_INFORMATION)
					   alloca (PFQI_SIZE);

	    pgqi->NextEntryOffset = 0;
	    pgqi->SidLength = RtlLengthSid (sid);
	    RtlCopySid (RtlLengthSid (sid), &pgqi->Sid, sid);
	    status = NtQueryQuotaInformationFile (fh, &io, pfqi, PFQI_SIZE,
						  TRUE, pgqi, PGQI_SIZE,
						  NULL, TRUE);
	    if (!NT_SUCCESS (status))
	      break;
	    __try
	      {
		struct dqblk *dq = (struct dqblk *) addr;

		if (!(dq->dqb_valid & QIF_BLIMITS))
		  break;
		pfqi->QuotaLimit.QuadPart = dq->dqb_bhardlimit;
		if (pfqi->QuotaLimit.QuadPart != -1)
		  pfqi->QuotaLimit.QuadPart *= BLOCK_SIZE;
		pfqi->QuotaThreshold.QuadPart = dq->dqb_bsoftlimit;
		if (pfqi->QuotaThreshold.QuadPart != -1)
		  pfqi->QuotaThreshold.QuadPart *= BLOCK_SIZE;
	      }
	    __except (EFAULT)
	      {
		ret = -1;
		break;
	      }
	    __endtry
	    status = NtSetQuotaInformationFile (fh, &io, pfqi, PFQI_SIZE);
	  }
	break;
      }
  if (!NT_SUCCESS (status))
    {
      __seterrno_from_nt_status (status);
      ret = -1;
    }
  return ret;
}
