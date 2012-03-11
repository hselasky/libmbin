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
#include <string.h>

#include "math_bin.h"

uint32_t
mbin_baseM_next_32(uint32_t a1, uint32_t a0, uint32_t xor, uint32_t pol)
{
	return (xor ^ a1 ^ (2 * ((a1 ^ a0) & (pol ^ a0))));
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
 * "x" using the given "xor" and "pol":
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
 * The following function will increment the state variables by "xor"
 * and "pol":
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

/* This function is optimised for "pol" = 0 */

uint32_t
mbin_baseM_bits_slow_32(uint32_t x, uint32_t xor)
{
	uint32_t r = mbin_base_2toM_32(x - 1, xor, 0);
	uint32_t t;
	uint32_t n;
	uint32_t m;

	for (n = 0; n != 32; n++) {
		t = xor << n;
		for (m = 0; m < n; m++) {
			t &= mbin_base_2toM_32(x - 2 - m, xor, 0) << (m + 1);
		}
		r ^= t;
	}
	return (r);
}

void
mbin_baseM_bits_init_32(struct mbin_baseM_bits32 *st, uint32_t x, uint32_t xor)
{
	uint32_t m;
	uint32_t n;
	uint32_t t;

	t = -1U;

	memset(st, 0, sizeof(*st));

	st->f = mbin_greyB_inv32(xor);
	st->a = mbin_base_2toM_32(x - 1, xor, 0);

	for (m = 0; m != 32; m++) {

		t &= mbin_base_2toM_32(x - 2 - m, xor, 0) << (m + 1);

		for (n = 0; n != 32; n++) {
			if (t & (1 << n))
				st->set[n]++;
		}
	}

	/* check bits */

	for (n = 0; n != 32; n++) {
		if ((2 * st->f) & (1 << (n - st->set[n])))
			st->c |= 1 << n;
	}
}

/*
 * This function computes the next value in the sequence like
 * "mbin_baseM_next_32()", except it uses a different statemachine.
 */
uint32_t
mbin_baseM_bits_step_32(struct mbin_baseM_bits32 *st)
{
	uint32_t a0 = st->a;
	uint32_t n;

	a0 ^= st->f;

	/* check bits */

	for (n = 0; n != 32; n++) {
		if ((2 * st->f) & (1 << (n - st->set[n])))
			a0 ^= 1 << n;
	}

	/* shift all bit counters up */

	for (n = 31; n != 0; n--)
		st->set[n] = st->set[n - 1];

	st->set[0] = 0;

	/* update bit counters using previous value */

	for (n = 1; n != 32; n++) {
		if (st->a & (1 << (n - 1)))
			st->set[n]++;
		else
			st->set[n] = 0;
	}

	st->a = a0;

	return (a0);
}

uint32_t
mbin_baseM_bits_step_alt_32(struct mbin_baseM_bits32 *st)
{
	uint32_t a;
	uint32_t c;

	/*
	 * NOTE: By using "a = a ^ f", we get a full half adder here,
	 * where "a" and "f" and "c" is summed:
	 */

	a = st->a ^ st->f ^ st->c;

	c = 2 * ((st->c & st->a) | (st->f & ~st->a));

	st->a = a;
	st->c = c;

	return (a);
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
