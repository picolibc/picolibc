#ifndef __LOCAL_H__
#define __LOCAL_H__

#include <_ansi.h>
#include <reent.h>
#include <wchar.h>
#include <sys/types.h>

#ifndef NLS_ENVVAR_NAME
#  define NLS_ENVVAR_NAME "NLSPATH"
#endif
#ifndef NLS_DEFAULT_NLSPATH
#  define NLS_DEFAULT_NLSPATH  "/usr/locale/"
#endif

/* void* type for K&R compilers compatability */
#define _VOID_PTR _PTR

/* Charset aliases file */
#define ICONV_ALIASES_FNAME "charset.aliases"
/* iconv CCS data path */
#define ICONV_DATA_PATH NLS_DEFAULT_NLSPATH"iconv_data/"
/* iconv data files extention */
#define ICONV_DATA_EXT ".cct"

/* Unicode character data types and constants. */
#define UCS_CHAR_ZERO_WIDTH_NBSP 0xFEFF
#define UCS_CHAR_INVALID         0xFFFE
#define UCS_CHAR_NONE            0xFFFF

typedef __uint16_t ucs2_t;    /* Unicode character [D5] */
typedef __uint32_t ucs4_t;    /* Unicode scalar character [D28] */
#define ucs_t ucs4_t

#define iconv_char32bit(ch) ((ch) & 0xFFFF0000)

/* CCS file descriptor */
typedef struct
{
  void *mem;
  size_t len;
} _iconv_fd_t;

char *
_EXFUN(_iconv_resolve_cs_name, (struct _reent *, _CONST char *, _CONST char *));

char *
_EXFUN(_iconv_resolve_alias, (struct _reent *, _CONST char *, _CONST char *,
                                                         int, _CONST char *));

int
_EXFUN(_iconv_load_file, (struct _reent *, _CONST char *, _iconv_fd_t *));

int
_EXFUN(_iconv_unload_file, (struct _reent *, _iconv_fd_t *));

/* Builtin CCS and CES data. */
typedef struct {
    _CONST char *key;
    _CONST _VOID_PTR value;
} iconv_builtin_table_t;

extern _CONST iconv_builtin_table_t _iconv_builtin_ccs[];
extern _CONST iconv_builtin_table_t _iconv_builtin_ces[];
extern _CONST char _iconv_builtin_aliases[];

/* Table-driven coded character set (CCS) definitions. */
struct iconv_ccs;

/* Basic CCS functions */
typedef ucs2_t
_EXFUN(iconv_ccs_convert_t, (_CONST _VOID_PTR table, ucs2_t ch));

typedef int 
_EXFUN(iconv_ccs_close_t, (struct _reent *rptr, struct iconv_ccs *desc));

/* CCS structure */
struct iconv_ccs {
    _CONST _VOID_PTR from_ucs;  /* FROM table pointer */
    _CONST _VOID_PTR to_ucs;    /* TO table pointer   */
    iconv_ccs_convert_t *convert_from_ucs;  /* converter */
    iconv_ccs_convert_t *convert_to_ucs;    /* functions */
    iconv_ccs_close_t *close; /* close function      */
    _VOID_PTR extra;           /* optional extra data */
    unsigned int nbits;       /* number of bits      */
};

/* CCS initialisation function */
int 
_EXFUN(_iconv_ccs_init, (struct _reent *rptr, struct iconv_ccs *ccs, 
                         _CONST char *name));

/* CCS conversion macros */
#define ICONV_CCS_CONVERT_FROM_UCS(ccs, ch) \
        ((ccs)->convert_from_ucs((ccs)->from_ucs, (ch)))
#define ICONV_CCS_CONVERT_TO_UCS(ccs, ch) \
        ((ccs)->convert_to_ucs((ccs)->to_ucs, (ch)))
#define ICONV_CCS_NBITS

/* Module-driven character encoding scheme (CES) definitions */
struct iconv_ces;

/* Basic CES function types */
typedef int
_EXFUN(iconv_ces_init_t, (struct _reent *, _VOID_PTR *, /* void** */ 
                          _CONST char *, _CONST _VOID_PTR));

typedef int
_EXFUN(iconv_ces_close_t, (struct _reent *, _VOID_PTR));

typedef _VOID
_EXFUN(iconv_ces_reset_t, (_VOID_PTR));

typedef ssize_t
_EXFUN(iconv_ces_convert_from_ucs_t, (struct iconv_ces *data, ucs_t in, 
                                      unsigned char **outbuf, 
                                      size_t *outbytesleft));

typedef ucs_t
_EXFUN(iconv_ces_convert_to_ucs_t, (struct iconv_ces *data, 
                                    _CONST unsigned char **inbuf,
                                    size_t *inbytesleft));

/* CES descriptor structure - CES class data */
struct iconv_ces_desc {
    iconv_ces_init_t *init;
    iconv_ces_close_t *close;  /* class-specific close function */
    iconv_ces_reset_t *reset;
    iconv_ces_convert_from_ucs_t *convert_from_ucs;
    iconv_ces_convert_to_ucs_t   *convert_to_ucs;
    _CONST _VOID_PTR data;      /* optional specific CES class data */
};

/* explicit CES class for table (CCS) driven charsets */
extern _CONST struct iconv_ces_desc _iconv_ces_table_driven;

