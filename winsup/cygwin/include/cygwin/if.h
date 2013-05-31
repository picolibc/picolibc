/* cygwin/if.h

   Copyright 1996, 2000, 2001, 2007, 2012, 2013 Red Hat, Inc.

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

#ifndef _CYGWIN_IF_H_
#define _CYGWIN_IF_H_

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <sys/types.h>
#include <cygwin/socket.h>

/* Standard interface flags. */
#define IFF_UP           0x1             /* interface is up               */
#define IFF_BROADCAST    0x2             /* broadcast address valid       */
#define IFF_LOOPBACK     0x8             /* is a loopback net             */
#define IFF_POINTOPOINT  0x10            /* is a point-to-point interface */
#define IFF_NOTRAILERS   0x20            /* avoid use of trailers         */
#define IFF_RUNNING      0x40            /* resources allocated           */
#define IFF_NOARP        0x80            /* no ARP protocol               */
#define IFF_PROMISC      0x100           /* receive all packets           */
#define IFF_MULTICAST    0x1000          /* Supports multicast            */
#define IFF_LOWER_UP     0x10000         /* driver signals L1 up          */
#define IFF_DORMANT      0x20000         /* driver signals dormant        */

struct if_nameindex {
  unsigned  if_index;
  char     *if_name;
};

/* This is the structure expected by ioctl when the application requests
   the friendly adapter name (>= XP SP1).  ifru_data should point to such
   a structure when ioctl is called with SIOCGIFFRNDLYNAM. */
#define IFRF_FRIENDLYNAMESIZ 260

struct ifreq_frndlyname {
  int  ifrf_len;
  char ifrf_friendlyname[IFRF_FRIENDLYNAMESIZ];
};

/*
 * Interface request structure used for socket
 * ioctl's.  All interface ioctl's must have parameter
 * definitions which begin with ifr_name.  The
 * remainder may be interface specific.
 */
#define IFNAMSIZ        44
#define IF_NAMESIZE     IFNAMSIZ
#define IFHWADDRLEN     6

struct ifreq {
  union {
    char    ifrn_name[IFNAMSIZ];   /* Unique Windows Adapter name (A GUID) */
  } ifr_ifrn;

  union {
    struct  sockaddr ifru_addr;
    struct  sockaddr ifru_broadaddr;
    struct  sockaddr ifru_dstaddr;
    struct  sockaddr ifru_netmask;
    struct  sockaddr ifru_hwaddr;
    int     ifru_flags;
    int     ifru_metric;
    int     ifru_mtu;
    int     ifru_ifindex;
    /* The space must be preallocated by the application. */
    void   *ifru_data;
    /* Pad to sizeof sockaddr_in6 for further extensions. */
    char    __ifru_pad[28];
  } ifr_ifru;
};

#define ifr_name        ifr_ifrn.ifrn_name      /* interface name       */
#define ifr_addr        ifr_ifru.ifru_addr      /* address              */
#define ifr_broadaddr   ifr_ifru.ifru_broadaddr /* broadcast address    */
#define ifr_dstaddr     ifr_ifru.ifru_dstaddr   /* destination address  */
#define ifr_netmask     ifr_ifru.ifru_netmask   /* interface net mask   */
#define ifr_flags       ifr_ifru.ifru_flags     /* flags                */
#define ifr_hwaddr      ifr_ifru.ifru_hwaddr    /* MAC address          */
#define ifr_metric      ifr_ifru.ifru_metric    /* metric               */
#define ifr_mtu         ifr_ifru.ifru_mtu       /* mtu                  */
#define ifr_ifindex     ifr_ifru.ifru_ifindex   /* interface index      */
#define ifr_data        ifr_ifru.ifru_data      /* for use by interface */
#define ifr_frndlyname  ifr_ifru.ifru_data      /* Windows friendly if name */

/*
 * Structure used in SIOCGIFCONF request.
 * Used to retrieve interface configuration
 * for machine (useful for programs which
 * must know all networks accessible).
 */

struct ifconf
{
  int     ifc_len;                        /* size of buffer       */
  union
  {
    caddr_t ifcu_buf;
    struct  ifreq *ifcu_req;
  } ifc_ifcu;
};

#define ifc_buf ifc_ifcu.ifcu_buf               /* buffer address       */
#define ifc_req ifc_ifcu.ifcu_req               /* array of structures  */

#ifndef __INSIDE_CYGWIN_NET__
extern unsigned             if_nametoindex (const char *);
extern char                *if_indextoname (unsigned, char *);
extern struct if_nameindex *if_nameindex (void);
extern void                 if_freenameindex (struct if_nameindex *);
#endif

#ifdef __cplusplus
};
#endif /* __cplusplus */

#endif /* _CYGWIN_IF_H_ */
