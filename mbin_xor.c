/*-
 * Copyright (c) 2013 Hans Petter Selasky. All rights reserved.
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
 * This file implements helper functions for XOR multiplication,
 * division, exponent, logarithm and so on.
 */

#include <stdio.h>
#include <stdint.h>
#include <string.h>

#include "math_bin.h"

/*
 * ========================================================================
 *                              BASE-2 XOR
 * ========================================================================
 */

#define	MBIN_XOR2_MOD_64(p) ((1ULL << (p)) + 1ULL)
#define	MBIN_XOR2_BASE_64(p) 3ULL

uint64_t
mbin_xor2_rol_mod_64(uint64_t val, uint8_t shift, uint8_t p)
{
	val = (val << shift) | (val >> (p - shift));
	val &= (1ULL << p) - 1ULL;
	return (val);
}

uint64_t
mbin_xor2_ror_mod_64(uint64_t val, uint8_t shift, uint8_t p)
{
	val = (val >> shift) | (val << (p - shift));
	val &= (1ULL << p) - 1ULL;
	return (val);
}

uint64_t
mbin_xor2_square_mod_64(uint64_t x, uint8_t p)
{
	uint64_t r = 0;
	uint8_t n;
	uint8_t q;

	for (q = n = 0; n != p; n++) {
		if (x & (1ULL << n))
			r ^= 1ULL << q;
		q += 2;
		if (q >= p)
			q -= p;
	}
	return (r);
}

uint64_t
mbin_xor2_multi_square_mod_64(uint64_t x, uint8_t y, uint8_t p)
{
	uint64_t r = 0;
	uint8_t q = 0;

	do {
		if (x & 1)
			r ^= (1ULL << q);
		q += y;
		if (q >= p)
			q -= p;
	} while ((x /= 2));
	return (r);
}

uint64_t
mbin_xor2_root_mod_64(uint64_t x, uint8_t p)
{
	uint64_t r = 0;
	uint8_t n;
	uint8_t q;

	for (q = n = 0; n != p; n++) {
		if (x & (1ULL << q))
			r ^= 1ULL << n;
		q += 2;
		if (q >= p)
			q -= p;
	}
	return (r);
}

uint64_t
mbin_xor2_exp3_mod_64(uint64_t x, uint64_t y, uint8_t p)
{
	uint8_t n;
	uint8_t r;

	for (r = 1, n = 0; n != p; r *= 2, n++) {
		if (r >= p)
			r -= p;
		if (y & (1ULL << n))
			x = x ^ mbin_xor2_rol_mod_64(x, r, p);
	}
	return (x);
}

uint64_t
mbin_xor2_crc2bin_64(uint64_t z, uint8_t p)
{
	uint64_t r;
	uint8_t x;
	uint8_t y;

	r = (z & 1);

	for (y = 1, x = 1; x != p; y *= 2, x++) {
		if (y >= p)
			y -= p;
		if (z & (1ULL << y))
			r |= (1ULL << x);
	}
	return (r);
}

uint64_t
mbin_xor2_bin2crc_64(uint64_t z, uint8_t p)
{
	uint64_t r;
	uint8_t x;
	uint8_t y;

	r = (z & 1);

	for (y = 1, x = 1; x != p; y *= 2, x++) {
		if (y >= p)
			y -= p;
		if (z & (1ULL << x))
			r |= (1ULL << y);
	}
	return (r);
}

void
mbin_xor2_compute_inverse_table_64(struct mbin_xor2_pair_64 src,
    struct mbin_xor2_pair_64 dst, uint64_t *ptr, uint8_t lmax)
{
	uint64_t table[lmax][2];
	uint64_t temp[2][lmax];
	uint64_t m;
	uint64_t u;
	uint64_t v;
	uint8_t x;
	uint8_t y;

	for (u = v = 1, x = 0; x != lmax; x++) {
		temp[0][x] = u;
		temp[1][x] = v;
		u = mbin_xor2_mul_mod_any_64(u, src.val, src.mod);
		v = mbin_xor2_mul_mod_any_64(v, dst.val, dst.mod);
	}

	memset(table, 0, sizeof(table));
	memset(ptr, 0, sizeof(ptr[0]) * lmax);

	for (x = 0; x != lmax; x++) {
		for (y = 0; y != lmax; y++) {
			table[y][0] |= ((temp[0][x] >> y) & 1) << x;
			table[y][1] |= ((temp[1][x] >> y) & 1) << x;
		}
	}
	for (x = 0; x != lmax; x++) {
		if (table[x][0] == 0)
			continue;
		m = mbin_msb64(table[x][0]);
		for (y = 0; y != lmax; y++) {
			if (y == x)
				continue;
			if (table[y][0] & m) {
				table[y][0] ^= table[x][0];
				table[y][1] ^= table[x][1];
			}
		}
	}
	for (x = 0; x != lmax; x++) {
		if (table[x][0] == 0)
			continue;
		y = mbin_sumbits64(mbin_msb64(table[x][0]) - 1ULL);
		ptr[y] = table[x][1];
	}
}

uint64_t
mbin_xor2_compute_inverse_64(uint64_t x, const uint64_t *ptr, uint8_t lmax)
{
	uint64_t r;
	uint8_t y;

	for (r = 0, y = 0; y != lmax; y++) {
		if (mbin_sumbits64(x & ptr[y]) & 1)
			r ^= (1ULL << y);
	}
	return (r);
}

uint64_t
mbin_xor2_compute_div_64(uint64_t rem, uint64_t div,
    uint64_t mod, uint64_t *pmod)
{
	uint8_t lmax = mbin_sumbits64(mbin_msb64(mod) - 1ULL);
	uint64_t table[lmax];
	struct mbin_xor2_pair_64 src = {div, mod};
	struct mbin_xor2_pair_64 dst = {2, mod};

	mbin_xor2_compute_inverse_table_64(src, dst, table, lmax);

	if (pmod != NULL) {
		/* compute modulus */
		uint64_t temp = div;

		temp = mbin_xor2_exp_mod_any_64(temp, lmax, mod);
		temp = mbin_xor2_compute_inverse_64(temp, table, mod) ^
		    (1ULL << lmax);
		*pmod = temp;
	}
	return (mbin_xor2_compute_inverse_64(rem, table, mod));
}

