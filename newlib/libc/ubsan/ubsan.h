/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright © 2025 Keith Packard
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above
 *    copyright notice, this list of conditions and the following
 *    disclaimer in the documentation and/or other materials provided
 *    with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its
 *    contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
 * INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
 * OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef __UBSAN_H_
#define __UBSAN_H_

#include <inttypes.h>
#include <stddef.h>
#include <stdbool.h>
#include <stdarg.h>

enum {
    type_kind_int = 0,
    type_kind_float = 1,
    type_unknown = 0xffff
};

enum {
    type_check_load_of,
    type_check_store_to,
    type_check_reference_binding_to,
    type_check_member_access_within,
    type_check_member_call_on,
    type_check_constructor_call_on,
    type_check_downcast_of_0,
    type_check_downcast_of_1,
    type_check_upcast_of,
    type_check_cast_to_virtual_base_of,
    type_check_nonnull_binding_to,
    type_check_dynamic_operation_on
};

enum {
    builtin_check_kind_ctz_passed_zero,
    builtin_check_kind_clz_passed_zero,
    builtin_check_kind_assume_passed_false,
};

enum {
    implicit_conversion_integer_truncation = 0,
    implicit_conversion_unsigned_integer_truncation = 1,
    implicit_conversion_signed_integer_truncation = 2,
    implicit_conversion_integer_sign_change = 3,
    implicit_conversion_signed_integer_truncation_or_sign_change = 4,
};

enum {
    cfi_type_check_v_call,
    cfi_type_check_nv_call,
    cfi_type_check_derived_cast,
    cfi_type_check_unrelated_cast,
    cfi_type_check_i_call,
    cfi_type_check_nvmf_call,
    cfi_type_check_vmf_call,
};

struct type_descriptor {
    uint16_t type_kind;
    uint16_t type_info;
    char type_name[];
};

static inline bool
type_is_signed(struct type_descriptor *type)
{
    return !!(type->type_info & 1);
}

static inline unsigned int
type_int_width(struct type_descriptor *type)
{
    return 1U << (type->type_info >> 1);
}

static inline unsigned int
type_float_width(struct type_descriptor *type)
{
    return type->type_info;
}

struct source_location {
    const char *file_name;
    unsigned int line;
    unsigned int column;
};

struct overflow_data {
    struct source_location location;
    struct type_descriptor *type;
};

struct pointer_overflow_data {
    struct source_location location;
};

struct float_cast_overflow_data {
    struct source_location location;
    struct type_descriptor *from_type;
    struct type_descriptor *to_type;
};

struct dynamic_type_cache_miss_data {
    struct source_location location;
    struct type_descriptor *type;
    void *type_info;
    unsigned char type_check_kind;
};

struct type_mismatch_data {
    struct source_location location;
    struct type_descriptor *type;
    unsigned long alignment;
    unsigned char type_check_kind;
};

struct type_mismatch_data_v1 {
    struct source_location location;
    struct type_descriptor *type;
    unsigned char log_alignment;
    unsigned char type_check_kind;
};

struct function_type_mismatch_data {
    struct source_location location;
    struct type_descriptor *type;
};

struct type_mismatch_data_common {
    struct source_location *location;
    struct type_descriptor *type;
    unsigned long alignment;
    unsigned char type_check_kind;
};

struct nonnull_arg_data {
    struct source_location location;
    struct source_location attr_location;
    int arg_index;
};

struct nonnull_return_data {
    struct source_location location;
};

struct out_of_bounds_data {
    struct source_location location;
    struct type_descriptor *array_type;
    struct type_descriptor *index_type;
};

struct shift_out_of_bounds_data {
    struct source_location location;
    struct type_descriptor *lhs_type;
    struct type_descriptor *rhs_type;
};

struct invalid_builtin_data {
    struct source_location location;
    unsigned char kind;
};

struct invalid_objc_cast_data {
    struct source_location location;
    struct type_descriptor *expected_type;
};

struct unreachable_data {
    struct source_location location;
};

struct implicit_conversion_data {
    struct source_location location;
    struct type_descriptor *from_type;
    struct type_descriptor *to_type;
    unsigned char kind;
};

