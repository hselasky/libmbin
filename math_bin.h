/*-
 * Copyright (c) 2008-2012 Hans Petter Selasky. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#ifndef _MATH_BIN_H_
#define	_MATH_BIN_H_

#include <sys/queue.h>

void	mbin_print8(const char *fmt, uint8_t x);
void	mbin_print16(const char *fmt, uint16_t x);
void	mbin_print32(const char *fmt, uint32_t x);
void	mbin_print64(const char *fmt, uint64_t x);
void	mbin_print8_abc_mask(uint8_t x, uint8_t m);
void	mbin_print16_abc_mask(uint16_t x, uint16_t m);
void	mbin_print32_abc_mask(uint32_t x, uint32_t m);
void	mbin_print8_abc(uint8_t x);
void	mbin_print16_abc(uint16_t x);
void	mbin_print32_abc(uint32_t x);
uint8_t	mbin_eval32_abc(uint32_t x, const char *str);
void	mbin_print32_const(uint32_t x);
uint32_t mbin_print_xor_analyse_fwd_32x32(uint32_t *ptr, uint32_t mask, uint32_t fslice);
uint32_t mbin_print_multi_analyse_fwd_32x32(uint32_t *ptr, uint32_t *temp, const char *remove, uint32_t mask, uint8_t do_xor);

void	mbin_expand_gte_32x32(uint32_t *ptr, uint32_t set_bits, uint32_t mask, uint32_t val);
void	mbin_expand_xor_gte_32x32(uint32_t *ptr, uint32_t set_bits, uint32_t mask, uint32_t val);
void	mbin_expand_add_32x32(uint32_t *ptr, uint32_t set_bits, uint32_t mask, uint32_t val);
void	mbin_expand_add_mod_32x32(uint32_t *ptr, uint32_t set_bits, uint32_t mask, uint32_t val, uint32_t mod);
void	mbin_expand_xor_32x32(uint32_t *ptr, uint32_t val, uint32_t mask, uint32_t slice);
void	mbin_expand_xor_16x32(uint16_t *ptr, uint32_t val, uint32_t mask, uint16_t slice);
void	mbin_expand_xor_8x32(uint8_t *ptr, uint32_t val, uint32_t mask, uint8_t slice);

uint32_t mbin_inc32(uint32_t val, uint32_t mask);
uint16_t mbin_inc16(uint16_t val, uint16_t mask);
uint8_t	mbin_inc8(uint8_t val, uint8_t mask);

uint32_t mbin_dec32(uint32_t val, uint32_t mask);
uint16_t mbin_dec16(uint16_t val, uint16_t mask);
uint8_t	mbin_dec8(uint8_t val, uint8_t mask);

void	mbin_optimise_xor_32x32(uint32_t *ptr, uint32_t mask, uint32_t func_slices, uint32_t tmp_slices);
void	mbin_transform_add_fwd_32x32(uint32_t *ptr, uint32_t *temp, uint32_t mask);
void	mbin_transform_add_mod_fwd_32x32(uint32_t *ptr, uint32_t *temp, uint32_t mask, uint32_t mod);
void	mbin_transform_xor_fwd_32x32(uint32_t *ptr, uint32_t mask, uint32_t f_slice, uint32_t t_slice);
void	mbin_transform_multi_xor_fwd_32x32(uint32_t *ptr, uint32_t *temp, uint32_t mask);
void	mbin_transform_gte_fwd_32x32(uint32_t *ptr, uint32_t *temp, uint32_t mask);
void	mbin_transform_xor_gte_fwd_32x32(uint32_t *ptr, uint32_t *temp, uint32_t mask);
void	mbin_transform_sos_fwd_32x32(uint32_t *ptr, uint8_t lmax);
void	mbin_transform_sos_inv_32x32(uint32_t *ptr, uint8_t lmax);
void	mbin_transform_sos_mod_fwd_32x32(uint32_t *ptr, uint32_t *temp, uint32_t *scratch, uint32_t mask, uint32_t mod);
void	mbin_transform_poly_fwd_32x32(uint32_t *ptr, uint32_t *temp, uint32_t *scratch, uint32_t mask);
void	mbin_transform_find_negative_32x1(uint32_t *ptr, uint32_t *neg, uint32_t mask, uint32_t slice);
void	mbin_transform_find_gte_32x1(uint32_t *ptr, uint32_t *gte, uint32_t mask, uint32_t slice);
uint8_t	mbin_compute_value_32x32(uint32_t *ptr, const uint8_t *premap, uint32_t mask, uint32_t set_bits, uint32_t work_slice);

uint8_t	mbin_sumbits64(uint64_t val);
uint8_t	mbin_sumbits32(uint32_t val);
uint8_t	mbin_sumbits16(uint16_t val);
uint8_t	mbin_sumbits8(uint8_t val);

uint8_t	mbin_fld_64(uint64_t y);
uint8_t	mbin_fld_32(uint32_t y);
uint8_t	mbin_fld_16(uint16_t y);
uint8_t	mbin_fld_8(uint8_t y);

uint32_t mbin_lsb32(uint32_t val);
uint16_t mbin_lsb16(uint16_t val);
uint8_t	mbin_lsb8(uint8_t val);

uint32_t mbin_msb32(uint32_t val);
uint16_t mbin_msb16(uint16_t val);
uint8_t	mbin_msb8(uint8_t val);

uint32_t mbin_greyA_inv32(uint32_t t);
uint32_t mbin_greyA_fwd32(uint32_t t);
uint32_t mbin_greyB_inv32(uint32_t t);
uint32_t mbin_greyB_fwd32(uint32_t t);

uint32_t mbin_recodeA_fwd32(uint32_t val, const uint8_t *premap);
uint16_t mbin_recodeA_fwd16(uint16_t val, const uint8_t *premap);
uint8_t	mbin_recodeA_fwd8(uint8_t val, const uint8_t *premap);
void	mbin_recodeA_def(uint8_t *premap, uint8_t start, uint8_t max);
void	mbin_recodeA_inv(const uint8_t *src, uint8_t *dst, uint8_t max);

uint32_t mbin_recodeB_fwd32(uint32_t x, const uint32_t *ptr);
uint32_t mbin_recodeB_inv32(uint32_t x, const uint32_t *ptr);

uint32_t mbin_polarise32(uint32_t val, uint32_t neg_pol);
uint16_t mbin_polarise16(uint16_t val, uint16_t neg_pol);
uint8_t	mbin_polarise8(uint8_t val, uint8_t neg_pol);

uint32_t mbin_depolarise32(uint32_t val, uint32_t neg_pol);
uint16_t mbin_depolarise16(uint16_t val, uint16_t neg_pol);
uint8_t	mbin_depolarise8(uint8_t val, uint8_t neg_pol);

uint32_t mbin_depolar_div32(uint32_t rem, uint32_t div);
uint16_t mbin_depolar_div16(uint16_t rem, uint16_t div);
uint8_t	mbin_depolar_div8(uint8_t rem, uint8_t div);

void	mbin_xor_common32(uint32_t *pa, uint32_t *pb);
void	mbin_xor_common16(uint16_t *pa, uint16_t *pb);
void	mbin_xor_common8(uint8_t *pa, uint8_t *pb);

uint32_t mbin_div_odd32(uint32_t r, uint32_t div);
uint64_t mbin_div_odd64(uint64_t r, uint64_t div);
uint64_t mbin_div_odd64_alt1(uint64_t r, uint64_t div);
uint32_t mbin_div_odd32_alt1(uint32_t r, uint32_t div);
uint32_t mbin_div_odd32_alt2(uint32_t r, uint32_t div);
uint32_t mbin_div_odd32_alt3(uint32_t r, uint32_t div);
uint32_t mbin_div_odd32_alt4(uint32_t r, uint32_t div);
uint32_t mbin_div_odd32_alt5(uint32_t r, uint32_t div);
uint32_t mbin_div_odd32_alt6(uint32_t r, uint32_t div);
uint32_t mbin_div_odd32_alt7(uint32_t r, uint32_t div);
uint16_t mbin_div_odd16(uint16_t r, uint16_t div);
uint8_t	mbin_div_odd8(uint8_t r, uint8_t div);
uint32_t mbin_div_by3_32(uint32_t x);
uint32_t mbin_div_by3_32_alt1(uint32_t x);
uint32_t mbin_div3_grey_32(uint32_t r);

uint32_t mbin_mul_baseN_32(uint32_t a, uint32_t b, uint32_t n);
uint32_t mbin_add_baseN_32(uint32_t a, uint32_t b, uint32_t f, uint32_t n);
uint32_t mbin_sub_baseN_32(uint32_t a, uint32_t b, uint32_t f, uint32_t n);
uint32_t mbin_sumdigits_baseN_32(uint32_t a, uint32_t n);
uint32_t mbin_convert_2toN_32(uint32_t a, uint32_t n);
uint32_t mbin_convert_Nto2_32(uint32_t a, uint32_t n);

uint32_t mbin_base_2toT_32(uint32_t tm, uint32_t tp, uint32_t r);
uint32_t mbin_base_Tto2_32(uint32_t tm, uint32_t tp, uint32_t r);
uint32_t mbin_base_T_add_32(uint32_t tm, uint32_t tp, uint32_t a, uint32_t b);
uint32_t mbin_base_T_sub_32(uint32_t tm, uint32_t tp, uint32_t a, uint32_t b);
uint32_t mbin_base_T_div_odd_32(uint32_t tm, uint32_t tp, uint32_t rem, uint32_t div);

uint64_t mbin_bitrev64(uint64_t a);
uint32_t mbin_bitrev32(uint32_t a);
uint16_t mbin_bitrev16(uint16_t a);
uint8_t	mbin_bitrev8(uint8_t a);

uint32_t mbin_base_2toL_32(uint32_t b2);
uint32_t mbin_base_Lto2_32(uint32_t bl);

struct mbin_baseM_state32 {
	uint32_t a;
	uint32_t c;
	uint32_t xor_val;
	uint32_t pol_val;
};

struct mbin_baseM_bits32 {
	uint32_t set[32];
	uint32_t a;
	uint32_t f;
	uint32_t c;
};

uint32_t mbin_baseM_next_32(uint32_t a1, uint32_t a0, uint32_t xor_val, uint32_t pol);
uint32_t mbin_base_2toM_32(uint32_t bm, uint32_t xor_val, uint32_t pol_val);
uint32_t mbin_base_Mto2_32(uint32_t bm, uint32_t xor_val, uint32_t pol_val);
void	mbin_baseM_get_state32(struct mbin_baseM_state32 *ps, uint32_t x, uint32_t xor_val, uint32_t pol_val);
void	mbin_baseM_inc_state32(struct mbin_baseM_state32 *ps);
uint32_t mbin_baseM_bits_slow_32(uint32_t x, uint32_t xor_val);
void	mbin_baseM_bits_init_32(struct mbin_baseM_bits32 *st, uint32_t x, uint32_t xor_val);
uint32_t mbin_baseM_bits_step_32(struct mbin_baseM_bits32 *st);
uint32_t mbin_baseM_bits_step_alt_32(struct mbin_baseM_bits32 *st);

struct mbin_baseG_state32 {
	uint32_t a;			/* result: a+b */
	uint32_t b;
	uint32_t c;
};

