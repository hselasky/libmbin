/*-
 * Copyright (c) 2012 Hans Petter Selasky. All rights reserved.
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
 * This file implements XOR multiplication and some helper functions.
 */

#include <stdint.h>

#include "math_bin.h"

uint64_t
mbin_xor_exp_mod_64(uint64_t x, uint64_t y, uint64_t mod)
{
	uint64_t r = 1;

	while (y) {
		if (y & 1)
			r = mbin_xor_mul_mod_64(r, x, mod);
		x = mbin_xor_mul_mod_64(x, x, mod);
		y /= 2;
	}
	return (r);
}

uint64_t
mbin_xor_log_mod_64(uint64_t x, uint64_t mod)
{
	uint64_t msb = mbin_msb64(mod);
	uint64_t r;

	if (x == 0)
		return (0);

	for (r = 0; r != msb; r++) {
		if (mbin_xor_exp_mod_64(2, r, mod) == x)
			break;
	}
	return (r);
}

uint64_t
mbin_xor_mul_mod_64(uint64_t x, uint64_t y, uint64_t mod)
{
	uint64_t msb = mbin_msb64(mod);
	uint64_t r = 0;
	uint8_t n;

	for (n = 0; n != 64; n++) {
		if (y & (1ULL << n))
			r ^= x;

		x <<= 1;
		if (x & msb)
			x ^= mod;
	}
	return (r);
}

uint64_t
mbin_xor_mod_64(uint64_t x, uint64_t div)
{
	uint64_t msb = mbin_msb64(div);
	uint64_t xsb = mbin_msb64(x);
	uint8_t n;

	if (x == 0 || div == 0)
		return (0);

	if (xsb < msb)
		return (x);

	for (n = 0; n != 64; n++) {
		if (xsb & (msb << n))
			break;
	}

	do {
		if (x & (msb << n))
			x ^= (div << n);
	} while (n--);

	return (x);
}

uint64_t
mbin_xor_div_64(uint64_t x, uint64_t div)
{
	uint64_t msb = mbin_msb64(div);
	uint64_t xsb = mbin_msb64(x);
	uint64_t r = 0;
	uint8_t n;

	if (x == 0 || div == 0)
		return (0);

	if (xsb < msb)
		return (0);

	for (n = 0; n != 64; n++) {
		if (xsb & (msb << n))
			break;
	}

	do {
		if (x & (msb << n)) {
			x ^= (div << n);
			r |= (1ULL << n);
		}
	} while (n--);

	return (r);
}

uint64_t
mbin_xor_div_odd_64(uint64_t x, uint64_t div)
{
	uint64_t r = 0;
	uint8_t n;

	div |= 1;

	for (n = 0; n != 64; n++) {
		if (x & (1ULL << n)) {
			r |= (1ULL << n);
			x ^= (div << n);
		}
	}
	return (r);
}

uint64_t
mbin_xor_factor_slow(uint64_t x)
{
	uint64_t y;

	for (y = 0; y <= (x + 1) / 2; y++) {
		if (mbin_xor_div_odd_64(x, (2 * y) | 1) < x)
			return ((2 * y) | 1);
	}
	return (0);
}

uint64_t
mbin_xor_find_mod_64(uint8_t pwr)
{
	uint64_t max = (1ULL << pwr) - 1ULL;
	uint64_t len = (1ULL << (pwr - 1));

	for (; max > len; max -= 2) {
		if (mbin_xor_exp_mod_64(2, len, max) == 2)
			break;
	}
	return (max);
}
