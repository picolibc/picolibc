#ifndef _RAS_H_
#define _RAS_H_

#ifdef __cplusplus
extern "C" {
#endif

#define RAS_MaxDeviceType 16
#define RAS_MaxDeviceName 128

#define RASDT_Modem TEXT("modem")
#define RASDT_Isdn TEXT("isdn")
#define RASDT_X25 TEXT("x25")
#define RASDT_Vpn TEXT("vpn")
#define RASDT_Pad TEXT("pad")

typedef struct tagRASDEVINFOA {
  DWORD dwSize;
  CHAR szDeviceType[RAS_MaxDeviceType+1];
  CHAR szDeviceName[RAS_MaxDeviceName+1];
};
#define RASDEVINFOA struct tagRASDEVINFOA
#define LPRASDEVINFOA RASDEVINFOA*

typedef struct tagRASDEVINFOW {
  DWORD dwSize;
  WCHAR szDeviceType[RAS_MaxDeviceType+1];
  WCHAR szDeviceName[RAS_MaxDeviceName+1];
};
#define RASDEVINFOW struct tagRASDEVINFOW
#define LPRASDEVINFOW RASDEVINFOW*

DWORD WINAPI RasEnumDevicesA(LPRASDEVINFOA,LPDWORD,LPDWORD);
DWORD WINAPI RasEnumDevicesW(LPRASDEVINFOW,LPDWORD,LPDWORD);

#ifdef UNICODE
#define RASDEVINFO RASDEVINFOW
#define RasEnumDevices RasEnumDevicesW
#else
#define RASDEVINFO RASDEVINFOA
#define RasEnumDevices RasEnumDevicesA
#endif

#define LPRASDEVINFO  RASDEVINFO*

#ifdef __cplusplus
}
#endif

#endif /* _RAS_H_ */
