#ifndef _ROUTPROT_H
#define _ROUTPROT_H
#if __GNUC__ >= 3
#pragma GCC system_header
#endif

#ifdef __cplusplus
extern "C" {
#endif

#if (_WIN32_WINNT >= 0x0500)
typedef struct IP_ADAPTER_BINDING_INFO {
	ULONG AddressCount;
	DWORD RemoteAddress;
	ULONG Mtu;
	ULONGLONG Speed;
	IP_LOCAL_BINDING Address[];
} IP_ADAPTER_BINDING_INFO,*PIP_ADAPTER_BINDING_INFO;
typedef struct IP_LOCAL_BINDING {
	DWORD Address;
	DWORD Mask;
} IP_LOCAL_BINDING,*PIP_LOCAL_BINDING;
typedef struct IPX_ADAPTER_BINDING_INFO {
	ULONG AdapterIndex;
	UCHAR Network[4];
	UCHAR LocalNode[6];
	UCHAR RemoteNode[6];
	ULONG MaxPacketSize;
	ULONG LinkSpeed;
} IPX_ADAPTER_BINDING_INFO,*PIPX_ADAPTER_BINDING_INFO;
#endif

#ifdef __cplusplus
}
#endif
#endif
