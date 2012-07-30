/**
    sdkddkver.h - Versioning file for Windows SDK/DDK.

    This file is part of a free library for the Windows API.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
*/
#ifndef _SDKDDKVER_H
#define _SDKDDKVER_H

/**
 * Define version masks
 * http://msdn.microsoft.com/en-us/library/windows/desktop/aa383745(v=vs.85).aspx
 * Values and names are guessed based on comments in the documentation.
 */
#define OSVERSION_MASK            0xFFFF0000
#define SPVERSION_MASK            0x0000FF00
#define SUBVERSION_MASK           0x000000FF

/**
 * Macros to extract values from NTDDI version.
 * Derived from comments on MSDN or social.microsoft.com
 */
#define OSVER(ver) ((ver) & OSVERSION_MASK)
#define SPVER(ver) (((ver) & SPVERSION_MASK) >> 8)
#define SUBVER(ver) ((ver) & SUBVERSION_MASK)

/**
 * Macros to create the minimal NTDDI version from _WIN32_WINNT value.
 */
#define NTDDI_VERSION_FROM_WIN32_WINNT(ver) _NTDDI_VERSION_FROM_WIN32_WINNT(ver)
#define _NTDDI_VERSION_FROM_WIN32_WINNT(ver) ver##0000

/**
 * Version constants defining _WIN32_WINNT versions.
 * http://msdn.microsoft.com/en-us/library/windows/desktop/aa383745(v=vs.85).aspx
 */
#define _WIN32_WINNT_NT4          0x0400
#define _WIN32_WINNT_WIN2K        0x0500
#define _WIN32_WINNT_WINXP        0x0501
#define _WIN32_WINNT_WS03         0x0502
#define _WIN32_WINNT_WIN6         0x0600
#define _WIN32_WINNT_VISTA        0x0600
#define _WIN32_WINNT_WS08         0x0600
#define _WIN32_WINNT_LONGORN      0x0600
#define _WIN32_WINNT_WIN7         0x0601

/**
 * Version constants defining _WIN32_IE versions.
 * http://msdn.microsoft.com/en-us/library/windows/desktop/aa383745(v=vs.85).aspx
 */
#define _WIN32_IE_IE50            0x0500
#define _WIN32_IE_IE501           0x0501
#define _WIN32_IE_IE55            0x0550
#define _WIN32_IE_IE60            0x0600
#define _WIN32_IE_IE60SP1         0x0601
#define _WIN32_IE_IE60SP2         0x0603
#define _WIN32_IE_IE70            0x0700
#define _WIN32_IE_IE80            0x0800

/**
 * Version constants defining NTDDI_VERSION.
 * http://msdn.microsoft.com/en-us/library/windows/desktop/aa383745(v=vs.85).aspx
 */
#define __NTDDI_WIN5               0x05000000
#define __NTDDI_WIN51              0x05010000
#define __NTDDI_WIN52              0x05020000
#define __NTDDI_WIN6               0x06000000
#define __NTDDI_WIN61              0x06010000
#define __NTDDI_SP0                0x00000000
#define __NTDDI_SP1                0x00000100
#define __NTDDI_SP2                0x00000200
#define __NTDDI_SP3                0x00000300
#define __NTDDI_SP4                0x00000400

#define NTDDI_WIN2K               __NTDDI_WIN5 + __NTDDI_SP0
#define NTDDI_WIN2KSP1            __NTDDI_WIN5 + __NTDDI_SP1
#define NTDDI_WIN2KSP2            __NTDDI_WIN5 + __NTDDI_SP2
#define NTDDI_WIN2KSP3            __NTDDI_WIN5 + __NTDDI_SP3
#define NTDDI_WIN2KSP4            __NTDDI_WIN5 + __NTDDI_SP4

#define NTDDI_WINXP               __NTDDI_WIN51 + __NTDDI_SP0
#define NTDDI_WINXPSP1            __NTDDI_WIN51 + __NTDDI_SP1
#define NTDDI_WINXPSP2            __NTDDI_WIN51 + __NTDDI_SP2
#define NTDDI_WINXPSP3            __NTDDI_WIN51 + __NTDDI_SP3

#define NTDDI_WS03                __NTDDI_WIN52 + __NTDDI_SP0
#define NTDDI_WS03SP1             __NTDDI_WIN52 + __NTDDI_SP1
#define NTDDI_WS03SP2             __NTDDI_WIN52 + __NTDDI_SP2

#define NTDDI_VISTA               __NTDDI_WIN6 + __NTDDI_SP0
#define NTDDI_VISTASP1            __NTDDI_WIN6 + __NTDDI_SP1
#define NTDDI_VISTASP2		  __NTDDI_WIN6 + __NTDDI_SP2

#define NTDDI_LONGHORN            NTDDI_VISTA

#define NTDDI_WIN6                NTDDI_VISTA
#define NTDDI_WIN6SP1             NTDDI_VISTASP1
#define NTDDI_WIN6SP2		  NTDDI_VISTASP2

#define NTDDI_WS08                __NTDDI_WIN6 + __NTDDI_SP1

#define NTDDI_WIN7                __NTDDI_WIN61 + __NTDDI_SP0

/**
 * Assign defaults
 */
#ifdef NTDDI_VERSION
#  ifdef _WIN32_WINNT
#    if _WIN32_WINNT != OSDIR(NTDDI_VERSION)
#      error The _WIN32_WINNT value does not match NTDDI_VERSION
#    endif
#  else
#    define _WIN32_WINNT OSVER(NTDDI_VERSION)
#    ifndef WINVER
#      define WINVER _WIN32_WINNT
#    endif
#  endif
#endif

#ifndef _WIN32_WINNT
#  ifdef WINVER
#    define _WIN32_WINNT WINVER
#  else
#    warning _WIN32_WINNT is defaulting to _WIN32_WINNT_WIN2K
#    define _WIN32_WINNT _WIN32_WINNT_WIN2K
#  endif
#endif

#ifndef WINVER
#  define WINVER _WIN32_WINNT
#endif

#ifndef NTDDI_VERSION
#  warning NTDDI_VERSION is defaulting to _WIN32_WINNT version SPK0
#  define NTDDI_VERSION NTDDI_VERSION_FROM_WIN32_WINNT(_WIN32_WINNT)
#endif

#endif
