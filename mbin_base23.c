/*-
 * Copyright (c) 2011 Hans Petter Selasky. All rights reserved.
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
 * This file contains a software implementation of the two third base
 * system and associated mathematical functions.
 */

#include <stdint.h>

#include "math_bin.h"

uint32_t
mbin_base_2to23_32(uint32_t x)
{
	uint32_t k;
	uint32_t m;
	uint32_t r;

	k = 1;
	r = 0;

	for (m = 1; m; m *= 2) {
		if (x & m) {
			x -= k;
			r |= m;
		}
		k *= 0x55555556;	/* 2/3 */
	}
	return (r);
}

uint32_t
mbin_base_23to2_32(uint32_t x)
{
	uint32_t k;
	uint32_t m;
	uint32_t r;

	k = 1;
	r = 0;

	for (m = 1; m; m *= 2) {
		if (x & m)
			r += k;

		k *= 0x55555556;	/* 2/3 */
	}
	return (r);
}

void
mbin_base23_add_32(struct mbin_base23_state32 *st, uint32_t b23)
{
	uint32_t g;
	uint32_t h;

	/* reset carry out */
	g = 0;

	/* add "b23" */
	h = (st->a[0] & b23);
	st->a[0] ^= b23;
	g |= st->a[1] & h;
	st->a[1] ^= h;

	/* add carry in */
	g |= st->a[1] & st->a[2];
	st->a[1] ^= st->a[2];

	/* add "g" */
	h = (st->a[0] & g);
	st->a[0] ^= g;
	st->a[1] ^= h;

	/* carry down */
	st->a[2] = g / 2;
}

void
mbin_base23_clean_carry_32(struct mbin_base23_state32 *st)
{
	while (st->a[2])
		mbin_base23_add_32(st, 0);
}

void
mbin_base23_mul_32(uint32_t a, uint32_t b, struct mbin_base23_state32 *st)
{
	uint8_t x;

	st->a[0] = 0;
	st->a[1] = 0;
	st->a[2] = 0;

	for (x = 0; x != 32; x++) {
		if (a & (1 << x))
			mbin_base23_add_32(st, b << x);
	}
}

uint32_t
mbin_base23_to_linear(struct mbin_base23_state32 *st)
{
	return (mbin_base_23to2_32(st->a[0]) +
	    (2 * (mbin_base_23to2_32(st->a[1]) +
	    mbin_base_23to2_32(st->a[2]))));
}

void
mbin_base23_from_linear(struct mbin_base23_state32 *st, uint32_t v)
{
	st->a[0] = mbin_base_2to23_32(v);
	st->a[1] = 0;
	st->a[2] = 0;
}
