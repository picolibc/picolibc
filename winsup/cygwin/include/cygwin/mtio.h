/* cygwin/mtio.h

   Written by Corinna Vinschen <corinna@vinschen.de>

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

/* cygwin/mtio.h header file for Cygwin.

   by C. Vinschen.  */

#ifndef _CYGWIN_MTIO_H
#define _CYGWIN_MTIO_H

#include <sys/ioctl.h>
#include <asm/socket.h>

#ifndef DEFTAPE
#define DEFTAPE "/dev/tape"
#endif

/*
 * Structures and definitions for mag tape io control commands
 */

/* structure for MTIOCTOP - mag tape op command */
struct	mtop {
	short	mt_op;		/* operations defined below */
	int	mt_count;	/* how many of them */
};

/* Magnetic Tape operations [Not all operations supported by all drivers]: */
#define MTRESET 0	/* reset drive in case of problems */
#define MTFSF	1	/* forward space over FileMark,
			 * position at first record of next file
			 */
#define MTBSF	2	/* backward space FileMark (position before FM) */
#define MTFSR	3	/* forward space record */
#define MTBSR	4	/* backward space record */
#define MTWEOF	5	/* write an end-of-file record (mark) */
#define MTREW	6	/* rewind */
#define MTOFFL	7	/* rewind and put the drive offline (eject?) */
#define MTNOP	8	/* no op, set status only (read with MTIOCGET) */
#define MTRETEN 9	/* retension tape */
#define MTBSFM	10	/* +backward space FileMark, position at FM */
#define MTFSFM  11	/* +forward space FileMark, position at FM */
#define MTEOM	12	/* goto end of recorded media (for appending files).
			 * MTEOM positions after the last FM, ready for
			 * appending another file.
			 */
#define MTERASE 13	/* erase tape -- be careful! */

#define MTRAS1  14	/* run self test 1 (nondestructive) */
#define MTRAS2	15	/* run self test 2 (destructive) */
#define MTRAS3  16	/* reserved for self test 3 */

#define MTSETBLK 20	/* set block length (SCSI) */
#define MTSETDENSITY 21	/* set tape density (SCSI) */
#define MTSEEK	22	/* seek to block (Tandberg, etc.) */
#define MTTELL	23	/* tell block (Tandberg, etc.) */
#define MTSETDRVBUFFER 24 /* set the drive buffering according to SCSI-2 */
			/* ordinary buffered operation with code 1 */
#define MTFSS	25	/* space forward over setmarks */
#define MTBSS	26	/* space backward over setmarks */
#define MTWSM	27	/* write setmarks */

#define MTLOCK  28	/* lock the drive door */
#define MTUNLOCK 29	/* unlock the drive door */
#define MTLOAD  30	/* execute the SCSI load command */
#define MTUNLOAD 31	/* execute the SCSI unload command */
#define MTCOMPRESSION 32/* control compression with SCSI mode page 15 */
#define MTSETPART 33	/* Change the active tape partition */
#define MTMKPART  34	/* Format the tape with one or two partitions */

/* structure for MTIOCGET - mag tape get status command */

struct	mtget {
	long	mt_type;	/* type of magtape device */
	long	mt_resid;	/* residual count: (not sure)
				 *	number of bytes ignored, or
				 *	number of files not skipped, or
				 *	number of records not skipped.
				 *  Cygwin:
				 *      remaining KB until 1.5.7.
				 *      active partition until 1.7.24.
				 *      active partition in low 16 bits,
				 *      number of partitions on this tape
				 *      in next 16 bits, since 1.7.25.
				 */
	/* the following registers are device dependent */
	long	mt_dsreg;	/* status register, Contains blocksize and
				   density code.  See MT_ST_xxx macros below */
	long	mt_gstat;	/* generic (device independent) status */
	long	mt_erreg;	/* error register */
	/* The next two fields are not always used */
	long	mt_fileno;	/* number of current file on tape */
	long	mt_blkno;	/* current block number */
	/* The next are Windows NT specific */
	long long	mt_capacity;	/* Tape capacity in bytes */
	long long	mt_remaining;	/* Remaining bytes */
	int		mt_minblksize;
	int		mt_maxblksize;
	int		mt_defblksize;
	unsigned long	mt_featureslow;
	unsigned long	mt_featureshigh;
	unsigned long	mt_eotwarningzonesize;
};