uint32_t mbin_base_2toG_32(uint32_t f, uint32_t b2);
uint32_t mbin_base_Gto2_32(uint32_t f, uint32_t bg);
void	mbin_baseG_get_state32(struct mbin_baseG_state32 *ps, uint32_t f, uint32_t index);
void	mbin_baseG_inc_state32(struct mbin_baseG_state32 *ps);
uint32_t mbin_baseG_decipher_state32(struct mbin_baseG_state32 *ps);
uint32_t mbin_baseG_inc_32(uint32_t f, uint32_t a);
uint32_t mbin_baseG_carry_32(uint32_t f, uint32_t a);

struct mbin_baseH_state32 {
	uint32_t a;
	uint32_t c;
	uint8_t	s;
};

uint32_t mbin_baseH_gen_div(uint8_t);
uint32_t mbin_base_2toH_32(uint32_t, uint8_t);
uint32_t mbin_base_Hto2_32(uint32_t, uint8_t);
void	mbin_baseH_get_state32(struct mbin_baseH_state32 *, uint32_t, uint8_t);
void	mbin_baseH_inc_state32(struct mbin_baseH_state32 *);
uint32_t mbin_baseH_decipher_state32(struct mbin_baseH_state32 *);
uint32_t mbin_baseH_2toVX_32(uint32_t, uint8_t);
uint32_t mbin_baseH_VXto2_32(uint32_t, uint8_t);
uint32_t mbin_baseH_to_linear(struct mbin_baseH_state32 *);
uint32_t mbin_baseHM2_rev32(uint32_t, uint32_t);
uint32_t mbin_baseHM2_fwd32(uint32_t, uint32_t);
uint32_t mbin_baseHM2_add32(uint32_t, uint32_t);
uint32_t mbin_baseHM2_sub32(uint32_t, uint32_t);
uint32_t mbin_baseHM2_mul3_32(uint32_t, uint32_t);

