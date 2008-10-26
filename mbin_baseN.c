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

struct mbin_base {
	uint32_t stepmask;
	uint8_t	stepshift;
	uint8_t	maxshift;
};

static void
mbin_get_base(struct mbin_base *mb, uint32_t n)
{
	uint32_t msb;

	msb = mbin_msb32((2 * n) - 1);

	mb->stepmask = msb - 1;
	mb->stepshift = 0;
	while (msb >>= 1) {
		mb->stepshift++;
	}
	mb->maxshift = 32 - (32 % mb->stepshift);
	return;
}

uint32_t
mbin_mul_baseN_32(uint32_t a, uint32_t b, uint32_t n)
{
	struct mbin_base mb;
	uint32_t z;
	uint8_t s;

	mbin_get_base(&mb, n);

	z = 0;
	for (s = 0; s != mb.maxshift; s += mb.stepshift) {
		z = mbin_add_baseN_32(z, b << s,
		    ((a >> s) & mb.stepmask), n);
	}
	return (z);
}

uint32_t
mbin_add_baseN_32(uint32_t a, uint32_t b, uint32_t f, uint32_t n)
{
	struct mbin_base mb;
	uint32_t r;
	uint32_t t;
	uint8_t s;

	mbin_get_base(&mb, n);

	r = 0;
	t = 0;

	for (s = 0; s != mb.maxshift; s += mb.stepshift) {

		if (f == 1) {
			r = r + ((a >> s) & mb.stepmask) +
			    (((b >> s) & mb.stepmask) * 1);
		} else {
			r = r + ((a >> s) & mb.stepmask) +
			    (((b >> s) & mb.stepmask) * f);
		}

		t |= (r % n) << s;

		r /= n;
	}
	return (t);
}

uint32_t
mbin_sub_baseN_32(uint32_t a, uint32_t b, uint32_t f, uint32_t n)
{
	struct mbin_base mb;
	uint32_t r;
	uint32_t t;
	uint8_t s;

	mbin_get_base(&mb, n);
	r = 1;
	t = 0;

	if (f != 1) {
		b = mbin_add_baseN_32(b, 0, f, n);
	}
	for (s = 0; s != mb.maxshift; s += mb.stepshift) {
		r = r + (n - 1) -
		    ((b >> s) & mb.stepmask) +
		    ((a >> s) & mb.stepmask);
		t |= (r % n) << s;
		r /= n;
	}
	return (t);
}

uint32_t
mbin_sumdigits_baseN_32(uint32_t a, uint32_t n)
{
	struct mbin_base mb;
	uint32_t z;
	uint8_t s;

	mbin_get_base(&mb, n);

	z = 0;
	for (s = 0; s != mb.maxshift; s += mb.stepshift) {
		z = z + ((a >> s) & mb.stepmask);
	}
	return (z);
}

uint32_t
mbin_convert_2toN_32(uint32_t a, uint32_t n)
{
	struct mbin_base mb;
	uint32_t z;
	uint8_t s;

	mbin_get_base(&mb, n);

	z = 0;

	for (s = 0; s != mb.maxshift; s += mb.stepshift) {
		z |= (a % n) << s;
		a /= n;
	}
	return (z);
}

uint32_t
mbin_convert_Nto2_32(uint32_t a, uint32_t n)
{
	struct mbin_base mb;
	uint32_t z;
	uint32_t f;
	uint8_t s;

	mbin_get_base(&mb, n);

	z = 0;
	f = 1;

	for (s = 0; s != mb.maxshift; s += mb.stepshift) {
		z = z + (f * ((a >> s) & mb.stepmask));
		f *= n;
	}
	return (z);
}
