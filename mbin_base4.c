/*-
 * Copyright (c) 2010-2022 Hans Petter Selasky
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
 * This file contains a software implementation of simple 4-base
 * mathematical functions, like addition, subtraction, division and
 * multiplication using more-or-less the classical approach.
 */

#include <stdint.h>
#include <stdio.h>

#include "math_bin.h"

static const uint32_t k = 0x55555555;

uint8_t
mbin_is2_div_by3_32(uint32_t x)
{
	while (x >= 4) {
		x = mbin_sumbits32(x & k) +
		(2 * mbin_sumbits32(x & ~k));
	}
	return (x == 3 || x == 0);
}

uint32_t
mbin_lt4_32(uint32_t a, uint32_t b)
{
	uint32_t xor;

	xor = (a ^ b);

	return (((xor & b & ~k) / 2) | (~(xor / 2) & xor & b & k));
}

uint32_t
mbin_inv4_32(uint32_t a)
{
	return (mbin_xor4_32(~a, k));
}

void
mbin_expand_xor4_16x32(uint32_t *ptr, uint32_t set_bits,
    uint32_t mask, uint32_t slice)
{
	uint32_t x;

	set_bits |= (~mask);
	x = set_bits;

	while (1) {
		ptr[x & mask] = mbin_xor4_32(ptr[x & mask], slice);

		if (x == (uint32_t)(0 - 1)) {
			break;
		}
		x++;
		x |= set_bits;
	}
}

void
mbin_transform_multi_xor4_fwd_16x32(uint32_t *ptr, uint32_t *temp,
    uint32_t mask)
{
	uint32_t x;
	uint32_t val;

	x = 0;
	while (1) {
		temp[x] = ptr[x];
		if (x == mask)
			break;
		x++;
	}

	/* transform "ptr" */
	x = 0;
	while (1) {
		val = temp[x];

		if (val) {
			/* invert value */
			val = mbin_inv4_32(val);
			/* expand logic expression */
			mbin_expand_xor4_16x32(temp, x, mask, val);
			/* invert value */
			val = mbin_inv4_32(val);
			/* restore original value */
			temp[x] = val;
		}
		if (x == mask)
			break;
		x++;
	}
}

uint32_t
mbin_split4_32(uint32_t x)
{
	uint32_t r;
	uint8_t n;

	r = 0;

	for (n = 0; n != 32; n += 2) {
		if (x & (1 << n))
			r |= (1 << (n / 2));
		if (x & (2 << n))
			r |= (0x10000 << (n / 2));
	}
	return (r);
}

uint32_t
mbin_join4_32(uint32_t x)
{
	uint32_t r;
	uint8_t n;

	r = 0;

	for (n = 0; n != 32; n += 2) {
		if (x & (1 << (n / 2)))
			r |= (1 << n);
		if (x & (0x10000 << (n / 2)))
			r |= (2 << n);
	}
	return (r);
}

uint64_t
mbin_mul_4_64(uint64_t sa, uint64_t sb)
{
	uint64_t retval = 0;

	for (uint8_t a = 0; a != 16; a++) {
		if (~(sa >> a) & 1)
			continue;
		for (uint8_t b = 0; b != 16; b++) {
			if (~(sb >> b) & 1)
				continue;
			retval += 1ULL << (2 * (a + b));
		}
	}
	return (retval);
}

uint16_t
mbin_sqrt_4_64(uint64_t z, uint64_t *prem)
{
	uint64_t y = 0;
	uint64_t zc = 0;
	uint16_t r = 0;
	int8_t k;

	for (k = 60; k != -4; k -= 4) {
		if (mbin_sub_if_gte_64(&z, &zc, (y | 1ULL) << k)) {
			y |= 2;
			r |= 1U << (k / 4);
		}
		y *= 4;
	}
	if (prem != NULL)
		*prem = z - zc;
	return (r);
}
