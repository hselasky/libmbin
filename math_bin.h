/*-
 * Copyright (c) 2008 Hans Petter Selasky. All rights reserved.
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

void	mbin_print8(const char *fmt, uint8_t x);
void	mbin_print16(const char *fmt, uint16_t x);
void	mbin_print32(const char *fmt, uint32_t x);
void	mbin_print64(const char *fmt, uint64_t x);
void	mbin_print8_abc(uint8_t x);
void	mbin_print16_abc(uint16_t x);
void	mbin_print32_abc(uint32_t x);

void	mbin_expand_add_32x32(uint32_t *ptr, uint32_t set_bits, uint32_t mask, uint32_t val);
void	mbin_expand_xor_32x32(uint32_t *ptr, uint32_t val, uint32_t mask, uint32_t slice);
void	mbin_expand_xor_16x32(uint16_t *ptr, uint32_t val, uint32_t mask, uint16_t slice);
void	mbin_expand_xor_8x32(uint8_t *ptr, uint32_t val, uint32_t mask, uint8_t slice);

uint32_t mbin_inc32(uint32_t val, uint32_t mask);
uint16_t mbin_inc16(uint16_t val, uint16_t mask);
uint8_t	mbin_inc8(uint8_t val, uint8_t mask);

uint32_t mbin_dec32(uint32_t val, uint32_t mask);
uint16_t mbin_dec16(uint16_t val, uint16_t mask);
uint8_t	mbin_dec8(uint8_t val, uint8_t mask);

void	mbin_optimise_32x32(uint32_t *ptr, const uint8_t *premap, uint32_t mask, uint32_t set_bits, uint32_t def_slice, uint32_t work_slice);
void	mbin_transform_add_fwd_32x32(uint32_t *ptr, uint32_t *temp, uint32_t mask);
void	mbin_transform_xor_fwd_32x32(uint32_t *ptr, uint32_t mask, uint32_t set_bits, uint32_t f_slice, uint32_t t_slice);
uint8_t	mbin_compute_value_32x32(uint32_t *ptr, const uint8_t *premap, uint32_t mask, uint32_t set_bits, uint32_t work_slice);

uint8_t	mbin_sumbits32(uint32_t val);
uint8_t	mbin_sumbits16(uint16_t val);
uint8_t	mbin_sumbits8(uint8_t val);

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
uint32_t mbin_div_odd32_alt1(uint32_t r, uint32_t div);
uint32_t mbin_div_odd32_alt2(uint32_t r, uint32_t div);
uint16_t mbin_div_odd16(uint16_t r, uint16_t div);
uint8_t	mbin_div_odd8(uint8_t r, uint8_t div);

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

uint32_t mbin_bitrev32(uint32_t a);
uint16_t mbin_bitrev16(uint16_t a);
uint8_t	mbin_bitrev8(uint8_t a);

uint32_t mbin_base_2toL_32(uint32_t b2);
uint32_t mbin_base_Lto2_32(uint32_t bl);

struct mbin_baseM_state32 {
	uint32_t k;			/* (d OR c) */
	uint32_t d;
};

uint32_t mbin_base_2toM_32(uint32_t bm);
uint32_t mbin_base_Mto2_32(uint32_t bm);
void	mbin_baseM_get_state32(struct mbin_baseM_state32 *ps, uint32_t x);
void	mbin_baseM_inc_state32(struct mbin_baseM_state32 *ps);

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
};

uint32_t mbin_base_2toH_32(uint32_t b2);
uint32_t mbin_base_Hto2_32(uint32_t bh);
void	mbin_baseH_get_state32(struct mbin_baseH_state32 *ps, uint32_t index);
void	mbin_baseH_inc_state32(struct mbin_baseH_state32 *ps);
uint32_t mbin_baseH_decipher_state32(struct mbin_baseH_state32 *ps);

struct mbin_fp {
	uint64_t remainder;		/* odd number, if not zero */
	int16_t	exponent;		/* 2**exponent */
	uint8_t	defined;		/* number of defined bits in
					 * 'remainder' */
};

typedef struct mbin_fp mbin_fp_t;

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

#endif					/* _MATH_BIN_H_ */
