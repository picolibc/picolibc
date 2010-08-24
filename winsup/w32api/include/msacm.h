/*author: Adrian Sandor
  written for MinGW*/
#ifndef _MSACM_H
#define _MSACM_H

#if __GNUC__ >= 3
#pragma GCC system_header
#endif

#ifdef __cplusplus
extern "C" {
#endif

DECLARE_HANDLE(HACMDRIVERID);
typedef HACMDRIVERID *PHACMDRIVERID;
typedef HACMDRIVERID *LPHACMDRIVERID;
DECLARE_HANDLE(HACMDRIVER);
typedef HACMDRIVER *PHACMDRIVER;
typedef HACMDRIVER *LPHACMDRIVER;
DECLARE_HANDLE(HACMSTREAM);
typedef HACMSTREAM *PHACMSTREAM;
typedef HACMSTREAM *LPHACMSTREAM;
DECLARE_HANDLE(HACMOBJ);
typedef HACMOBJ *PHACMOBJ;
typedef HACMOBJ *LPHACMOBJ;

/*found through experimentation*/
#define ACMDRIVERDETAILS_SHORTNAME_CHARS 32
#define ACMDRIVERDETAILS_LONGNAME_CHARS 128
#define ACMDRIVERDETAILS_COPYRIGHT_CHARS 80
#define ACMDRIVERDETAILS_LICENSING_CHARS 128

/*I don't know the right values for these macros*/
#define ACMFORMATDETAILS_FORMAT_CHARS 256
#define ACMFORMATTAGDETAILS_FORMATTAG_CHARS 256
#define ACMDRIVERDETAILS_FEATURES_CHARS 256

/*msdn.microsoft.com/library/default.asp?url=/library/en-us/multimed/htm/_win32_acmformatdetails_str.asp*/
typedef struct {
	DWORD          cbStruct;
	DWORD          dwFormatIndex;
	DWORD          dwFormatTag;
	DWORD          fdwSupport;
	LPWAVEFORMATEX pwfx;
	DWORD          cbwfx;
	char szFormat[ACMFORMATDETAILS_FORMAT_CHARS];
} ACMFORMATDETAILSA, *LPACMFORMATDETAILSA;
typedef struct {
	DWORD          cbStruct;
	DWORD          dwFormatIndex;
	DWORD          dwFormatTag;
	DWORD          fdwSupport;
	LPWAVEFORMATEX pwfx;
	DWORD          cbwfx;
	WCHAR szFormat[ACMFORMATDETAILS_FORMAT_CHARS];
} ACMFORMATDETAILSW, *LPACMFORMATDETAILSW;

/*msdn.microsoft.com/en-us/library/dd742926%28VS.85%29.aspx*/
typedef struct {
  DWORD     cbStruct;
  DWORD     fdwStatus;
  DWORD_PTR dwUser;
  LPBYTE    pbSrc;
  DWORD     cbSrcLength;
  DWORD     cbSrcLengthUsed;
  DWORD_PTR dwSrcUser;
  LPBYTE    pbDst;
  DWORD     cbDstLength;
  DWORD     cbDstLengthUsed;
  DWORD_PTR dwDstUser;
  DWORD     dwReservedDriver[10];
} ACMSTREAMHEADER, *LPACMSTREAMHEADER;

/*msdn.microsoft.com/en-us/library/dd757711%28v=VS.85%29.aspx*/
typedef struct {
  DWORD cbStruct;
  DWORD dwFilterTag;
  DWORD fdwFilter;
  DWORD dwReserved[5];
} WAVEFILTER, *LPWAVEFILTER;

/*msdn.microsoft.com/library/default.asp?url=/library/en-us/multimed/htm/_win32_acmformattagdetails_str.asp*/
typedef struct {
	DWORD cbStruct;
	DWORD dwFormatTagIndex;
	DWORD dwFormatTag;
	DWORD cbFormatSize;
	DWORD fdwSupport;
	DWORD cStandardFormats;
	char szFormatTag[ACMFORMATTAGDETAILS_FORMATTAG_CHARS];
} ACMFORMATTAGDETAILSA, *LPACMFORMATTAGDETAILSA;
typedef struct {
	DWORD cbStruct;
	DWORD dwFormatTagIndex;
	DWORD dwFormatTag;
	DWORD cbFormatSize;
	DWORD fdwSupport;
	DWORD cStandardFormats;
	WCHAR szFormatTag[ACMFORMATTAGDETAILS_FORMATTAG_CHARS];
} ACMFORMATTAGDETAILSW, *LPACMFORMATTAGDETAILSW;

/*msdn.microsoft.com/library/default.asp?url=/library/en-us/multimed/htm/_win32_acmdriverdetails_str.asp*/
typedef struct {
	DWORD  cbStruct;
	FOURCC fccType;
	FOURCC fccComp;
	WORD   wMid;
	WORD   wPid;
	DWORD  vdwACM;
	DWORD  vdwDriver;
	DWORD  fdwSupport;
	DWORD  cFormatTags;
	DWORD  cFilterTags;
	HICON  hicon;
	char  szShortName[ACMDRIVERDETAILS_SHORTNAME_CHARS];
	char  szLongName[ACMDRIVERDETAILS_LONGNAME_CHARS];
	char  szCopyright[ACMDRIVERDETAILS_COPYRIGHT_CHARS];
	char  szLicensing[ACMDRIVERDETAILS_LICENSING_CHARS];
	char  szFeatures[ACMDRIVERDETAILS_FEATURES_CHARS];
} ACMDRIVERDETAILSA, *LPACMDRIVERDETAILSA;
typedef struct {
	DWORD  cbStruct;
	FOURCC fccType;
	FOURCC fccComp;
	WORD   wMid;
	WORD   wPid;
	DWORD  vdwACM;
	DWORD  vdwDriver;
	DWORD  fdwSupport;
	DWORD  cFormatTags;
	DWORD  cFilterTags;
	HICON  hicon;
	WCHAR  szShortName[ACMDRIVERDETAILS_SHORTNAME_CHARS];
	WCHAR  szLongName[ACMDRIVERDETAILS_LONGNAME_CHARS];
	WCHAR  szCopyright[ACMDRIVERDETAILS_COPYRIGHT_CHARS];
	WCHAR  szLicensing[ACMDRIVERDETAILS_LICENSING_CHARS];
	WCHAR  szFeatures[ACMDRIVERDETAILS_FEATURES_CHARS];
} ACMDRIVERDETAILSW, *LPACMDRIVERDETAILSW;

/*msdn.microsoft.com/library/default.asp?url=/library/en-us/multimed/htm/_win32_acmformatenumcallback.asp*/
typedef BOOL (CALLBACK *ACMFORMATENUMCBA) (
	HACMDRIVERID        hadid,
	LPACMFORMATDETAILSA pafd,
	DWORD_PTR           dwInstance,
	DWORD               fdwSupport
);
typedef BOOL (CALLBACK *ACMFORMATENUMCBW) (
	HACMDRIVERID        hadid,
	LPACMFORMATDETAILSW pafd,
	DWORD_PTR           dwInstance,
	DWORD               fdwSupport
);

/*msdn.microsoft.com/library/default.asp?url=/library/en-us/multimed/htm/_win32_acmformattagenumcallback.asp*/
typedef BOOL (CALLBACK *ACMFORMATTAGENUMCBA) (
	HACMDRIVERID           hadid,
	LPACMFORMATTAGDETAILSA paftd,
	DWORD_PTR              dwInstance,
	DWORD                  fdwSupport
);
typedef BOOL (CALLBACK *ACMFORMATTAGENUMCBW) (
	HACMDRIVERID           hadid,
	LPACMFORMATTAGDETAILSW paftd,
	DWORD_PTR              dwInstance,
	DWORD                  fdwSupport
);

/*msdn.microsoft.com/library/default.asp?url=/library/en-us/multimed/htm/_win32_acmdriverenumcallback.asp*/
typedef BOOL (CALLBACK *ACMDRIVERENUMCB) (
	HACMDRIVERID hadid,
	DWORD_PTR    dwInstance,
	DWORD        fdwSupport
);

/*and now the functions...*/

/*msdn.microsoft.com/library/default.asp?url=/library/en-us/multimed/htm/_win32_acmdriveropen.asp*/
MMRESULT WINAPI acmDriverOpen(LPHACMDRIVER phad, HACMDRIVERID hadid, DWORD fdwOpen);
/*msdn.microsoft.com/library/default.asp?url=/library/en-us/multimed/htm/_win32_acmdriverenum.asp*/
MMRESULT WINAPI acmDriverEnum(ACMDRIVERENUMCB fnCallback, DWORD_PTR dwInstance, DWORD fdwEnum);
/*msdn.microsoft.com/library/default.asp?url=/library/en-us/multimed/htm/_win32_acmformatenum.asp*/
MMRESULT WINAPI acmFormatEnumA(HACMDRIVER had, LPACMFORMATDETAILSA pafd, ACMFORMATENUMCBA fnCallback, DWORD_PTR dwInstance, DWORD fdwEnum);
MMRESULT WINAPI acmFormatEnumW(HACMDRIVER had, LPACMFORMATDETAILSW pafd, ACMFORMATENUMCBW fnCallback, DWORD_PTR dwInstance, DWORD fdwEnum);

/*msdn.microsoft.com/en-us/library/dd742885%28VS.85%29.aspx*/
MMRESULT WINAPI acmDriverAddA(LPHACMDRIVERID phadid, HINSTANCE hinstModule, LPARAM lParam, DWORD dwPriority, DWORD fdwAdd);
MMRESULT WINAPI acmDriverAddW(LPHACMDRIVERID phadid, HINSTANCE hinstModule, LPARAM lParam, DWORD dwPriority, DWORD fdwAdd);

/*msdn.microsoft.com/en-us/library/dd742897%28v=VS.85%29.aspx*/
MMRESULT WINAPI acmDriverRemove(HACMDRIVERID hadid, DWORD fdwRemove);

/*msdn.microsoft.com/library/default.asp?url=/library/en-us/multimed/htm/_win32_acmdriverclose.asp*/
MMRESULT WINAPI acmDriverClose(HACMDRIVER had, DWORD fdwClose);
/*msdn.microsoft.com/library/default.asp?url=/library/en-us/multimed/htm/_win32_acmdriverdetails.asp*/
MMRESULT WINAPI acmDriverDetailsA(HACMDRIVERID hadid, LPACMDRIVERDETAILSA padd, DWORD fdwDetails);
MMRESULT WINAPI acmDriverDetailsW(HACMDRIVERID hadid, LPACMDRIVERDETAILSW padd, DWORD fdwDetails);
/*msdn.microsoft.com/library/default.asp?url=/library/en-us/multimed/htm/_win32_acmformattagenum.asp*/
MMRESULT WINAPI acmFormatTagEnumA(HACMDRIVER had, LPACMFORMATTAGDETAILSA paftd, ACMFORMATTAGENUMCBA fnCallback, DWORD_PTR dwInstance, DWORD fdwEnum);
MMRESULT WINAPI acmFormatTagEnumW(HACMDRIVER had, LPACMFORMATTAGDETAILSW paftd, ACMFORMATTAGENUMCBW fnCallback, DWORD_PTR dwInstance, DWORD fdwEnum);

/*msdn.microsoft.com/en-us/library/dd742922(VS.85).aspx*/
MMRESULT WINAPI acmMetrics(HACMOBJ hao, UINT uMetric, LPVOID pMetric);

/*msdn.microsoft.com/en-us/library/dd742928%28VS.85%29.aspx*/
MMRESULT WINAPI acmStreamOpen(LPHACMSTREAM phas, HACMDRIVER had, LPWAVEFORMATEX pwfxSrc, LPWAVEFORMATEX pwfxDst, LPWAVEFILTER pwfltr, DWORD_PTR dwCallback, DWORD_PTR dwInstance, DWORD fdwOpen);

/*msdn.microsoft.com/en-us/library/dd742931%28VS.85%29.aspx*/
MMRESULT WINAPI acmStreamSize(HACMSTREAM has, DWORD cbInput, LPDWORD pdwOutputBytes, DWORD fdwSize);

/*msdn.microsoft.com/en-us/library/dd742929%28VS.85%29.aspx*/
MMRESULT WINAPI acmStreamPrepareHeader(HACMSTREAM has, LPACMSTREAMHEADER pash, DWORD fdwPrepare);

/*msdn.microsoft.com/en-us/library/dd742932%28VS.85%29.aspx*/
MMRESULT WINAPI acmStreamUnprepareHeader(HACMSTREAM has, LPACMSTREAMHEADER pash, DWORD fdwUnprepare);

/*msdn.microsoft.com/en-us/library/dd742930%28VS.85%29.aspx*/
MMRESULT WINAPI acmStreamReset(HACMSTREAM has, DWORD fdwReset);

/*msdn.microsoft.com/en-us/library/dd742923%28VS.85%29.aspx*/
MMRESULT WINAPI acmStreamClose(HACMSTREAM has, DWORD fdwClose);

/*msdn.microsoft.com/en-us/library/dd742924%28VS.85%29.aspx*/
MMRESULT WINAPI acmStreamConvert(HACMSTREAM has, LPACMSTREAMHEADER pash, DWORD fdwConvert);

#ifdef UNICODE

typedef ACMFORMATDETAILSW ACMFORMATDETAILS, *LPACMFORMATDETAILS;
typedef ACMFORMATTAGDETAILSW ACMFORMATTAGDETAILS, *LPACMFORMATTAGDETAILS;
typedef ACMDRIVERDETAILSW ACMDRIVERDETAILS, *LPACMDRIVERDETAILS;
typedef ACMFORMATENUMCBW ACMFORMATENUMCB;
typedef ACMFORMATTAGENUMCBW ACMFORMATTAGENUMCB;
#define acmFormatEnum acmFormatEnumW
#define acmDriverDetails acmDriverDetailsW
#define acmFormatTagEnum acmFormatTagEnumW
#define acmDriverAdd acmDriverAddW

#else /*ifdef UNICODE*/

typedef ACMFORMATDETAILSA ACMFORMATDETAILS, *LPACMFORMATDETAILS;
typedef ACMFORMATTAGDETAILSA ACMFORMATTAGDETAILS, *LPACMFORMATTAGDETAILS;
typedef ACMDRIVERDETAILSA ACMDRIVERDETAILS, *LPACMDRIVERDETAILS;
typedef ACMFORMATENUMCBA ACMFORMATENUMCB;
typedef ACMFORMATTAGENUMCBA ACMFORMATTAGENUMCB;
#define acmFormatEnum acmFormatEnumA
#define acmDriverDetails acmDriverDetailsA
#define acmFormatTagEnum acmFormatTagEnumA
#define acmDriverAdd acmDriverAddA

#endif /*ifdef UNICODE*/

#ifdef __cplusplus
}
#endif

#endif
