/* cyglsa.h: Header file for Cygwin LSA authentication

   Copyright 2006, 2008 Red Hat, Inc.

   Written by Corinna Vinschen <corinna@vinschen.de>

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for details. */

#ifndef _CYGLSA_H
#define _CYGLSA_H

#ifdef __cplusplus
extern "C" {
#endif

#define CYG_LSA_PKGNAME "CygwinLsa"

#define CYG_LSA_MAGIC_OLD1 0x0379f014LU
/* First change to cyglsa_t.
   - Username and domain are now of type WCHAR instead of char.
   - domain is MAX_DOMAIN_NAME_LEN instead of INTERNET_MAX_HOST_NAME_LENGTH. */
#define CYG_LSA_MAGIC 0x0379f115LU

/* Datastructures not defined in w32api. */
typedef PVOID *PLSA_CLIENT_REQUEST;

typedef UNICODE_STRING SECURITY_STRING, *PSECURITY_STRING;

typedef struct _SECPKG_CLIENT_INFO
{
  LUID LogonId;
  ULONG ProcessID;
  ULONG ThreadID;
  BOOLEAN HasTcbPrivilege;
  BOOLEAN Impersonating;
  BOOLEAN Restricted;
} SECPKG_CLIENT_INFO, *PSECPKG_CLIENT_INFO;

typedef enum _SECPKG_NAME_TYPE
{
  SecNameSamCompatible,
  SecNameAlternateId,
  SecNameFlat,
  SecNameDN,
  SecNameSPN
} SECPKG_NAME_TYPE, *PSECPKG_NAME_TYPE;

typedef struct _SECPKG_CALL_INFO
{
  ULONG ProcessId;
  ULONG ThreadId;
  ULONG Attributes;
  ULONG CallCount;
} SECPKG_CALL_INFO, *PSECPKG_CALL_INFO;

/* The table returned by LsaApInitializePackage is actually a
   LSA_SECPKG_FUNCTION_TABLE even though that's not documented.
   We need only a subset of this table, basically the LSA_DISPATCH_TABLE
   plus the pointer to the GetClientInfo function. */
typedef struct _LSA_SECPKG_FUNCS
{
  NTSTATUS (NTAPI *CreateLogonSession)(PLUID);
  NTSTATUS (NTAPI *DeleteLogonSession)(PLUID);
  NTSTATUS (NTAPI *AddCredentials)(PLUID, ULONG, PLSA_STRING, PLSA_STRING);
  NTSTATUS (NTAPI *GetCredentials)(PVOID); /* wrong prototype, unused */
  NTSTATUS (NTAPI *DeleteCredentials)(PVOID); /* wrong prototype, unused */
  PVOID (NTAPI *AllocateLsaHeap)(ULONG);
  VOID (NTAPI *FreeLsaHeap)(PVOID);
  NTSTATUS (NTAPI *AllocateClientBuffer)(PLSA_CLIENT_REQUEST, ULONG, PVOID *);
  NTSTATUS (NTAPI *FreeClientBuffer)(PLSA_CLIENT_REQUEST, PVOID);
  NTSTATUS (NTAPI *CopyToClientBuffer)(PLSA_CLIENT_REQUEST, ULONG,
				       PVOID, PVOID);
  NTSTATUS (NTAPI *CopyFromClientBuffer)(PLSA_CLIENT_REQUEST, ULONG,
					 PVOID, PVOID);
  NTSTATUS (NTAPI *ImpersonateClient)(VOID);
  NTSTATUS (NTAPI *UnloadPackage)(VOID);
  NTSTATUS (NTAPI *DuplicateHandle)(HANDLE, PHANDLE);
  NTSTATUS (NTAPI *SaveSupplementalCredentials)(VOID);
  NTSTATUS (NTAPI *CreateThread)(PVOID); /* wrong prototype, unused */
  NTSTATUS (NTAPI *GetClientInfo)(PSECPKG_CLIENT_INFO);
  NTSTATUS (NTAPI *RegisterNotification)(PVOID); /* wrong prototype, unused */
  NTSTATUS (NTAPI *CancelNotification)(PVOID); /* wrong prototype, unused */
  NTSTATUS (NTAPI *MapBuffer)(PVOID); /* wrong prototype, unused */
  NTSTATUS (NTAPI *CreateToken)(PVOID); /* wrong prototype, unused */
  NTSTATUS (NTAPI *AuditLogon)(PVOID); /* wrong prototype, unused */
  NTSTATUS (NTAPI *CallPackage)(PVOID); /* wrong prototype, unused */
  NTSTATUS (NTAPI *FreeReturnBuffer)(PVOID); /* wrong prototype, unused */
  BOOLEAN  (NTAPI *GetCallInfo)(PSECPKG_CALL_INFO);
  NTSTATUS (NTAPI *CallPackageEx)(PVOID); /* wrong prototype, unused */
  NTSTATUS (NTAPI *CreateSharedMemory)(PVOID); /* wrong prototype, unused */
  NTSTATUS (NTAPI *AllocateSharedMemory)(PVOID); /* wrong prototype, unused */
  NTSTATUS (NTAPI *FreeSharedMemory)(PVOID); /* wrong prototype, unused */
  NTSTATUS (NTAPI *DeleteSharedMemory)(PVOID); /* wrong prototype, unused */
  NTSTATUS (NTAPI *OpenSamUser)(PSECURITY_STRING, SECPKG_NAME_TYPE,
				PSECURITY_STRING, BOOLEAN, ULONG, PVOID *);
  NTSTATUS (NTAPI *GetUserCredentials)(PVOID, PVOID, PULONG, PVOID *, PULONG);
  NTSTATUS (NTAPI *GetUserAuthData)(PVOID, PUCHAR *, PULONG);
  NTSTATUS (NTAPI *CloseSamUser)(PVOID);
  NTSTATUS (NTAPI *ConvertAuthDataToToken)(PVOID, ULONG,
					   SECURITY_IMPERSONATION_LEVEL,
					   PTOKEN_SOURCE, SECURITY_LOGON_TYPE,
					   PUNICODE_STRING, PHANDLE, PLUID,
					   PUNICODE_STRING, PNTSTATUS);
  NTSTATUS (NTAPI *ClientCallback)(PVOID); /* wrong prototype, unused */
  NTSTATUS (NTAPI *UpdateCredentials)(PVOID); /* wrong prototype, unused */
  NTSTATUS (NTAPI *GetAuthDataForUser)(PSECURITY_STRING, SECPKG_NAME_TYPE,
				       PSECURITY_STRING, PUCHAR *, PULONG,
				       PUNICODE_STRING);
  NTSTATUS (NTAPI *CrackSingleName)(PVOID); /* wrong prototype, unused */
  NTSTATUS (NTAPI *AuditAccountLogon)(PVOID); /* wrong prototype, unused */
  NTSTATUS (NTAPI *CallPackagePassthrough)(PVOID); /* wrong prototype, unused */
} LSA_SECPKG_FUNCS, *PLSA_SECPKG_FUNCS;

typedef enum _LSA_TOKEN_INFORMATION_TYPE
{
  LsaTokenInformationNull,
  LsaTokenInformationV1,
  LsaTokenInformationV2
} LSA_TOKEN_INFORMATION_TYPE, *PLSA_TOKEN_INFORMATION_TYPE;

typedef struct _LSA_TOKEN_INFORMATION_V2
{
  LARGE_INTEGER ExpirationTime;
  TOKEN_USER User;
  PTOKEN_GROUPS Groups;
  TOKEN_PRIMARY_GROUP PrimaryGroup;
  PTOKEN_PRIVILEGES Privileges;
  TOKEN_OWNER Owner;
  TOKEN_DEFAULT_DACL DefaultDacl;
} LSA_TOKEN_INFORMATION_V2, *PLSA_TOKEN_INFORMATION_V2;

/* These structures are eqivalent to the appropriate Windows structures,
   using 32 bit offsets instead of pointers.  These datastructures are
   used to transfer the logon information to the LSA authentication package.
   We can't use the LSA_TOKEN_INFORMATION_V2 structure directly, because
   its size differs between 32 bit and 64 bit Windows. */

typedef DWORD OFFSET;

typedef struct _CYG_SID_AND_ATTRIBUTES
{
  OFFSET Sid;
  DWORD Attributes;
} CYG_SID_AND_ATTRIBUTES, *PCYG_SID_AND_ATTRIBUTES;

typedef struct _CYG_TOKEN_USER
{
  CYG_SID_AND_ATTRIBUTES User;
} CYG_TOKEN_USER, *PCYG_TOKEN_USER;

typedef struct _CYG_TOKEN_GROUPS
{
  DWORD GroupCount;
  CYG_SID_AND_ATTRIBUTES Groups[ANYSIZE_ARRAY];
} CYG_TOKEN_GROUPS, *PCYG_TOKEN_GROUPS;

typedef struct _CYG_TOKEN_PRIMARY_GROUP
{
  OFFSET PrimaryGroup;
} CYG_TOKEN_PRIMARY_GROUP, *PCYG_TOKEN_PRIMARY_GROUP;

typedef struct _CYG_TOKEN_OWNER
{
  OFFSET Owner;
} CYG_TOKEN_OWNER, *PCYG_TOKEN_OWNER;

typedef struct _CYG_TOKEN_DEFAULT_DACL
{
  OFFSET DefaultDacl;
} CYG_TOKEN_DEFAULT_DACL, *PCYG_TOKEN_DEFAULT_DACL;

typedef struct _CYG_LSA_TOKEN_INFORMATION
{
  LARGE_INTEGER ExpirationTime;
  CYG_TOKEN_USER User;
  OFFSET Groups;
  CYG_TOKEN_PRIMARY_GROUP PrimaryGroup;
  OFFSET Privileges;
  CYG_TOKEN_OWNER Owner;
  CYG_TOKEN_DEFAULT_DACL DefaultDacl;
} CYG_LSA_TOKEN_INFORMATION, *PCYG_LSA_TOKEN_INFORMATION;

/* This is the structure created by security.cc:lsaauth(), which is given to
   LsaApLogonUser to create the token information returned to the LSA. */
typedef struct
{
  DWORD magic;
  DWORD checksum;
  WCHAR username[UNLEN + 1];
  WCHAR domain[MAX_DOMAIN_NAME_LEN + 1];
  ULONG inf_size;
  CYG_LSA_TOKEN_INFORMATION inf;
  BYTE data[1];
} cyglsa_t;

typedef struct
{
  DWORD magic_pre;
  HANDLE token;
  DWORD magic_post;
} cygprf_t;

#define MAGIC_PRE  0x12345678UL
#define MAGIC_POST 0x87654321UL

#ifdef __cplusplus
}
#endif

#endif /* _CYGLSA_H */
