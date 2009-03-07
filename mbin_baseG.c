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
 * baseG is a multiplication function
 */

#include <stdint.h>

#include "math_bin.h"

/*
 * This function converts from base-2 to base-G using factor "f".
 * This function has similarities to "mbin_recodeB_fwd32()".
 */
uint32_t
mbin_base_2toG_32(uint32_t f, uint32_t b2)
{
	uint32_t bias;
	uint32_t t;
	uint32_t m;

	bias = (b2 * f);
	t = 0;
	m = 1;

	while (m) {
		bias -= f & (m - 1);
		t |= (bias & m);
		m <<= 1;
	}
	return (t);
}

/*
 * This function converts from base-G to base-2 using factor "f".
 * This function has similarities to "mbin_recodeB_inv32()".
 *
 * NOTE: factor "f" should be odd
 */
uint32_t
mbin_base_Gto2_32(uint32_t f, uint32_t bg)
{
	uint32_t bias;
	uint32_t b2;
	uint32_t m;

	bias = 0;
	b2 = 0;
	m = 1;

	while (m) {
		bias -= f & (m - 1);
		b2 |= (((b2 + bias) ^ bg) & m);
		m <<= 1;
	}

	return (mbin_div_odd32(b2, f));
}

/*
 * This function restores the state at the given index.
 */
void
mbin_baseG_get_state32(struct mbin_baseG_state32 *ps,
    uint32_t f, uint32_t index)
{
	uint32_t ap;
	uint32_t a;
	uint32_t c;

	ap = mbin_base_2toG_32(f, index - 1);
	a = mbin_base_2toG_32(f, index);
	c = f;

	ps->a = a;
	ps->c = c;
	ps->b = 2 * ((ap & (~a)) | ((ap | (~a)) & c));
	return;
}

/*
 * This function increments the state by one.
 */
void
mbin_baseG_inc_state32(struct mbin_baseG_state32 *ps)
{
	uint32_t a;
	uint32_t b;
	uint32_t c;

	a = ps->a;
	b = ps->b;
	c = ps->c;

	/* standard addition formula */

	ps->a = a ^ b ^ c;
	ps->b = 2 * ((a & b) | (a & c) | (b & c));
	return;
}

/*
 * This function deciphers the state like a multiplication of
 * "f" and "b2".
 */
uint32_t
mbin_baseG_decipher_state32(struct mbin_baseG_state32 *ps)
{
	return (ps->a + ps->b);
}
