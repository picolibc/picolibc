#ifndef _LMUSE_H
#define _LMUSE_H
#ifdef __cplusplus
extern "C" {
#endif
#include <lmuseflg.h> 
#define USE_LOCAL_PARMNUM 1
#define USE_REMOTE_PARMNUM 2
#define USE_PASSWORD_PARMNUM 3
#define USE_ASGTYPE_PARMNUM 4
#define USE_USERNAME_PARMNUM 5
#define USE_DOMAINNAME_PARMNUM 6
#define USE_OK 0
#define USE_PAUSED 1
#define USE_SESSLOST 2
#define USE_DISCONN 2
#define USE_NETERR 3
#define USE_CONN 4
#define USE_RECONN 5
#define USE_WILDCARD ((DWORD)-1)
#define USE_DISKDEV 0
#define USE_SPOOLDEV 1
#define USE_CHARDEV 2
#define USE_IPC 3
typedef struct _USE_INFO_0 {
	LPTSTR ui0_local;
	LPTSTR ui0_remote;
}USE_INFO_0,*PUSE_INFO_0,*LPUSE_INFO_0;
typedef struct _USE_INFO_1 {
	LPTSTR ui1_local;
	LPTSTR ui1_remote;
	LPTSTR ui1_password;
	DWORD ui1_status;
	DWORD ui1_asg_type;
	DWORD ui1_refcount;
	DWORD ui1_usecount;
}USE_INFO_1,*PUSE_INFO_1,*LPUSE_INFO_1;
typedef struct _USE_INFO_2 {
	LPTSTR ui2_local;
	LPTSTR ui2_remote;
	LPTSTR ui2_password;
	DWORD ui2_status;
	DWORD ui2_asg_type;
	DWORD ui2_refcount;
	DWORD ui2_usecount;
	LPTSTR ui2_username;
	LPTSTR ui2_domainname;
}USE_INFO_2,*PUSE_INFO_2,*LPUSE_INFO_2;
NET_API_STATUS WINAPI NetUseAdd(LPTSTR,DWORD,PBYTE,PDWORD);
NET_API_STATUS WINAPI NetUseDel(LPTSTR,LPTSTR,DWORD);
NET_API_STATUS WINAPI NetUseEnum(LPTSTR,DWORD,PBYTE*,DWORD,PDWORD,PDWORD,PDWORD);
NET_API_STATUS WINAPI NetUseGetInfo(LPTSTR,LPTSTR,DWORD,PBYTE*);
#ifdef __cplusplus
}
#endif
#endif 