/* structure for MTIOCPOS - mag tape get position command */

struct	mtpos {
	long	mt_blkno;	/* current block number */
};


/* mag tape io control commands */
#define	MTIOCTOP	_IOW('m', 1, struct mtop)	/* do a mag tape op */
#define	MTIOCGET	_IOR('m', 2, struct mtget)	/* get tape status */
#define	MTIOCPOS	_IOR('m', 3, struct mtpos)	/* get tape position */

/* Generic Mag Tape (device independent) status macros for examining
 * mt_gstat -- HP-UX compatible.
 * There is room for more generic status bits here, but I don't
 * know which of them are reserved. At least three or so should
 * be added to make this really useful.
 */
#define GMT_EOF(x)              ((x) & 0x80000000)
#define GMT_BOT(x)              ((x) & 0x40000000)
#define GMT_EOT(x)              ((x) & 0x20000000)
#define GMT_SM(x)               ((x) & 0x10000000)  /* DDS setmark */
#define GMT_EOD(x)              ((x) & 0x08000000)  /* DDS EOD */
#define GMT_WR_PROT(x)          ((x) & 0x04000000)
#define GMT_REP_SM(x)           ((x) & 0x02000000)  /* Cygwin: rep. setmarks */
#define GMT_ONLINE(x)           ((x) & 0x01000000)
#define GMT_D_6250(x)           ((x) & 0x00800000)
#define GMT_D_1600(x)           ((x) & 0x00400000)
#define GMT_D_800(x)            ((x) & 0x00200000)
#define GMT_PADDING(x)          ((x) & 0x00100000)  /* Cygwin: data padding */
#define GMT_HW_ECC(x)           ((x) & 0x00080000)  /* Cygwin: HW error corr. */
#define GMT_DR_OPEN(x)          ((x) & 0x00040000)  /* door open (no tape) */
#define GMT_HW_COMP(x)          ((x) & 0x00020000)  /* Cygwin: HW compression */
#define GMT_IM_REP_EN(x)        ((x) & 0x00010000)  /* immediate report mode */
#define GMT_CLN(x)              ((x) & 0x00008000)  /* cleaning requested */
/* 15 generic status bits unused */
/* Cygwin only: The below settings are also used by the GNU-Linux SCSI tape
   driver but they aren't usually reported here.  Unfortunately, there's no
   other official method to retrieve the values of these settings and
   reporting them here apparently doesn't hurt. */
#define GMT_TWO_FM(x)           ((x) & 0x00000080)  /* two fm after write */
#define GMT_FAST_MTEOM(x)       ((x) & 0x00000040)  /* fast seek to eom */
#define GMT_AUTO_LOCK(x)        ((x) & 0x00000020)  /* auto door lock on r/w */
#define GMT_SYSV(x)		((x) & 0x00000010)  /* SYSV read semantics */
#define GMT_NOWAIT(x)		((x) & 0x00000008)  /* don't wait for positioning commands */
#define GMT_ASYNC(x)		((x) & 0x00000004)  /* asynchronous writes */


/* SCSI-tape specific definitions */
/* Bitfield shifts in the status mt_dsreg */
#define MT_ST_BLKSIZE_SHIFT	0
#define MT_ST_BLKSIZE_MASK	0xffffff
#define MT_ST_DENSITY_SHIFT	24
#define MT_ST_DENSITY_MASK	0xff000000

#define MT_ST_SOFTERR_SHIFT	0
#define MT_ST_SOFTERR_MASK	0xffff

/* Bitfields for the MTSETDRVBUFFER ioctl.  */
#define MT_ST_OPTIONS           0xf0000000
#define MT_ST_BOOLEANS          0x10000000
#define MT_ST_SETBOOLEANS       0x30000000
#define MT_ST_CLEARBOOLEANS     0x40000000
#define MT_ST_WRITE_THRESHOLD   0x20000000	/* Not supported */
#define MT_ST_DEF_OPTIONS       0x60000000	/* Not supported */
#define MT_ST_EOT_WZ_SIZE	0xf0000000	/* Cygwin only */

