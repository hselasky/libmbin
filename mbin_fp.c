/*-
 * Copyright (c) 2005-2009 Hans Petter Selasky. All rights reserved.
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

#include <stdio.h>
#include <stdint.h>

#include "math_bin.h"

#define	BITS_REMAINDER 64

static mbin_fp_t
fix_fp_num(mbin_fp_t result)
{
	if (result.remainder) {
		/*
		 * Remove zero bits at the beginning of the remainder:
		 */
		while (!(result.remainder & 1)) {
			if (result.defined)
				result.defined--;

			result.exponent++;
			result.remainder /= 2;
		}
	}
	if (result.defined > BITS_REMAINDER)
		result.defined = BITS_REMAINDER;
	/*
	 * Just mask undefined bits to zero:
	 */
	if (result.defined < BITS_REMAINDER)
		result.remainder &= (1ULL << result.defined) - 1ULL;

	return (result);
}

uint64_t
mbin_fp_remainder(mbin_fp_t temp, int16_t _exp)
{
	uint64_t result;
	int32_t exp_lsb;

	exp_lsb = temp.exponent - _exp;

	if (exp_lsb <= -BITS_REMAINDER)
		result = 0;
	else if (exp_lsb < 0)
		result = temp.remainder >> -exp_lsb;
	else if (exp_lsb < BITS_REMAINDER)
		result = temp.remainder << exp_lsb;
	else
		result = 0;

	return (result);
}

mbin_fp_t
mbin_fp_number(uint64_t x, int16_t _exp)
{
	mbin_fp_t result;

	result.exponent = _exp;
	result.defined = BITS_REMAINDER;
	result.remainder = x;
	return (fix_fp_num(result));
}

mbin_fp_t
mbin_fp_power(mbin_fp_t base, uint64_t power)
{
	mbin_fp_t temp;

	temp = mbin_fp_number(1, 0);

	while (power) {
		if (power & 1) {
			temp = mbin_fp_mul(temp, base);
		}
		base = mbin_fp_mul(base, base);
		power /= 2;
	}
	return (temp);
}

static void
align_fp_num(mbin_fp_t *pa, mbin_fp_t *pb)
{
	uint32_t diff_exp;

	/*
	 * Align the numbers at the same exponent. Currently the
	 * lowest exponent is choosen.
	 *
	 * Check if number is undefined:
	 */
	if (pa->remainder == 0) {
		pa->exponent = pb->exponent;
		pa->defined = pb->defined;

	} else {
		if (pb->remainder == 0) {
			pb->exponent = pa->exponent;
			pb->defined = pa->defined;
		}
	}

	if (pa->exponent > pb->exponent) {
		diff_exp = pa->exponent - pb->exponent;

		if (diff_exp < BITS_REMAINDER)
			pa->remainder <<= diff_exp;
		else
			pa->remainder = 0;

		pa->exponent = pb->exponent;

		pa->defined += diff_exp;

	} else {
		diff_exp = pb->exponent - pa->exponent;

		if (diff_exp < BITS_REMAINDER)
			pb->remainder <<= diff_exp;
		else
			pb->remainder = 0;

		pb->exponent = pa->exponent;

		pb->defined += diff_exp;
	}
}

mbin_fp_t
mbin_fp_add(mbin_fp_t a, mbin_fp_t b)
{
	align_fp_num(&a, &b);
	a.remainder += b.remainder;

	if (b.defined < a.defined)
		a.defined = b.defined;

	return (fix_fp_num(a));
}

mbin_fp_t
mbin_fp_sub(mbin_fp_t a, mbin_fp_t b)
{
	align_fp_num(&a, &b);
	a.remainder -= b.remainder;

	if (b.defined < a.defined)
		a.defined = b.defined;

	return (fix_fp_num(a));
}

mbin_fp_t
mbin_fp_mul(mbin_fp_t a, mbin_fp_t b)
{
	a.remainder *= b.remainder;
	a.exponent += b.exponent;

	if (b.defined < a.defined)
		a.defined = b.defined;

	return (a);
}

mbin_fp_t
mbin_fp_div(mbin_fp_t a, mbin_fp_t b)
{
	a.remainder = mbin_div_odd64(a.remainder, b.remainder);
	a.exponent -= b.exponent;

	if (b.defined < a.defined)
		a.defined = b.defined;

	return (a);
}

uint8_t
mbin_fp_test_bit(mbin_fp_t a, int16_t _exp)
{
	int32_t exponent;

	exponent = _exp - a.exponent;

	if (exponent < 0)
		return 0;

	if (exponent >= BITS_REMAINDER)
		return (0);

	return ((a.remainder & (1ULL << exponent)) != 0);
}

void
mbin_fp_print(mbin_fp_t temp)
{
	if (temp.remainder == 0)
		printf("0");
	else
		printf("0x%016llx:%s0x%02x:0x%02x",
		    temp.remainder,
		    (temp.exponent < 0) ? "e=-" : "e=+",
		    (temp.exponent < 0) ? -temp.exponent : temp.exponent,
		    temp.defined);
}