uint64_t
mbin_xor2_compute_exp_64(uint64_t base, uint64_t exp, uint64_t mod)
{
	uint8_t lmax = mbin_sumbits64(mbin_msb64(mod) - 1ULL);
	uint8_t value[lmax];
	uint8_t a;
	uint8_t x;
	uint8_t y;
	uint64_t func[lmax];
	uint64_t table[lmax];
	uint64_t m;
	uint64_t u;

	/*
	 * Compute modulus for exponent
	 */
	for (u = 1, x = 0; x != lmax; x++) {
		func[x] = u;
		u = mbin_xor2_mul_mod_any_64(u, base, mod);
	}

	memset(table, 0, sizeof(table));

	for (x = 0; x != lmax; x++) {
		for (y = 0; y != lmax; y++) {
			table[y] |= ((func[x] >> y) & 1ULL) << x;
		}
		value[x] = (u >> x) & 1;
	}
	for (x = 0; x != lmax; x++) {
		m = mbin_lsb64(table[x]);
		for (y = 0; y != lmax; y++) {
			if (y == x)
				continue;
			if (table[y] & m) {
				table[y] ^= table[x];
				value[y] ^= value[x];
			}
		}
	}
	for (u = (1ULL << lmax), x = 0; x != lmax; x++) {
		if (table[x] && value[x]) {
			y = mbin_sumbits64(table[x] - 1ULL);
			u |= 1ULL << y;
		}
	}

	/* compute exponent */
	u = mbin_xor2_exp_mod_any_64(2, exp, u);

	for (m = 0, a = 0; a != lmax; a++) {
		if ((u >> a) & 1)
			m ^= func[a];
	}
	return (m);
}

/*
 * Example:
 * mod=0xa41825, base=2, len=2520
 *
 * Properties:
 * f(a,b) = f(b,a)
 */
uint64_t
mbin_xor2_multiply_plain_64(uint64_t a, uint64_t b, uint64_t mod)
{
	uint8_t lmax = mbin_sumbits64(mbin_msb64(mod) - 1ULL);
	uint8_t x;
	uint64_t r;
	uint64_t s;

	for (r = 0, s = 1, x = 0; x != lmax; x++) {
		if ((a >> x) & 1)
			r ^= s;
		s = mbin_xor2_mul_mod_any_64(s, b, mod);
	}
	return (r);
}

uint64_t
mbin_xor2_divide_plain_64(uint64_t rem, uint64_t div,
    uint64_t mod, uint8_t *psolved)
{
	uint8_t lmax = mbin_sumbits64(mbin_msb64(mod) - 1ULL);
	uint8_t value[lmax];
	uint8_t x;
	uint8_t y;
	uint64_t func[lmax];
	uint64_t table[lmax];
	uint64_t m;
	uint64_t u;

	for (u = 1, x = 0; x != lmax; x++) {
		func[x] = u;
		u = mbin_xor2_mul_mod_any_64(u, div, mod);
	}

	memset(table, 0, sizeof(table));

	for (x = 0; x != lmax; x++) {
		for (y = 0; y != lmax; y++) {
			table[y] |= ((func[x] >> y) & 1ULL) << x;
		}
		value[x] = (rem >> x) & 1;
	}
	for (x = 0; x != lmax; x++) {
		m = mbin_lsb64(table[x]);
		for (y = 0; y != lmax; y++) {
			if (y == x)
				continue;
			if (table[y] & m) {
				table[y] ^= table[x];
				value[y] ^= value[x];
			}
		}
	}

	if (psolved)
		*psolved = 1;

	/* gather solution */

	for (u = 0, x = 0; x != lmax; x++) {
		if (table[x]) {
			if (value[x]) {
				y = mbin_sumbits64(table[x] - 1ULL);
				u |= 1ULL << y;
			}
		} else {
			if (psolved)
				*psolved = 0;
		}
	}
	return (u);
}

uint64_t
mbin_xor2_log3_mod_64(uint64_t x, uint8_t p)
{
	uint64_t mask;
	uint64_t d2;
	uint64_t y;
	uint64_t z;
	uint8_t pm;
	uint8_t n;
	uint8_t r;
	uint8_t sbx;
	uint8_t sby;
	uint8_t ntable[p];

	/* Check for NUL */
	if (x <= 1)
		return (0);

	/* setup MOD table */

	pm = (p - 1) / 2;
	mask = p * ((1ULL << pm) - 1ULL);
	d2 = ((1ULL << pm) - 1ULL);

	/* there might be unused entries */
	memset(ntable, 0, sizeof(ntable));

	for (r = 1, n = 0; n != p; r *= 2, n++) {
		if (r >= p)
			r -= p;
		ntable[r] = n;
	}

	z = 0;

	/* Iterate until a solution is found */

	sbx = mbin_sumbits64(x);

	for (r = 1;;) {
		if (sbx >= (p - 2) || sbx <= 2)
			break;
		y = x ^ mbin_xor2_rol_mod_64(x, r, p);
		sby = mbin_sumbits64(y);
		if ((sby < (p / 2) && sby < sbx) ||
		    (sby >= (p / 2) && sby >= sbx)) {
			z += (1ULL << ntable[r]);
			x = y;
			sbx = sby;
		} else {
			r *= 2;
			if (r >= p)
				r -= p;
		}
	}

	if (sbx >= (p - 2))
		x ^= (1ULL << p) - 1ULL;

	sbx = mbin_sumbits64(x);

	/* sanity check */

	if (sbx == 0)
		return (0);

	/* shift down */

	while (!(x & 1ULL)) {
		z += d2;
		x /= 2ULL;
	}

	if (sbx == 1)
		return ((mask - (z % mask)) % mask);
	if (sbx != 2)
		return (0);

	/* locate second bit */

	r = mbin_sumbits64(x - 2ULL);

	/* properly MOD the "z" variable  */

	z = (mask + (1ULL << ntable[r]) - (z % mask)) % mask;

	return (z);
}

