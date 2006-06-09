/* cguid-uuid.c */
/* Generate GUIDs for CGUID interfaces */

/* All IIDs defined in this file were extracted from
 * HKEY_LOCAL_MACHINE\SOFTWARE\Classes\Interface\ */

#define INITGUID
#include <basetyps.h>
DEFINE_GUID(GUID_NULL,0,0,0,0,0,0,0,0,0,0,0);
DEFINE_OLEGUID(IID_IRpcChannel,0x4,0,0);
DEFINE_OLEGUID(IID_IRpcStub,0x5,0,0);
DEFINE_OLEGUID(IID_IRpcProxy,0x7,0,0);
DEFINE_OLEGUID(IID_IPSFactory,0x9,0,0);
