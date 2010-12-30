/*-
 * Copyright (c) 2009-2010 Hans Petter Selasky. All rights reserved.
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

#include "math_bin.h"

uint32_t
mbin_power_32(uint32_t x, uint32_t y)
{
	uint32_t r = 1;

	while (y) {
		if (y & 1)
			r *= x;
		x *= x;
		y /= 2;
	}
	return (r);
}

uint64_t
mbin_power_64(uint64_t x, uint64_t y)
{
	uint64_t r = 1;

	while (y) {
		if (y & 1)
			r *= x;
		x *= x;
		y /= 2;
	}
	return (r);
}

uint32_t
mbin_power_mod_32(uint32_t x, uint32_t y, uint32_t mod)
{
	uint64_t r = 1;
	uint64_t t = x % mod;

	while (y) {
		if (y & 1) {
			r *= t;
			r %= mod;
		}
		t *= t;
		t %= mod;
		y /= 2;
	}
	return (r);
}

static const uint32_t mbin_log_32_table[32] = {
	0x00000000,
	0x00000000,
	0xd3cfd984,
	0x9ee62e18,
	0xe83d9070,
	0xb59e81e0,
	0xa17407c0,
	0xce601f80,
	0xf4807f00,
	0xe701fe00,
	0xbe07fc00,
	0xfc1ff800,
	0xf87ff000,
	0xf1ffe000,
	0xe7ffc000,
	0xdfff8000,
	0xffff0000,
	0xfffe0000,
	0xfffc0000,
	0xfff80000,
	0xfff00000,
	0xffe00000,
	0xffc00000,
	0xff800000,
	0xff000000,
	0xfe000000,
	0xfc000000,
	0xf8000000,
	0xf0000000,
	0xe0000000,
	0xc0000000,
	0x80000000,
};

/*
 * The following table generator was created by the help of
 * Donald E. Knuth at Stanford University.
 */

void
mbin_log_table_gen_32(uint32_t *pt)
{
	const uint32_t d = 32;		/* number of bits */
	uint32_t j;
	uint32_t k;
	uint32_t s;
	uint32_t x;

	pt[d - 1] = 1 << (d - 1);
	pt[0] = 0;

	for (k = d - 2; k != 1; k--) {
		x = 1 + (1 << k);
		x += (x << k);
		for (j = k + 1, s = 0; x != 1; j++) {
			if (x & (1 << j)) {
				x += x << j;
				s += pt[j];
			}
		}
		pt[k] = -(s >> 1);
	}
}

uint32_t
mbin_log_32(uint32_t r, uint32_t x)
{
	uint8_t n;

	for (n = 2; n != 16; n++) {
		if (x & (1 << n)) {
			x = x + (x << n);
			r -= mbin_log_32_table[n];
		}
	}

	for (; n != 32; n++) {
		if (x & (1 << n)) {
			x = x + (x << n);
			r += (1 << n);
		}
	}
	return (r);
}

uint32_t
mbin_exp_32(uint32_t r, uint32_t x)
{
	uint8_t n;

	for (n = 2; n != 16; n++) {
		if (x & (1 << n)) {
			r = r + (r << n);
			x -= mbin_log_32_table[n];
		}
	}

	for (; n != 32; n++) {
		if (x & (1 << n)) {
			r = r + (r << n);
			x += (1 << n);
		}
	}
	return (r);
}

uint32_t
mbin_power_odd_32(uint32_t rem, uint32_t base, uint32_t exp)
{
	if (base & 2) {
		/* divider is considered negative */
		base = -base;
		/* check if result should be negative */
		if (exp & 1)
			rem = -rem;
	}
	return (mbin_exp_32(rem, mbin_log_32(0, base) * exp));
}

uint64_t
mbin_log_non_linear_64(uint64_t a)
{
	uint64_t b;
	uint64_t c;
	uint64_t an;
	uint64_t cn;
	uint64_t r;
	uint8_t n;

	if (!(a & 1))
		return (0);		/* number must be odd */

	c = 0;
	r = 0;

	/* cascade style implementation */

	for (n = 1; n != 64; n++) {
		if (a & (1ULL << n)) {
			b = c & ~a;

			/* try to move carry into "a" */

			c ^= b;
			a ^= b;

			/* half-adder, no carry out! */

			c = (c ^ (c << n)) ^ (2 * (c & (c << n)));

			b = a << n;

			/* we found a factor, store result */

			r |= 1ULL << n;

			/* half-adder, 3-var-in */

			an = a ^ b ^ c;
			cn = 2 * ((a & b) | (b & c) | (a & c));

			a = an;
			c = cn;
		}
		/* half-adder */

		an = a ^ c;
		cn = 2 * (a & c);

		a = an;
		c = cn;
	}
	return (r);
}

uint64_t
mbin_exp_non_linear_64(uint64_t a, uint64_t d)
{
	uint64_t b;
	uint64_t c;
	uint64_t an;
	uint64_t cn;
	uint8_t n;

	c = 0;

	/* cascade style implementation */

	for (n = 1; n != 64; n++) {
		if (d & (1ULL << n)) {
			b = c & ~a;

			/* try to move carry into "a" */

			c ^= b;
			a ^= b;

			/* half-adder, no carry out! */

			c = (c ^ (c << n)) ^ (2 * (c & (c << n)));

			b = a << n;

			/* half-adder, 3-var-in */

			an = a ^ b ^ c;
			cn = 2 * ((a & b) | (b & c) | (a & c));

			a = an;
			c = cn;
		}
		/* half-adder */

		an = a ^ c;
		cn = 2 * (a & c);

		a = an;
		c = cn;
	}
	return (a);
}

uint64_t
mbin_div_odd64_alt1(uint64_t r, uint64_t div)
{
	return (mbin_exp_non_linear_64(r, mbin_log_non_linear_64(div)));
}

uint32_t
mbin_power3_32(uint32_t x)
{
	return (mbin_power_32(3, x));
}

uint32_t
mbin_power3_32_alt1(uint32_t x)
{
	return (mbin_power_odd_32(1, 3, x));
}

uint32_t
mbin_power3_32_alt2(uint32_t x)
{
	uint32_t y;
	uint32_t z;

	/* compute power of 3 */

	for (y = z = 0; z != 32; z++)
		y += mbin_sos_32(x - z + 1, z) << z;

	return (y);
}

uint32_t
mbin_power3_32_alt3(uint32_t x)
{
	uint32_t y;
	uint32_t z;

	/* compute power of 3 */

	for (y = z = 0; z != 32; z++)
		y += mbin_coeff_32(x, z) << z;

	return (y);
}
