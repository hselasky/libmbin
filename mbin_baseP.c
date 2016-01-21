/*-
 * Copyright (c) 2016 Hans Petter Selasky. All rights reserved.
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

uint32_t
mbin_baseP_add_32(uint32_t a, uint32_t b)
{
	while (b) {
		uint32_t an = mbin_xor3_32(a, b);
		uint32_t cn = 4 * (a & b);

		a = an;
		b = cn;
	}
	return (a);
}

uint32_t
mbin_baseP_negate_32(uint32_t a)
{
	return ((a & 0xAAAAAAAA) / 2) | ((a & 0x55555555) * 2);
}

uint32_t
mbin_baseP_sub_32(uint32_t a, uint32_t b)
{
	return (mbin_baseP_add_32(a, mbin_baseP_negate_32(b)));
}

uint32_t
mbin_baseP_exp_32(uint32_t a, uint32_t b)
{
	uint32_t r = 0;

	while (b) {
		if (b & 1)
			r = mbin_baseP_add_32(r, a);
		a = mbin_baseP_add_32(a, a);
		b /= 2;
	}
	return (r);
}

uint32_t
mbin_baseP_is_negative_32(uint32_t a)
{
	return ((mbin_msb32(a) & 0xAAAAAAAAU) ? 1 : 0);
}

uint32_t
mbin_baseP_multiply_32(uint32_t a, uint32_t b)
{
	uint32_t r = 0;

	while (a) {
		if (a & 2)
			r = mbin_baseP_add_32(r, mbin_baseP_negate_32(b));
		else if (a & 1)
			r = mbin_baseP_add_32(r, b);
		a /= 4;
		b *= 4;
	}
	return (r);
}

uint32_t
mbin_base_2toP_32(uint32_t r)
{
	return (mbin_baseP_exp_32(1, r));
}

uint32_t
mbin_base_Pto2_32(uint32_t a)
{
	uint32_t k = 1;
	uint32_t r = 0;

	while (a) {
		if (a & 1)
			r += k;
		else if (a & 2)
			r -= k;
		k *= 3;
		a /= 4;
	}
	return (r);
}

int
mbin_baseP_cmp_32(uint32_t a, uint32_t b)
{
	if (a == b)
		return (0);
	if (mbin_baseP_is_negative_32(mbin_baseP_sub_32(a, b)))
		return (-1);
	else
		return (1);
}
