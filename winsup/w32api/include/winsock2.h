/*

  Definitions for winsock 2

  FIXME: This is mostly a stub for now. Taken from the Wine project.
  
  Portions Copyright (c) 1980, 1983, 1988, 1993
  The Regents of the University of California.  All rights reserved.

  Portions Copyright (c) 1993 by Digital Equipment Corporation.
 */

#ifndef _WINSOCK2_H
#define _WINSOCK2_H
#define _GNU_H_WINDOWS32_SOCKETS
#ifdef __cplusplus
extern "C" {
#endif

#include <winsock.h>

#define FD_MAX_EVENTS 10
#define FD_READ_BIT 0
#define FD_WRITE_BIT 1
#define FD_OOB_BIT 2
#define FD_ACCEPT_BIT 3
#define FD_CONNECT_BIT 4
#define FD_CLOSE_BIT 5
#define FROM_PROTOCOL_INFO -1
#define SO_GROUP_ID 0x2001
#define SO_GROUP_PRIORITY 0x2002
#define SO_MAX_MSG_SIZE 0x2003
#define SO_PROTOCOL_INFOA 0x2004
#define SO_PROTOCOL_INFOW 0x2005
#define MAX_PROTOCOL_CHAIN 7
#define BASE_PROTOCOL 1
#define LAYERED_PROTOCOL 0
#define WSAPROTOCOL_LEN 255
#define WSA_FLAG_OVERLAPPED 0x01
#define WSA_FLAG_MULTIPOINT_C_ROOT 0x02
#define WSA_FLAG_MULTIPOINT_C_LEAF 0x04
#define WSA_FLAG_MULTIPOINT_D_ROOT 0x08
#define WSA_FLAG_MULTIPOINT_D_LEAF 0x10
typedef HANDLE WSAEVENT;
typedef unsigned int GROUP;
#ifndef GUID_DEFINED
#define GUID_DEFINED
typedef struct _GUID {
    unsigned long Data1;
    unsigned short Data2;
    unsigned short Data3;
    unsigned char Data4[8];
} GUID,*REFGUID,*LPGUID;
#endif /* GUID_DEFINED */
typedef struct _WSANETWORKEVENTS {
	long lNetworkEvents;
	int iErrorCode[FD_MAX_EVENTS];
} WSANETWORKEVENTS, *LPWSANETWORKEVENTS;
typedef struct _WSAPROTOCOLCHAIN {
	int ChainLen;
	DWORD ChainEntries[MAX_PROTOCOL_CHAIN];
} WSAPROTOCOLCHAIN,*LPWSAPROTOCOLCHAIN;
typedef struct _WSAPROTOCOL_INFOA {
	DWORD dwServiceFlags1;
	DWORD dwServiceFlags2;
	DWORD dwServiceFlags3;
	DWORD dwServiceFlags4;
	DWORD dwProviderFlags;
	GUID ProviderId;
	DWORD dwCatalogEntryId;
	WSAPROTOCOLCHAIN ProtocolChain;
	int iVersion;
	int iAddressFamily;
	int iMaxSockAddr;
	int iMinSockAddr;
	int iSocketType;
	int iProtocol;
	int iProtocolMaxOffset;
	int iNetworkByteOrder;
	int iSecurityScheme;
	DWORD dwMessageSize;
	DWORD dwProviderReserved;
	CHAR   szProtocol[WSAPROTOCOL_LEN+1];
} WSAPROTOCOL_INFOA,*LPWSAPROTOCOL_INFOA;
typedef struct _WSAPROTOCOL_INFOW {
	DWORD dwServiceFlags1;
	DWORD dwServiceFlags2;
	DWORD dwServiceFlags3;
	DWORD dwServiceFlags4;
	DWORD dwProviderFlags;
	GUID ProviderId;
	DWORD dwCatalogEntryId;
	WSAPROTOCOLCHAIN ProtocolChain;
	int iVersion;
	int iAddressFamily;
	int iMaxSockAddr;
	int iMinSockAddr;
	int iSocketType;
	int iProtocol;
	int iProtocolMaxOffset;
	int iNetworkByteOrder;
	int iSecurityScheme;
	DWORD dwMessageSize;
	DWORD dwProviderReserved;
	WCHAR  szProtocol[WSAPROTOCOL_LEN+1];
} WSAPROTOCOL_INFOW,*LPWSAPROTOCOL_INFOW;
SOCKET PASCAL WSASocketA(int,int,int,LPWSAPROTOCOL_INFOA,GROUP,DWORD);
SOCKET PASCAL WSASocketW(int,int,int,LPWSAPROTOCOL_INFOW,GROUP,DWORD);
#define WSACreateEvent() CreateEvent(NULL, TRUE, FALSE, NULL)
int PASCAL WSAEnumNetworkEvents(SOCKET s, WSAEVENT hEventObject, LPWSANETWORKEVENTS lpNetworkEvents);
int PASCAL WSAEventSelect(SOCKET s, WSAEVENT hEventObject, long lNetworkEvents);

#ifdef UNICODE
#define SO_PROTOCOL_INFO SO_PROTOCOL_INFOW
#define WSAPROTOCOL_INFO WSAPROTOCOL_INFOW
#define LPWSAPROTOCOL_INFO LPWSAPROTOCOL_INFOW
#define WSASocket WSASocketW
#else
#define SO_PROTOCOL_INFO SO_PROTOCOL_INFOA
#define WSAPROTOCOL_INFO WSAPROTOCOL_INFOA
#define LPWSAPROTOCOL_INFO LPWSAPROTOCOL_INFOA
#define WSASocket WSASocketA
#endif /* UNICODE */

#ifdef __cplusplus
}
#endif
#endif

