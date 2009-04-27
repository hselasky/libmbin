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

/*
 * baseH is an exponent function
 */

#include <stdint.h>

#include "math_bin.h"

static uint32_t
mbin_baseH_gen_div(void)
{
#if 0
	uint8_t n = 32;
	uint32_t x = 1;

	while (n--) {
		x = (x * x) + 4;
	}
	return (x);
#else
	return (0x3ef7226d);
#endif
}

static uint32_t
mbin_baseH_gen_mul(void)
{
#if 0
	return (mbin_div_odd32(1, mbin_baseH_gen_div()));
#else
	return (0xf0423765);
#endif
}

uint32_t
mbin_base_2toH_32(uint32_t index)
{
	uint32_t f;
	uint32_t g;
	uint32_t t;
	uint8_t n;

#if 0
	/* XXX constant origin needs to be investigated */
	/* XXX probably this constant should be removed */
	index -= 0x30c41244;
#endif

	f = mbin_baseH_gen_mul();

	g = mbin_power_32(f, index);

	t = 1;
	f = mbin_baseH_gen_div();
	for (n = 2; n != 32; n++) {
		t |= g & (1 << n);
		g = g * f;
	}
	return (t);
}

uint32_t
mbin_base_Hto2_32(uint32_t bh)
{
	uint32_t b2 = 0;
	uint32_t m = 4;

	while (m) {
		if ((mbin_base_2toH_32(b2) ^ bh) & m) {
			b2 ^= m / 4;
		}
		m *= 2;
	}
	return (b2);
}

/*
 * This function restores the state at the given index.
 */
void
mbin_baseH_get_state32(struct mbin_baseH_state32 *ps,
    uint32_t index)
{
	uint32_t ap;
	uint32_t an;

	ap = mbin_base_2toH_32(index);
	an = mbin_base_2toH_32(index + 1);

	ps->a = ap;
	ps->c = an ^ ap ^ (4 * ap);
}

/*
 * This function increments the state by one.
 */
void
mbin_baseH_inc_state32(struct mbin_baseH_state32 *ps)
{
	uint32_t a;
	uint32_t c;

	a = ps->a;
	c = ps->c;

	/* standard addition formula */

	ps->a = a ^ (4 * a) ^ c;
	ps->c = 2 * (((a ^ c) & (4 * a)) | (a & c));
}

/*
 * This function deciphers the state like an exponent,
 */
uint32_t
mbin_baseH_decipher_state32(struct mbin_baseH_state32 *ps)
{
	struct mbin_baseH_state32 psc = *ps;
	uint32_t t;
	uint8_t n;

	t = 0;
	for (n = 0; n != 32; n++) {
		t |= psc.a & (1 << n);
		mbin_baseH_inc_state32(&psc);
	}
	return (t);
}
