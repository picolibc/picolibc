/* Internal interfaces to the demangler for g++ V3 ABI.
   Copyright (C) 2003 Free Software Foundation, Inc.
   Written by Ian Lance Taylor <ian@wasabisystems.com>.

   This file is part of the libiberty library, which is part of GCC.

   This file is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   In addition to the permissions in the GNU General Public License, the
   Free Software Foundation gives you unlimited permission to link the
   compiled version of this file into combinations with other programs,
   and to distribute those combinations without any restriction coming
   from the use of this file.  (The General Public License restrictions
   do apply in other respects; for example, they cover modification of
   the file, and distribution when not linked into a combined
   executable.)

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA. 
*/

/* This header exports some internal interfaces used by the g++ V3 demangler.
   They are only exported for the use of GDB, and are subject to change.  */

#if defined (IN_GDB) || defined (IN_CP_DEMANGLE)

#include "ansidecl.h"

/* How to print the value of a builtin type.  */

enum d_builtin_type_print
{
  /* Print as (type)val.  */
  D_PRINT_DEFAULT,
  /* Print as integer.  */
  D_PRINT_INT,
  /* Print as long, with trailing `l'.  */
  D_PRINT_LONG,
  /* Print as bool.  */
  D_PRINT_BOOL,
  /* Print in usual way, but here to detect void.  */
  D_PRINT_VOID
};

/* Information we keep for a builtin type.  */

struct d_builtin_type_info
{
  /* Type name.  */
  const char *name;
  /* Length of type name.  */
  int len;
  /* Type name when using Java.  */
  const char *java_name;
  /* Length of java name.  */
  int java_len;
  /* How to print a value of this type.  */
  enum d_builtin_type_print print;
};

/* Component types found in mangled names.  */

enum d_comp_type
{
  /* A name.  */
  D_COMP_NAME,
  /* A qualified name.  */
  D_COMP_QUAL_NAME,
  /* A local name.  */
  D_COMP_LOCAL_NAME,
  /* A typed name.  */
  D_COMP_TYPED_NAME,
  /* A template.  */
  D_COMP_TEMPLATE,
  /* A template parameter.  */
  D_COMP_TEMPLATE_PARAM,
  /* A constructor.  */
  D_COMP_CTOR,
  /* A destructor.  */
  D_COMP_DTOR,
  /* A vtable.  */
  D_COMP_VTABLE,
  /* A VTT structure.  */
  D_COMP_VTT,
  /* A construction vtable.  */
  D_COMP_CONSTRUCTION_VTABLE,
  /* A typeinfo structure.  */
  D_COMP_TYPEINFO,
  /* A typeinfo name.  */
  D_COMP_TYPEINFO_NAME,
  /* A typeinfo function.  */
  D_COMP_TYPEINFO_FN,
  /* A thunk.  */
  D_COMP_THUNK,
  /* A virtual thunk.  */
  D_COMP_VIRTUAL_THUNK,
  /* A covariant thunk.  */
  D_COMP_COVARIANT_THUNK,
  /* A Java class.  */
  D_COMP_JAVA_CLASS,
  /* A guard variable.  */
  D_COMP_GUARD,
  /* A reference temporary.  */
  D_COMP_REFTEMP,
  /* A standard substitution.  */
  D_COMP_SUB_STD,
  /* The restrict qualifier.  */
  D_COMP_RESTRICT,
  /* The volatile qualifier.  */
  D_COMP_VOLATILE,
  /* The const qualifier.  */
  D_COMP_CONST,
  /* The restrict qualifier modifying a member function.  */
  D_COMP_RESTRICT_THIS,
  /* The volatile qualifier modifying a member function.  */
  D_COMP_VOLATILE_THIS,
  /* The const qualifier modifying a member function.  */
  D_COMP_CONST_THIS,
  /* A vendor qualifier.  */
  D_COMP_VENDOR_TYPE_QUAL,
  /* A pointer.  */
  D_COMP_POINTER,
  /* A reference.  */
  D_COMP_REFERENCE,
  /* A complex type.  */
  D_COMP_COMPLEX,
  /* An imaginary type.  */
  D_COMP_IMAGINARY,
  /* A builtin type.  */
  D_COMP_BUILTIN_TYPE,
  /* A vendor's builtin type.  */
  D_COMP_VENDOR_TYPE,
  /* A function type.  */
  D_COMP_FUNCTION_TYPE,
  /* An array type.  */
  D_COMP_ARRAY_TYPE,
  /* A pointer to member type.  */
  D_COMP_PTRMEM_TYPE,
  /* An argument list.  */
  D_COMP_ARGLIST,
  /* A template argument list.  */
  D_COMP_TEMPLATE_ARGLIST,
  /* An operator.  */
  D_COMP_OPERATOR,
  /* An extended operator.  */
  D_COMP_EXTENDED_OPERATOR,
  /* A typecast.  */
  D_COMP_CAST,
  /* A unary expression.  */
  D_COMP_UNARY,
  /* A binary expression.  */
  D_COMP_BINARY,
  /* Arguments to a binary expression.  */
  D_COMP_BINARY_ARGS,
  /* A trinary expression.  */
  D_COMP_TRINARY,
  /* Arguments to a trinary expression.  */
  D_COMP_TRINARY_ARG1,
  D_COMP_TRINARY_ARG2,
  /* A literal.  */
  D_COMP_LITERAL,
  /* A negative literal.  */
  D_COMP_LITERAL_NEG
};

