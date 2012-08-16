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
 * This file implements helper functions for XOR multiplication,
 * division, exponent, logarithm and so on.
 */

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
mbin_xor2_log3_mod_64(uint64_t x, uint8_t p)
{
	uint64_t mask;
	uint64_t history = 0;
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

	sbx = 0;
	sby = 0;

	/* locate the two bits */

	for (r = 0; r != p; r++) {
		if (x & (1ULL << r)) {
			sbx = r;
			for (r++; r != p; r++) {
				if (x & (1ULL << r)) {
					sby = r;
					break;
				}
			}
			break;
		}
	}

	/* solve "x" into an odd number */

	while (sbx && sby) {
		if (history & (1ULL << sbx)) {
			/* we are going into a circle */
			history = 0;
			r = (p + sby - sbx) % p;
			sby = (p + (2 * sby) - sbx) % p;
		} else {
			/* try next combination */
			history |= (1ULL << sbx);
			r = (p + sbx - sby) % p;
			sbx = (p + (2 * sbx) - sby) % p;
		}

		z += (1ULL << ntable[r]);

		x = x ^ mbin_xor2_rol_mod_64(x, r, p);
	}

	/* final step */

	if (sbx > sby)
		r = sbx;
	else
		r = sby;

	/* properly MOD the "z" variable  */

	if (ntable[r])
		z = (mask + (1ULL << ntable[r]) - (z % mask)) % mask;
	else
		z = (mask - (z % mask)) % mask;

	return (z);
}

uint64_t
mbin_xor2_exp_mod_64(uint64_t x, uint64_t y, uint8_t p)
{
	uint64_t r = 1;

	while (y) {
		if (y & 1)
			r = mbin_xor2_mul_mod_64(r, x, p);
		x = mbin_xor2_mul_mod_64(x, x, p);
		y /= 2;
	}
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
mbin_xor2_neg_mod_64(uint64_t x, uint8_t p)
{
	uint8_t n = (p - 3) / 2;
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
	uint64_t msb = 1ULL << p;
	uint64_t r;

	if (x == 0)
		return (0);

	for (r = 0; r != msb; r++) {
		if (mbin_xor2_exp_mod_64(MBIN_XOR2_BASE_64(p), r, p) == x)
			break;
	}
	return (r);
}

uint64_t
mbin_xor2_mul_64(uint64_t x, uint64_t y)
{
	uint64_t r = 0;
	uint8_t n;

	for (n = 0; n != 64; n++) {
		if (y & (1ULL << n))
			r ^= x;

		x <<= 1;
	}
	return (r);
}

uint64_t
mbin_xor2_mul_mod_64(uint64_t x, uint64_t y, uint8_t p)
{
	uint64_t r = 0;
	uint8_t n;

	/* r = (a + b) */

	for (n = 0; n != 64; n++) {
		if (y & (1ULL << n))
			r ^= x;

		x <<= 1;
		if (x & (1ULL << p))
			x ^= (1ULL << p) ^ 1;
	}
	return (r);
}

uint64_t
mbin_xor2_mul_mod_any_64(uint64_t x, uint64_t y, uint64_t mod)
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
mbin_xor2_mod_64(uint64_t x, uint64_t div)
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

	for (y = 0; y <= x; y++) {
		if (mbin_xor2_div_odd_64(x, (2 * y) | 1) < x)
			return ((2 * y) | 1);
	}
	return (0);
}

uint8_t
mbin_xor2_find_mod_64(uint8_t *pbit, uint64_t *plen)
{
	uint8_t x;
	uint8_t power = *pbit;

	while (1) {
		if (power >= 64)
			return (1);	/* not found */
		for (x = 2; x != power; x++) {
			if ((power % x) == 0)
				break;
		}
		if ((x == power) && ((power + 1) & power))
			break;

		/*
	         * Criteria: Power must be a prime and not power of two
	         * minus one
	         */
		power++;
	}
	*pbit = power;

	if (plen)
		*plen = (1ULL << (power - 1) / 2) - 1ULL;
	return (0);
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
