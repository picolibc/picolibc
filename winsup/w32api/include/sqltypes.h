#ifndef _SQLTYPES_H
#define _SQLTYPES_H
#ifdef __cplusplus
extern "C" {
#endif
#define SQL_API __stdcall
#pragma pack(push,1)
typedef signed char SCHAR;
typedef long SDWORD;
typedef short SWORD;
typedef ULONG UDWORD;
typedef USHORT UWORD;
typedef long SLONG;
typedef short SSHORT;
typedef double SDOUBLE;
typedef double LDOUBLE;
typedef float SFLOAT;
typedef PVOID PTR;
typedef PVOID HENV;
typedef PVOID HDBC;
typedef PVOID HSTMT;
typedef short RETCODE;
typedef UCHAR SQLCHAR;
typedef SCHAR SQLSCHAR;
typedef SDWORD SQLINTEGER;
typedef SWORD SQLSMALLINT;
typedef UDWORD SQLUINTEGER;
typedef UWORD SQLUSMALLINT;
typedef PVOID SQLPOINTER;
typedef HENV SQLHENV;
typedef HDBC SQLHDBC;
typedef HSTMT SQLHSTMT;
typedef SQLSMALLINT SQLRETURN;
typedef HWND SQLHWND;
typedef ULONG BOOKMARK;
typedef struct tagDATE_STRUCT {
	SQLSMALLINT year;
	SQLUSMALLINT month;
	SQLUSMALLINT day;
} DATE_STRUCT;
typedef struct tagTIME_STRUCT {
	SQLUSMALLINT hour;
	SQLUSMALLINT minute;
	SQLUSMALLINT second;
} TIME_STRUCT;
typedef struct tagTIMESTAMP_STRUCT {
	SQLSMALLINT year;
	SQLUSMALLINT month;
	SQLUSMALLINT day;
	SQLUSMALLINT hour;
	SQLUSMALLINT minute;
	SQLUSMALLINT second;
	SQLUINTEGER fraction;
} TIMESTAMP_STRUCT;
typedef void* SQLHANDLE; 
typedef SQLHANDLE SQLHDESC; 
#pragma pack(pop)
#ifdef __cplusplus
}
#endif
#endif
