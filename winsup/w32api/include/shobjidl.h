#ifndef _SHLOBJIDL_H
#define _SHLOBJIDL_H
#if __GNUC__ >= 3
#pragma GCC system_header
#endif

#ifdef __cplusplus
extern "C" {
#endif

#include <ole2.h>
#include <shlguid.h>
#include <shellapi.h>
#pragma pack(push,1)
#include <commctrl.h>

extern const IID IID_ITaskbarList3;
extern const GUID CLSID_ITaskbarList;

/* http://msdn.microsoft.com/en-us/library/windows/desktop/dd562322%28v=vs.85%29.aspx */
typedef enum THUMBBUTTONMASK {
  THB_BITMAP    = 0x00000001,
  THB_ICON      = 0x00000002,
  THB_TOOLTIP   = 0x00000004,
  THB_FLAGS     = 0x00000008 
} THUMBBUTTONMASK;

/* http://msdn.microsoft.com/en-us/library/windows/desktop/dd562321%28v=vs.85%29.aspx */
typedef enum THUMBBUTTONFLAGS {
  THBF_ENABLED          = 0x00000000,
  THBF_DISABLED         = 0x00000001,
  THBF_DISMISSONCLICK   = 0x00000002,
  THBF_NOBACKGROUND     = 0x00000004,
  THBF_HIDDEN           = 0x00000008,
  THBF_NONINTERACTIVE   = 0x00000010 
} THUMBBUTTONFLAGS;

/* http://msdn.microsoft.com/en-us/library/windows/desktop/dd391559%28v=vs.85%29.aspx */
typedef struct THUMBBUTTON {
  THUMBBUTTONMASK  dwMask;
  UINT             iId;
  UINT             iBitmap;
  HICON            hIcon;
  WCHAR            szTip[260];
  THUMBBUTTONFLAGS dwFlags;
} THUMBBUTTON, *LPTHUMBBUTTON;

/* http://msdn.microsoft.com/en-us/library/windows/desktop/dd391697%28v=vs.85%29.aspx */
typedef enum TBPFLAG {
    TBPF_NOPROGRESS    = 0x00000000,
    TBPF_INDETERMINATE = 0x00000001,
    TBPF_NORMAL        = 0x00000002,
    TBPF_ERROR         = 0x00000004,
    TBPF_PAUSED        = 0x00000008
} TBPFLAG;

/* http://msdn.microsoft.com/en-us/library/windows/desktop/bb774652%28v=vs.85%29.aspx */
#define INTERFACE ITaskbarList
DECLARE_INTERFACE_(ITaskbarList, IUnknown)
{
	STDMETHOD(QueryInterface)(THIS_ REFIID,PVOID*) PURE;
	STDMETHOD_(ULONG,AddRef)(THIS) PURE;
	STDMETHOD_(ULONG,Release)(THIS) PURE;
/* http://msdn.microsoft.com/en-us/library/windows/desktop/bb774650%28v=vs.85%29.aspx */
	STDMETHOD(HrInit)(THIS) PURE;
/* http://msdn.microsoft.com/en-us/library/windows/desktop/bb774646%28v=vs.85%29.aspx */
	STDMETHOD(AddTab)(THIS_ HWND) PURE;
/* http://msdn.microsoft.com/en-us/library/windows/desktop/bb774648%28v=vs.85%29.aspx */
	STDMETHOD(DeleteTab)(THIS_ HWND) PURE;
/* http://msdn.microsoft.com/en-us/library/windows/desktop/bb774644%28v=vs.85%29.aspx */
	STDMETHOD(ActivateTab)(THIS_ HWND) PURE;
/* http://msdn.microsoft.com/en-us/library/windows/desktop/bb774655%28v=vs.85%29.aspx */
	STDMETHOD(SetActiveAlt)(THIS_ HWND) PURE;
};
#undef INTERFACE
typedef ITaskbarList *LPTASKBARLIST;

/* http://msdn.microsoft.com/en-us/library/windows/desktop/bb774638%28v=vs.85%29.aspx */
#define INTERFACE ITaskbarList2
DECLARE_INTERFACE_(ITaskbarList2, ITaskbarList)
{
	STDMETHOD(QueryInterface)(THIS_ REFIID,PVOID*) PURE;
	STDMETHOD_(ULONG,AddRef)(THIS) PURE;
	STDMETHOD_(ULONG,Release)(THIS) PURE;
	STDMETHOD(HrInit)(THIS) PURE;
	STDMETHOD(AddTab)(THIS_ HWND) PURE;
	STDMETHOD(DeleteTab)(THIS_ HWND) PURE;
	STDMETHOD(ActivateTab)(THIS_ HWND) PURE;
	STDMETHOD(SetActiveAlt)(THIS_ HWND) PURE;
/* http://msdn.microsoft.com/en-us/library/windows/desktop/bb774640%28v=vs.85%29.aspx */
	STDMETHOD(MarkFullscreenWindow)(THIS_ HWND,BOOL) PURE;
};
#undef INTERFACE
typedef ITaskbarList2 *LPTASKBARLIST2;

/* http://msdn.microsoft.com/en-us/library/windows/desktop/dd391692%28v=vs.85%29.aspx */
#define INTERFACE ITaskbarList3
DECLARE_INTERFACE_(ITaskbarList3, ITaskbarList2)
{
	STDMETHOD(QueryInterface)(THIS_ REFIID,PVOID*) PURE;
	STDMETHOD_(ULONG,AddRef)(THIS) PURE;
	STDMETHOD_(ULONG,Release)(THIS) PURE;
	STDMETHOD(HrInit)(THIS) PURE;
	STDMETHOD(AddTab)(THIS_ HWND) PURE;
	STDMETHOD(DeleteTab)(THIS_ HWND) PURE;
	STDMETHOD(ActivateTab)(THIS_ HWND) PURE;
	STDMETHOD(SetActiveAlt)(THIS_ HWND) PURE;
	STDMETHOD(MarkFullscreenWindow)(THIS_ HWND,BOOL) PURE;
/* http://msdn.microsoft.com/en-us/library/windows/desktop/dd391698%28v=vs.85%29.aspx */
	STDMETHOD(SetProgressValue)(THIS_ ULONGLONG,ULONGLONG) PURE;
/* http://msdn.microsoft.com/en-us/library/windows/desktop/dd391697%28v=vs.85%29.aspx */
	STDMETHOD(SetProgressState)(THIS_ HWND,TBPFLAG) PURE;
/* http://msdn.microsoft.com/en-us/library/windows/desktop/dd391694%28v=vs.85%29.aspx */
	STDMETHOD(RegisterTab)(THIS_ HWND,HWND) PURE;
/* http://msdn.microsoft.com/en-us/library/windows/desktop/dd391706%28v=vs.85%29.aspx */
	STDMETHOD(UnregisterTab)(THIS_ HWND) PURE;
/* http://msdn.microsoft.com/en-us/library/windows/desktop/dd391700%28v=vs.85%29.aspx */
	STDMETHOD(SetTabOrder)(THIS_ HWND,HWND) PURE;
/* http://msdn.microsoft.com/en-us/library/windows/desktop/dd391699%28v=vs.85%29.aspx */
	STDMETHOD(SetTabActive)(THIS_ HWND,HWND,DWORD) PURE;
/* http://msdn.microsoft.com/en-us/library/windows/desktop/dd391703%28v=vs.85%29.aspx */
	STDMETHOD(ThumbBarAddButtons)(THIS_ HWND,UINT,LPTHUMBBUTTON) PURE;
/* http://msdn.microsoft.com/en-us/library/windows/desktop/dd391705%28v=vs.85%29.aspx */
	STDMETHOD(ThumbBarUpdateButtons)(THIS_ HWND,UINT,LPTHUMBBUTTON) PURE;
/* http://msdn.microsoft.com/en-us/library/windows/desktop/dd391704%28v=vs.85%29.aspx */
	STDMETHOD(ThumbBarSetImageList)(THIS_ HWND,HIMAGELIST) PURE;
/* http://msdn.microsoft.com/en-us/library/windows/desktop/dd391696%28v=vs.85%29.aspx */
	STDMETHOD(SetOverlayIcon)(THIS_ HWND,HICON,LPCWSTR) PURE;
/* http://msdn.microsoft.com/en-us/library/windows/desktop/dd391702%28v=vs.85%29.aspx */
	STDMETHOD(SetThumbnailTooltip)(THIS_ HWND,LPCWSTR) PURE;
/* http://msdn.microsoft.com/en-us/library/windows/desktop/dd391701%28v=vs.85%29.aspx */
	STDMETHOD(SetThumbnailClip)(THIS_ HWND,RECT*) PURE;
};
#undef INTERFACE
typedef ITaskbarList3 *LPTASKBARLIST3;

#pragma pack(pop)
#ifdef __cplusplus
}
#endif


#endif /* _SHLOBJIDL_H */