#define MT_ST_BUFFER_WRITES     0x00000001
#define MT_ST_ASYNC_WRITES	0x00000002
#define MT_ST_READ_AHEAD        0x00000004	/* Not supported */
#define MT_ST_DEBUGGING         0x00000008	/* Not supported */
#define MT_ST_TWO_FM		0x00000010
#define MT_ST_FAST_MTEOM        0x00000020
#define MT_ST_AUTO_LOCK		0x00000040
#define MT_ST_DEF_WRITES        0x00000080	/* Not supported */
#define MT_ST_CAN_BSR           0x00000100	/* Not supported */
#define MT_ST_NO_BLKLIMS        0x00000200	/* Not supported */
#define MT_ST_CAN_PARTITIONS    0x00000400	/* Not supported */
#define MT_ST_SCSI2LOGICAL      0x00000800	/* Not supported */
#define MT_ST_SYSV              0x00001000
#define MT_ST_NOWAIT            0x00002000
#define MT_ST_ECC		0x00010000	/* Cygwin only */
#define MT_ST_PADDING		0x00020000	/* Cygwin only */
#define MT_ST_REPORT_SM		0x00040000	/* Cygwin only */

/*
 * Constants for mt_type. Not all of these are supported,
 * and these are not all of the ones that are supported.
 *
 * Only used when not colliding with Windows codes (see below)
 */
#define MT_ISUNKNOWN		0x01
#define MT_ISQIC02		0x02	/* Generic QIC-02 tape streamer */
#define MT_ISWT5150		0x03	/* Wangtek 5150EQ, QIC-150, QIC-02 */
#define MT_ISARCHIVE_5945L2	0x04	/* Archive 5945L-2, QIC-24, QIC-02? */
#define MT_ISCMSJ500		0x05	/* CMS Jumbo 500 (QIC-02?) */
#define MT_ISTDC3610		0x06	/* Tandberg 6310, QIC-24 */
#define MT_ISARCHIVE_VP60I	0x07	/* Archive VP60i, QIC-02 */
#define MT_ISARCHIVE_2150L	0x08	/* Archive Viper 2150L */
#define MT_ISARCHIVE_2060L	0x09	/* Archive Viper 2060L */
#define MT_ISARCHIVESC499	0x0A	/* Archive SC-499 QIC-36 controller */
#define MT_ISQIC02_ALL_FEATURES	0x0F	/* Generic QIC-02 with all features */
#define MT_ISWT5099EEN24	0x11	/* Wangtek 5099-een24, 60MB, QIC-24 */
#define MT_ISTEAC_MT2ST		0x12	/* Teac MT-2ST 155mb drive, Teac DC-1 card (Wangtek type) */
#define MT_ISEVEREX_FT40A	0x32	/* Everex FT40A (QIC-40) */
#define MT_ISDDS1		0x51	/* DDS device without partitions */
#define MT_ISDDS2		0x52	/* DDS device with partitions */
#define MT_ISSCSI1		0x71	/* Generic ANSI SCSI-1 tape unit */
#define MT_ISSCSI2		0x72	/* Generic ANSI SCSI-2 tape unit */

/* More constants for mt_type.  These are the codes used by Windows >= 5.1 */
#define MT_ISDDS_4mm 		0x20
#define MT_ISMiniQic 		0x21
#define MT_ISTravan 		0x22
#define MT_ISQIC 		0x23
#define MT_ISMP_8mm 		0x24
#define MT_ISAME_8mm 		0x25
#define MT_ISAIT1_8mm 		0x26
#define MT_ISDLT 		0x27
#define MT_ISNCTP 		0x28
#define MT_ISIBM_3480 		0x29
#define MT_ISIBM_3490E 		0x2a
#define MT_ISIBM_Magstar_3590 	0x2b
#define MT_ISIBM_Magstar_MP 	0x2c
#define MT_ISSTK_DATA_D3 	0x2d
#define MT_ISSONY_DTF 		0x2e
#define MT_ISDV_6mm 		0x2f
#define MT_ISDMI 		0x30
#define MT_ISSONY_D2 		0x31
#define MT_ISCLEANER_CARTRIDGE 	0x32
#define MT_ISAVATAR_F2 		0x4f
#define MT_ISMP2_8mm 		0x50
#define MT_ISDST_S 		0x51
#define MT_ISDST_M 		0x52
#define MT_ISDST_L 		0x53
#define MT_ISVXATape_1 		0x54
#define MT_ISVXATape_2 		0x55
#define MT_ISSTK_9840 		0x56
#define MT_ISLTO_Ultrium 	0x57
#define MT_ISLTO_Accelis 	0x58
#define MT_ISAIT_8mm 		0x5a
#define MT_ISADR_1 		0x5b
#define MT_ISADR_2 		0x5c
#define MT_ISSTK_9940 		0x5d

