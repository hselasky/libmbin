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
 * This file contains a software implementation of simple 3-base
 * mathematical functions, like addition, subtraction, division and
 * multiplication using more-or-less the classical approach.
 */

#include <stdint.h>

#include "math_bin.h"

static const uint32_t k = 0x55555555;

uint32_t
mbin_is3_valid_32(uint32_t x)
{
	return ((x & (x / 2) & k) ? 0 : 1);
}

uint8_t
mbin_is3_div_by2_32(uint32_t x)
{
	return ((mbin_sumbits32(x & k) & 1) ? 0 : 1);
}

uint32_t
mbin_xor3_32(uint32_t a, uint32_t b)
{
	uint32_t r;
	uint32_t t;
	uint32_t b1;
	uint32_t b2;

	b1 = b & k;
	b2 = (b & ~k) / 2;

	r = a;

	/* half-adder */
	r = (r ^ b1) ^ (2 * (r & b1 & k));

	/* mod-3 */
	t = (r & (r / 2) & k);
	r = r & ~(t | (2 * t));

	/* half-adder */
	r = (r ^ b2) ^ (2 * (r & b2 & k));

	/* mod-3 */
	t = (r & (r / 2) & k);
	r = r & ~(t | (2 * t));

	/* half-adder */
	r = (r ^ b2) ^ (2 * (r & b2 & k));

	/* mod-3 */
	t = (r & (r / 2) & k);
	r = r & ~(t | (2 * t));

	return (r);
}

uint32_t
mbin_add3_32(uint32_t a, uint32_t b)
{
	uint32_t t = 0;
	uint8_t n;
	uint8_t r = 0;

	for (n = 0; n != 32; n += 2) {
		r = (a & 3) + (b & 3) + r;
		if (r >= 3) {
			r -= 3;
			t |= ((uint32_t)r) << n;
			r = 1;
		} else {
			t |= ((uint32_t)r) << n;
			r = 0;
		}
		b >>= 2;
		a >>= 2;
	}
	return (t);
}

uint32_t
mbin_sub3_32(uint32_t a, uint32_t b)
{
	uint32_t t = 0;
	uint8_t n;
	uint8_t r = 0;

	for (n = 0; n != 32; n += 2) {
		r = (a & 3) - (b & 3) - r;
		if (r >= 3) {
			r += 3;
			t |= ((uint32_t)r) << n;
			r = 1;
		} else {
			t |= ((uint32_t)r) << n;
			r = 0;
		}
		b >>= 2;
		a >>= 2;
	}
	return (t);
}

uint32_t
mbin_inv3_32(uint32_t a)
{
	return (((a & k) * 2) | ((a & ~k) / 2));
}

uint32_t
mbin_mul3_32(uint32_t a, uint32_t b1)
{
	uint32_t t;
	uint32_t b2;

	b2 = mbin_add3_32(b1, b1);

	t = 0;

	while (a) {

		if (a & 2)
			t = mbin_add3_32(t, b2);
		else if (a & 1)
			t = mbin_add3_32(t, b1);

		b1 <<= 2;
		b2 <<= 2;
		a >>= 2;
	}
	return (t);
}

uint32_t
mbin_div3_odd_32(uint32_t r, uint32_t d)
{
	uint32_t t;
	uint8_t n;

	if (!(d & 3))
		return (0);		/* number must be odd */

	t = 0;

	for (n = 0; n != 32; n += 2) {
		while (r & (3 << n)) {
			r = mbin_sub3_32(r, d);
			t = mbin_add3_32(t, 1 << n);
		}
		d <<= 2;
	}
	return (t);
}

uint32_t
mbin_div3_32(uint32_t r, uint32_t d)
{
	uint32_t t;
	uint8_t n;

	if (d == 0)
		return (0);		/* number must be non-zero */

	for (n = 0; n != 32; n += 2) {
		if ((d << n) & 0xc0000000)
			break;
	}

	t = 0;

	for (; n != (uint8_t)-2; n -= 2) {
		while (r >= (d << n)) {
			r = mbin_sub3_32(r, (d << n));
			t = mbin_add3_32(t, (1 << n));
		}
	}
	return (t);
}

void
mbin_expand_xor3_16x32(uint32_t *ptr, uint32_t set_bits,
    uint32_t mask, uint32_t slice)
{
	uint32_t x;

	set_bits |= (~mask);
	x = set_bits;

	while (1) {
		ptr[x & mask] = mbin_xor3_32(ptr[x & mask], slice);

		if (x == (uint32_t)(0 - 1)) {
			break;
		}
		x++;
		x |= set_bits;
	}
}

void
mbin_transform_multi_xor3_fwd_16x32(uint32_t *ptr, uint32_t *temp,
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
			val = mbin_inv3_32(val);
			/* expand logic expression */
			mbin_expand_xor3_16x32(temp, x, mask, val);
			/* invert value */
			val = mbin_inv3_32(val);
		}
		/* restore original value */
		temp[x] = val;

		if (x == mask)
			break;
		x++;
	}
}

uint32_t
mbin_rebase_322_32(uint32_t x)
{
	uint32_t d = 1;
	uint32_t t = 0;

	while (x) {

		if (x & 2)
			t += 2 * d;
		else if (x & 1)
			t += d;

		x >>= 2;
		d *= 3;
	}
	return (t);
}

uint32_t
mbin_rebase_223_32(uint32_t x)
{
	uint32_t t = 0;
	uint8_t n = 0;

	while (x && (n != 32)) {
		t |= (x % 3) << n;
		x /= 3;
		n += 2;
	}
	return (t);
}

uint32_t
mbin_split3_32(uint32_t x)
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
mbin_join3_32(uint32_t x)
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
