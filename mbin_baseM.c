/*-
 * Copyright (c) 2008 Hans Petter Selasky. All rights reserved.
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

/*
 * Number base conversion from base2 to baseM.
 */
uint32_t
mbin_base_2toM_32(uint32_t b2)
{
	uint32_t m = 1;
	uint32_t t = 0;
	uint32_t x;
	uint32_t xl = 1;
	uint32_t xp = 3;
	uint32_t y = 0;

	static const uint8_t t0[3] = {0, 1, 1};
	static const uint8_t t1[3] = {0, 0, 1};
	static const uint8_t t2[3] = {0, 1, 0};

	while (m) {

		if (b2 & m) {
			y = y + 1;
			if (y >= 3)
				y -= 3;
		}
		x = (b2 & ((2 * m) - 1));
		if (x < xp)
			t |= t0[y] ? (2 * m) : 0;
		else
			t |= t1[y] ? (2 * m) : 0;

		m *= 2;

		if (b2 & m) {
			y = y + 2;
			if (y >= 3)
				y -= 3;
		}
		x = (b2 & ((2 * m) - 1));
		if (x < xp)
			t |= t0[y] ? (2 * m) : 0;
		else
			t |= t2[y] ? (2 * m) : 0;

		m *= 2;

		xp = (4 * xp) - (3 * xl);
		xl = xl + 2;
	}
	t ^= b2;
	return (t);
}

/*
 * Number base conversion from baseM to base2.
 */
uint32_t
mbin_base_Mto2_32(uint32_t bm)
{
	uint32_t b2 = 0;
	uint32_t m = 1;
	uint32_t t = 0;
	uint32_t x;
	uint32_t xl = 1;
	uint32_t xp = 3;
	uint32_t y = 0;

	static const uint8_t t0[3] = {0, 1, 1};
	static const uint8_t t1[3] = {0, 0, 1};
	static const uint8_t t2[3] = {0, 1, 0};

	while (m) {

		if ((t ^ bm) & m) {
			b2 |= m;
			y = y + 1;
			if (y >= 3)
				y -= 3;
		}
		x = (b2 & ((2 * m) - 1));
		if (x < xp)
			t |= t0[y] ? (2 * m) : 0;
		else
			t |= t1[y] ? (2 * m) : 0;
		m *= 2;

		if ((t ^ bm) & m) {
			b2 |= m;
			y = y + 2;
			if (y >= 3)
				y -= 3;
		}
		x = (b2 & ((2 * m) - 1));
		if (x < xp)
			t |= t0[y] ? (2 * m) : 0;
		else
			t |= t2[y] ? (2 * m) : 0;
		m *= 2;

		xp = (4 * xp) - (3 * xl);
		xl = xl + 2;
	}
	return (b2);
}

#if 0
uint32_t
mbin_base_Mto2_32(uint32_t bm)
{
	uint32_t b2 = 0;
	uint32_t m = 1;

	while (m) {

		if ((mbin_base_2toM_32(b2) ^ bm) & m) {
			b2 ^= m;
		}
		m *= 2;
	}
	return (b2);
}

#endif

/*
 * The following function will restore the state
 * variables at index "x":
 */
void
mbin_baseM_get_state32(struct mbin_baseM_state32 *ps, uint32_t x)
{
	uint32_t kp;
	uint32_t k;
	uint32_t d2;

	kp = mbin_base_2toM_32(x - 1);
	k = mbin_base_2toM_32(x);
	ps->k = k;
	d2 = (k ^ ~kp);
	/* the MSB of "d" is not used! */
	ps->d = ((d2 & ~(d2 / 2)) ^ ~kp) & 0x7FFFFFFF;
	return;
}

/*
 * The following function will increment the state
 * variables by one.
 */
void
mbin_baseM_inc_state32(struct mbin_baseM_state32 *ps)
{
	uint32_t k;
	uint32_t d;

	k = ps->k;
	d = ps->d;

	ps->k = ((2 * d) ^ ~k);
	ps->d = (((2 * d) & ~d) ^ ~k) & 0x7FFFFFFF;

	return;
}

/*
	Extra formulas:
	==============

	b = c;
	c = (2*d) & d;
	d = (~d & ~b & ~(2*d));

	key = c ^ d;
	altkey = c - d
*/
