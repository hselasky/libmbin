/*-
 * Copyright (c) 2009-2019 Hans Petter Selasky
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
mbin_log_table_gen_32(uint32_t *pt, uint32_t factor)
{
	const uint32_t d = 32;		/* number of bits */
	uint32_t j;
	uint32_t k;
	uint32_t s;
	uint32_t x;

	/* Set the known entries and starting point: */
	pt[d - 1] = 1 << (d - 1);
	pt[0] = 0;

	for (k = d - 2; k != 1; k--) {

		/*
		 * Compute the squared of the two bit factor "x". The
		 * square yields a doubling of the logarithmic value,
		 * which we divide in the end:
		 */
		x = 1 + (1 << k);
		x += (x << k);

		/*
		 * Compute the logarithm for the value "x". We can
		 * observe that only previously computed logarithmic
		 * values are needed for this computation:
		 */
		for (j = k + 1, s = 0; x != 1; j++) {
			if (x & (1 << j)) {
				x += x << j;
				s += pt[j];
			}
		}

		/* Store result, and divide by two */
		pt[k] = -(s >> 1);

		/* XOR factor */
		if (factor & 1)
			pt[k] ^= (1U << 31);

		factor >>= 1;
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

	r -= (x & 0xFFFF0000);

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

	r *= 1 - (x & 0xFFFF0000);

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

uint32_t
mbin_log_table_32(uint32_t r, const uint32_t *table, uint32_t x)
{
	uint8_t n;

	for (n = 2; n != 32; n++) {
		if (x & (1 << n)) {
			x = x + (x << n);
			r -= table[n];
		}
	}
	return (r);
}

uint32_t
mbin_exp_table_32(uint32_t r, const uint32_t *table, uint32_t x)
{
	uint8_t n;

	for (n = 2; n != 32; n++) {
		if (x & (1 << n)) {
			r = r + (r << n);
			x -= table[n];
		}
	}
	return (r);
}

uint32_t
mbin_power_odd_table_32(uint32_t rem, const uint32_t *table,
    uint32_t base, uint32_t exp)
{
	if (base & 2) {
		/* divider is considered negative */
		base = -base;
		/* check if result should be negative */
		if (exp & 1)
			rem = -rem;
	}
	return (mbin_exp_table_32(rem, table,
	    mbin_log_table_32(0, table, base) * exp));
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

uint32_t
mbin_exp_non_linear_32(uint32_t f, uint32_t x)
{
	uint32_t a;
	uint32_t b;
	uint32_t c;
	uint32_t d;
	uint32_t e;
	uint32_t m;

	x /= 2;

	/*
	 * Resource usage:
	 * 32-bit multiplications: 8
	 * Single bit additions: 16 * 8
	 */

	for (m = 0; m != 32; m += 4) {
		e = 0;

		/* "e" value can be pre-computed based on "m" and "a" */

		for (a = 0; a != 16; a++) {
			b = a << m;

			if ((b & x) == b) {
				c = 0;
				for (d = m; d != (m + 4); d++) {
					if (b & (1 << d))
						c += d + 1;
				}

				if (c < 32)
					e += 1U << c;
			}
		}
		f *= e;
	}
	return (f);
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

uint32_t
mbin_power3_32_alt4(uint32_t x)
{
	uint32_t s;
	uint32_t r = 1;
	uint32_t f = 1;

	x += x & -2;

	for (s = 0; s != 32; s++) {
		if (s >= 3)
			f = f + ((f * f) << (s - 1));
		if (x & (1U << s))
			r = r + ((r * f) << (s + 1));
	}
	return (r);
}

uint32_t
mbin_inv_odd_non_linear_32(uint32_t val, uint32_t mod)
{
	return (mbin_power_mod_32(val, mod, mod));
}

uint32_t
mbin_inv_odd_prime_32(uint32_t val, uint32_t mod)
{
	return (mbin_power_mod_32(val, mod - 2, mod));
}

/* Standard converging power series for cosinus in 2-adic form: */

uint64_t
mbin_cos_b2_odd_64(uint64_t x)
{
	uint64_t s = 1;
	uint64_t div = 1;
	uint64_t k = x * x;
	uint64_t y = 2;
	uint8_t z;
	uint8_t p2 = 2 * 2;

	while (p2 < 64) {

		y--;
		z = mbin_fld_64(y);
		div *= (y >> z);
		y++;
		p2 -= z;

		z = mbin_fld_64(y);
		div *= (y >> z);
		p2 -= z;

		if (y & 2) {
			s -= mbin_div_odd64(k, div) << p2;
		} else {
			s += mbin_div_odd64(k, div) << p2;
		}

		k *= x;
		k *= x;

		y += 2;
		p2 += 4;
	}
	return (s);
}

/* Standard converging power series for sinus in 2-adic form: */

uint64_t
mbin_sin_b2_odd_64(uint64_t x)
{
	uint64_t s = 0;
	uint64_t div = 1;
	uint64_t k = x;
	uint64_t y = 1;
	uint8_t z;
	uint8_t p2 = 1 * 2;

	while (p2 < 64) {

		if (y != 1) {
			y--;
			z = mbin_fld_64(y);
			div *= (y >> z);
			y++;
			p2 -= z;
		}
		z = mbin_fld_64(y);
		div *= (y >> z);
		p2 -= z;

		if (y & 2) {
			s -= mbin_div_odd64(k, div) << p2;
		} else {
			s += mbin_div_odd64(k, div) << p2;
		}

		k *= x;
		k *= x;

		y += 2;
		p2 += 4;
	}
	return (s);
}

/* greatest common divisor, Euclid equation */
uint64_t
mbin_gcd_64(uint64_t a, uint64_t b)
{
	uint64_t an;
	uint64_t bn;

	while (b != 0) {
		an = b;
		bn = a % b;
		a = an;
		b = bn;
	}
	return (a);
}

uint64_t
mbin_factor_slow_64(uint64_t x)
{
	uint64_t limit;
	uint64_t y;

	if (x <= 1)
		return (0);
	if (!(x & 1))
		return (2);

	limit = (mbin_sqrt_64(x) + 4) | 1;

	if (limit > x)
		limit = x;
	if (limit < 3)
		limit = 3;

	for (y = 3; y != limit; y += 2) {
		if ((x % y) == 0)
			return (y);
	}
	return (0);
}

uint64_t
mbin_factor_slower_64(uint64_t x)
{
	uint64_t a;
	uint64_t b;
	uint64_t c;
	uint64_t d;

	if (x <= 1)
		return (0);
	if (!(x & 1))
		return (2);

	d = mbin_sqrt_64(x);
	c = d * d;
	b = 0;
	a = x;

	while (a != c) {
		a += (2 * b + 1);
		b++;
		while (a > c) {
			c += (2 * d + 1);
			d++;
		}
	}
	b += d;
	if (b < x)
		return (b);
	return (0);
}

uint64_t
mbin_factor_slowest_64(uint64_t x)
{
	uint64_t a;
	uint64_t b;

	if (x <= 1)
		return (0);

	/* check for even number */
	if (!(x & 1))
		return (2);

	/* check for square */
	b = mbin_sqrt_64(x);
	if (x == (b * b))
		return (b);

	/* compute offset */
	a = (x + 1) / 2;
	a = (a * a - 2 * a) % x;

	for (b = 0; b < x; b += 2) {
		a += b;
		if (a >= x)
			a -= x;
		if (a == 0) {
			if ((b + 3) == x)
				return (0);	/* is prime */
			else
				return (mbin_gcd_64(b + 3, x));
		}
	}
	return (0);	/* not reachable */
}