uint64_t
mbin_xor2_log3_mod_64_alt1(uint64_t x, uint8_t p)
{
	uint64_t mask;
	uint64_t d2;
	uint64_t y;
	uint64_t z;
	uint8_t pm;
	uint8_t n;
	uint8_t r;
	uint8_t sbx;
	uint8_t sby;
	uint8_t ntable[p];
	uint8_t to = p;

	/* Check for NUL */
	if (x <= 1)
		return (0);

	/* setup MOD table */

	pm = (p - 1) / 2;
	mask = p * ((1ULL << pm) - 1ULL);
	d2 = ((1ULL << pm) - 1ULL);

	/* there might be unused entries */
	memset(ntable, 0, sizeof(ntable));

	for (r = 1, n = 0; n != p; r *= 2, n++) {
		if (r >= p)
			r -= p;
		ntable[r] = n;
	}

	z = 0;
	sbx = mbin_sumbits64(x);

	/* Iterate until a solution is found */

	while (to--) {
		if (sbx == 2)
			break;
		for (r = 0; r != p; r++) {
			if (ntable[r] == 0)
				continue;
			y = x ^ mbin_xor2_rol_mod_64(x, r, p);
			sby = mbin_sumbits64(y);
			if (sby <= sbx) {
				z += (1ULL << ntable[r]);
				x = y;
				sbx = sby;
				break;
			}
		}

		/* invert value */
		if (r == p) {
			y = x ^ ((1ULL << p) - 1ULL);
			sby = mbin_sumbits64(y);
			x = y;
			sbx = sby;
		}
	}

	if (to == (uint8_t)-1)
		return (0);

	/* shift down */

	while (!(x & 1ULL)) {
		z += d2;
		x /= 2ULL;
	}

	/* locate the second bit */

	for (r = 1; r != p; r++) {
		if (x & (1ULL << r))
			break;
	}

	/* properly MOD the "z" variable  */

	if (ntable[r])
		z = (mask + (1ULL << ntable[r]) - (z % mask)) % mask;
	else
		z = (mask - (z % mask)) % mask;

	return (z);
}

uint64_t
mbin_xor2_is_div_by_3_64(uint64_t x)
{
	return (mbin_sumbits64(x) & 1);
}

uint64_t
mbin_xor2_sliced_exp_64(uint64_t x, uint64_t y,
    uint64_t base, uint64_t mod)
{
	uint64_t r;
	uint64_t bb;
	uint8_t msb;
	uint8_t a;
	uint8_t b;

	msb = mbin_sumbits64(mbin_msb64(mod) - 1ULL);

	for (a = 0; a != msb; a++) {
		if (y & (1ULL << a)) {
			bb = base;
			r = 0;
			for (b = 0; b != msb; b++) {
				r |= (mbin_sumbits64(x & bb) & 1) << b;
				bb = mbin_xor2_mul_mod_any_64(2, bb, mod);
			}
			x = r;
		}
		base = mbin_xor2_mul_mod_any_64(base, base, mod);
	}
	return (x);
}

uint8_t
mbin_xor2_is_mirror_64(uint64_t x)
{
	uint8_t end = mbin_sumbits64(mbin_msb64(x) - 1ULL);
	uint8_t a, b, c;

	for (a = 0; a != (end / 2); a++) {
		b = (x >> a) & 1;
		c = (x >> (end - a)) & 1;
		if (b != c)
			return (0);
	}
	return (1);
}

uint64_t
mbin_xor2_reduce_64(uint64_t x, uint8_t p)
{
	if (x & (1ULL << (p - 1)))
		x ^= (1ULL << p) - 1ULL;
	return (x);
}

uint64_t
mbin_xor2_exp_mod_64(uint64_t x, uint64_t y, uint8_t p)
{
	uint64_t r = 1;
	uint8_t n = 1;

	do {
		if (y & 1) {
			uint64_t z;

			z = mbin_xor2_multi_square_mod_64(x, n, p);
			if (r == 1)
				r = z;
			else
				r = mbin_xor2_mul_mod_64(r, z, p);
		}
		n *= 2;
		if (n >= p)
			n -= p;

	} while ((y /= 2));
	return (r);
}

uint64_t
mbin_xor2_exp_64(uint64_t x, uint64_t y)
{
	uint64_t r = 1;

	while (y) {
		if (y & 1)
			r = mbin_xor2_mul_64(r, x);
		x = mbin_xor2_mul_64(x, x);
		y /= 2;
	}
	return (r);
}

uint64_t
mbin_xor2_exp_mod_any_64(uint64_t x, uint64_t y, uint64_t p)
{
	uint64_t r = 1;

	while (y) {
		if (y & 1)
			r = mbin_xor2_mul_mod_any_64(r, x, p);
		x = mbin_xor2_mul_mod_any_64(x, x, p);
		y /= 2;
	}
	return (r);
}

uint32_t
mbin_xor2_exp_mod_any_32(uint32_t x, uint32_t y, uint32_t p)
{
	uint32_t r = 1;

	while (y) {
		if (y & 1)
			r = mbin_xor2_mul_mod_any_32(r, x, p);
		x = mbin_xor2_mul_mod_any_32(x, x, p);
		y /= 2;
	}
	return (r);
}

uint64_t
mbin_xor2_neg_mod_64(uint64_t x, uint8_t p)
{
	uint8_t n = p - 2;
	uint64_t r = 1;

	while (n--) {
		x = mbin_xor2_mul_mod_64(x, x, p);
		r = mbin_xor2_mul_mod_64(r, x, p);
	}
	return (r);
}