/* A component of the mangled name.  */

struct d_comp
{
  /* The type of this component.  */
  enum d_comp_type type;
  union
  {
    /* For D_COMP_NAME.  */
    struct
    {
      /* A pointer to the name (not NULL terminated) and it's
	 length.  */
      const char *s;
      int len;
    } s_name;

    /* For D_COMP_OPERATOR.  */
    struct
    {
      /* Operator.  */
      const struct d_operator_info *op;
    } s_operator;

    /* For D_COMP_EXTENDED_OPERATOR.  */
    struct
    {
      /* Number of arguments.  */
      int args;
      /* Name.  */
      struct d_comp *name;
    } s_extended_operator;

    /* For D_COMP_CTOR.  */
    struct
    {
      enum gnu_v3_ctor_kinds kind;
      struct d_comp *name;
    } s_ctor;

    /* For D_COMP_DTOR.  */
    struct
    {
      enum gnu_v3_dtor_kinds kind;
      struct d_comp *name;
    } s_dtor;

    /* For D_COMP_BUILTIN_TYPE.  */
    struct
    {
      const struct d_builtin_type_info *type;
    } s_builtin;

    /* For D_COMP_SUB_STD.  */
    struct
    {
      const char* string;
      int len;
    } s_string;

    /* For D_COMP_TEMPLATE_PARAM.  */
    struct
    {
      long number;
    } s_number;

    /* For other types.  */
    struct
    {
      struct d_comp *left;
      struct d_comp *right;
    } s_binary;

  } u;
};

#define d_left(dc) ((dc)->u.s_binary.left)
#define d_right(dc) ((dc)->u.s_binary.right)

/* Prototypes for exported functions.  */

struct d_info;

extern struct d_comp *cp_v3_d_make_empty PARAMS ((struct d_info *,
						  enum d_comp_type));
extern struct d_comp *cp_v3_d_make_comp PARAMS ((struct d_info *,
						 enum d_comp_type,
						 struct d_comp *,
						 struct d_comp *));
extern struct d_comp *cp_v3_d_make_name PARAMS ((struct d_info *,
						 const char *,
						 int));
extern struct d_comp *cp_v3_d_make_builtin_type PARAMS ((struct d_info *,
							 int));
extern struct d_comp *cp_v3_d_make_operator_from_string
						PARAMS ((struct d_info *,
							 const char *));
extern struct d_comp *cp_v3_d_make_extended_operator
						PARAMS ((struct d_info *,
							 int,
							 struct d_comp *));
extern struct d_comp *cp_v3_d_make_ctor PARAMS ((struct d_info *,
						 enum gnu_v3_ctor_kinds,
						 struct d_comp *));
extern struct d_comp *cp_v3_d_make_dtor PARAMS ((struct d_info *,
						 enum gnu_v3_dtor_kinds,
						 struct d_comp *));

extern char *cp_v3_d_print PARAMS ((int, const struct d_comp *, int,
				    size_t *));

extern struct d_info *cp_v3_d_init_info_alloc PARAMS ((const char *, int,
						       size_t));
extern void cp_v3_d_free_info PARAMS ((struct d_info *));

#endif /* IN_GDB || IN_CP_DEMANGLE */
