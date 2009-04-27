/*-
 * Copyright (c) 2009 Hans Petter Selasky. All rights reserved.
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
 * baseU is a "(1/3)**n" exponent function
 */

#include <stdint.h>

#include "math_bin.h"

uint32_t
mbin_base_2toU_32(uint32_t b2)
{
	uint32_t f;
	uint32_t g;
	uint8_t n;

	g = mbin_power_32(3, -b2);

	f = 0;
	for (n = 0; n != 32; n++) {
		f |= (g & (1 << n));
		g *= 3;
	}
	return (f);
}

uint32_t
mbin_base_Uto2_32(uint32_t bu)
{
	uint32_t b2 = 0;
	uint32_t m = 8;

	if (!(bu & 2))
		b2 ^= 1;

	while (m) {
		if ((mbin_base_2toU_32(b2) ^ bu) & m) {
			b2 ^= m / 4;
		}
		m *= 2;
	}
	return (b2);
}

void
mbin_baseU_get_state32(struct mbin_baseU_state32 *ps, uint32_t index)
{
	uint32_t an;
	uint32_t ap;

	ap = mbin_base_2toU_32(index);
	an = mbin_base_2toU_32(index + 1);

	ps->a = ap;
	ps->c = an ^ ap ^ ~(2 * ap);
}

void
mbin_baseU_inc_state32(struct mbin_baseU_state32 *ps)
{
	uint32_t a;
	uint32_t b;
	uint32_t c;

	a = ps->a;
	b = ~(2 * a);
	c = ps->c | 1;

	ps->a = a ^ b ^ c;
	ps->c = 2 * ((a & b) ^ (c & b) ^ (a & c));
}

uint32_t
mbin_baseU_decipher_state32(struct mbin_baseU_state32 *ps)
{
	struct mbin_baseU_state32 temp = *ps;
	uint32_t t;
	uint8_t n;

	t = 0;
	for (n = 0; n != 32; n++) {
		t |= temp.a & (1 << n);
		mbin_baseU_inc_state32(&temp);
	}
	return (t);
}