struct mbin_baseU_state32 {
	uint32_t a;
	uint32_t c;
};

uint32_t mbin_baseU_next_32(uint32_t a1, uint32_t a0);
uint32_t mbin_base_2toU_32(uint32_t b2);
uint32_t mbin_base_Uto2_32(uint32_t bu);
void	mbin_baseU_get_state32(struct mbin_baseU_state32 *ps, uint32_t index);
void	mbin_baseU_inc_state32(struct mbin_baseU_state32 *ps);
uint32_t mbin_baseU_decipher_state32(struct mbin_baseU_state32 *ps);

struct mbin_baseV_state32 {
	uint32_t a;
	uint32_t c;
};

uint32_t mbin_baseV_next_32(uint32_t a1, uint32_t a0);
uint32_t mbin_base_2toV_32(uint32_t);
uint32_t mbin_base_Vto2_32(uint32_t);
void	mbin_baseV_get_state32(struct mbin_baseV_state32 *, uint32_t);
void	mbin_baseV_inc_state32(struct mbin_baseV_state32 *);
uint32_t mbin_baseV_decipher_state32(struct mbin_baseV_state32 *);
uint32_t mbin_baseV_2toVX_32(uint32_t value);
uint32_t mbin_baseV_VXto2_32(uint32_t value);