/* CES structure - CES instance data */
struct iconv_ces {
    _CONST struct iconv_ces_desc *desc;  /* descriptor/class pointer */
    iconv_ces_close_t *close;            /* instance-specific close function */
    _VOID_PTR data;      /* optional extra data */
    _VOID_PTR handle;    /* optional handle */
};

/* Basic CES functions and macros */
extern int 
_EXFUN(_iconv_ces_init, (struct _reent *rptr, struct iconv_ces *ces,
                        _CONST char *name));

#define ICONV_CES_CLOSE(rptr, ces) ((ces)->close(rptr, ces))
#define ICONV_CES_RESET(ces) ((ces)->desc->reset((ces)->data))
#define ICONV_CES_CONVERT_FROM_UCS(cesd, in, outbuf, outbytes) \
        ((cesd)->desc->convert_from_ucs((cesd), (in), (outbuf), (outbytes)))
#define ICONV_CES_CONVERT_TO_UCS(cesd, inbuf, inbytes) \
        ((cesd)->desc->convert_to_ucs((cesd), (inbuf), (inbytes)))

/* Virtual CES initialisation function type */
typedef int 
_EXFUN(iconv_ces_init_int_t, (struct _reent *rptr, _VOID_PTR* /* void ** */, 
                              _CONST _VOID_PTR, size_t));

/* CES subclass macros (for EUC and ISO-2022) */
#define ICONV_CES_DRIVER_DECL(name) \
    iconv_ces_init_int_t         _iconv_##name##_init; \
    iconv_ces_close_t            _iconv_##name##_close; \
    iconv_ces_reset_t            _iconv_##name##_reset; \
    iconv_ces_convert_from_ucs_t _iconv_##name##_convert_from_ucs; \
    iconv_ces_convert_to_ucs_t   _iconv_##name##_convert_to_ucs; \

/* CES functions and macros for stateless encodings */
iconv_ces_init_t  _iconv_ces_init_null;
iconv_ces_close_t _iconv_ces_close_null;
iconv_ces_reset_t _iconv_ces_reset_null;

#define ICONV_CES_STATELESS_MODULE_DECL(name) \
    _CONST struct iconv_ces_desc _iconv_ces_module_##name = { \
        _iconv_ces_init_null, \
        _iconv_ces_close_null, \
        _iconv_ces_reset_null, \
        convert_from_ucs, \
        convert_to_ucs, \
        NULL \
    }

/* CES functions and macros for stateful (integer state) encodings */
iconv_ces_init_t  _iconv_ces_init_state;
iconv_ces_close_t _iconv_ces_close_state;
iconv_ces_reset_t _iconv_ces_reset_state;

#define ICONV_CES_STATEFUL_MODULE_DECL(name) \
    _CONST struct iconv_ces_desc _iconv_ces_module_##name = { \
        _iconv_ces_init_state, \
        _iconv_ces_close_state, \
        _iconv_ces_reset_state, \
        convert_from_ucs, \
        convert_to_ucs, \
        NULL \
    }

/* CES functions and macros for other encodings */
#define ICONV_CES_MODULE_DECL(type, name) \
    static int \
    module_init(struct _reent *rptr, _VOID_PTR *data, /* void ** */ \
                _CONST char *cs_name, _CONST _VOID_PTR desc_data) \
    { \
        return _iconv_##type##_init(rptr, data, desc_data, \
                                    sizeof(ccsattr) / \
                                    sizeof(iconv_ces_##type##_ccs_t)); \
    } \
    \
    _CONST struct iconv_ces_desc _iconv_ces_module_##name = { \
        module_init, \
        _iconv_##type##_close, \
        _iconv_##type##_reset, \
        _iconv_##type##_convert_from_ucs, \
        _iconv_##type##_convert_to_ucs, \
        &ccsattr \
    }

/* EUC character encoding schemes and functions */
typedef struct {
    _CONST char *name;
    _CONST char *prefix;
    size_t      prefixlen;
} iconv_ces_euc_ccs_t;

ICONV_CES_DRIVER_DECL(euc);
#define _iconv_euc_reset _iconv_ces_reset_null

/* ISO-2022 character encoding schemes and functions. */
enum {ICONV_SHIFT_SI = 0, ICONV_SHIFT_SO, ICONV_SHIFT_SS2, ICONV_SHIFT_SS3};

typedef struct {
    _CONST char *name;
    _CONST char *designator;
    size_t designatorlen;
    int shift;
} iconv_ces_iso2022_ccs_t;

ICONV_CES_DRIVER_DECL(iso2022);


/* Converter structure and functions. */
typedef size_t
_EXFUN(iconv_conv_t, (struct _reent *, _VOID_PTR, _CONST unsigned char **,
                      size_t *, unsigned char **, size_t *));

typedef int
_EXFUN(iconv_close_t, (struct _reent *rptr, _VOID_PTR));

/* Generic converter structure. */
typedef struct {
    iconv_conv_t *convert;
    iconv_close_t *close;
}iconv_converter_t;

typedef struct {
    struct iconv_ces from;
    struct iconv_ces to;
    ucs_t  missing;
} unicode_converter_t;

/* Converter initialisers */
iconv_converter_t *
_EXFUN(_iconv_unicode_conv_init, (struct _reent *rptr, _CONST char *to, 
                                  _CONST char *from));

iconv_converter_t *
_EXFUN(_iconv_null_conv_init, (struct _reent *rptr, _CONST char *to,
                               _CONST char *from));

#endif /* __LOCAL_H__ */

