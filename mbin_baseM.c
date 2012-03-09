/*-
 * Copyright (c) 2008,2012 Hans Petter Selasky. All rights reserved.
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
 * baseM implements a standard integer multiplier
 */

#include <stdint.h>

#include "math_bin.h"

uint32_t
mbin_baseM_next_32(uint32_t a1, uint32_t a0, uint32_t xor)
{
	uint32_t a, b, c;

	a = (1 * a1);
	b = (2 * a1);
	c = (2 * a0);

	return (xor ^ a ^ b ^ (b & c));
}

/*
 * Number base conversion from base2 to baseM.
 */
uint32_t
mbin_base_2toM_32(uint32_t b2, uint32_t xor)
{
	uint32_t f = mbin_greyB_inv32(xor);
	uint32_t r = 0;
	uint32_t x;

	b2 *= f;

	for (x = 0; x != 32; x++) {
		r |= (b2 & (1 << x));
		b2 -= f;
	}

	r ^= ~f;

	return (r);
}

/*
 * Number base conversion from baseM to base2.
 */
uint32_t
mbin_base_Mto2_32(uint32_t bm, uint32_t xor)
{
	uint32_t f = mbin_greyB_inv32(xor);
	uint32_t b2 = 0;
	uint32_t r = 0;
	uint32_t x;

	/* xor must be odd */
	if (!(f & 1))
		return (0);

	bm ^= ~f;

	for (x = 0; x != 32; x++) {

		if ((bm ^ b2) & (1 << x)) {
			b2 += f << x;
			r |= 1 << x;
		}
		b2 -= f;
	}

	return (r);
}

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

	kp = mbin_base_2toM_32(x - 1, -1);
	k = mbin_base_2toM_32(x, -1);
	ps->k = k;
	d2 = (k ^ ~kp);
	/* the MSB of "d" is not used! */
	ps->d = ((d2 & ~(d2 / 2)) ^ ~kp) & 0x7FFFFFFF;
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
