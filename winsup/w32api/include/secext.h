#ifndef _SECEXT_H
#define _SECEXT_H
#if __GNUC__ >= 3
#pragma GCC system_header
#endif

#ifndef RC_INVOKED
#if (_WIN32_WINNT >= 0x0500)
typedef enum 
{
  NameUnknown = 0, 
  NameFullyQualifiedDN = 1, 
  NameSamCompatible = 2, 
  NameDisplay = 3, 
  NameUniqueId = 6, 
  NameCanonical = 7, 
  NameUserPrincipal = 8, 
  NameCanonicalEx = 9, 
  NameServicePrincipal = 10, 
  NameDnsDomain = 12
} EXTENDED_NAME_FORMAT, *PEXTENDED_NAME_FORMAT;
#endif /* ! RC_INVOKED */
#endif /* _WIN32_WINNT >= 0x0500 */
#endif /* ! _SECEXT_H */