struct mbin_fp {
	uint64_t remainder;		/* odd number, if not zero */
	int16_t	exponent;		/* 2**exponent */
	uint8_t	defined;		/* number of defined bits in
					 * 'remainder' */
};

typedef struct mbin_fp mbin_fp_t;

void	mbin_fp_set_modulus32(uint32_t prime, uint32_t off);
uint64_t mbin_fp_remainder(mbin_fp_t temp, int16_t _exp);
mbin_fp_t mbin_fp_number(uint64_t x, int16_t _exp);
mbin_fp_t mbin_fp_power(mbin_fp_t base, uint64_t power);
mbin_fp_t mbin_fp_add(mbin_fp_t a, mbin_fp_t b);
mbin_fp_t mbin_fp_sub(mbin_fp_t a, mbin_fp_t b);
mbin_fp_t mbin_fp_mul(mbin_fp_t a, mbin_fp_t b);
mbin_fp_t mbin_fp_div(mbin_fp_t a, mbin_fp_t b);
uint8_t	mbin_fp_test_bit(mbin_fp_t a, int16_t _exp);
void	mbin_fp_print(mbin_fp_t temp);
uint8_t	mbin_fp_inv_mat(mbin_fp_t *table, uint32_t size);
void	mbin_fp_print_mat(mbin_fp_t *table, uint32_t size, uint8_t print_invert);

uint32_t mbin_power_32(uint32_t x, uint32_t y);
uint64_t mbin_power_64(uint64_t x, uint64_t y);
uint32_t mbin_power_mod_32(uint32_t x, uint32_t y, uint32_t mod);
uint32_t mbin_power3_32(uint32_t x);
uint32_t mbin_power3_32_alt1(uint32_t x);
uint32_t mbin_power3_32_alt2(uint32_t x);
uint32_t mbin_power3_32_alt3(uint32_t x);
uint32_t mbin_power3_32_alt4(uint32_t x);
uint32_t mbin_inv_odd_non_linear_32(uint32_t val, uint32_t mod);
uint32_t mbin_inv_odd_prime_32(uint32_t val, uint32_t mod);
uint64_t mbin_cos_b2_odd_64(uint64_t x);
uint64_t mbin_sin_b2_odd_64(uint64_t x);

void	mbin_log_table_gen_32(uint32_t *pt);
uint32_t mbin_log_32(uint32_t r, uint32_t x);
uint32_t mbin_exp_32(uint32_t r, uint32_t x);
uint32_t mbin_power_odd_32(uint32_t rem, uint32_t base, uint32_t exp);
uint32_t mbin_exp_non_linear_32(uint32_t f, uint32_t x);
uint64_t mbin_log_non_linear_64(uint64_t a);
uint64_t mbin_exp_non_linear_64(uint64_t a, uint64_t d);

uint32_t mbin_parse32_abc(const char *ptr, const char *end);
void	mbin_parse32_add(const char *ptr, uint32_t *ptable, uint32_t mask);
void	mbin_parse32_xor(const char *ptr, uint32_t *ptable, uint32_t mask);
void	mbin_parse32_factor(const char *ptr, uint32_t *ptable, uint32_t mask, uint32_t var, uint8_t level, uint8_t is_xor);

