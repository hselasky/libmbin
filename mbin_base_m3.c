/*-
 * Copyright (c) 2010 Hans Petter Selasky. All rights reserved.
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
 * This file contains a software implementation of simple minus 3-base
 * mathematical functions, like addition, subtraction, division and
 * multiplication using more-or-less the classical approach.
 */

#include <stdint.h>

#include "math_bin.h"

static const uint32_t k = 0x55555555;

uint32_t
mbin_is_valid_m3_32(uint32_t x)
{
	return ((x & (x / 2) & k) ? 0 : 1);
}

uint8_t
mbin_is_m3_div_by2_32(uint32_t x)
{
	uint8_t t;

	t = mbin_sumbits32(x & k);

	return ((t & 1) ? 0 : 1);
}

uint8_t
mbin_is_m3_div_by4_32(uint32_t x)
{
	uint8_t t;

	t = mbin_sumbits32(x & k) + (2 * mbin_sumbits32(x & ~k));

	return ((t & 3) ? 0 : 1);
}

uint32_t
mbin_xor_m3_32(uint32_t a, uint32_t b)
{
	/* half-adder mod-4 */
	return ((a ^ b) ^ (2 * (a & b & k)));
}

uint32_t
mbin_inv_m3_32(uint32_t a)
{
	/* -a = ~a + 1 */

	a = ~a;

	return ((a ^ k) ^ (2 * (a & k)));
}

void
mbin_expand_xor_m3_16x32(uint32_t *ptr, uint32_t set_bits,
    uint32_t mask, uint32_t slice)
{
	uint32_t x;

	set_bits |= (~mask);
	x = set_bits;

	while (1) {
		ptr[x & mask] = mbin_xor_m3_32(ptr[x & mask], slice);

		if (x == (uint32_t)(0 - 1)) {
			break;
		}
		x++;
		x |= set_bits;
	}
}

void
mbin_transform_multi_xor_m3_fwd_16x32(uint32_t *ptr,
    uint32_t *temp, uint32_t mask)
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
			val = mbin_inv_m3_32(val);
			/* expand logic expression */
			mbin_expand_xor_m3_16x32(temp, x, mask, val);
			/* invert value */
			val = mbin_inv_m3_32(val);
		}
		/* restore original value */
		temp[x] = val;

		if (x == mask)
			break;
		x++;
	}
}

uint32_t
mbin_rebase_m3_22_32(uint32_t x)
{
	uint32_t d = 1;
	uint32_t t = 0;

	while (x) {

		if (x & 2)
			t += 2 * d;
		if (x & 1)
			t += d;

		x >>= 2;

		d = d - (4*d);
	}
	return (t);
}

uint32_t
mbin_rebase_22_m3_32(uint32_t x)
{
	uint32_t t = 0;
	uint8_t n = 0;
	int8_t a;

	while (x && (n != 32)) {

		a = ((int32_t)x) % ((int32_t)3);

		if (a < 0) {
			x -= 3;
			a = a + 3;
		}

		t |= a << n;

		x = ((int32_t)x) / ((int32_t)-3);
		n += 2;
	}
	return (t);
}
