#ifndef _NTDEF_H_
#define _NTDEF_H_

#define NTAPI __stdcall

#define OBJ_INHERIT 2L
#define OBJ_PERMANENT 16L
#define OBJ_EXCLUSIVE 32L
#define OBJ_CASE_INSENSITIVE 64L
#define OBJ_OPENIF 128L
#define OBJ_OPENLINK 256L
#define OBJ_VALID_ATTRIBUTES 498L

#define InitializeObjectAttributes(p,n,a,r,s) { \
  (p)->Length = sizeof( OBJECT_ATTRIBUTES ); \
  (p)->RootDirectory = r; \
  (p)->Attributes = a; \
  (p)->ObjectName = n; \
  (p)->SecurityDescriptor = s; \
  (p)->SecurityQualityOfService = NULL; \
}

#define STATUS_SUCCESS ((NTSTATUS)0)
#define NT_SUCCESS(x) ((x)>=0)

typedef LONG NTSTATUS, *PNTSTATUS;
typedef LARGE_INTEGER PHYSICAL_ADDRESS, *PPHYSICAL_ADDRESS;
typedef struct _UNICODE_STRING {
  USHORT Length;
  USHORT MaximumLength;
  PWSTR  Buffer;
} UNICODE_STRING, *PUNICODE_STRING;
typedef enum _SECTION_INHERIT {
  ViewShare = 1,
  ViewUnmap = 2
} SECTION_INHERIT;
typedef struct _OBJECT_ATTRIBUTES {
  ULONG Length;
  HANDLE RootDirectory;
  PUNICODE_STRING ObjectName;
  ULONG Attributes;                      
  PVOID SecurityDescriptor;              
  PVOID SecurityQualityOfService;
} OBJECT_ATTRIBUTES, *POBJECT_ATTRIBUTES;

#endif /* _NTDEF_H_ */
