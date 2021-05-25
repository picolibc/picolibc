/* mqueue_types.h: internal POSIX message queue types

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

#pragma once

#define MQI_MAGIC	0x98765432UL

#define MQ_PATH "/dev/mqueue/"
#define MQ_LEN  (sizeof (MQ_PATH) - 1)

/* The mq_attr structure is defined using long datatypes per POSIX.
   The mq_fattr is the in-file representation of the mq_attr struct.
   Originally created this way for 32/64 bit interoperability, this
   is of no concern anymore. */
#pragma pack (push, 4)
struct mq_fattr
{
  uint32_t mq_flags;
  uint32_t mq_maxmsg;
  uint32_t mq_msgsize;
  uint32_t mq_curmsgs;
};

struct mq_hdr
{
  struct mq_fattr   mqh_attr;	 /* the queue's attributes */
  int32_t           mqh_head;	 /* index of first message */
  int32_t           mqh_free;	 /* index of first free message */
  int32_t           mqh_nwait;	 /* #threads blocked in mq_receive() */
  pid_t             mqh_pid;	 /* nonzero PID if mqh_event set */
  uint32_t        __mqh_ext[36]; /* free for extensions */
  union {
    struct sigevent mqh_event;	 /* for mq_notify() */
    uint64_t      __mqh_dummy[4];
  };
  uint64_t        __mqh_ext2[4]; /* free for extensions */
  uint32_t          mqh_magic;	 /* Expect MQI_MAGIC here, otherwise it's
				    an old-style message queue. */
};

struct msg_hdr
{
  int32_t         msg_next;	 /* index of next on linked list */
  int32_t         msg_len;	 /* actual length */
  unsigned int    msg_prio;	 /* priority */
};
#pragma pack (pop)

struct mq_info
{
  struct mq_hdr  *mqi_hdr;	 /* start of mmap'ed region */
  HANDLE          mqi_sect;      /* file mapping section handle */
  SIZE_T          mqi_sectsize;  /* file mapping section size */
  mode_t          mqi_mode;      /* st_mode of the mapped file */
  HANDLE          mqi_lock;	 /* mutex lock */
  HANDLE          mqi_waitsend;	 /* and condition variable for full queue */
  HANDLE          mqi_waitrecv;	 /* and condition variable for empty queue */
  uint32_t        mqi_magic;	 /* magic number if open */
};


