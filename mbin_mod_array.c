/*-
 * Copyright (c) 2013 Hans Petter Selasky
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

#define	U64(x) ((uint64_t)(x))

/* This function creates a mod array */
void
mbin_moda_create_32(uint32_t *mod, const uint32_t n)
{
	uint32_t x;
	uint32_t y;
	uint32_t z;

	for (y = 3, x = 0; x != n; x++) {
		mod[x] = y;

		/* find valid modulus */
		do {
			y += 2;
			for (z = 3; z != y; z += 2) {
				if ((y % z) == 0)
					break;
			}
		} while (z != y);
	}
}

/* This function computes a linear value from a set of modular values */
void
mbin_lina_by_moda_slow_32(uint32_t *ptr, const uint32_t *mod, const uint32_t n)
{
	uint32_t x;
	uint32_t y;

	for (x = 0; x != n; x++) {
		for (y = x + 1; y != n; y++) {
			ptr[y] = (U64(U64(mod[y]) + U64(ptr[y]) - U64(ptr[x])) *
			    U64(mbin_power_mod_32(mod[x],
			    mod[y] - 2, mod[y]))) % U64(mod[y]);
		}
	}
}

void
mbin_mod_table_create(const uint32_t *mod, uint32_t *table, const uint32_t n)
{
	uint32_t x;
	uint32_t y;
	uint32_t z;

	for (z = x = 0; x != n; x++) {
		for (y = x + 1; y != n; y++) {
			table[z++] = mbin_power_mod_32(mod[x], mod[y] - 2, mod[y]);
		}
	}
}

void
mbin_lina_by_moda_lookup_32(uint32_t *ptr, const uint32_t *mod, const uint32_t *table, const uint32_t n)
{
	uint32_t x;
	uint32_t y;
	uint32_t z;

	for (z = x = 0; x != n; x++) {
		for (y = x + 1; y != n; y++) {
			ptr[y] = (U64(U64(mod[y]) + U64(ptr[y]) - U64(ptr[x])) *
			    U64(table[z++])) % U64(mod[y]);
		}
	}
}

/* This function compute the modular values from a linear value */
void
mbin_moda_by_lina_slow_32(uint32_t *ptr, const uint32_t *mod, const uint32_t n)
{
	uint32_t x;
	uint32_t y;

	for (x = n - 1U; x != -1U; x--) {
		for (y = x + 1; y != n; y++) {
			ptr[y] = ((U64(ptr[y]) * U64(mod[x])) +
			    U64(ptr[x])) % U64(mod[y]);
		}
	}
}

void
mbin_moda_add_32(const uint32_t *pa, const uint32_t *pb, uint32_t *pc,
    const uint32_t *mod, const uint32_t n)
{
	uint32_t x;

	for (x = 0; x != n; x++)
		pc[x] = (U64(pa[x]) + U64(pb[x])) % U64(mod[x]);
}

void
mbin_moda_sub_32(const uint32_t *pa, const uint32_t *pb, uint32_t *pc,
    const uint32_t *mod, const uint32_t n)
{
	uint32_t x;

	for (x = 0; x != n; x++)
		pc[x] = (U64(mod[x]) + U64(pa[x]) - U64(pb[x])) % U64(mod[x]);
}

void
mbin_moda_mul_32(const uint32_t *pa, const uint32_t *pb, uint32_t *pc,
    const uint32_t *mod, const uint32_t n)
{
	uint32_t x;

	for (x = 0; x != n; x++)
		pc[x] = (U64(pa[x]) * U64(pb[x])) % U64(mod[x]);
}

/* NOTE: multiplicative inverse "division" */
void
mbin_moda_div_32(const uint32_t *pa, const uint32_t *pb, uint32_t *pc,
    const uint32_t *mod, const uint32_t n)
{
	uint32_t x;

	for (x = 0; x != n; x++) {
		pc[x] = (U64(pa[x]) * U64(mbin_power_mod_32(pb[x],
		    mod[x] - 2, mod[x]))) % U64(mod[x]);
	}
}

/* compute power */
void
mbin_moda_power_32(const uint32_t *pa, uint32_t *pc,
    const uint32_t *mod, const uint32_t power, const uint32_t n)
{
	uint32_t x;

	for (x = 0; x != n; x++)
		pc[x] = mbin_power_mod_32(pa[x], power, mod[x]);
}

uint8_t
mbin_mod_is_square_32(const uint32_t x, const uint32_t mod)
{
	uint32_t y;

	for (y = 0; y != mod; y++) {
		if (((U64(y) * U64(y)) % U64(mod)) == x)
			return (1);
	}
	return (0);
}

uint8_t
mbin_moda_is_square_32(const uint32_t *pa, const uint32_t *mod,
    const uint32_t n)
{
	uint32_t x;

	for (x = 0; x != n; x++) {
		if (!mbin_mod_is_square_32(pa[x], mod[x]))
			return (0);
	}
	return (1);
}

/* compute leading value */
uint32_t
mbin_leading_by_lina_32(const uint32_t *pa, const uint32_t *mod, const uint32_t n)
{
	uint32_t k = 1;
	uint32_t x;
	uint32_t y;

	for (y = x = 0; x != n; x++) {
		y += pa[x] * k;
		k *= mod[x];
	}
	return (y);
}

/* compute linear value */
void
mbin_lina_by_leading_32(uint32_t x, uint32_t *ptr, const uint32_t *mod, const uint32_t n)
{
	uint32_t y;

	for (y = 0; y != n; y++) {
		ptr[y] = x % mod[y];
		x /= mod[y];
	}
}

/*========================================================================*
 * XOR2 - mod array
 *========================================================================*/
static void
mbin_xor2_moda_next_mod_64(uint64_t *pmod, uint64_t k, uint64_t *plen)
{
	uint64_t mod = *pmod;
	uint64_t len;
	uint64_t gcd;

	while (1) {
		len = mbin_xor2_mod_len_slow_64(2, mod);

		if (mbin_msb64(mod / 2) != mbin_msb64(len)) {
			mod += 2;
			continue;
		}
		gcd = mbin_gcd_64(k, len);

		if (gcd != 1 || gcd == len) {
			mod += 2;
			continue;
		}
		break;
	}
	*pmod = mod;
	*plen = len;
}

void
mbin_xor2_moda_create_32(uint32_t *mod, const uint32_t start, const uint32_t n)
{
	uint32_t x;
	uint64_t y;
	uint64_t k = 1;
	uint64_t len;

	for (y = start | 1, x = 0; x != n; x++) {
		mbin_xor2_moda_next_mod_64(&y, k, &len);
		mod[x] = y;
		k *= len;
		y += 2;
	}
}

void
mbin_xor2_moda_mul_32(const uint32_t *pa, const uint32_t *pb, uint32_t *pc,
    const uint32_t *mod, const uint32_t n)
{
	uint32_t x;

	for (x = 0; x != n; x++)
		pc[x] = mbin_xor2_mul_mod_any_32(pa[x], pb[x], mod[x]);
}

void
mbin_xor2_moda_power_32(const uint32_t *pa, uint32_t *pc,
    const uint32_t *mod, const uint32_t power, const uint32_t n)
{
	uint32_t x;

	for (x = 0; x != n; x++)
		pc[x] = mbin_xor2_exp_mod_any_32(pa[x], power, mod[x]);
}
