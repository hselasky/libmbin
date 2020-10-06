/*-
 * Copyright (c) 2001-2020 Hans Petter Selasky. All rights reserved.
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

#include <stdint.h>
#include <math.h>

#include "math_bin.h"

/*
 * This square root function should be faster than Newtons method:
 *
 * xn = (x + N/x) / 2
 */

uint32_t
mbin_sqrt_64(uint64_t a)
{
	uint64_t b = 0x4000000000000000ULL;

	if (a >= b) {
		a -= b;
		b >>= 1;
		b ^= 0x7000000000000000ULL;
	} else {
		b >>= 1;
		b ^= 0x3000000000000000ULL;
	}
	if (a >= b) {
		a -= b;
		b >>= 1;
		b ^= 0x1c00000000000000ULL;
	} else {
		b >>= 1;
		b ^= 0xc00000000000000ULL;
	}
	if (a >= b) {
		a -= b;
		b >>= 1;
		b ^= 0x700000000000000ULL;
	} else {
		b >>= 1;
		b ^= 0x300000000000000ULL;
	}
	if (a >= b) {
		a -= b;
		b >>= 1;
		b ^= 0x1c0000000000000ULL;
	} else {
		b >>= 1;
		b ^= 0xc0000000000000ULL;
	}
	if (a >= b) {
		a -= b;
		b >>= 1;
		b ^= 0x70000000000000ULL;
	} else {
		b >>= 1;
		b ^= 0x30000000000000ULL;
	}
	if (a >= b) {
		a -= b;
		b >>= 1;
		b ^= 0x1c000000000000ULL;
	} else {
		b >>= 1;
		b ^= 0xc000000000000ULL;
	}
	if (a >= b) {
		a -= b;
		b >>= 1;
		b ^= 0x7000000000000ULL;
	} else {
		b >>= 1;
		b ^= 0x3000000000000ULL;
	}
	if (a >= b) {
		a -= b;
		b >>= 1;
		b ^= 0x1c00000000000ULL;
	} else {
		b >>= 1;
		b ^= 0xc00000000000ULL;
	}
	if (a >= b) {
		a -= b;
		b >>= 1;
		b ^= 0x700000000000ULL;
	} else {
		b >>= 1;
		b ^= 0x300000000000ULL;
	}
	if (a >= b) {
		a -= b;
		b >>= 1;
		b ^= 0x1c0000000000ULL;
	} else {
		b >>= 1;
		b ^= 0xc0000000000ULL;
	}
	if (a >= b) {
		a -= b;
		b >>= 1;
		b ^= 0x70000000000ULL;
	} else {
		b >>= 1;
		b ^= 0x30000000000ULL;
	}
	if (a >= b) {
		a -= b;
		b >>= 1;
		b ^= 0x1c000000000ULL;
	} else {
		b >>= 1;
		b ^= 0xc000000000ULL;
	}
	if (a >= b) {
		a -= b;
		b >>= 1;
		b ^= 0x7000000000ULL;
	} else {
		b >>= 1;
		b ^= 0x3000000000ULL;
	}
	if (a >= b) {
		a -= b;
		b >>= 1;
		b ^= 0x1c00000000ULL;
	} else {
		b >>= 1;
		b ^= 0xc00000000ULL;
	}
	if (a >= b) {
		a -= b;
		b >>= 1;
		b ^= 0x700000000ULL;
	} else {
		b >>= 1;
		b ^= 0x300000000ULL;
	}
	if (a >= b) {
		a -= b;
		b >>= 1;
		b ^= 0x1c0000000ULL;
	} else {
		b >>= 1;
		b ^= 0xc0000000ULL;
	}
	if (a >= b) {
		a -= b;
		b >>= 1;
		b ^= 0x70000000ULL;
	} else {
		b >>= 1;
		b ^= 0x30000000ULL;
	}
	if (a >= b) {
		a -= b;
		b >>= 1;
		b ^= 0x1c000000ULL;
	} else {
		b >>= 1;
		b ^= 0xc000000ULL;
	}
	if (a >= b) {
		a -= b;
		b >>= 1;
		b ^= 0x7000000ULL;
	} else {
		b >>= 1;
		b ^= 0x3000000ULL;
	}
	if (a >= b) {
		a -= b;
		b >>= 1;
		b ^= 0x1c00000ULL;
	} else {
		b >>= 1;
		b ^= 0xc00000ULL;
	}
	if (a >= b) {
		a -= b;
		b >>= 1;
		b ^= 0x700000ULL;
	} else {
		b >>= 1;
		b ^= 0x300000ULL;
	}
	if (a >= b) {
		a -= b;
		b >>= 1;
		b ^= 0x1c0000ULL;
	} else {
		b >>= 1;
		b ^= 0xc0000ULL;
	}
	if (a >= b) {
		a -= b;
		b >>= 1;
		b ^= 0x70000ULL;
	} else {
		b >>= 1;
		b ^= 0x30000ULL;
	}
	if (a >= b) {
		a -= b;
		b >>= 1;
		b ^= 0x1c000ULL;
	} else {
		b >>= 1;
		b ^= 0xc000ULL;
	}
	if (a >= b) {
		a -= b;
		b >>= 1;
		b ^= 0x7000ULL;
	} else {
		b >>= 1;
		b ^= 0x3000ULL;
	}
	if (a >= b) {
		a -= b;
		b >>= 1;
		b ^= 0x1c00ULL;
	} else {
		b >>= 1;
		b ^= 0xc00ULL;
	}
	if (a >= b) {
		a -= b;
		b >>= 1;
		b ^= 0x700ULL;
	} else {
		b >>= 1;
		b ^= 0x300ULL;
	}
	if (a >= b) {
		a -= b;
		b >>= 1;
		b ^= 0x1c0ULL;
	} else {
		b >>= 1;
		b ^= 0xc0ULL;
	}
	if (a >= b) {
		a -= b;
		b >>= 1;
		b ^= 0x70ULL;
	} else {
		b >>= 1;
		b ^= 0x30ULL;
	}
	if (a >= b) {
		a -= b;
		b >>= 1;
		b ^= 0x1cULL;
	} else {
		b >>= 1;
		b ^= 0xcULL;
	}
	if (a >= b) {
		a -= b;
		b >>= 1;
		b ^= 0x7ULL;
	} else {
		b >>= 1;
		b ^= 0x3ULL;
	}
	if (a >= b) {
		a -= b;
		b >>= 1;
		b ^= 0x1ULL;
	} else {
		b >>= 1;
	}
	return (b);
}