uint32_t mbin_sqrt_64(uint64_t a);
uint32_t mbin_sqrt_odd_32(uint32_t x);
uint32_t mbin_sqrt_inv_odd32(uint32_t rem, uint32_t div);
uint64_t mbin_sqrt_inv_64(uint64_t rem, uint64_t div);

/* Base-3 */

uint32_t mbin_is3_valid_32(uint32_t x);
uint8_t	mbin_is3_div_by2_32(uint32_t x);
uint32_t mbin_lt3_32(uint32_t a, uint32_t b);
uint32_t mbin_xor3_32(uint32_t a, uint32_t b);
uint32_t mbin_and3_32(uint32_t a, uint32_t b);
uint32_t mbin_or3_32(uint32_t a, uint32_t b);
uint32_t mbin_add3_32(uint32_t a, uint32_t b);
uint32_t mbin_sub3_32(uint32_t a, uint32_t b);
uint32_t mbin_inv3_32(uint32_t a);
uint32_t mbin_mul3_32(uint32_t a, uint32_t b1);
uint32_t mbin_div3_odd_32(uint32_t r, uint32_t d);
uint32_t mbin_div3_32(uint32_t r, uint32_t d);
void	mbin_expand_add3_16x32(uint32_t *ptr, uint32_t set_bits, uint32_t mask, uint32_t slice);
void	mbin_expand_xor3_16x32(uint32_t *ptr, uint32_t set_bits, uint32_t mask, uint32_t slice);
void	mbin_transform_multi_xor3_fwd_16x32(uint32_t *ptr, uint32_t *temp, uint32_t mask);
void	mbin_transform_multi_add3_fwd_16x32(uint32_t *ptr, uint32_t *temp, uint32_t mask);
uint32_t mbin_rebase_322_32(uint32_t x);
uint32_t mbin_rebase_223_32(uint32_t x);

/* Base-m3 */

uint32_t mbin_is_valid_m3_32(uint32_t x);
uint8_t	mbin_is_m3_div_by2_32(uint32_t x);
uint8_t	mbin_is_m3_div_by4_32(uint32_t x);
uint32_t mbin_xor_m3_32(uint32_t a, uint32_t b);
uint32_t mbin_inv_m3_32(uint32_t a);
void	mbin_expand_xor_m3_16x32(uint32_t *ptr, uint32_t set_bits, uint32_t mask, uint32_t slice);
void	mbin_transform_multi_xor_m3_fwd_16x32(uint32_t *ptr, uint32_t *temp, uint32_t mask);
uint32_t mbin_rebase_m3_22_32(uint32_t x);
uint32_t mbin_rebase_22_m3_32(uint32_t x);

/* Base-4 */

uint8_t	mbin_is2_div_by3_32(uint32_t x);
uint32_t mbin_lt4_32(uint32_t a, uint32_t b);
uint32_t mbin_xor4_32(uint32_t a, uint32_t b);
uint32_t mbin_inv4_32(uint32_t a);
void	mbin_expand_xor4_16x32(uint32_t *ptr, uint32_t set_bits, uint32_t mask, uint32_t slice);
void	mbin_transform_multi_xor4_fwd_16x32(uint32_t *ptr, uint32_t *temp, uint32_t mask);
uint32_t mbin_split4_32(uint32_t x);
uint32_t mbin_join4_32(uint32_t x);

/* Base-5 */

uint32_t mbin_is5_valid_32(uint32_t x);
uint32_t mbin_add5_32(uint32_t a, uint32_t b);
uint32_t mbin_sub5_32(uint32_t a, uint32_t b);
uint32_t mbin_mul5_32(uint32_t a, uint32_t b1);
uint32_t mbin_div5_odd_32(uint32_t r, uint32_t d);
uint32_t mbin_div5_32(uint32_t r, uint32_t d);
uint32_t mbin_rebase_522_32(uint32_t x);
uint32_t mbin_rebase_225_32(uint32_t x);

/* Base-6 */

uint32_t mbin_is6_valid_32(uint32_t x);
uint32_t mbin_add6_32(uint32_t a, uint32_t b);
uint32_t mbin_sub6_32(uint32_t a, uint32_t b);
uint32_t mbin_mul6_32(uint32_t a, uint32_t b1);
uint32_t mbin_div6_odd_32(uint32_t r, uint32_t d);
uint32_t mbin_div6_32(uint32_t r, uint32_t d);
uint32_t mbin_rebase_622_32(uint32_t x);
uint32_t mbin_rebase_226_32(uint32_t x);

