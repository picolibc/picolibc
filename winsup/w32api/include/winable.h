#ifndef _WINABLE_H
#define _WINABLE_H
#if __GNUC__ >=3
#pragma GCC system_header
#endif

#ifdef __cplusplus
extern "C" {
#endif

#define OBJID_WINDOW 0x00000000
#define OBJID_SYSMENU 0xFFFFFFFF
#define OBJID_TITLEBAR 0xFFFFFFFE
#define OBJID_MENU 0xFFFFFFFD
#define OBJID_CLIENT 0xFFFFFFFC
#define OBJID_VSCROLL 0xFFFFFFFB
#define OBJID_HSCROLL 0xFFFFFFFA
#define OBJID_SIZEGRIP 0xFFFFFFF9
#define OBJID_CARET 0xFFFFFFF8
#define OBJID_CURSOR 0xFFFFFFF7
#define OBJID_ALERT 0xFFFFFFF6
#define OBJID_SOUND 0xFFFFFFF5

#ifdef __cplusplus
}
#endif
#endif /* _WINABLE_H */
