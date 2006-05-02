#ifndef _STM_H
#define _STM_H
#if __GNUC__ >= 3
#pragma GCC system_header
#endif

#ifdef __cplusplus
extern "C" {
#endif

#if (_WIN32_WINNT >= 0x0500)
#define ADMIN_STATE_DISABLED 0x00000001
#define ADMIN_STATE_ENABLED 0x00000002
#define ADMIN_STATE_ENABLED_ONLY_FOR_NETBIOS_STATIC_ROUTING 0x00000003
#define ADMIN_STATE_ENABLED_ONLY_FOR_OPER_STATE_UP 0x00000004
#endif

#ifdef __cplusplus
}
#endif
#endif