/* Base-7 */

uint32_t mbin_is7_valid_32(uint32_t x);
uint32_t mbin_add7_32(uint32_t a, uint32_t b);
uint32_t mbin_sub7_32(uint32_t a, uint32_t b);
uint32_t mbin_mul7_32(uint32_t a, uint32_t b1);
uint32_t mbin_div7_odd_32(uint32_t r, uint32_t d);
uint32_t mbin_div7_32(uint32_t r, uint32_t d);
uint32_t mbin_rebase_722_32(uint32_t x);
uint32_t mbin_rebase_227_32(uint32_t x);

/* Base-2/3 */

struct mbin_base23_state32 {
	uint32_t a[3];
};

uint32_t mbin_base_2to23_32(uint32_t);
uint32_t mbin_base_23to2_32(uint32_t);

void	mbin_base23_reduce_32(struct mbin_base23_state32 *);
void	mbin_base23_clean_carry_32(struct mbin_base23_state32 *);
void	mbin_base23_add_32(struct mbin_base23_state32 *, uint32_t);
void	mbin_base23_mul_32(uint32_t, uint32_t, struct mbin_base23_state32 *);

uint32_t mbin_base23_to_linear(struct mbin_base23_state32 *);
void	mbin_base23_from_linear(struct mbin_base23_state32 *, uint32_t);

/* Expression prototypes */

struct mbin_expr_and;
struct mbin_expr_xor;
struct mbin_expr;

#define	MBIN_EXPR_TYPE_VAR_A 0
#define	MBIN_EXPR_TYPE_VAR_B 1
#define	MBIN_EXPR_TYPE_VAR_C 2
#define	MBIN_BITS_MAX 127
#define	MBIN_BITS_MIN (-127)

uint32_t mbin_expr_get_value_xor(struct mbin_expr_xor *pxor);
void	mbin_expr_set_value_xor(struct mbin_expr_xor *pxor, uint32_t value);
uint32_t mbin_expr_get_value_and(struct mbin_expr_and *pxor);
void	mbin_expr_set_value_and(struct mbin_expr_and *pxor, uint32_t value);
int8_t	mbin_expr_get_type_and(struct mbin_expr_and *pand);
void	mbin_expr_set_type_and(struct mbin_expr_and *pand, int8_t type);
int8_t	mbin_expr_get_subtype_and(struct mbin_expr_and *pand);
void	mbin_expr_set_subtype_and(struct mbin_expr_and *pand, int8_t subtype);
int8_t	mbin_expr_get_shift_and(struct mbin_expr_and *pand);
void	mbin_expr_set_shift_and(struct mbin_expr_and *pand, int8_t shift);
void	mbin_expr_enqueue_and(struct mbin_expr_xor *pxor, struct mbin_expr_and *pand);
void	mbin_expr_dequeue_and(struct mbin_expr_xor *pxor, struct mbin_expr_and *pand);
struct mbin_expr_and *mbin_expr_foreach_and(struct mbin_expr_xor *pxor, struct mbin_expr_and *pand);
void	mbin_expr_enqueue_xor(struct mbin_expr *pexpr, struct mbin_expr_xor *pxor);
void	mbin_expr_dequeue_xor(struct mbin_expr *pexpr, struct mbin_expr_xor *pxor);
struct mbin_expr_xor *mbin_expr_foreach_xor(struct mbin_expr *pexpr, struct mbin_expr_xor *pxor);
int8_t	mbin_expr_max_shift_xor(struct mbin_expr_xor *pxa);
void	mbin_expr_substitute_and_simple(struct mbin_expr *pexpr, int8_t from_type, int32_t delta);
struct mbin_expr_and *mbin_expr_parse_and(struct mbin_expr_xor *pxor, const char *ptr, int *plen);
void	mbin_expr_print_and(struct mbin_expr_and *paa);
struct mbin_expr_xor *mbin_expr_parse_xor(struct mbin_expr *pexpr, const char *ptr, int *plen);
void	mbin_expr_print_xor(struct mbin_expr_xor *pxa);
struct mbin_expr *mbin_expr_parse(const char *ptr);
void	mbin_expr_print(struct mbin_expr *pexpr);
struct mbin_expr_and *mbin_expr_dup_and(struct mbin_expr_and *pand_old, struct mbin_expr_xor *pxor);
struct mbin_expr *mbin_expr_substitute_and_full(struct mbin_expr *pexpr, struct mbin_expr *psubst, int8_t type, int8_t subtype);
struct mbin_expr_and *mbin_expr_alloc_and(struct mbin_expr_xor *pxor);
struct mbin_expr_xor *mbin_expr_dup_xor(struct mbin_expr_xor *pxor_old, struct mbin_expr *pexpr);
struct mbin_expr_xor *mbin_expr_alloc_xor(struct mbin_expr *pexpr);
struct mbin_expr *mbin_expr_dup(struct mbin_expr *pexpr_old);
struct mbin_expr *mbin_expr_alloc(void);
void	mbin_expr_free_and(struct mbin_expr_xor *pxor, struct mbin_expr_and *pand);
void	mbin_expr_free_xor(struct mbin_expr *pexpr, struct mbin_expr_xor *pxor);
void	mbin_expr_free(struct mbin_expr *pexpr);
void	mbin_expr_optimise(struct mbin_expr *pexpr, uint32_t mask);

