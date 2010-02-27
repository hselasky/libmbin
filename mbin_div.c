/*-
 * Copyright (c) 2008-2010 Hans Petter Selasky. All rights reserved.
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

void
mbin_xor_common32(uint32_t *pa, uint32_t *pb)
{
	uint32_t and;

	and = *pa & *pb;

	*pa ^= and;
	*pb ^= and;
}

void
mbin_xor_common16(uint16_t *pa, uint16_t *pb)
{
	uint16_t and;

	and = *pa & *pb;

	*pa ^= and;
	*pb ^= and;
}

void
mbin_xor_common8(uint8_t *pa, uint8_t *pb)
{
	uint8_t and;

	and = *pa & *pb;

	*pa ^= and;
	*pb ^= and;
}

uint32_t
mbin_div_odd32(uint32_t r, uint32_t div)
{
	uint32_t m;

	div = -div + 1;

	m = 1;
	while (m) {

		if (r & m) {
			r += div;
		}
		m *= 2;
		div *= 2;
	}
	return (r);
}

uint64_t
mbin_div_odd64(uint64_t r, uint64_t div)
{
	uint64_t m;

	div = -div + 1;

	m = 1;
	while (m) {

		if (r & m) {
			r += div;
		}
		m *= 2;
		div *= 2;
	}
	return (r);
}

/*
 * First version of Near Carry Less, NCL, binary division. This
 * function runs faster in hardware than in software.
 */
uint32_t
mbin_div_odd32_alt1(uint32_t rem, uint32_t div)
{
	uint32_t neg;
	uint32_t m;
	uint32_t t;
	uint32_t a;
	uint32_t b;

	m = 1;				/* mask */
	neg = 0;			/* no negative bits */
	t = 0;				/* temp variable */

	while (m) {

		if ((rem ^ neg) & m) {

			t |= m;

			/*
			 * Subtract "div" from "(rem-neg)" using near
			 * carry less, NCL, addition where "a" is
			 * remainder and "b" is carry.
			 */
			a = neg ^ div;
			b = (neg & div);

			/* remove common bits */

			mbin_xor_common32(&a, &rem);

			/* compute final part of NCL addition */

			neg = ((2 * a) & ~a) ^ (2 * b);
			rem = rem ^ ((~(2 * a)) & a);

			/* remove common bits */

			mbin_xor_common32(&rem, &neg);
		}
		m *= 2;
		div *= 2;
	}
	return (t);
}

/*
 * Optimised version of Near Carry Less, NCL, binary division. This
 * function runs faster in hardware than in software.
 */
uint32_t
mbin_div_odd32_alt2(uint32_t rem, uint32_t div)
{
	uint32_t neg;
	uint32_t m;
	uint32_t t;
	uint32_t z;
	uint32_t y;

	m = 1;				/* mask */
	neg = 0;			/* no negative bits */
	t = 0;				/* temp variable */

	while (m) {
		if (rem & m) {
			t |= m;

			/* "a XOR b" = "(~a AND b)" XOR "(a AND ~b)" */

			/*
			 * The following equation computes
			 * "((rem & ~neg) - (rem & neg)) -= div;"
			 */

			z = neg | (div & ~rem);
			y = (div ^ rem);

			neg = (2 * z) & (~y);
			rem = (2 * z) ^ y;
		}
		m *= 2;
		div *= 2;
	}
	return (t);
}

uint32_t
mbin_div_odd32_alt3(uint32_t rem, uint32_t div)
{
	uint8_t n;

	div = -div + 1;

	div = mbin_bitrev32(div / 2);

	for (n = 1; n != 32; n++) {
		rem += mbin_sumbits32(rem & (div >> (32 - n))) << n;
	}
	return (rem);
}

uint32_t
mbin_div_odd32_alt4(uint32_t rem, uint32_t div)
{
	uint8_t n;

	for (n = 1; n != 32; n++) {
		if (div & (1 << n)) {
			div = div + (div << n);
			rem = rem + (rem << n);
		}
	}
	return (rem);
}

uint32_t
mbin_div_odd32_alt5(uint32_t rem, uint32_t div)
{
	/*
	 * Not as fast in hardware like the algorithm above, but same
	 * principle.
	 */

	if (!(div & 1))
		return (0);

	while (div != 1) {
		rem = rem * div;
		div = div * div;
	}
	return (rem);
}

uint16_t
mbin_div_odd16(uint16_t r, uint16_t div)
{
	uint16_t m;

	div = -div + 1;

	m = 1;
	while (m) {

		if (r & m) {
			r += div;
		}
		m *= 2;
		div *= 2;
	}
	return (r);
}

uint8_t
mbin_div_odd8(uint8_t r, uint8_t div)
{
	uint8_t m;

	div = -div + 1;

	m = 1;
	while (m) {

		if (r & m) {
			r += div;
		}
		m *= 2;
		div *= 2;
	}
	return (r);
}
