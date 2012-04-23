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

static uint32_t mod_prime = 0;
static uint32_t mod_off = 0;

void
mbin_fp_set_modulus32(uint32_t prime, uint32_t mod_off)
{
	mod_prime = prime;
	mod_off = mod_off;
}

static mbin_fp_t
fix_fp_num(mbin_fp_t result)
{
	if (mod_prime != 0) {
		if (result.remainder & (1ULL << 63)) {
			/* negative */
			result.remainder = -result.remainder;
			result.remainder %= mod_prime;
			if (result.remainder != 0)
				result.remainder = mod_prime - result.remainder;
		} else {
			/* positive */
			result.remainder %= mod_prime;
		}
		result.defined = BITS_REMAINDER;
		result.exponent = 0;
		goto done;
	}
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

done:
	return (result);
}

uint64_t
mbin_fp_remainder(mbin_fp_t temp, int16_t _exp)
{
	uint64_t result;
	int32_t exp_lsb;

	exp_lsb = temp.exponent - _exp;

	if (mod_prime != 0) {

		result = temp.remainder;

		while (exp_lsb > 0) {
			result *= 2;
			if (result >= mod_prime)
				result -= mod_prime;
			exp_lsb--;
		}

		while (exp_lsb < 0) {
			if (result & 1)
				result += mod_prime;
			result /= 2;
			exp_lsb++;
		}
	}
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

	if (mod_prime != 0) {
		x %= mod_prime;

		while (_exp > 0) {
			x *= 2;
			if (x >= mod_prime)
				x -= mod_prime;
			_exp--;
		}
		while (_exp < 0) {
			if (x & 1)
				x += mod_prime;
			x /= 2;
			_exp++;
		}
	}
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

	if (mod_prime)
		return;

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

	return (fix_fp_num(a));
}

mbin_fp_t
mbin_fp_div(mbin_fp_t a, mbin_fp_t b)
{
	if (mod_prime != 0) {
		a.remainder = a.remainder *
		mbin_power_mod_32(b.remainder, mod_prime - mod_off, mod_prime);
	} else {
		a.remainder = mbin_div_odd64(a.remainder, b.remainder);
	}
	a.exponent -= b.exponent;

	if (b.defined < a.defined)
		a.defined = b.defined;

	return (fix_fp_num(a));
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
		printf("0x%016llx:%s0x%02x:0x%02x", 0ULL, "---", 0, 0);
	else
		printf("0x%016llx:%s0x%02x:0x%02x",
		    (long long unsigned int)temp.remainder,
		    (temp.exponent < 0) ? "e=-" : "e=+",
		    (temp.exponent < 0) ? -temp.exponent : temp.exponent,
		    temp.defined);
}

uint8_t
mbin_fp_inv_mat(mbin_fp_t *table, uint32_t size)
{
	mbin_fp_t temp;
	uint32_t x;
	uint32_t y;
	uint32_t z;
	uint32_t u;
	uint8_t retval = 0;

	/* invert matrix */

	for (y = 0; y != size; y++) {

		/* find non-zero entry in row */

		for (x = 0; x != size; x++) {
			if (table[(size * x) + y].remainder != 0) {
				goto found_non_zero;
			}
		}

		retval = 1;		/* failure */
		continue;

found_non_zero:

		/* normalise row */

		temp = table[(size * x) + y];

		for (z = 0; z != (2 * size); z++) {
			table[(size * z) + y] =
			    mbin_fp_div(table[(size * z) + y], temp);
		}

		/* subtract row */

		for (z = 0; z != size; z++) {
			if ((z != y) && (table[(size * x) + z].remainder != 0)) {
				temp = table[(size * x) + z];
				for (u = 0; u != (2 * size); u++) {
					table[(size * u) + z] = mbin_fp_sub(
					    table[(size * u) + z],
					    mbin_fp_mul(temp, table[(size * u) + y]));
				}
				if (table[(size * x) + z].remainder)
					return (2);	/* failure */
			}
		}
	}

	/* sort matrix */

	for (y = 0; y != size; y++) {
		for (x = 0; x != size; x++) {
			if (table[(size * x) + y].remainder) {
				if (x != y) {
					/* wrong order - swap */
					for (z = 0; z != (2 * size); z++) {
						temp = table[(size * z) + x];
						table[(size * z) + x] = table[(size * z) + y];
						table[(size * z) + y] = temp;
					}
					y--;
				}
				break;
			}
		}
		if (x == size)
			retval = 3;	/* failure */
	}
	return (retval);		/* success */
}

void
mbin_fp_print_mat(mbin_fp_t *table, uint32_t size, uint8_t print_invert)
{
	mbin_fp_t temp;
	uint32_t x;
	uint32_t y;
	uint32_t off;
	int16_t min_exponent;
	int16_t min_temp;
	uint8_t no_solution = 0;

	off = print_invert ? size : 0;

	for (y = 0; y != size; y++) {
		printf("0x%02x | ", y);
		min_exponent = 128;

		for (x = 0; x != size; x++) {
			temp = table[((x + off) * size) + y];

			mbin_fp_print(temp);

			if (temp.defined == 0)
				no_solution |= 1;

			min_temp = temp.defined + temp.exponent;

			if (min_temp < min_exponent)
				min_exponent = min_temp;

			if (x != (size - 1))
				printf(", ");
		}
		printf("; undef=2**%d\n", min_exponent);
	}
	if (no_solution)
		printf("this matrix has no solution due to undefined bits!\n");
}