bool
mbin_sub_if_gt_64(uint64_t *pa, uint64_t *ps, uint64_t value)
{
	uint64_t x;
	uint64_t y;

	x = *pa ^ *ps ^ value;
	y = 2 * ((~*pa & *ps) | (~(*pa & ~*ps) & value));

	if (x > y) {
		*pa = x;
		*ps = y;
		return (1);
	}
	return (0);
}

bool
mbin_sub_if_gte_64(uint64_t *pa, uint64_t *ps, uint64_t value)
{
	uint64_t x;
	uint64_t y;

	x = *pa ^ *ps ^ value;
	y = 2 * ((~*pa & *ps) | (~(*pa & ~*ps) & value));

	if (x >= y) {
		*pa = x;
		*ps = y;
		return (1);
	}
	return (0);
}

/*
 * Carry optimised version of mbin_sqrt_64() above:
 */
uint32_t
mbin_sqrt_carry_optimised_64(uint64_t z)
{
	uint64_t y = 0;
	uint64_t zc = 0;
	int8_t k;

	for (k = 62; k != -2; k -= 2) {
		if (mbin_sub_if_gte_64(&z, &zc, (y | 1) << k))
			y |= 2;
		y *= 2;
	}
	return (y / 4);
}

uint32_t
mbin_sqrt_odd_32(uint32_t x)
{
	uint32_t m;
	uint8_t s;

	/* All input numbers are assumed to be (x & 7) = 1. */

	x = -(x & ~7);

	for (s = 3; s != 32; s++) {

		m = (1 << s);

		if (x & m)
			x -= (~x & (m - 8)) << (s - 2);
		else
			x -= (x & (m - 8)) << (s - 2);
	}

	x = (((int32_t)x) >> 1) | 1;

	return (x);
}

uint64_t
mbin_sqrt_odd_64(uint64_t x)
{
	uint64_t m;
	uint8_t s;

	/* All input numbers are assumed to be (x & 7) = 1. */

	x = -x - 0x555555555555555FULL;

	if (x & 7)
		return (0);		/* not a square number */

	for (s = 3; s != 64; s++) {
		m = (1ULL << s);

		if (x & m)
			x += (x & (m - 8ULL)) << (s - 2);
		else
			x += (~x & (m - 8ULL)) << (s - 2);
	}

	x = (((int64_t)x) >> 1) | 1ULL;

	return (x);
}

uint32_t
mbin_sqrt_inv_odd32(uint32_t rem, uint32_t div)
{
	uint8_t n;

	div |= 7;
	div ^= 6;

	for (n = 1; n != 16; n++) {
		if (div & (1 << (n + 1))) {
			div = div + (div << (2 * n)) + (div << (n + 1));
			rem = rem + (rem << n);
		}
	}
	for (; n != 31; n++) {
		if (div & (1 << (n + 1))) {
			div = div + (div << (n + 1));
			rem = rem + (rem << n);
		}
	}
	return (rem & 0x7FFFFFFF);
}