int32_t	mbin_correlate_32x32(uint32_t *pa, uint32_t *pb, uint32_t mask, uint32_t slice_a, uint32_t slice_b);

uint32_t mbin_sos_32(int32_t x, int32_t y);

uint32_t mbin_swm32(uint32_t x, uint32_t m, int8_t count);

uint32_t mbin_integrate_32(uint32_t *ptr, uint32_t max);
void	mbin_derivate_32(uint32_t *ptr, uint32_t max);
uint32_t mbin_sum_32(uint32_t *ptr, uint32_t max, uint8_t sstep);

struct mbin_fet_32_mod {
	uint32_t base;
	uint32_t mod;
	uint32_t length;
	uint32_t conv;
};

void	mbin_fet_32_generate(uint8_t);
uint32_t mbin_fet_32_generate_power(uint32_t f, uint32_t n);

/* Equation prototypes */

uint32_t *mbin_compress_tab_32(const uint32_t *ptr, uint32_t last, uint32_t slice);
void	mbin_expand_xor_tab_32(uint32_t *ptr, uint32_t *pcomp, uint32_t last, uint32_t slice);
void	mbin_expand_add_tab_32(uint32_t *ptr, uint32_t *pcomp, uint32_t last, uint32_t slice);
void	mbin_expand_sub_tab_32(uint32_t *ptr, uint32_t *pcomp, uint32_t last, uint32_t slice);
uint32_t *mbin_foreach_tab_32(uint32_t *pcomp, uint32_t *ptr);
void	mbin_free_tab_32(uint32_t *pcomp);
uint32_t mbin_count_tab_32(uint32_t *pcomp);

uint32_t mbin_coeff_32(int32_t n, int32_t x);

struct mbin_complex_32 {
	int32_t	x;
	int32_t	y;
};

struct mbin_complex_double {
	double	x;
	double	y;
};

/* Fast version of transforms */

void	mbin_multiply_xform_32(const uint32_t *, const uint32_t *, uint32_t *, uint8_t);
void	mbin_multiply_xform_64(const uint64_t *, const uint64_t *, uint64_t *, uint8_t);
void	mbin_multiply_xform_double(const double *, const double *, double *, uint8_t);
void
mbin_multiply_xform_complex_double(const struct mbin_complex_double *,
    const struct mbin_complex_double *,
    struct mbin_complex_double *, uint8_t log2_max);

