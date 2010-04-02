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
 * This file implements a fast transform that can be used for fast
 * integer correlation.
 */

#include <stdint.h>

#include <stdlib.h>

#include <string.h>

#include "math_bin.h"

struct mbin_fet_32 {
	uint32_t k_fact[32];
	uint32_t n_exp;
	uint32_t mod_value;
	uint32_t m;
	uint32_t p;
	uint32_t corr_f;
	uint32_t k_exp[0];
};

static void
mbin_fet_32_factor(uint32_t n, uint32_t *factbuf)
{
	uint32_t p;

	if (n == 0)
		return;

	while (!(n & 1)) {
		n /= 2;

		*factbuf++ = 2;
		*factbuf++ = n;
	}

	if (n == 1)
		return;

	while (1) {

		for (p = 3;; p += 2) {

			if ((n % p) == 0) {
				n /= p;

				*factbuf++ = p;
				*factbuf++ = n;

				if (n == 1)
					return;
				break;
			}
		}
	}
}

struct mbin_fet_32 *
mbin_fet_32_init_alloc(uint32_t base, uint32_t nexp, uint32_t mod)
{
	struct mbin_fet_32 *pfet;
	uint64_t temp;
	uint32_t size;
	uint32_t n;

	size = sizeof(*pfet) + (nexp * sizeof(uint32_t));

	pfet = malloc(size);

	if (pfet == NULL)
		return (NULL);

	memset(pfet, 0, sizeof(*pfet));

	pfet->n_exp = nexp;

	pfet->mod_value = mod;

	pfet->corr_f = mbin_power_mod_32(nexp, mod - 2, mod);

	mbin_fet_32_factor(nexp, pfet->k_fact);

	temp = 1;

	for (n = 0; n != nexp; n++) {
		pfet->k_exp[n] = temp;
		temp *= base;
		temp %= mod;
	}

	if (temp != 1) {
		/* not periodic */
		free(pfet);
		return (NULL);
	}
	return (pfet);
}

uint32_t *
mbin_fet_32_ds_alloc(struct mbin_fet_32 *pfet)
{
	uint32_t *ptr;
	uint32_t size;

	size = sizeof(uint32_t) * pfet->n_exp;

	ptr = malloc(size);
	if (ptr)
		memset(ptr, 0, size);

	return (ptr);
}

/*
 * This function is inspired by the KISS FFT library written by:
 * Mark Borgerding
 */

static void mbin_fet_32_generic(struct mbin_fet_32 *pfet, uint32_t *p_out, const uint32_t fstride);

static void mbin_fet_32_2(struct mbin_fet_32 *pfet, uint32_t *p_out, const uint32_t fstride);

static void
mbin_fet_32_work(
    struct mbin_fet_32 *pfet,
    uint32_t *p_out,
    const uint32_t *f,
    const uint32_t factor,
    const uint32_t fstride)
{
	uint32_t *p_out_beg;
	uint32_t *p_out_end;
	uint32_t p;
	uint32_t m;

	p = pfet->k_fact[factor];
	m = pfet->k_fact[factor + 1];

	p_out_beg = p_out;
	p_out_end = p_out + (p * m);

	if (m == 1) {
		do {
			*p_out = *f;

			f += fstride;

			p_out++;

		} while (p_out != p_out_end);
	} else {
		do {

			mbin_fet_32_work(pfet, p_out, f, factor + 2, fstride * p);

			f += fstride;
			p_out += m;

		} while (p_out != p_out_end);
	}

	pfet->p = p;
	pfet->m = m;

	switch (p) {
	case 2:
		mbin_fet_32_2(pfet, p_out_beg, fstride);
		break;
	default:
		mbin_fet_32_generic(pfet, p_out_beg, fstride);
		break;
	}
}

