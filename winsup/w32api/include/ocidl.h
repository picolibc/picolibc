#ifndef _OCIDL_H
#define _OCIDL_H
#ifdef __cplusplus
extern "C" {
#endif

#include <ole2.h>
#include <olectl.h>

typedef enum tagREADYSTATE {
	READYSTATE_UNINITIALIZED = 0,
	READYSTATE_LOADING = 1,
	READYSTATE_LOADED = 2,
	READYSTATE_INTERACTIVE = 3,
	READYSTATE_COMPLETE = 4
} READYSTATE;

EXTERN_C const IID IID_IOleInPlaceSiteEx;
#undef INTERFACE
#define INTERFACE IOleInPlaceSiteEx
DECLARE_INTERFACE_(IOleInPlaceSiteEx,IOleInPlaceSite)
{
	STDMETHOD(QueryInterface)(THIS_ REFIID,PVOID*) PURE;
	STDMETHOD_(ULONG,AddRef)(THIS) PURE;
	STDMETHOD_(ULONG,Release)(THIS) PURE;
	STDMETHOD(GetWindow)(THIS_ HWND*) PURE;
	STDMETHOD(ContextSensitiveHelp)(THIS_ BOOL) PURE;
	STDMETHOD(CanInPlaceActivate)(THIS) PURE;
	STDMETHOD(OnInPlaceActivate)(THIS) PURE;
	STDMETHOD(OnUIActivate)(THIS) PURE;
	STDMETHOD(GetWindowContext)(THIS_ IOleInPlaceFrame**,IOleInPlaceUIWindow**,LPRECT,LPRECT,LPOLEINPLACEFRAMEINFO) PURE;
	STDMETHOD(Scroll)(THIS_ SIZE) PURE;
	STDMETHOD(OnUIDeactivate)(THIS_ BOOL) PURE;
	STDMETHOD(OnInPlaceDeactivate)(THIS) PURE;
	STDMETHOD(DiscardUndoState)(THIS) PURE;
	STDMETHOD(DeactivateAndUndo)(THIS) PURE;
	STDMETHOD(OnPosRectChange)(THIS_ LPCRECT) PURE;

	STDMETHOD(OnInPlaceActivateEx)(THIS_ BOOL*,DWORD) PURE;
	STDMETHOD(OnInPlaceDeactivateEx)(THIS_ BOOL) PURE;
	STDMETHOD(RequestUIActivate)(THIS) PURE;
};

#ifdef __cplusplus
}
#endif
#endif
