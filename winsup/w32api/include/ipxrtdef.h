#ifndef _IPXRTDEF_H
#define _IPXRTDEF_H
#if __GNUC__ >= 3
#pragma GCC system_header
#endif

#include <stm.h>

#ifdef __cplusplus
extern "C" {
#endif

#if (_WIN32_WINNT >= 0x0500)
typedef struct _IPX_IF_INFO {
	ULONG AdminState;
	ULONG NetbiosAccept;
	ULONG NetbiosDeliver;
} IPX_IF_INFO,*PIPX_IF_INFO;
typedef IPX_SERVER_ENTRY IPX_STATIC_SERVICE_INFO,*PIPX_STATIC_SERVICE_INFO;
typedef struct _IPXWAN_IF_INFO {
	ULONG AdminState;
} IPXWAN_IF_INFO,*PIPXWAN_IF_INFO;
#endif

#ifdef __cplusplus
}
#endif
#endif