static void
mbin_fet_32_generic(struct mbin_fet_32 *pfet, uint32_t *p_out, const uint32_t fstride)
{
	uint32_t u;
	uint32_t k;
	uint32_t a;
	uint32_t b;
	uint32_t *scratchbuf;
	uint64_t t0;
	uint64_t t1;

	scratchbuf = alloca(sizeof(uint32_t) * pfet->p);

	for (u = 0; u != pfet->m; u++) {
		k = u;
		for (a = 0; a != pfet->p; a++) {
			scratchbuf[a] = p_out[k];
			k += pfet->m;
		}

		k = u;
		for (a = 0; a != pfet->p; a++) {
			uint32_t i_exp = 0;

			t0 = scratchbuf[0];
			t1 = 0;
			for (b = 1; b != pfet->p; b++) {
				uint64_t t;

				i_exp += fstride * k;
				if (i_exp >= pfet->n_exp)
					i_exp -= pfet->n_exp;
				t = (((uint64_t)scratchbuf[b]) * ((uint64_t)pfet->k_exp[i_exp]));
				t0 += (uint32_t)(t);
				t1 += (uint32_t)(t >> 32);
			}

			t1 %= pfet->mod_value;
			t0 %= pfet->mod_value;

			t0 += t1 << 32;
			t0 %= pfet->mod_value;

			p_out[k] = t0;

			k += pfet->m;
		}
	}
}

static void
mbin_fet_32_2(struct mbin_fet_32 *pfet, uint32_t *p_out, const uint32_t fstride)
{
	uint32_t *p_out_m;
	uint32_t *p_out_end;
	uint32_t *p_exp = pfet->k_exp;
	uint64_t s;

	s = ((uint64_t)pfet->mod_value) * ((uint64_t)pfet->mod_value);

	p_out_end = p_out_m = p_out + pfet->m;

	while (p_out != p_out_end) {

		uint64_t t;

		t = ((uint64_t)*p_out_m) * ((uint64_t)*p_exp);

		p_exp += fstride;

		*p_out_m = (s - t + ((uint64_t)*p_out)) % pfet->mod_value;
		p_out_m++;

		*p_out = (t + ((uint64_t)*p_out)) % pfet->mod_value;
		p_out++;
	}
}

void
mbin_fet_32(struct mbin_fet_32 *pfet, const uint32_t *fin, uint32_t *fout)
{
	mbin_fet_32_work(pfet, fout, fin, 0, 1);
}


void
mbin_fet_32_correlate(struct mbin_fet_32 *pfet,
    const uint32_t *pa,
    const uint32_t *pb,
    uint32_t *pc,
    uint32_t *ta,
    uint32_t *tb,
    uint32_t *tc)
{
	uint32_t x;
	uint32_t mod;
	uint64_t mod_s;

	mbin_fet_32(pfet, pa, ta);
	mbin_fet_32(pfet, pb, tb);

	mod = pfet->mod_value;

	mod_s = (((uint64_t)mod) * ((uint64_t)mod));

	pc[0] = (((uint64_t)ta[0]) * ((uint64_t)tb[0])) % mod;

	for (x = 1; x != pfet->n_exp; x++) {
		pc[x] = (((uint64_t)ta[x]) * ((uint64_t)tb[pfet->n_exp - x])) % mod;
	}

	mbin_fet_32(pfet, pc, tc);

	/* adjust output */
	for (x = 0; x != pfet->n_exp; x++) {
		tc[x] = (((uint64_t)tc[x]) * ((uint64_t)pfet->corr_f)) % mod;
	}
}

uint8_t
mbin_fet_16_multiply(
    uint8_t *pa, uint16_t abits,
    uint8_t *pb, uint16_t bbits,
    uint8_t *pc, uint16_t cbits)
{

	return (0);			/* success */
}

void
mbin_fet_32_find_mod(struct mbin_fet_32_mod *pmod)
{
	/* search for a suitable prime */

	uint32_t max;
	uint32_t x;
	uint32_t y;

	pmod->mod |= 1;
	pmod->mod += 2;

	if (pmod->mod < 11)
		pmod->mod = 11;

	if (pmod->base == 0)
		pmod->base = 3;

	for (; pmod->mod != 0xFFFFFFFFU; pmod->mod += 2) {

		if (!(pmod->mod % 3U))
			continue;

		if (!(pmod->mod % 5U))
			continue;

		if (!(pmod->mod % 7U))
			continue;

		max = (mbin_sqrt_64(pmod->mod) | 1) + 2;

		for (x = 3; x != max; x += 2) {
			if (!(pmod->mod % x))
				break;
		}

		if (x != max)
			continue;

		/* verify the prime */

		x = 1;
		y = 0;

		while (1) {

			x = (((uint64_t)x) * ((uint64_t)pmod->base)) % pmod->mod;
			y++;
			if (x == 1)
				break;

			if (y == pmod->mod)
				goto skip;
		}

		pmod->length = y;
		pmod->corr = mbin_power_mod_32(y, pmod->mod - 2, pmod->mod);

		return;
	skip:;
	}

	pmod->length = 0;
}
