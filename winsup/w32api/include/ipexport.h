#ifndef _IPEXPORT_H_
#define _IPEXPORT_H_

#ifdef __cplusplus
extern "C" {
#endif

#ifndef ANY_SIZE
#define ANY_SIZE 1
#endif

#define MAX_ADAPTER_NAME 128

typedef unsigned long IPAddr, IPMask, IP_STATUS;

typedef struct {
  ULONG Index;
  WCHAR  Name[MAX_ADAPTER_NAME];
} IP_ADAPTER_INDEX_MAP, *PIP_ADAPTER_INDEX_MAP;

typedef struct {
  LONG NumAdapters;
  IP_ADAPTER_INDEX_MAP Adapter[ANY_SIZE];
} IP_INTERFACE_INFO, *PIP_INTERFACE_INFO;

#ifdef __cplusplus
}
#endif

#endif /* _IPEXPORT_H_ */