uint64_t
mbin_xor2_log_mod_64(uint64_t x, uint8_t p)
{
	uint64_t mask = (1ULL << p) - 1ULL;
	uint64_t r = mask ^ 1ULL;
	uint64_t y = 0;

	/* Check for NUL */
	if (x == 0)
		return (0);

	if (x & 1)
		x = ~x & mask;
	else
		x = x & mask;

	while (x != r) {
		r = mbin_xor2_mul_mod_64(r, p, p);
		if (r & 1)
			r = ~r & mask;
		if (y > mask)
			return (0);
		y++;
	}
	return (y);
}

uint64_t
mbin_xor2_mul_64(uint64_t x, uint64_t y)
{
	uint64_t temp[4];
	uint64_t r = 0;
	uint8_t n;

	/* optimise */
	temp[0] = 0;
	temp[1] = x;
	temp[2] = 2 * x;
	temp[3] = x ^ (2 * x);

	for (n = 0; n != 64; n += 2) {
		r ^= temp[y & 3] << n;
		y /= 4;
	}
	return (r);
}

uint64_t
mbin_xor2_mul_mod_64(uint64_t x, uint64_t y, uint8_t p)
{
	uint64_t temp[4];
	uint64_t rl;
	uint64_t rh = 0;
	uint64_t z;
	uint8_t n;

	/* optimise */
	temp[0] = 0;
	temp[1] = x;
	temp[2] = 2 * x;
	temp[3] = x ^ (2 * x);

	rl = temp[y & 3];
	y /= 4;

	for (n = 2; n < p; n += 2) {
		z = temp[y & 3];
		rl ^= z << n;
		rh ^= z >> (64 - n);
		y /= 4;
	}

	n = (64 % p);

	if (n != 0)
		rh = (rh >> n) | (rh << (64 - n));

	rl ^= rh;

	do {
		z = (rl >> p);
		rl = (rl & ((1ULL << p) - 1ULL)) ^ z;
	} while (z);

	return (rl);
}

uint64_t
mbin_xor2_mul_mod_any_64(uint64_t x, uint64_t y, uint64_t mod)
{
	uint64_t temp[4];
	uint64_t r = 0;
	uint8_t n;

	/* optimise */
	temp[0] = 0;
	temp[1] = x;
	temp[2] = 2 * x;
	temp[3] = x ^ (2 * x);

	for (n = 0; n != 64; n += 2) {
		r ^= temp[y & 3] << n;
		y /= 4;
	}
	return (mbin_xor2_mod_64(r, mod));
}

uint32_t
mbin_xor2_mul_mod_any_32(uint32_t x, uint32_t y, uint32_t mod)
{
	uint32_t temp[4];
	uint32_t r = 0;
	uint8_t n;

	/* optimise */
	temp[0] = 0;
	temp[1] = x;
	temp[2] = 2 * x;
	temp[3] = x ^ (2 * x);

	for (n = 0; n != 32; n += 2) {
		r ^= temp[y & 3] << n;
		y /= 4;
	}
	return (mbin_xor2_mod_32(r, mod));
}

uint64_t
mbin_xor2_mod_64(uint64_t x, uint64_t div)
{
	uint64_t msb = mbin_msb64(div);
	uint64_t temp[4];
	uint8_t n;
	uint8_t shift;

	if (x < msb)
		return (x);

	shift = 62 - mbin_sumbits64(msb - 1);

	/* optimise */
	temp[0] = 0;
	temp[1] = div << shift;
	temp[2] = (2 * temp[1]);
	temp[3] = (2 * temp[1]);

	if (temp[2] & (1ULL << 62))
		temp[2] ^= temp[1];
	else
		temp[3] ^= temp[1];

	for (n = 0; n <= shift; n += 2) {
		x ^= temp[(x >> 62) & 3];
		x *= 4;
	}

	if ((shift & 1) && (x & (1ULL << 63)))
		x ^= 2 * temp[1];

	x >>= n;

	return (x);
}

uint32_t
mbin_xor2_mod_32(uint32_t x, uint32_t div)
{
	uint32_t msb = mbin_msb32(div);
	uint32_t temp[4];
	uint8_t n;
	uint8_t shift;

	if (x < msb)
		return (x);

	shift = 30 - mbin_sumbits32(msb - 1);

	/* optimise */
	temp[0] = 0;
	temp[1] = div << shift;
	temp[2] = (2 * temp[1]);
	temp[3] = (2 * temp[1]);

	if (temp[2] & (1ULL << 30))
		temp[2] ^= temp[1];
	else
		temp[3] ^= temp[1];

	for (n = 0; n <= shift; n += 2) {
		x ^= temp[(x >> 30) & 3];
		x *= 4;
	}

	if ((shift & 1) && (x & (1U << 31)))
		x ^= 2 * temp[1];

	x >>= n;

	return (x);
}

uint64_t
mbin_xor2_div_64(uint64_t x, uint64_t div)
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
mbin_xor2_div_odd_64(uint64_t x, uint64_t div)
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
mbin_xor2_lin_mul_64(uint64_t x, uint64_t y, uint8_t p)
{
	x = mbin_xor2_exp3_mod_64(1, x, p);
	y = mbin_xor2_exp_mod_64(x, y, p);
	return (mbin_xor2_log3_mod_64(y, p));
}

uint64_t
mbin_xor2_bit_reduce_mod_64(uint64_t x, uint8_t p)
{
	if (mbin_sumbits64(x) > (p / 2))
		x = x ^ ((1ULL << p) - 1ULL);
	return (x);
}

uint64_t
mbin_xor2_factor_slow(uint64_t x)
{
	uint64_t y;
	uint64_t z;

	if (x != 0 && !(x & 1))
		return (2);

	z = 2 * mbin_sqrt_64(x);

	for (y = 3; y <= z; y += 2) {
		if (mbin_xor2_mod_64(x, y) == 0)
			return (y);
	}
	return (0);
}

