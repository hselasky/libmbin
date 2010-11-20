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
 * This file contains a software implementation of simple 6-base
 * mathematical functions, like addition, subtraction, division and
 * multiplication using more-or-less the classical approach.
 */

#include <stdint.h>

#include "math_bin.h"

static const uint32_t k = 0x49249249;

uint32_t
mbin_is_valid7_32(uint32_t x)
{
	return ((x & (x / 2) & (x / 4) & k) ? 0 : 1);
}

uint32_t
mbin_add7_32(uint32_t a, uint32_t b)
{
	uint32_t t = 0;
	uint8_t n;
	uint8_t r = 0;

	for (n = 0; n != 30; n += 3) {
		r = (a & 7) + (b & 7) + r;
		if (r >= 7) {
			r -= 7;
			t |= ((uint32_t)r) << n;
			r = 1;
		} else {
			t |= ((uint32_t)r) << n;
			r = 0;
		}
		b >>= 3;
		a >>= 3;
	}
	return (t);
}

uint32_t
mbin_sub7_32(uint32_t a, uint32_t b)
{
	uint32_t t = 0;
	uint8_t n;
	uint8_t r = 0;

	for (n = 0; n != 30; n += 3) {
		r = (a & 7) - (b & 7) - r;
		if (r >= 7) {
			r += 7;
			t |= ((uint32_t)r) << n;
			r = 1;
		} else {
			t |= ((uint32_t)r) << n;
			r = 0;
		}
		b >>= 3;
		a >>= 3;
	}
	return (t);
}

uint32_t
mbin_mul7_32(uint32_t a, uint32_t b1)
{
	uint32_t t;
	uint32_t b2;
	uint32_t b4;

	b2 = mbin_add7_32(b1, b1);
	b4 = mbin_add7_32(b2, b2);

	t = 0;

	while (a) {

		if (a & 4)
			t = mbin_add7_32(t, b4);
		if (a & 2)
			t = mbin_add7_32(t, b2);
		if (a & 1)
			t = mbin_add7_32(t, b1);

		b1 <<= 3;
		b2 <<= 3;
		a >>= 3;
	}
	return (t);
}

uint32_t
mbin_div7_odd_32(uint32_t r, uint32_t d)
{
	uint32_t t;
	uint8_t n;

	if (!(d & 7))
		return (0);		/* number must be odd */

	t = 0;

	for (n = 0; n != 30; n += 3) {
		while (r & (7 << n)) {
			r = mbin_sub7_32(r, d);
			t = mbin_add7_32(t, 1 << n);
		}
		d <<= 3;
	}
	return (t);
}

uint32_t
mbin_div7_32(uint32_t r, uint32_t d)
{
	uint32_t t;
	uint8_t n;

	if (d == 0)
		return (0);		/* number must be non-zero */

	for (n = 0; n != 30; n += 3) {
		if ((d << n) & (7 << 27))
			break;
	}

	t = 0;

	for (; n != (uint8_t)-3; n -= 3) {
		while (r >= (d << n)) {
			r = mbin_sub7_32(r, (d << n));
			t = mbin_add7_32(t, (1 << n));
		}
	}
	return (t);
}

uint32_t
mbin_rebase_722_32(uint32_t x)
{
	uint32_t d = 1;
	uint32_t t = 0;

	while (x) {

		if (x & 4)
			t += 4 * d;
		if (x & 2)
			t += 2 * d;
		if (x & 1)
			t += d;

		x >>= 3;

		d *= 7;
	}
	return (t);
}

uint32_t
mbin_rebase_227_32(uint32_t x)
{
	uint32_t t = 0;
	uint8_t n = 0;

	while (x && (n != 30)) {
		t |= (x % 7) << n;
		x /= 7;
		n += 3;
	}
	return (t);
}
