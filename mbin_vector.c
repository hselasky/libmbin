/*-
 * Copyright (c) 2011 Hans Petter Selasky. All rights reserved.
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
#include <string.h>

#include "math_bin.h"

void
mbin_vector_or_double(double *a, double *b, double *c, uint8_t bits)
{
	uint32_t max = 1U << bits;
	uint32_t x;

	double at[max];
	double bt[max];

	memcpy(at, a, sizeof(at));
	memcpy(bt, b, sizeof(bt));

	mbin_forward_add_xform_double(at, bits);
	mbin_forward_add_xform_double(bt, bits);

	for (x = 0; x != max; x++)
		c[x] = at[x] * bt[x];

	mbin_inverse_add_xform_double(c, bits);
}

void
mbin_vector_or_32(uint32_t *a, uint32_t *b, uint32_t *c, uint8_t bits)
{
	uint32_t max = 1U << bits;
	uint32_t x;

	uint32_t at[max];
	uint32_t bt[max];

	memcpy(at, a, sizeof(at));
	memcpy(bt, b, sizeof(bt));

	mbin_forward_add_xform_32(at, bits);
	mbin_forward_add_xform_32(bt, bits);

	for (x = 0; x != max; x++)
		c[x] = at[x] * bt[x];

	mbin_inverse_add_xform_32(c, bits);
}

void
mbin_vector_and_double(double *a, double *b, double *c, uint8_t bits)
{
	uint32_t max = 1U << bits;
	uint32_t x;

	double at[max];
	double bt[max];

	memcpy(at, a, sizeof(at));
	memcpy(bt, b, sizeof(bt));

	mbin_forward_rev_add_xform_double(at, bits);
	mbin_forward_rev_add_xform_double(bt, bits);

	for (x = 0; x != max; x++)
		c[x] = at[x] * bt[x];

	mbin_inverse_rev_add_xform_double(c, bits);
}

void
mbin_vector_and_32(uint32_t *a, uint32_t *b, uint32_t *c, uint8_t bits)
{
	uint32_t max = 1U << bits;
	uint32_t x;

	uint32_t at[max];
	uint32_t bt[max];

	memcpy(at, a, sizeof(at));
	memcpy(bt, b, sizeof(bt));

	mbin_forward_rev_add_xform_32(at, bits);
	mbin_forward_rev_add_xform_32(bt, bits);

	for (x = 0; x != max; x++)
		c[x] = at[x] * bt[x];

	mbin_inverse_rev_add_xform_32(c, bits);
}

void
mbin_vector_xor_double(double *a, double *b, double *c, uint8_t bits)
{
	uint32_t max = 1U << bits;
	uint32_t x;

	double at[max];
	double bt[max];

	memcpy(at, a, sizeof(at));
	memcpy(bt, b, sizeof(bt));

	mbin_sumdigits_r2_xform_double(at, bits);
	mbin_sumdigits_r2_xform_double(bt, bits);

	for (x = 0; x != max; x++)
		c[x] = at[x] * bt[x];

	mbin_sumdigits_r2_xform_double(c, bits);
}

void
mbin_vector_xor_32(uint32_t *a, uint32_t *b, uint32_t *c, uint8_t bits)
{
	uint32_t max = 1U << bits;
	uint32_t x;

	uint32_t at[max];
	uint32_t bt[max];

	memcpy(at, a, sizeof(at));
	memcpy(bt, b, sizeof(bt));

	mbin_sumdigits_r2_xform_32(at, bits);
	mbin_sumdigits_r2_xform_32(bt, bits);

	for (x = 0; x != max; x++)
		c[x] = at[x] * bt[x];

	mbin_sumdigits_r2_xform_32(c, bits);
}