uint8_t
mbin_xor2_find_mod_64(uint32_t *pbit)
{
	uint32_t x;
	uint32_t y;
	uint32_t power = *pbit;

	while (1) {
		if (power >= (1U << 24))
			return (1);	/* not found */
		/*
		 * Find a suitable modulus,
		 * typically two power of a prime:
		 */
		y = 1;
		x = 0;
		while (1) {
			y *= 2;
			if (y >= power)
				y -= power;
			x++;
			if (y == 1)
				break;
			if (x == power)
				break;
		}
		if ((x == (power - 1)) && (y == 1))
			break;

		power++;
	}
	*pbit = power;

	return (0);
}

/*
 * The following function is used to solve binary matrices.
 */

uint8_t
mbin_xor2_inv_mat_mod_any_32(uint32_t *table,
    struct mbin_poly_32 *poly, uint32_t size)
{
	uint32_t temp;
	uint32_t x;
	uint32_t y;
	uint32_t z;
	uint32_t u;
	uint8_t retval = 0;

	/* invert matrix */

	for (y = 0; y != size; y++) {

		/* find non-zero entry in row */

		for (x = 0; x != size; x++) {
			if (table[(size * x) + y] != 0)
				goto found_non_zero;
		}

		retval = 1;		/* failure */
		continue;

found_non_zero:

		/* normalise row */

		temp = table[(size * x) + y];

		/* invert temp - negative */

		temp = mbin_xor2_exp_mod_any_32(temp,
		    poly->length - 1, poly->poly);

		for (z = 0; z != (2 * size); z++) {
			table[(size * z) + y] =
			    mbin_xor2_mul_mod_any_32(table[(size * z) + y],
			    temp, poly->poly);
		}

		table[(size * x) + y] = 1;

		/* subtract row */

		for (z = 0; z != size; z++) {
			if ((z != y) && (table[(size * x) + z] != 0)) {
				temp = table[(size * x) + z];
				for (u = 0; u != (2 * size); u++) {
					table[(size * u) + z] =
					    table[(size * u) + z] ^
					    mbin_xor2_mul_mod_any_32(temp,
					    table[(size * u) + y], poly->poly);
				}
				if (table[(size * x) + z] != 0)
					return (2);	/* failure */
			}
		}
	}

	/* sort matrix */

