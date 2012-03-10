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
mbin_baseM_next_32(uint32_t a1, uint32_t a0, uint32_t xor, uint32_t pol)
{
	uint32_t a, b, c;

	a = (1 * a1);
	b = (2 * a1);
	c = (2 * a0);

	return (xor ^ a ^ (b & (pol ^ c)));
}

/*
 * Number base conversion from base2 to baseM.
 */
uint32_t
mbin_base_2toM_32(uint32_t b2, uint32_t xor, uint32_t pol)
{
	uint32_t f = mbin_greyB_inv32(xor);
	uint32_t r = 0;
	uint32_t x;

	b2 *= f;

	for (x = 0; x != 32; x++) {
		r |= (b2 & (1 << x));
		b2 -= f;
	}

	r ^= pol;
	r ^= f;

	return (r);
}

/*
 * Number base conversion from baseM to base2.
 */
uint32_t
mbin_base_Mto2_32(uint32_t bm, uint32_t xor, uint32_t pol)
{
	uint32_t f = mbin_greyB_inv32(xor);
	uint32_t b2 = 0;
	uint32_t r = 0;
	uint32_t x;

	/* xor must be odd */
	if (!(f & 1))
		return (0);

	bm ^= pol;
	bm ^= f;

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
 * The following function will restore the state variables at index
 * "x" using the given "xor":
 */
void
mbin_baseM_get_state32(struct mbin_baseM_state32 *ps, uint32_t x, uint32_t xor, uint32_t pol)
{
	uint32_t a;
	uint32_t an;
	uint32_t c;

	a = mbin_base_2toM_32(x, xor, pol);
	an = mbin_base_2toM_32(x + 1, xor, pol);
	c = a ^ an ^ xor;

	ps->a = a;
	ps->c = c;
	ps->xor = xor;
	ps->pol = pol;
}

/*
 * The following function will increment the state variables by "xor" and pol:
 */
void
mbin_baseM_inc_state32(struct mbin_baseM_state32 *ps)
{
	uint32_t a;
	uint32_t c;
	uint32_t xor;
	uint32_t pol;

	a = ps->a;
	c = ps->c;
	xor = ps->xor;
	pol = ps->pol;

	ps->a = a ^ xor ^ c;
	ps->c = 2 * ((xor ^ c) & (pol ^ a));
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
