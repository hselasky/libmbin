/*-
 * Copyright (c) 2009-2020 Hans Petter Selasky. All rights reserved.
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

/*
 * Function to compute the Sum of Sums, SoS:
 *
 *  +-------------->  (y)
 *  | 1   0   0   0   0   0
 *  | 1   1   1   1   1   1
 *  | 1   2   3   4   5   6
 *  | 1   3   6  10  15  21
 *  | 1   4  10  20  35  56
 *  | 1   5  15  35  70 126
 *  | 1   6  21  56 126 252
 *  V
 *
 * (x)
 */

#include <stdint.h>

#include "math_bin.h"

uint32_t
mbin_sos_32(int32_t x, int32_t y)
{
	uint32_t rem;
	uint32_t div;
	uint32_t n;
	uint32_t temp;
	uint32_t fact;

	/* handle some special cases */

	if ((x < 0) || (y < 0))
		return (0);

	if (y == 0)
		return (1);

	if (x == 0)
		return (0);

	/* try to optimise */

	if (x < (y + 1)) {
		/* swap X and Y due to symmetry */
		temp = (x - 1);
		x = y + 1;
		y = temp;
	}
	rem = 1;
	div = 1;

	for (n = 0; n != (uint32_t)y; n++) {
		/* compute scaling factor */
		temp = (~n) & (n + 1);
		div *= (n + 1) / temp;
		fact = (x + n);

		/* remove common divisor */
		while ((~fact) & (~temp) & 1) {
			fact /= 2;
			temp /= 2;
		}

		/* compute Sum of Sum */
		rem *= fact;
		rem /= temp;
	}

	/* for better accuracy modulo (2**n) division is used */

	rem = mbin_div_odd32(rem, div);

	return (rem);
}

/*
 * This function computes the sum of squares for a 2**log2_level block
 * starting at "start" and returns the result.
 */
uint64_t
mbin_sos_block_2nd_64(const uint64_t start, uint8_t log2_level)
{
	uint64_t result;
	uint64_t k0;
	uint64_t k1;
	uint8_t x;

	if (log2_level == 0)
		return (start * start);

	k0 = 1ULL << (log2_level - 1);
	k1 = k0 * (k0 + k0 - 1 + start + start);

	result = k1 * k1;

	for (x = 0; x != log2_level; x++) {
		result += 1ULL << (2 * (log2_level - 1 + x));
	}

	result >>= log2_level;
	return (result);
}

/*
 * This function computes the sum of squares up to and including "x"
 * and returns the result:
 * 0, 1, 5, 14, 30, 55, 91, 140, 204, 285, 385, 506, 650, 819, 1015, 1240 ...
 *
 * The return value can also be expressed like:
 * (x**3) / 3 + (x**2) / 2 + (x / 6)
 */
uint64_t
mbin_sos_2nd_64(uint64_t x)
{
	uint64_t result = 0;
	uint64_t m0 = 1;
	uint64_t start = 0;
	uint8_t log2_m0 = 0;

	x++;

	while (m0 <= x) {
		m0 *= 2;
		log2_m0++;
	}

	m0 /= 2;
	log2_m0--;

	while (m0 != 0) {
		if (x & m0) {
			result += mbin_sos_block_2nd_64(start, log2_m0);
			start = x & -m0;
		}

		m0 /= 2.0;
		log2_m0--;
	}
	return (result);
}

uint64_t
mbin_sos_2nd_search_64(const uint64_t value)
{
	uint64_t m = 1;
	uint64_t x;

	while (mbin_sos_2nd_64(m) <= value)
		m *= 2;

	m /= 2;
	x = m;

	while (m != 0) {
		if (mbin_sos_2nd_64(m + x) <= value)
			x += m;
		m /= 2;
	}
	return (x);
}

uint32_t
mbin_sos_block_block_2nd_mod_32(uint32_t log2_log2_level, const uint32_t mod)
{
	uint64_t value = 1;

	while (log2_log2_level--) {
		value += value *
		    (uint64_t)mbin_power_mod_32(2, 2U << log2_log2_level, mod);
		value %= mod;
	}
	return (value);
}

uint32_t
mbin_sos_block_2nd_mod_32(const uint32_t log2_level, const uint32_t mod)
{
	uint64_t result;
	uint64_t start;
	uint64_t mask;
	uint64_t k0;
	uint64_t k1;

	if (log2_level == 0)
		return (0);

	k0 = mbin_power_mod_32(2, log2_level - 1, mod);
	k1 = (k0 * (k0 + k0 - 1)) % mod;

	result = (k1 * k1) % mod;

	mask = 1;
	while (mask <= log2_level)
		mask *= 2;

	start = 0;
	for (mask /= 2; mask != 0; mask /= 2) {
		if (log2_level & mask) {
			uint64_t chunk =
			  (uint64_t)mbin_power_mod_32(2, 2 * start + 2 * log2_level - 2, mod) *
			  (uint64_t)mbin_sos_block_block_2nd_mod_32(mbin_sumbits64(mask - 1), mod);

			result += (chunk % mod);
			result %= mod;
			start = log2_level & -mask;
		}
	}

	result *= mbin_power_mod_32((1 + mod) / 2, log2_level, mod);
	result %= mod;
	return (result);
}