struct invalid_value_data {
    struct source_location location;
    struct type_descriptor *type;
};

struct alignment_assumption_data {
    struct source_location location;
    struct source_location assumption_location;
    struct type_descriptor *type;
};

struct cfi_check_fail_data {
    unsigned char cfi_type_check_kind;
    struct source_location location;
    struct type_descriptor *type;
};

struct vla_bound_not_positive_data {
    struct source_location location;
    struct type_descriptor *type;
};

void
__ubsan_handle_add_overflow(void *data,
                            void *lhs,
                            void *rhs);

void
__ubsan_handle_alignment_assumption(void *data,
                                    void *ptr,
                                    void *align,
                                    void *offset);

void
__ubsan_handle_builtin_unreachable(void *data);

void
__ubsan_handle_cfi_bad_type(void *data,
                            void *vtable,
                            void *valid_vtable,
                            void *opts);


void
__ubsan_handle_cfi_check_fail(void *data,
                              void *function,
                              void *vtable_is_valid);

void
__ubsan_handle_divrem_overflow(void *data,
                               void *lhs,
                               void *rhs);

void
__ubsan_handle_dynamic_type_cache_miss(void *data,
                                       void *pointer,
                                       void *hash);

void
__ubsan_handle_float_cast_overflow(void *data,
                                   void *from);

void
__ubsan_handle_function_type_mismatch(void *data,
                                      void *ptr);

void
__ubsan_handle_implicit_conversion(void *data,
                                   void *src,
                                   void *dst);

void
__ubsan_handle_invalid_builtin(void *data);

void
__ubsan_handle_invalid_objc_cast(void *data,
                                 void *pointer);

void
__ubsan_handle_load_invalid_value(void *data,
                                  void *val);

void
__ubsan_handle_local_out_of_bounds(
    );

void
__ubsan_handle_missing_return(void *data);

void
__ubsan_handle_mul_overflow(void *data,
                            void *lhs,
                            void *rhs);

void
__ubsan_handle_negate_overflow(void *data,
                               void *val);

void
__ubsan_handle_nonnull_arg(void *data);

void
__ubsan_handle_nonnull_return(void *data);

void
__ubsan_handle_nonnull_return_v1(void *data,
                                 void *location);

void
__ubsan_handle_nullability_arg(void *data);

void
__ubsan_handle_nullability_return(void *data);

void
__ubsan_handle_nullability_return_v1(void *data,
                                     void *location);

void
__ubsan_handle_out_of_bounds(void *data,
                             void *index);

void
__ubsan_handle_pointer_overflow(void *data,
                                void *val, void *result);

void
__ubsan_handle_shift_out_of_bounds(void *data,
                                   void *lhs,
                                   void *rhs);

void
__ubsan_handle_sub_overflow(void *data,
                            void *lhs,
                            void *rhs);

void
__ubsan_handle_type_mismatch(void *data,
                             void *ptr);

void
__ubsan_handle_type_mismatch_v1(void *data,
                                void *ptr);

void
__ubsan_handle_vla_bound_not_positive(void *data,
                                      void *bound);

void __noreturn __picolibc_format(printf, 3, 4)
__ubsan_error(struct source_location *source,
                const char *fail,
                const char *fmt,
                ...);

void __picolibc_format(printf, 3, 4)
__ubsan_warning(struct source_location *source,
                const char *fail,
                const char *fmt,
                ...);

void
__ubsan_message(struct source_location *source,
                const char *msg,
                const char *fail,
                const char *fmt,
                va_list ap);

#define VAL_STR_LEN     32

void
__ubsan_val_to_string(char str[static VAL_STR_LEN],
                      struct type_descriptor *type,
                      void *value);

intmax_t
__ubsan_val_to_imax(struct type_descriptor *type,
                    void *value, intmax_t def);

uintmax_t
__ubsan_val_to_umax(struct type_descriptor *type,
                    void *value, uintmax_t def);

const char*
__ubsan_type_check_to_string(unsigned char type_check_kind);

const char*
__ubsan_cfi_type_check_to_string(unsigned char cfi_type_check_kind);

#endif /* _UBSAN_H_ */