void	mbin_inverse_rev_add_xform_32(uint32_t *, uint8_t);
void	mbin_forward_rev_add_xform_32(uint32_t *, uint8_t);
void	mbin_inverse_rev_add_xform_double(double *, uint8_t);
void	mbin_forward_rev_add_xform_double(double *, uint8_t);
void	mbin_inverse_rev_add_xform_complex_double(struct mbin_complex_double *, uint8_t);
void	mbin_forward_rev_add_xform_complex_double(struct mbin_complex_double *, uint8_t);
void	mbin_inverse_gte_xform_32(uint32_t *, uint8_t);
void	mbin_forward_gte_xform_32(uint32_t *, uint8_t);
void	mbin_inverse_gte_mask_xform_32(uint32_t *, uint8_t);
void	mbin_forward_gte_mask_xform_32(uint32_t *, uint8_t);
void	mbin_inverse_add_xform_32(uint32_t *, uint8_t);
void	mbin_forward_add_xform_32(uint32_t *, uint8_t);
void	mbin_inverse_add_xform_double(double *, uint8_t);
void	mbin_forward_add_xform_double(double *, uint8_t);
void	mbin_inverse_add_xform_complex_double(struct mbin_complex_double *, uint8_t);
void	mbin_forward_add_xform_complex_double(struct mbin_complex_double *, uint8_t);
void	mbin_xor_xform_32(uint32_t *, uint8_t);
void	mbin_xor_xform_8(uint8_t *, uint8_t);
void	mbin_sumdigits_r2_xform_64(uint64_t *, uint8_t);
void	mbin_sumdigits_r2_xform_32(uint32_t *, uint8_t);
void	mbin_sumdigits_r2_xform_double(double *, uint8_t);
void	mbin_sumdigits_r2_xform_complex_double(struct mbin_complex_double *, uint8_t);
void	mbin_sumdigits_r3_xform_complex_double(struct mbin_complex_double *, uint8_t);
void	mbin_sumdigits_r4_xform_complex_double(struct mbin_complex_double *, uint8_t);
uint32_t mbin_sumdigits_reorder_32(uint32_t, uint32_t, uint32_t);
uint32_t mbin_sumdigits_predmax_32(uint32_t, uint32_t, uint32_t);

/* Fast vector operations */

void	mbin_vector_or_double(double *, double *, double *, uint8_t);
void	mbin_vector_or_32(uint32_t *, uint32_t *, uint32_t *, uint8_t);
void	mbin_vector_and_double(double *, double *, double *, uint8_t);
void	mbin_vector_and_32(uint32_t *, uint32_t *, uint32_t *, uint8_t);
void	mbin_vector_xor_double(double *, double *, double *, uint8_t);
void	mbin_vector_xor_32(uint32_t *, uint32_t *, uint32_t *, uint8_t);

/* Noise functions */

int32_t	mbin_noise_24(void);

/* Sumdigit functions */

uint32_t mbin_sumdigit_32(uint32_t, uint32_t, uint32_t);

/* Complex functions */

struct mbin_complex_double mbin_mul_complex_double(struct mbin_complex_double, struct mbin_complex_double);
struct mbin_complex_double mbin_div_complex_double(struct mbin_complex_double, struct mbin_complex_double);
struct mbin_complex_double mbin_add_complex_double(struct mbin_complex_double, struct mbin_complex_double);
struct mbin_complex_double mbin_sub_complex_double(struct mbin_complex_double, struct mbin_complex_double);
double	mbin_square_len_complex_double(struct mbin_complex_double);

/* Orthogonal functions */

void	mbin_find_orthogonal_key_32(const uint32_t *, uint32_t, uint32_t, uint32_t **);

/* Multiplication functions */

uint32_t mbin_mul3_grey_32(uint32_t x);

/* Factor functions */

struct mbin_xor_factor_leaf {
	TAILQ_ENTRY(mbin_xor_factor_leaf) entry;
	TAILQ_HEAD(, mbin_xor_factor_leaf) children;
	int8_t	nchildren;
	int8_t	var;
	int8_t	desc;
};

uint32_t mbin_xor_factor8_var2mask(int8_t var);
char	mbin_xor_factor8_var2char(int8_t var);
void	mbin_xor_factor8_leaf_free(struct mbin_xor_factor_leaf *ptr);
struct mbin_xor_factor_leaf *mbin_xor_factor8_build_tree(uint8_t *src, uint8_t lmax, uint8_t how);
void	mbin_xor_factor8_print_tree(struct mbin_xor_factor_leaf *ptr, uint8_t level);
void	mbin_xor_factor8_compress_tree(struct mbin_xor_factor_leaf *parent, uint32_t *pout, uint32_t *bit, uint8_t bits, uint8_t how, uint8_t val);

uint32_t mbin_get_bits32(const uint32_t *ptr, uint32_t *poff, uint32_t bits);
void	mbin_put_bits32(uint32_t *ptr, uint32_t *poff, uint32_t bits, uint32_t value);
uint32_t mbin_get_rev_bits32(const uint32_t *ptr, uint32_t *poff, uint32_t bits);
void	mbin_put_rev_bits32(uint32_t *ptr, uint32_t *poff, uint32_t bits, uint32_t value);

#endif					/* _MATH_BIN_H_ */
