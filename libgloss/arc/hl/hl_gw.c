/*
 * hl_gw.c -- Hostlink gateway, low-level hostlink functions.
 *
 * Copyright (c) 2024 Synopsys Inc.
 *
 * The authors hereby grant permission to use, copy, modify, distribute,
 * and license this software and its documentation for any purpose, provided
 * that existing copyright notices are retained in all copies and that this
 * notice is included verbatim in any distributions. No written agreement,
 * license, or royalty fee is required for any of the authorized uses.
 * Modifications to this software may be copyrighted by their authors
 * and need not follow the licensing terms described here, provided that
 * the new terms are clearly indicated on the first page of each file where
 * they apply.
 *
 */

#include <stdint.h>
#include "hl_gw.h"

#define HL_VERSION 1


/*
 * Maximum message size without service information,
 * see also HL_PAYLOAD_RESERVED.
 */
#ifndef HL_IOCHUNK
  #define HL_IOCHUNK 1024
#endif

/*
 * Each syscall argument have 4 bytes of service information in hostlink
 * protocol (2 bytes for type and 2 for size).  Here we reserve space for
 * 32 arguments.
 */
#define HL_PAYLOAD_RESERVED (32 * 4)

/* "No message here" mark.  */
#define HL_NOADDRESS 0xFFFFFFFF

/* Hostlink gateway structure.  */
struct _hl_hdr {
  uint32_t version;		/* Current version is 1.  */
  uint32_t target2host_addr;	/* Packet address from target to host.  */
  uint32_t host2target_addr;	/* Packet address from host to target.  */
  uint32_t buf_addr;		/* Address for host to write answer.  */
  uint32_t payload_size;	/* Buffer size without packet header.  */
  uint32_t options;		/* For future use.  */
  uint32_t break_to_mon_addr;	/* For future use.  */
} __uncached __packed;

/* Hostlink packet header.  */
struct _hl_pkt_hdr {
  uint32_t packet_id;   /* Packet id.  Always set to 1 here.  */
  uint32_t total_size;  /* Size of packet including header.  */
  uint32_t priority;    /* For future use.  */
  uint32_t type;	/* For future use.  */
  uint32_t checksum;    /* For future use.  */
} __uncached __packed;

/* Main hostlink structure.  */
struct _hl {
  volatile struct _hl_hdr hdr; /* General hostlink information.  */
  /* Start of the hostlink buffer.  */
  volatile struct _hl_pkt_hdr pkt_hdr;
  volatile char payload[HL_IOCHUNK + HL_PAYLOAD_RESERVED];
} __aligned (HL_MAX_DCACHE_LINE) __uncached __packed;


/*
 * Main structure.  Do not rename because simulator will look for the
 * '__HOSTLINK__' symbol.
 */
volatile struct _hl __HOSTLINK__ = {
  .hdr = {
    .version = 1 ,
    .target2host_addr = HL_NOADDRESS
  }
};

/* Get hostlink payload pointer.  */
volatile __uncached void *
_hl_payload (void)
{
  return (volatile __uncached void *) &__HOSTLINK__.payload[0];
}

/* Get hostlink payload size (iochunk + reserved space).  */
static uint32_t
_hl_payload_size (void)
{
  return sizeof (__HOSTLINK__.payload);
}

/* Get used space size in the payload.  */
static uint32_t
_hl_payload_used (volatile __uncached void *p)
{
  return (volatile __uncached char *) p
    - (volatile __uncached char *) _hl_payload ();
}

/* Fill hostlink packet header.  */
static void
_hl_pkt_init (volatile __uncached struct _hl_pkt_hdr *pkt, int size)
{
  pkt->packet_id = 1;
  pkt->total_size = ALIGN (size, 4) + sizeof (*pkt);
  pkt->priority = 0;
  pkt->type = 0;
  pkt->checksum = 0;
}

/* Get hostlink iochunk size.  */
uint32_t
_hl_iochunk_size (void)
{
  return HL_IOCHUNK;
}

/* Get free space size in the payload.  */
uint32_t
_hl_payload_left (volatile __uncached void *p)
{
  return _hl_payload_size () - _hl_payload_used (p);
}

/* Send hostlink packet to the host.  */
void
_hl_send (volatile __uncached void *p)
{
  volatile __uncached struct _hl_hdr *hdr = &__HOSTLINK__.hdr;
  volatile __uncached struct _hl_pkt_hdr *pkt_hdr = &__HOSTLINK__.pkt_hdr;

  _hl_pkt_init (pkt_hdr, _hl_payload_used (p));

#if defined (__ARC64__)
  /*
   * Here we pass only low 4 bytes of the packet address (pkt_hdr).
   * The high part of the address is obtained from __HOSTLINK__ address.
   */
  hdr->buf_addr = (uintptr_t) pkt_hdr & 0xFFFFFFFF;
#else
  hdr->buf_addr = (uint32_t) pkt_hdr;
#endif
  hdr->payload_size = _hl_payload_size ();
  hdr->host2target_addr = HL_NOADDRESS;
  hdr->version = HL_VERSION;
  hdr->options = 0;
  hdr->break_to_mon_addr = 0;

  /* This tells the debugger we have a command.
   * It is responsibility of debugger to set this back to HL_NOADDRESS
   * after receiving the packet.
   * Please note that we don't wait here because some implementations
   * use _hl_blockedPeek() function as a signal that we send a messege.
   */
  hdr->target2host_addr = hdr->buf_addr;
}

/*
 * Wait for host response and return pointer to hostlink payload.
 * Symbol _hl_blockedPeek() is used by the simulator as message signal.
 */
volatile __uncached char * __noinline
_hl_blockedPeek (void)
{
  while (__HOSTLINK__.hdr.host2target_addr == HL_NOADDRESS)
    {
      /* TODO: Timeout.  */
    }

  return _hl_payload ();
}

/* Get message from host.  */
volatile __uncached char *
_hl_recv (void)
{
  return _hl_blockedPeek ();
}

/* Mark hostlink buffer as "No message here".  */
void
_hl_delete (void)
{
  __HOSTLINK__.hdr.target2host_addr = HL_NOADDRESS;
}
