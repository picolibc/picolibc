#ifndef _MPRADMIN_H
#define _MPRADMIN_H
#if __GNUC__ >= 3
#pragma GCC system_header
#endif

#include <ras.h>

#ifdef __cplusplus
extern "C" {
#endif

#if (_WIN32_WINNT >= 0x0500)
#define PID_ATALK 0x00000029
#define PID_IP 0x00000021
#define PID_IPX 0x0000002B
#define PID_NBF 0x0000003F
#define MAX_INTERFACE_NAME_LEN 256
#define MAX_TRANSPORT_NAME_LEN 40
#define MPR_INTERFACE_ADMIN_DISABLED 0x00000002
#define MPR_INTERFACE_CONNECTION_FAILURE 0x00000004
#define MPR_INTERFACE_DIALOUT_HOURS_RESTRICTION 0x00000010
#define MPR_INTERFACE_OUT_OF_RESOURCES 0x00000001
#define MPR_INTERFACE_SERVICE_PAUSED 0x00000008
#define MPR_INTERFACE_NO_MEDIA_SENSE 0x00000020
#define MPR_INTERFACE_NO_DEVICE 0x00000040
#define MPR_MaxDeviceType RAS_MaxDeviceType
#define MPR_MaxDeviceName RAS_MaxDeviceName
#define MPR_MaxPadType RAS_MaxPadType
#define MPR_MaxX25Address RAS_MaxX25Address
#define MPR_MaxFacilities RAS_MaxFacilities
#define MPR_MaxUserData RAS_MaxUserData
#define MPR_MaxPhoneNumber RAS_MaxPhoneNumber
#define MPRIO_SpecificIpAddr RASEO_SpecificIpAddr
#define MPRIO_SpecificNameServers RASEO_SpecificNameServers
#define MPRIO_IpHeaderCompression RASEO_IpHeaderCompression
#define MPRIO_RemoteDefaultGateway RASEO_RemoteDefaultGateway
#define MPRIO_DisableLcpExtensions RASEO_DisableLcpExtensions
#define MPRIO_SwCompression RASEO_SwCompression
#define MPRIO_RequireEncryptedPw RASEO_RequireEncryptedPw
#define MPRIO_RequireMsEncryptedPw RASEO_RequireMsEncryptedPw
#define MPRIO_RequireDataEncryption RASEO_RequireDataEncryption
#define MPRIO_NetworkLogon RASEO_NetworkLogon
#define MPRIO_UseLogonCredentials RASEO_UseLogonCredentials
#define MPRIO_PromoteAlternates RASEO_PromoteAlternates
#define MPRIO_SecureLocalFiles RASEO_SecureLocalFiles
#define MPRIO_RequireEAP RASEO_RequireEAP
#define MPRIO_RequirePAP RASEO_RequirePAP
#define MPRIO_RequireSPAP RASEO_RequireSPAP
#define MPRIO_SharedPhoneNumbers RASEO_SharedPhoneNumbers
#define MPRIO_RequireCHAP RASEO_RequireCHAP
#define MPRIO_RequireMsCHAP RASEO_RequireMsCHAP
#define MPRIO_RequireMsCHAP2 RASEO_RequireMsCHAP2
#define MPRNP_Ipx RASNP_Ipx
#define MPRNP_Ip RASNP_Ip
#define MPRDT_Modem RASDT_Modem
#define MPRDT_Isdn RASDT_Isdn
#define MPRDT_X25 RASDT_X25
#define MPRDT_Vpn RASDT_Vpn
#define MPRDT_Pad RASDT_Pad
#define MPRDT_Generic RASDT_Generic
#define MPRDT_Serial RASDT_Serial        			
#define MPRDT_FrameRelay RASDT_FrameRelay
#define MPRDT_Atm RASDT_Atm
#define MPRDT_Sonet RASDT_Sonet
#define MPRDT_SW56 RASDT_SW56
#define MPRDT_Irda RASDT_Irda
#define MPRDT_Parallel RASDT_Parallel
#define MPRDM_DialAll RASEDM_DialAll
#define MPRDM_DialAsNeeded RASEDM_DialAsNeeded
#define MPRIDS_Disabled RASIDS_Disabled
#define MPRIDS_UseGlobalValue RASIDS_UseGlobalValue
#define MPRET_Phone RASET_Phone
#define MPRET_Vpn RASET_Vpn
#define MPRET_Direct RASET_Direct
#define MPR_ET_None ET_None         
#define MPR_ET_Require ET_Require      
#define MPR_ET_RequireMax ET_RequireMax   
#define MPR_ET_Optional ET_Optional     
#define MPR_VS_Default VS_Default		
#define MPR_VS_PptpOnly VS_PptpOnly	
#define MPR_VS_PptpFirst VS_PptpFirst	
#define MPR_VS_L2tpOnly VS_L2tpOnly 	
#define MPR_VS_L2tpFirst VS_L2tpFirst	

typedef struct _MPR_CREDENTIALSEX_0 {
	DWORD  dwSize;
	LPBYTE lpbCredentialsInfo;
} MPR_CREDENTIALSEX_0,*PMPR_CREDENTIALSEX_0;
#if (_WIN32_WINNT >= 0x0502)
typedef struct _MPR_CREDENTIALSEX_1 {
	DWORD  dwSize;
	LPBYTE lpbCredentialsInfo;
} MPR_CREDENTIALSEX_1,*PMPR_CREDENTIALSEX_1;
#endif
typedef struct _MPR_DEVICE_0 {
	WCHAR szDeviceType[MPR_MaxDeviceType+1];
	WCHAR szDeviceName[MPR_MaxDeviceName+1];
} MPR_DEVICE_0,*PMPR_DEVICE_0;
typedef struct _MPR_DEVICE_1 {
	WCHAR szDeviceType[MPR_MaxDeviceType+1];
	WCHAR szDeviceName[MPR_MaxDeviceName+1];
	WCHAR szLocalPhoneNumber[MPR_MaxPhoneNumber+1];
	PWCHAR szAlternates;
} MPR_DEVICE_1,*PMPR_DEVICE_1;
typedef struct _MPR_IFTRANSPORT_0 {
	DWORD dwTransportId;
	HANDLE hIfTransport;
	WCHAR wszIfTransportName[MAX_TRANSPORT_NAME_LEN+1];
} MPR_IFTRANSPORT_0,*PMPR_IFTRANSPORT_0;
typedef enum _ROUTER_INTERFACE_TYPE {
	ROUTER_IF_TYPE_CLIENT,
	ROUTER_IF_TYPE_HOME_ROUTER,
	ROUTER_IF_TYPE_FULL_ROUTER,
	ROUTER_IF_TYPE_DEDICATED,
	ROUTER_IF_TYPE_INTERNAL,
	ROUTER_IF_TYPE_LOOPBACK,
	ROUTER_IF_TYPE_TUNNEL1,
	ROUTER_IF_TYPE_DIALOUT
} ROUTER_INTERFACE_TYPE;
typedef enum _ROUTER_CONNECTION_STATE {
	ROUTER_IF_STATE_UNREACHABLE,
	ROUTER_IF_STATE_DISCONNECTED,
	ROUTER_IF_STATE_CONNECTING,
	ROUTER_IF_STATE_CONNECTED
} ROUTER_CONNECTION_STATE;
typedef struct _MPR_INTERFACE_0 {
	WCHAR wszInterfaceName[MAX_INTERFACE_NAME_LEN+1];
	HANDLE hInterface;
	BOOL fEnabled;
	ROUTER_INTERFACE_TYPE dwIfType;
	ROUTER_CONNECTION_STATE dwConnectionState;
	DWORD fUnReachabilityReasons;
	DWORD dwLastError;
} MPR_INTERFACE_0,*PMPR_INTERFACE_0;
typedef struct _MPR_INTERFACE_1 {
	WCHAR wszInterfaceName[MAX_INTERFACE_NAME_LEN+1];
	HANDLE hInterface;
	BOOL fEnabled;
	ROUTER_INTERFACE_TYPE IfType;
	ROUTER_CONNECTION_STATE dwConnectionState;
	DWORD fUnReachabilityReasons;
	DWORD dwLastError;
	LPWSTR lpwsDialoutHoursRestriction;
} MPR_INTERFACE_1,*PMPR_INTERFACE_1;
typedef struct _MPR_INTERFACE_2 {
	WCHAR wszInterfaceName[MAX_INTERFACE_NAME_LEN+1];
	HANDLE hInterface;
	BOOL fEnabled;
	ROUTER_INTERFACE_TYPE dwIfType;
	ROUTER_CONNECTION_STATE dwConnectionState;
	DWORD fUnReachabilityReasons;
	DWORD dwLastError;
	DWORD dwfOptions;
	WCHAR szLocalPhoneNumber[RAS_MaxPhoneNumber+1];
	PWCHAR szAlternates;
	DWORD ipaddr;
	DWORD ipaddrDns;
	DWORD ipaddrDnsAlt;
	DWORD ipaddrWins;
	DWORD ipaddrWinsAlt;
	DWORD dwfNetProtocols;
	WCHAR szDeviceType[MPR_MaxDeviceType+1];
	WCHAR szDeviceName[MPR_MaxDeviceName+1];
	WCHAR szX25PadType[MPR_MaxPadType+1];
	WCHAR szX25Address[MPR_MaxX25Address+1];
	WCHAR szX25Facilities[MPR_MaxFacilities+1];
	WCHAR szX25UserData[MPR_MaxUserData+1];
	DWORD dwChannels;
	DWORD dwSubEntries;
	DWORD dwDialMode;
	DWORD dwDialExtraPercent;
	DWORD dwDialExtraSampleSeconds;
	DWORD dwHangUpExtraPercent;
	DWORD dwHangUpExtraSampleSeconds;
	DWORD dwIdleDisconnectSeconds;
	DWORD dwType;
	DWORD dwEncryptionType;
	DWORD dwCustomAuthKey;
	DWORD dwCustomAuthDataSize;
	LPBYTE lpbCustomAuthData;
	GUID guidId;
	DWORD dwVpnStrategy;
} MPR_INTERFACE_2,*PMPR_INTERFACE_2;
typedef struct _MPR_INTERFACE_3 {
	WCHAR wszInterfaceName[MAX_INTERFACE_NAME_LEN+1];
	HANDLE hInterface;
	BOOL fEnabled;
	ROUTER_INTERFACE_TYPE dwIfType;
	ROUTER_CONNECTION_STATE dwConnectionState;
	DWORD fUnReachabilityReasons;
	DWORD dwLastError;
	DWORD dwfOptions;
	WCHAR szLocalPhoneNumber[RAS_MaxPhoneNumber+1];
	PWCHAR szAlternates;
	DWORD ipaddr;
	DWORD ipaddrDns;
	DWORD ipaddrDnsAlt;
	DWORD ipaddrWins;
	DWORD ipaddrWinsAlt;
	DWORD dwfNetProtocols;
	WCHAR szDeviceType[MPR_MaxDeviceType+1];
	WCHAR szDeviceName[MPR_MaxDeviceName+1];
	WCHAR szX25PadType[MPR_MaxPadType+1];
	WCHAR szX25Address[MPR_MaxX25Address+1];
	WCHAR szX25Facilities[MPR_MaxFacilities+1];
	WCHAR szX25UserData[MPR_MaxUserData+1];
	DWORD dwChannels;
	DWORD dwSubEntries;
	DWORD dwDialMode;
	DWORD dwDialExtraPercent;
	DWORD dwDialExtraSampleSeconds;
	DWORD dwHangUpExtraPercent;
	DWORD dwHangUpExtraSampleSeconds;
	DWORD dwIdleDisconnectSeconds;
	DWORD dwType;
	DWORD dwEncryptionType;
	DWORD dwCustomAuthKey;
	DWORD dwCustomAuthDataSize;
	LPBYTE lpbCustomAuthData;
	GUID guidId;
	DWORD dwVpnStrategy;
	ULONG AddressCount;
	IN6_ADDR ipv6addrDns;
	IN6_ADDR ipv6addrDnsAlt;
	IN6_ADDR* ipv6addr;
} MPR_INTERFACE_3,*PMPR_INTERFACE_3;
typedef struct _MPR_SERVER_0 {
	BOOL fLanOnlyMode;
	DWORD dwUpTime;
	DWORD dwTotalPorts;
	DWORD dwPortsInUse;
} MPR_SERVER_0,*PMPR_SERVER_0;
#if (_WIN32_WINNT >= 0x0502)
typedef struct _MPR_SERVER_1 {
	DWORD dwNumPptpPorts;
	DWORD dwPptpPortFlags;
	DWORD dwNumL2tpPorts;
	DWORD dwL2tpPortFlags;
} MPR_SERVER_1,*PMPR_SERVER_1;
#endif
typedef struct _MPR_TRANSPORT_0 {
	DWORD dwTransportId;
	HANDLE hTransport;
	WCHAR wszTransportName[MAX_TRANSPORT_NAME_LEN+1];
} MPR_TRANSPORT_0,*PMPR_TRANSPORT_0;
#endif

#ifdef __cplusplus
}
#endif
#endif
