/*
 * Copyright (c) 1982, 1986, 1989, 1993
 *	The Regents of the University of California.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *	This product includes software developed by the University of
 *	California, Berkeley and its contributors.
 * 4. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 *	@(#)if.h	8.1 (Berkeley) 6/10/93
 * $FreeBSD: src/sys/net/if.h,v 1.71 2002/03/19 21:54:16 alfred Exp $
 */

#ifndef _NET_IF_H
#define	_NET_IF_H

#include <phoenix/iface.h>
#include <sys/queue.h>
#include <sys/socket.h>
#include <sys/time.h>

struct ifnet;

/*
 * Length of interface external name, including terminating '\0'.
 * Note: this is the same size as a generic device's external name.
 */
#define	IFNAMSIZ		16
#define IF_NAMESIZE		IFNAMSIZ
#define	IF_MAXUNIT		0x7fff			/* ifp->if_unit is only 15 bits */

/* Structure describing a `cloning' interface. */
struct if_clone {
	LIST_ENTRY(if_clone) ifc_list;		/* On list of cloners */
	const char *ifc_name;				/* Name of device, e.g. `gif' */
	size_t ifc_namelen;					/* Length of name */
	int ifc_maxunit;					/* Maximum unit number */
	unsigned char *ifc_units;			/* Bitmap to handle units */
	int ifc_bmlen;						/* Bitmap length */

	int	(*ifc_create)(struct if_clone *, int);
	int	(*ifc_destroy)(struct ifnet *);
};

#define IF_CLONE_INITIALIZER(name, create, destroy, maxunit)		\
	{ { 0 }, name, sizeof(name) - 1, maxunit, NULL, 0, create, destroy }

/* Structure used to query names of interface cloners. */
struct if_clonereq {
	int	ifcr_total;			/* Total cloners (out) */
	int	ifcr_count;			/* Room for this many in user buffer */
	char *ifcr_buffer;		/* Buffer for cloner names */
};

/*
 * Structure describing information about an interface
 * which may be of interest to management entities.
 */
struct if_data {
	/* Generic interface information */
	u_char ifi_type;					/* Ethernet, tokenring, etc */
	u_char ifi_physical;				/* E.g., AUI, Thinnet, 10base-T, etc */
	u_char ifi_addrlen;					/* Media address length */
	u_char ifi_hdrlen;					/* Media header length */
	u_char ifi_recvquota;				/* Polling quota for receive intrs */
	u_char ifi_xmitquota;				/* Polling quota for xmit intrs */
	u_long ifi_mtu;						/* Maximum transmission unit */
	u_long ifi_metric;					/* Routing metric (external only) */
	u_long ifi_baudrate;				/* Linespeed */
	/* Volatile statistics */
	u_long ifi_ipackets;				/* Packets received on interface */
	u_long ifi_ierrors;					/* Input errors on interface */
	u_long ifi_opackets;				/* Packets sent on interface */
	u_long ifi_oerrors;					/* Output errors on interface */
	u_long ifi_collisions;				/* Collisions on csma interfaces */
	u_long ifi_ibytes;					/* Total number of octets received */
	u_long ifi_obytes;					/* Total number of octets sent */
	u_long ifi_imcasts;					/* Packets received via multicast */
	u_long ifi_omcasts;					/* Packets sent via multicast */
	u_long ifi_iqdrops;					/* Dropped on input, this interface */
	u_long ifi_noproto;					/* Destined for unsupported protocol */
	u_long ifi_hwassist;				/* HW offload capabilities */
	u_long ifi_unused;					/* XXX was ifi_xmittiming */
	struct timeval ifi_lastchange;		/* Time of last administrative change */
};

/*
 * The following flag(s) ought to go in if_flags, but we cannot change
 * struct ifnet because of binary compatibility, so we store them in
 * if_ipending, which is not used so far.
 * If possible, make sure the value is not conflicting with other
 * IFF flags, so we have an easier time when we want to merge them.
 */
#define	IFF_POLLING			0x10000		/* Interface is in polling mode. */

/* Flags set internally only */
#define	IFF_CANTCHANGE 		(IFF_BROADCAST | IFF_POINTOPOINT | IFF_RUNNING | IFF_OACTIVE | IFF_SIMPLEX | IFF_MULTICAST | IFF_ALLMULTI | IFF_NOTRAILERS)

/* Capabilities that interfaces can advertise. */
#define IFCAP_RXCSUM		0x0001		/* Can offload checksum on RX */
#define IFCAP_TXCSUM		0x0002		/* Can offload checksum on TX */
#define IFCAP_NETCONS		0x0004		/* Can be a network console */

#define IFCAP_HWCSUM		(IFCAP_RXCSUM | IFCAP_TXCSUM)