	for (y = 0; y != size; y++) {
		for (x = 0; x != size; x++) {
			if (table[(size * x) + y] != 0) {
				if (x != y) {
					/* wrong order - swap */
					for (z = 0; z != (2 * size); z++) {
						temp = table[(size * z) + x];
						table[(size * z) + x] =
						    table[(size * z) + y];
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

uint64_t
mbin_xor2_faculty_64(uint64_t n)
{
	uint64_t r = 1;

	while (n) {
		r = mbin_xor2_mul_64(r, n);
		n--;
	}
	return (r);
}

uint64_t
mbin_xor2_coeff_64(int64_t n, int64_t x)
{
	uint64_t shift = 1ULL << 32;
	uint64_t lsb;
	uint64_t y;
	uint64_t fa = 1ULL;
	uint64_t fb = 1ULL;

	/* check limits */
	if ((n < 0) || (x < 0))
		return (0);

	if (x > n)
		return (0);

	/* handle special case */
	if (x == n || x == 0)
		return (1ULL);

	for (y = 0; y != x; y++) {
		lsb = (y - n) & (n - y);
		shift *= lsb;
		fa = mbin_xor2_mul_64(fa, (n - y) / lsb);

		lsb = (-y - 1) & (y + 1);
		shift /= lsb;
		fb = mbin_xor2_mul_64(fb, (y + 1) / lsb);
	}
	return (mbin_xor2_mul_64(mbin_xor2_div_odd_64(fa, fb), (shift >> 32)));
}

/* greatest common divisor for CRC's (modified Euclid equation) */
uint64_t
mbin_xor2_gcd_64(uint64_t a, uint64_t b)
{
	uint64_t an;
	uint64_t bn;

	while (b != 0) {
		an = b;
		bn = mbin_xor2_mod_64(a, b);
		a = an;
		b = bn;
	}
	return (a);
}

/* extended Euclidean equation for CRC */
void
mbin_xor2_gcd_extended_64(uint64_t a, uint64_t b, uint64_t *pa, uint64_t *pb)
{
	uint64_t x = 0;
	uint64_t y = 1;
	uint64_t lastx = 1;
	uint64_t lasty = 0;
	uint64_t q;
	uint64_t an;
	uint64_t bn;

	while (b != 0) {
		q = mbin_xor2_div_64(a, b);
		an = b;
		bn = mbin_xor2_mod_64(a, b);
		a = an;
		b = bn;
		an = lastx ^ mbin_xor2_mul_64(q, x);
		bn = x;
		x = an;
		lastx = bn;
		an = lasty ^ mbin_xor2_mul_64(q, y);
		bn = y;
		y = an;
		lasty = bn;
	}
	*pa = lastx;
	*pb = lasty;
}

void
mbin_xor_print_mat_32(const uint32_t *table, uint32_t size, uint8_t print_invert)
{
	uint32_t temp;
	uint32_t x;
	uint32_t y;
	uint32_t off;
	uint8_t no_solution = 0;

	off = print_invert ? size : 0;

	for (y = 0; y != size; y++) {
		printf("0x%02x | ", y);

		for (x = 0; x != size; x++) {
			temp = table[((x + off) * size) + y];

			printf("0x%08x", temp);

			if (x != (size - 1))
				printf(", ");
		}
		printf(";\n");
	}
	if (no_solution)
		printf("this matrix has no solution due to undefined bits!\n");
}

/*
 * ========================================================================
 *                              BASE-3 XOR
 * ========================================================================
 */

#define	MBIN_XOR3_POLY(p,q) ((1ULL << (p)) | (1ULL << (q)) | (((p) & 2) ? 1ULL : 2ULL))

uint64_t
mbin_xor3_64(uint64_t a, uint64_t b)
{
	const uint64_t K = 0x5555555555555555ULL;
	uint64_t d;
	uint64_t e;

	d = b & K;
	e = (b & ~K) / 2;

	a += d;
	b = a & (a / 2) & K;
	a ^= b | (2 * b);

	a += e;
	b = a & (a / 2) & K;
	a ^= b | (2 * b);

	a += e;
	b = a & (a / 2) & K;
	a ^= b | (2 * b);

	return (a);
}

uint64_t
mbin_xor3_mul_mod_64(uint64_t x, uint64_t y, uint8_t p, uint8_t q)
{
	uint64_t poly = MBIN_XOR3_POLY(p, q);
	uint64_t r = 0;
	uint8_t n;

	for (n = 0; n != 64; n += 2) {
		uint8_t m = (y >> n) & 3;

		while (m--)
			r = mbin_xor3_64(r, x);

		x <<= 2;
		while (x & (3ULL << p))
			x = mbin_xor3_64(x, poly);
	}
	return (r);
}

uint64_t
mbin_xor3_exp_slow_64(uint64_t x, uint64_t y)
{
	uint64_t r = 1;

	while (y) {
		if (y & 1)
			r = mbin_xor3_mul_64(r, x);
		x = mbin_xor3_mul_64(x, x);
		y /= 2;
	}
	return (r);
}

uint64_t
mbin_xor3_qubic_64(uint64_t x)
{
	uint64_t r = 0;
	uint8_t n;

	for (n = 0; n != 16; n++)
		r |= (x & (3ULL << (2 * n))) << (4 * n);

	return (r);
}

uint64_t
mbin_xor3_qubic_mod_64(uint64_t x, uint64_t p)
{
	uint64_t r = 0;
	uint64_t m = 1;
	uint64_t msb;

	msb = mbin_msb64(p);
	if (msb & 0xAAAAAAAAAAAAAAAAULL)
		msb /= 2ULL;

	msb *= 3ULL;

	while (x) {
		while (x & 3) {
			r = mbin_xor3_64(r, m);
			x--;
		}

		x >>= 2;

		m <<= 2;
		while (m & msb)
			m = mbin_xor3_64(m, p);

		m <<= 2;
		while (m & msb)
			m = mbin_xor3_64(m, p);

		m <<= 2;
		while (m & msb)
			m = mbin_xor3_64(m, p);
	}
	return (r);
}

uint64_t
mbin_xor3_exp_64(uint64_t x, uint64_t y)
{
	uint64_t r = 1;

	while (y) {
		while (y % 3) {
			r = mbin_xor3_mul_64(r, x);
			y--;
		}
		x = mbin_xor3_qubic_64(x);
		y /= 3;
	}
	return (r);
}

uint64_t
mbin_xor3_mod_64(uint64_t rem, uint64_t mod)
{
	uint8_t sr;
	uint8_t sm;

	if (rem == 0 || mod == 0)
		return (0);

	sr = mbin_sumbits32(mbin_msb32(rem) - 1);
	sr &= ~1;

	sm = mbin_sumbits32(mbin_msb32(mod) - 1);
	sm &= ~1;

	if (sm > sr)
		return (rem);

	while (1) {
		while (rem & (3 << sr))
			rem = mbin_xor3_64(rem, mod << (sr - sm));
		if (sr == sm)
			break;
		sr -= 2;
	}
	return (rem);
}

uint64_t
mbin_xor3_div_64(uint64_t rem, uint64_t div)
{
	uint64_t r = 0;
	uint8_t sr;
	uint8_t sd;

	if (rem == 0 || div == 0)
		return (0);

	sr = mbin_sumbits32(mbin_msb32(rem) - 1);
	sr &= ~1;

	sd = mbin_sumbits32(mbin_msb32(div) - 1);
	sd &= ~1;

	if (sd > sr)
		return (0);

	while (1) {
		while (rem & (3 << sr)) {
			rem = mbin_xor3_64(rem, div << (sr - sd));
			r = mbin_xor3_64(r, 2 << (sr - sd));
		}
		if (sr == sd)
			break;
		sr -= 2;
	}
	return (r);
}

uint64_t
mbin_xor3_mul_64(uint64_t x, uint64_t y)
{
	uint64_t r = 0;
	uint8_t n;

	for (n = 0; n != 64; n += 2) {
		uint8_t m = (y >> n) & 3;

		while (m--)
			r = mbin_xor3_64(r, x);

		x <<= 2;
	}
	return (r);
}

uint64_t
mbin_xor3_mul_mod_any_64(uint64_t x, uint64_t y, uint64_t p)
{
	uint64_t r = 0;
	uint64_t msb = mbin_msb64(p);
	uint8_t n;

	if (msb & 0xAAAAAAAAAAAAAAAAULL)
		msb /= 2;

	msb = 3ULL * msb;

	for (n = 0; n != 64; n += 2) {
		uint8_t m = (y >> n) & 3;

		while (m--)
			r = mbin_xor3_64(r, x);

		x <<= 2;
		while (x & msb)
			x = mbin_xor3_64(x, p);
	}
	return (r);
}

uint64_t
mbin_xor3_factor_slow(uint64_t x)
{
	uint64_t y;
	uint64_t z;

	for (y = 1; y <= x; y++) {
		z = (2 * y) | 1;
		if (z & (z / 2) & 0x5555555555555555ULL)
			continue;
		if (mbin_xor3_mod_64(x, z) == 0)
			return (z);
	}
	return (0);
}

uint32_t
mbin_xor3_mul_mod_any_32(uint32_t x, uint32_t y, uint32_t p)
{
	uint32_t r = 0;
	uint32_t msb = mbin_msb32(p);
	uint8_t n;

	if (msb & 0xAAAAAAAAUL)
		msb /= 2;

	msb = 3 * msb;

	for (n = 0; n != 32; n += 2) {
		uint8_t m = (y >> n) & 3;

		while (m--)
			r = mbin_xor3_32(r, x);

		x <<= 2;
		while (x & msb)
			x = mbin_xor3_32(x, p);
	}
	return (r);
}

uint64_t
mbin_xor3_exp_mod_64(uint64_t x, uint64_t y, uint8_t p, uint8_t q)
{
	uint64_t r = 1;

	while (y) {
		if (y & 1)
			r = mbin_xor3_mul_mod_64(r, x, p, q);
		x = mbin_xor3_mul_mod_64(x, x, p, q);
		y /= 2;
	}
	return (r);
}

uint64_t
mbin_xor3_exp_slow_mod_any_64(uint64_t x, uint64_t y, uint64_t p)
{
	uint64_t r = 1;

	while (y) {
		if (y & 1)
			r = mbin_xor3_mul_mod_any_64(r, x, p);
		x = mbin_xor3_mul_mod_any_64(x, x, p);
		y /= 2;
	}
	return (r);
}

uint64_t
mbin_xor3_exp_mod_any_64(uint64_t x, uint64_t y, uint64_t p)
{
	uint64_t r = 1;

	while (y) {
		while (y % 3) {
			r = mbin_xor3_mul_mod_any_64(r, x, p);
			y--;
		}
		x = mbin_xor3_qubic_mod_64(x, p);
		y /= 3;
	}
	return (r);
}

uint64_t
mbin_xor3_neg_mod_64(uint64_t x, uint8_t p, uint8_t q)
{
	uint64_t len;
	uint8_t n;

	for (len = 1ULL, n = 0; n != p; n += 2)
		len *= 3ULL;

	len -= 2;

	return (mbin_xor3_exp_mod_64(x, len, p, q));
}

uint8_t
mbin_xor3_find_mod_64(uint8_t *pbit, uint8_t *qbit, uint64_t *plen)
{
	uint64_t len;
	uint8_t power = *pbit;
	uint8_t q;
	uint8_t x;

	if (power < 4)
		return (1);

	if (power & 1)
		power++;
	if (~power & 2)
		power += 2;

	for (len = 1, x = 0; x != power; x += 2)
		len *= 3ULL;

	for (q = 2; q < power; q++) {
		if (mbin_xor3_exp_mod_64(4, len, power, q) == 4)
			break;
	}
	if (q >= power)
		return (1);

	*pbit = power;
	*qbit = q;

	if (plen)
		*plen = len;
	return (0);
}

/*
 * ========================================================================
 *                              BASE-2 VECTOR XOR
 * ========================================================================
 */

struct mbin_xor2v_64 mbin_xor2v_zero_64 = {0, 1};
struct mbin_xor2v_64 mbin_xor2v_unit_64 = {1, 3};
struct mbin_xor2v_64 mbin_xor2v_nega_64 = {1, 0};

struct mbin_xor2v_32 mbin_xor2v_zero_32 = {0, 1};
struct mbin_xor2v_32 mbin_xor2v_unit_32 = {1, 3};
struct mbin_xor2v_32 mbin_xor2v_nega_32 = {1, 0};

static uint64_t
mbin_xor2_ungrey_mod_64(uint64_t x, uint8_t p)
{
	uint8_t q;

	/* assuming that input has even parity */

	for (q = 0; q != (p - 1); q++) {
		if (x & (1ULL << q))
			x ^= (2ULL << q);
	}
	return (x);
}

struct mbin_xor2v_64
mbin_xor2v_mul_mod_64(struct mbin_xor2v_64 x, struct mbin_xor2v_64 y, uint8_t p)
{
	struct mbin_xor2v_64 t;

	uint64_t val;

	val = y.a1 ^ y.a0 ^ mbin_xor2_rol_mod_64(y.a0, 1, p);

	t.a0 = mbin_xor2_mul_mod_64(x.a0, val, p) ^
	    mbin_xor2_mul_mod_64(x.a1, y.a0, p);
	t.a1 = mbin_xor2_mul_mod_64(x.a0, y.a0, p) ^
	    mbin_xor2_mul_mod_64(x.a1, y.a1, p);

	return (t);
}

struct mbin_xor2v_64
mbin_xor2v_mul_mod_any_64(struct mbin_xor2v_64 x, struct mbin_xor2v_64 y,
    uint64_t p)
{
	struct mbin_xor2v_64 t;

	uint64_t val;

	val = y.a1 ^ mbin_xor2_mul_mod_any_64(y.a0, 3, p);

	t.a0 = mbin_xor2_mul_mod_any_64(x.a0, val, p) ^
	    mbin_xor2_mul_mod_any_64(x.a1, y.a0, p);
	t.a1 = mbin_xor2_mul_mod_any_64(x.a0, y.a0, p) ^
	    mbin_xor2_mul_mod_any_64(x.a1, y.a1, p);

	return (t);
}

struct mbin_xor2v_32
mbin_xor2v_mul_mod_any_32(struct mbin_xor2v_32 x, struct mbin_xor2v_32 y,
    uint32_t p)
{
	struct mbin_xor2v_32 t;

	uint32_t val;

	val = y.a1 ^ mbin_xor2_mul_mod_any_32(y.a0, 3, p);

	t.a0 = mbin_xor2_mul_mod_any_32(x.a0, val, p) ^
	    mbin_xor2_mul_mod_any_32(x.a1, y.a0, p);
	t.a1 = mbin_xor2_mul_mod_any_32(x.a0, y.a0, p) ^
	    mbin_xor2_mul_mod_any_32(x.a1, y.a1, p);

	return (t);
}

struct mbin_xor2v_64
mbin_xor2v_square_mod_64(struct mbin_xor2v_64 x, uint8_t p)
{
	struct mbin_xor2v_64 t;

	uint64_t val;

	val = mbin_xor2_square_mod_64(x.a0, p);

	t.a0 = val ^ mbin_xor2_rol_mod_64(val, 1, p);
	t.a1 = val ^ mbin_xor2_square_mod_64(x.a1, p);

	return (t);
}

struct mbin_xor2v_64
mbin_xor2v_root_mod_64(struct mbin_xor2v_64 x, uint8_t p)
{
	struct mbin_xor2v_64 t;

	/* assuming that "x.a0" has even parity */
	t.a0 = mbin_xor2_ungrey_mod_64(x.a0, p);
	t.a0 = mbin_xor2_root_mod_64(t.a0, p);
	t.a1 = t.a0 ^ mbin_xor2_root_mod_64(x.a1, p);
#if 0
	/* the other solution */
	t.a0 ^= (1ULL << p) - 1ULL;
	t.a1 ^= (1ULL << p) - 1ULL;
#endif
	return (t);
}

uint64_t
mbin_xor2v_log_mod_64(struct mbin_xor2v_64 x, uint8_t p)
{
	uint64_t r = 0;

	/* TODO: Optimise */

	while (x.a0 != mbin_xor2v_zero_64.a0 ||
	    x.a1 != mbin_xor2v_zero_64.a1) {

		x = mbin_xor2v_mul_mod_64(x, mbin_xor2v_nega_64, p);
		r++;
	}
	return (r);
}

struct mbin_xor2v_64
mbin_xor2v_neg_mod_64(struct mbin_xor2v_64 x, uint8_t p)
{
	struct mbin_xor2v_64 t;

	/* simply swap */

	t.a0 = x.a1;
	t.a1 = x.a0;

	/* adjust by adding one */

	t = mbin_xor2v_mul_mod_64(t, mbin_xor2v_unit_64, p);

	return (t);

}

struct mbin_xor2v_32
mbin_xor2v_neg_mod_any_32(struct mbin_xor2v_32 x, uint32_t p)
{
	struct mbin_xor2v_32 t;

	/* simply swap */

	t.a0 = x.a1;
	t.a1 = x.a0;

	/* adjust by adding one */

	t = mbin_xor2v_mul_mod_any_32(t, mbin_xor2v_unit_32, p);

	return (t);

}

struct mbin_xor2v_64
mbin_xor2v_neg_sub_unit_mod_64(struct mbin_xor2v_64 x, uint8_t p)
{
	struct mbin_xor2v_64 t;

	/* simply swap */

	t.a0 = x.a1;
	t.a1 = x.a0;

	return (t);
}

struct mbin_xor2v_64
mbin_xor2v_exp_mod_64(struct mbin_xor2v_64 x, uint64_t y, uint8_t p)
{
	struct mbin_xor2v_64 r = mbin_xor2v_zero_64;

	while (y) {
		if (y & 1)
			r = mbin_xor2v_mul_mod_64(r, x, p);
		x = mbin_xor2v_mul_mod_64(x, x, p);
		y /= 2;
	}
	return (r);
}

struct mbin_xor2v_32
mbin_xor2v_exp_mod_any_32(struct mbin_xor2v_32 x, uint32_t y, uint32_t p)
{
	struct mbin_xor2v_32 r = mbin_xor2v_zero_32;

	while (y) {
		if (y & 1)
			r = mbin_xor2v_mul_mod_any_32(r, x, p);
		x = mbin_xor2v_mul_mod_any_32(x, x, p);
		y /= 2;
	}
	return (r);
}

struct mbin_xor2v_32
mbin_xor2v_xor_32(struct mbin_xor2v_32 x, struct mbin_xor2v_32 y)
{
	x.a0 ^= y.a0;
	x.a1 ^= y.a1;
	return (x);
}

void
mbin_xor2v_print_mat_32(const struct mbin_xor2v_32 *table,
    uint32_t size, uint8_t print_invert)
{
	struct mbin_xor2v_32 temp;
	uint32_t x;
	uint32_t y;
	uint32_t off;
	uint8_t no_solution = 0;

	off = print_invert ? size : 0;

	for (y = 0; y != size; y++) {
		printf("0x%02x | ", y);

		for (x = 0; x != size; x++) {
			temp = table[((x + off) * size) + y];

			printf("0x%08x.0x%08x", temp.a0, temp.a1);

			if (x != (size - 1))
				printf(", ");
		}
		printf(";\n");
	}
	if (no_solution)
		printf("this matrix has no solution due to undefined bits!\n");
}

/*
 * ========================================================================
 *                              BASE-4 XOR
 * ========================================================================
 */

uint32_t
mbin_xor4_32(uint32_t a, uint32_t b)
{
	const uint32_t K = 0x55555555UL;
	uint32_t d;
	uint32_t e;

	d = a ^ b;
	e = a & b & K;

	return (d ^ (2 * e));
}

uint32_t
mbin_xor4_mul_mod_any_32(uint32_t x, uint32_t y, uint32_t p)
{
	uint32_t r = 0;
	uint32_t msb = mbin_msb32(p);
	uint8_t n;

	if (msb & 0xAAAAAAAAUL)
		msb /= 2;

	msb = 3 * msb;

	for (n = 0; n != 32; n += 2) {
		uint8_t m = (y >> n) & 3;

		while (m--)
			r = mbin_xor4_32(r, x);

		x <<= 2;
		while (x & msb)
			x = mbin_xor4_32(x, p);
	}
	return (r);
}

uint32_t
mbin_xor4_exp_mod_any_32(uint32_t x, uint32_t y, uint32_t p)
{
	uint32_t r = 1;

	while (y) {
		if (y & 1)
			r = mbin_xor4_mul_mod_any_32(r, x, p);
		x = mbin_xor4_mul_mod_any_32(x, x, p);
		y /= 2;
	}
	return (r);
}
