#ifndef _OLEACC_H
#define _OLEACC_H

#define DISPID_ACC_PARENT                   (-5000)
#define DISPID_ACC_CHILDCOUNT               (-5001)
#define DISPID_ACC_CHILD                    (-5002)

#define DISPID_ACC_NAME                     (-5003)
#define DISPID_ACC_VALUE                    (-5004)
#define DISPID_ACC_DESCRIPTION              (-5005)
#define DISPID_ACC_ROLE                     (-5006)
#define DISPID_ACC_STATE                    (-5007)
#define DISPID_ACC_HELP                     (-5008)
#define DISPID_ACC_HELPTOPIC                (-5009)
#define DISPID_ACC_KEYBOARDSHORTCUT         (-5010)
#define DISPID_ACC_FOCUS                    (-5011)
#define DISPID_ACC_SELECTION                (-5012)
#define DISPID_ACC_DEFAULTACTION            (-5013)

#define DISPID_ACC_SELECT                   (-5014)
#define DISPID_ACC_LOCATION                 (-5015)
#define DISPID_ACC_NAVIGATE                 (-5016)
#define DISPID_ACC_HITTEST                  (-5017)
#define DISPID_ACC_DODEFAULTACTION          (-5018)

#define SELFLAG_NONE                    0x00000000
#define SELFLAG_TAKEFOCUS               0x00000001
#define SELFLAG_TAKESELECTION           0x00000002
#define SELFLAG_EXTENDSELECTION         0x00000004
#define SELFLAG_ADDSELECTION            0x00000008
#define SELFLAG_REMOVESELECTION         0x00000010
#define SELFLAG_VALID                   0x0000001F

/* DEFINE_GUID(LIBID_Accessibility, 0x1ea4dbf0, 0x3c3b,0x11cf, 0x81, 0x0c, 0x00, 0xaa, 0x00, 0x38, 0x9b, 0x71); */
/* DEFINE_GUID(IID_IAccessible,     0x618736e0, 0x3c3d,0x11cf, 0x81, 0x0c, 0x00, 0xaa, 0x00, 0x38, 0x9b, 0x71); */
EXTERN_C const IID IID_IAccessible;
#undef INTERFACE
#define INTERFACE IAccessible
DECLARE_INTERFACE_(IAccessible, IDispatch)
{
    STDMETHOD(QueryInterface)(THIS_ REFIID,PVOID*) PURE;
    STDMETHOD_(ULONG,AddRef)(THIS) PURE;
    STDMETHOD_(ULONG,Release)(THIS) PURE;
    STDMETHOD(GetTypeInfoCount)(THIS_ UINT*) PURE;
    STDMETHOD(GetTypeInfo)(THIS_ UINT,LCID,LPTYPEINFO*) PURE;
    STDMETHOD(GetIDsOfNames)(THIS_ REFIID,LPOLESTR*,UINT,LCID,DISPID*) PURE;
    STDMETHOD(Invoke)(THIS_ DISPID,REFIID,LCID,WORD,DISPPARAMS*,VARIANT*,EXCEPINFO*,UINT*) PURE;

    STDMETHOD(get_accParent)(THIS_ IDispatch**) PURE;
    STDMETHOD(get_accChildCount)(THIS_ long*) PURE;
    STDMETHOD(get_accChild)(THIS_ VARIANT, IDispatch **) PURE;
    STDMETHOD(get_accName)(THIS_ VARIANT, BSTR*) PURE;
    STDMETHOD(get_accValue)(THIS_ VARIANT, BSTR*) PURE;
    STDMETHOD(get_accDescription)(THIS_ VARIANT, BSTR*) PURE;
    STDMETHOD(get_accRole)(THIS_ VARIANT, VARIANT*) PURE;
    STDMETHOD(get_accState)(THIS_ VARIANT, VARIANT*) PURE;
    STDMETHOD(get_accHelp)(THIS_ VARIANT, BSTR*) PURE;
    STDMETHOD(get_accHelpTopic)(THIS_ BSTR*, VARIANT, long*) PURE;
    STDMETHOD(get_accKeyboardShortcut)(THIS_ VARIANT, BSTR*) PURE;
    STDMETHOD(get_accFocus)(THIS_ VARIANT*) PURE;
    STDMETHOD(get_accSelection)(THIS_ VARIANT*) PURE;
    STDMETHOD(get_accDefaultAction)(THIS_ VARIANT, BSTR*) PURE;

    STDMETHOD(accSelect)(THIS_ long, VARIANT) PURE;
    STDMETHOD(accLocation)(THIS_ long*, long*, long*, long*, VARIANT) PURE;
    STDMETHOD(accNavigate)(THIS_ long, VARIANT, VARIANT*) PURE;
    STDMETHOD(accHitTest)(THIS_ long, long, VARIANT*) PURE;
    STDMETHOD(accDoDefaultAction)(THIS_ VARIANT) PURE;

    STDMETHOD(put_accName)(THIS_ VARIANT, BSTR) PURE;
    STDMETHOD(put_accValue)(THIS_ VARIANT, BSTR) PURE;
};
typedef IAccessible* LPACCESSIBLE;


#endif /* _OLEACC_H */