/* Prescaling factor is (1ULL << BITS) */
uint64_t
mbin_sqrt_inv_64(uint64_t rem, uint64_t div)
{
	enum {
		BITS = 62,
	};

	uint64_t t;

	uint8_t x;
	uint8_t y;

	/* division by zero check */
	if (div < 4)
		return (1ULL << BITS);

	/* scale */
	div = div >> 2;

	/* scale */
	for (x = 0; !(div & (3ULL << (BITS - 2))); div <<= 2, x++);

	/* set result */
	rem <<= ((BITS - 2) / 2);

	/* compute */
	for (y = 0; y != (BITS / 2); y++) {
		t = div;

		div = div + (div >> y);
		div = div + (div >> y);

		if (div < (1ULL << BITS)) {
			rem = rem + (rem >> y);
		} else {
			div = t;
		}
	}

	/* return square root inverse */
	return (rem << x);
}

uint64_t
mbin_r64_square(r64_t const value)
{
	return ((int64_t)(int32_t)value.root * (int64_t)(int32_t)value.root + (int32_t)value.rem);
}

r64_t
mbin_r64_root(uint64_t value)
{
	uint64_t s = mbin_sqrt_64(value);

	return (r64_t){
		s, value - s * s
	};
}

r64_t
mbin_r64_add(const r64_t a, const r64_t b)
{
	return (r64_t){
		a.root + b.root, a.rem + b.rem
	};
}

r64_t
mbin_r64_sub(const r64_t a, const r64_t b)
{
	return (r64_t){
		a.root - b.root, a.rem - b.rem
	};
}

r64_t
mbin_r64_normalize(const r64_t a)
{
	return (mbin_r64_root(mbin_r64_square(a)));
}

r64_t
mbin_r64_add_exp(r64_t base, uint64_t exp)
{
	r64_t result = {};

	while (exp) {
		if (exp & 1)
			result = mbin_r64_add(result, base);

		base = mbin_r64_add(base, base);
		exp /= 2;
	}
	return (result);
}

uint32_t
mbin_r64_square_mod(r64_t const value, uint32_t mod)
{
	return ((uint64_t)value.root * (uint64_t)value.root + value.rem) % mod;
}

r64_t
mbin_r64_add_mod(const r64_t a, const r64_t b, uint32_t mod)
{
	return (r64_t){
		(a.root + b.root) % mod, (a.rem + b.rem) % mod
	};
}

r64_t
mbin_r64_add_exp_mod(r64_t base, uint64_t exp, uint32_t mod)
{
	r64_t result = {};

	while (exp) {
		if (exp & 1)
			result = mbin_r64_add_mod(result, base, mod);

		base = mbin_r64_add_mod(base, base, mod);
		exp /= 2;
	}
	return (result);
}

void
mbin_r2_sumdigits_xform_r64(r64_t *ptr, size_t max)
{
	size_t m, x, y;
	r64_t va, vb;

	for (m = 1; m != max; m *= 2) {
		for (x = 0; x != max; x += 2 * m) {
			for (y = 0; y != m; y++) {
				va = ptr[x + y];
				vb = ptr[x + y + m];

				ptr[x + y] = mbin_r64_add(va, vb);
				ptr[x + y + m] = mbin_r64_sub(va, vb);
			}
		}
	}
}

double
mbin_d64_square(d64_t const value)
{
	return (value.root * value.root + value.rem);
}

d64_t
mbin_d64_root(const double value)
{
	double s = floor(sqrt(value));

	return (d64_t){
		s, value - s * s
	};
}

d64_t
mbin_d64_add(const d64_t a, const d64_t b)
{
	return (d64_t){
		a.root + b.root, a.rem + b.rem
	};
}

d64_t
mbin_d64_sub(const d64_t a, const d64_t b)
{
	return (d64_t){
		a.root - b.root, a.rem - b.rem
	};
}

d64_t
mbin_d64_normalize(const d64_t a)
{
	return (mbin_d64_root(mbin_d64_square(a)));
}

d64_t
mbin_d64_add_exp(d64_t base, uint64_t exp)
{
	d64_t result = {};

	while (exp) {
		if (exp & 1)
			result = mbin_d64_add(result, base);

		base = mbin_d64_add(base, base);
		exp /= 2;
	}
	return (result);
}

void
mbin_r2_sumdigits_xform_d64(d64_t *ptr, size_t max)
{
	size_t m, x, y;
	d64_t va, vb;

	for (m = 1; m != max; m *= 2) {
		for (x = 0; x != max; x += 2 * m) {
			for (y = 0; y != m; y++) {
				va = ptr[x + y];
				vb = ptr[x + y + m];

				ptr[x + y] = mbin_d64_add(va, vb);
				ptr[x + y + m] = mbin_d64_sub(va, vb);
			}
		}
	}
}
