#ifndef _SHLDISP_H
#define _SHLDISP_H
#if __GNUC__ >= 3
#pragma GCC system_header
#endif

#ifdef __cplusplus
extern "C" {
#endif

typedef enum tagAUTOCOMPLETEOPTIONS {
    ACO_NONE = 0x00,
    ACO_AUTOSUGGEST = 0x01,
    ACO_AUTOAPPEND = 0x02,
    ACO_SEARCH = 0x04,
    ACO_FILTERPREFIXES = 0x08,
    ACO_USETAB = 0x10,
    ACO_UPDOWNKEYDROPSLIST = 0x20,
    ACO_RTLREADING = 0x40,
#if (_WIN32_WINNT >= 0x0600)
    ACO_WORD_FILTER = 0x80,
    ACO_NOPREFIXFILTERING = 0x100
#endif
} AUTOCOMPLETEOPTIONS;

#define INTERFACE IAutoComplete
DECLARE_INTERFACE_(IAutoComplete, IUnknown)
{
	STDMETHOD(QueryInterface)(THIS_ REFIID,PVOID*) PURE;
	STDMETHOD_(ULONG,AddRef)(THIS) PURE;
	STDMETHOD_(ULONG,Release)(THIS) PURE;
	STDMETHOD(Init)(THIS_ HWND,IUnknown*,LPCOLESTR,LPCOLESTR) PURE;
	STDMETHOD(Enable)(THIS_ BOOL) PURE;
};
#undef INTERFACE
typedef IAutoComplete *LPAUTOCOMPLETE;

#ifdef COBJMACROS
#define IAutoComplete_QueryInterface(T,a,b) (T)->lpVtbl->QueryInterface(T,a,b)
#define IAutoComplete_AddRef(T) (T)->lpVtbl->AddRef(T)
#define IAutoComplete_Release(T) (T)->lpVtbl->Release(T)
#define IAutoComplete_Init(T,a,b,c,d) (T)->lpVtbl->Init(T,a,b,c,d)
#define IAutoComplete_Enable(T,a) (T)->lpVtbl->Enable(T,a)
#endif

#define INTERFACE IAutoComplete2
DECLARE_INTERFACE_(IAutoComplete2, IAutoComplete)
{
	STDMETHOD(QueryInterface)(THIS_ REFIID,PVOID*) PURE;
	STDMETHOD_(ULONG,AddRef)(THIS) PURE;
	STDMETHOD_(ULONG,Release)(THIS) PURE;
	STDMETHOD(Init)(THIS_ HWND,IUnknown*,LPCOLESTR,LPCOLESTR) PURE;
	STDMETHOD(Enable)(THIS_ BOOL) PURE;
	STDMETHOD(SetOptions)(THIS_ DWORD) PURE;
	STDMETHOD(GetOptions)(THIS_ DWORD*) PURE;
};
#undef INTERFACE
typedef IAutoComplete2 *LPAUTOCOMPLETE2;

#ifdef COBJMACROS
#define IAutoComplete2_QueryInterface(T,a,b) (T)->lpVtbl->QueryInterface(T,a,b)
#define IAutoComplete2_AddRef(T) (T)->lpVtbl->AddRef(T)
#define IAutoComplete2_Release(T) (T)->lpVtbl->Release(T)
#define IAutoComplete2_Init(T,a,b,c,d) (T)->lpVtbl->Init(T,a,b,c,d)
#define IAutoComplete2_Enable(T,a) (T)->lpVtbl->Enable(T,a)
#define IAutoComplete2_SetOptions(T,a) (T)->lpVtbl->Enable(T,a)
#define IAutoComplete2_GetOptions(T,a) (T)->lpVtbl->Enable(T,a)
#endif

#ifdef __cplusplus
}
#endif

#endif /* _SHLDISP_H */