struct mt_tape_info {
	long t_type;		/* device type id (mt_type) */
	char *t_name;		/* descriptive name */
};

#define MT_TAPE_INFO	{ \
	{MT_ISUNKNOWN,		"Unknown type of tape device"}, \
	{MT_ISQIC02,		"Generic QIC-02 tape streamer"}, \
	{MT_ISWT5150,		"Wangtek 5150, QIC-150"}, \
	{MT_ISARCHIVE_5945L2,	"Archive 5945L-2"}, \
	{MT_ISCMSJ500,		"CMS Jumbo 500"}, \
	{MT_ISTDC3610,		"Tandberg TDC 3610, QIC-24"}, \
	{MT_ISARCHIVE_VP60I,	"Archive VP60i, QIC-02"}, \
	{MT_ISARCHIVE_2150L,	"Archive Viper 2150L"}, \
	{MT_ISARCHIVE_2060L,	"Archive Viper 2060L"}, \
	{MT_ISARCHIVESC499,	"Archive SC-499 QIC-36 controller"}, \
	{MT_ISQIC02_ALL_FEATURES, "Generic QIC-02 tape, all features"}, \
	{MT_ISWT5099EEN24,	"Wangtek 5099-een24, 60MB"}, \
	{MT_ISTEAC_MT2ST,	"Teac MT-2ST 155mb data cassette drive"}, \
	{MT_ISDDS_4mm,		"DDS"}, \
	{MT_ISMiniQic,		"MiniQic"}, \
	{MT_ISTravan,		"Travan tape"}, \
	{MT_ISQIC,		"QIC tape"}, \
	{MT_ISMP_8mm,		"8mm Exabyte metal particle tape"}, \
	{MT_ISAME_8mm,		"8mm Exabyte advanced metal evap tape"}, \
	{MT_ISAIT1_8mm,		"8mm Sony AIT1 tape"}, \
	{MT_ISDLT,		"DLT compact tape)"}, \
	{MT_ISNCTP,		"Philips NCTP tape"}, \
	{MT_ISIBM_3480,		"IBM 3480 tape"}, \
	{MT_ISIBM_3490E,	"IBM 3490E tape"}, \
	{MT_ISIBM_Magstar_3590,	"IBM Magstar 3590 tape"}, \
	{MT_ISIBM_Magstar_MP,	"IBM Magstar MP tape"}, \
	{MT_ISSTK_DATA_D3,	"STK data D3 tape"}, \
	{MT_ISSONY_DTF,		"Sony DTF tape"}, \
	{MT_ISDV_6mm,		"6mm digital video tape"}, \
	{MT_ISDMI,		"Exabyte DMI tape"}, \
	{MT_ISSONY_D2,		"Sony D2S or D2L tape"}, \
	{MT_ISCLEANER_CARTRIDGE, "Cleaner (all drive types that support cleaners)"}, \
	{MT_ISAVATAR_F2,	"Avatar 2"}, \
	{MT_ISMP2_8mm,		"8mm Hitachi tape"}, \
	{MT_ISDST_S,		"Ampex DST small tape"}, \
	{MT_ISDST_M,		"Ampex DST medium tape"}, \
	{MT_ISDST_L,		"Ampex DST large tape"}, \
	{MT_ISVXATape_1,	"Ecrix 8mm tape"}, \
	{MT_ISVXATape_2,	"Ecrix 8mm tape"}, \
	{MT_ISSTK_9840,		"STK 9840"}, \
	{MT_ISLTO_Ultrium,	"LTO Ultrium (IBM, HP, Seagate)"}, \
	{MT_ISLTO_Accelis,	"LTO Accelis (IBM, HP, Seagate)"}, \
	{MT_ISAIT_8mm,		"AIT tape (AIT2 or higher)"}, \
	{MT_ISADR_1,		"OnStream ADR1"}, \
	{MT_ISADR_2,		"OnStream ADR2"}, \
	{MT_ISSTK_9940,		"STK 9940"}, \
	{MT_ISSCSI1,		"Generic SCSI-1 tape"}, \
	{MT_ISSCSI2,		"Generic SCSI-2 tape"}, \
	{0, NULL} \
}

#endif /* _CYGWIN_MTIO_H */