#define	IFQ_MAXLEN			50
#define	IFNET_SLOWHZ		1			/* Granularity is 1 second */

/* Message format for use in obtaining information about interfaces from getkerninfo and the routing socket */
struct if_msghdr {
	u_short	ifm_msglen;			/* To skip over non-understood messages */
	u_char ifm_version;			/* Future binary compatibility */
	u_char ifm_type;			/* Message type */
	int	ifm_addrs;				/* Like rtm_addrs */
	int	ifm_flags;				/* Value of if_flags */
	u_short	ifm_index;			/* Index for associated ifp */
	struct if_data ifm_data;	/* Statistics and other data about if */
};

/* Message format for use in obtaining information about interface addresses from getkerninfo and the routing socket */
struct ifa_msghdr {
	u_short	ifam_msglen;		/* To skip over non-understood messages */
	u_char ifam_version;		/* Future binary compatibility */
	u_char ifam_type;			/* Message type */
	int	ifam_addrs;				/* Like rtm_addrs */
	int	ifam_flags;				/* Value of ifa_flags */
	u_short	ifam_index;			/* Index for associated ifp */
	int	ifam_metric;			/* Value of ifa_metric */
};

/* Message format for use in obtaining information about multicast addresses from the routing socket */
struct ifma_msghdr {
	u_short	ifmam_msglen;		/* To skip over non-understood messages */
	u_char ifmam_version;		/* Future binary compatibility */
	u_char ifmam_type;			/* Message type */
	int	ifmam_addrs;			/* Like rtm_addrs */
	int	ifmam_flags;			/* Value of ifa_flags */
	u_short	ifmam_index;		/* Index for associated ifp */
};

/* Message format announcing the arrival or departure of a network interface. */
struct if_announcemsghdr {
	u_short	ifan_msglen;		/* To skip over non-understood messages */
	u_char ifan_version;		/* Future binary compatibility */
	u_char ifan_type;			/* Message type */
	u_short	ifan_index;			/* Index for associated ifp */
	char ifan_name[IFNAMSIZ]; 	/* If name, e.g. "en0" */
	u_short	ifan_what;			/* What type of announcement */
};

#define	IFAN_ARRIVAL		0		/* Interface arrival */
#define	IFAN_DEPARTURE		1		/* Interface departure */

struct ifaliasreq {
	char ifra_name[IFNAMSIZ];	/* if name, e.g. "en0" */
	struct sockaddr ifra_addr;
	struct sockaddr ifra_broadaddr;
	struct sockaddr ifra_mask;
};

struct ifmediareq {
	char ifm_name[IFNAMSIZ];	/* If name, e.g. "en0" */
	int	ifm_current;			/* Current media options */
	int	ifm_mask;				/* Don't care mask */
	int	ifm_status;				/* Media status */
	int	ifm_active;				/* Active options */
	int	ifm_count;				/* # entries in ifm_ulist array */
	int	*ifm_ulist;				/* Media words */
};

/* 
 * Structure used to retrieve aux status data from interfaces.
 * Kernel suppliers to this interface should respect the formatting
 * needed by ifconfig(8): each line starts with a TAB and ends with
 * a newline.  The canonical example to copy and paste is in if_tun.c.
 */
#define	IFSTATMAX			800			/* 10 lines of text */
struct ifstat {
	char ifs_name[IFNAMSIZ];			/* if name, e.g. "en0" */
	char ascii[IFSTATMAX + 1];
};

/*
 * Structure used in SIOCGIFCONF request.
 * Used to retrieve interface configuration
 * for machine (useful for programs which
 * must know all networks accessible).
 */
struct	ifconf {
	int	ifc_len;		/* Size of associated buffer */
	union {
		caddr_t	ifcu_buf;
		struct	ifreq *ifcu_req;
	} ifc_ifcu;
#define	ifc_buf				ifc_ifcu.ifcu_buf		/* Buffer address */
#define	ifc_req				ifc_ifcu.ifcu_req		/* Array of structures returned */
};


/* Structure for SIOC[AGD]LIFADDR */
struct if_laddrreq {
	char iflr_name[IFNAMSIZ];
	u_int flags;
#define	IFLR_PREFIX			0x8000		/* in: prefix given  out: kernel fills id */
	u_int prefixlen;					/* in/out */
	struct sockaddr_storage addr;		/* in/out */
	struct sockaddr_storage dstaddr;	/* out */
};

struct if_nameindex {
	u_int if_index;		/* 1, 2, ... */
	char *if_name;		/* null terminated name: "le0", ... */
};

unsigned int if_nametoindex(const char *ifname);
char *if_indextoname(unsigned int ifindex, char *ifname);
struct if_nameindex *if_nameindex();
void if_freenameindex(struct if_nameindex *ptr);

#endif
