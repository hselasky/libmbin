/*-
 * Copyright (c) 2021 Hans Petter Selasky
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
 * This file implements the fast power wave transform.
 */

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <math.h>

#include "math_bin.h"

/* Helper function to add one bitreversed starting at "mask" */

static inline size_t
mbin_fpt_add_bitreversed(size_t x, size_t mask)
{
	do {
		x ^= mask;
	} while ((x & mask) == 0 && (mask /= 2) != 0);

	return (x);
}

/* Two dimensional vector addition for power wave functions. */

static inline mbin_cf_t
mbin_fpt_add_cf(mbin_cf_t a, mbin_cf_t b)
{
	return ((mbin_cf_t){ a.x + b.x, a.y + b.y });
}

/* Two dimensional vector subtraction for power wave functions. */

static inline mbin_cf_t
mbin_fpt_sub_cf(mbin_cf_t a, mbin_cf_t b)
{
	return ((mbin_cf_t){ a.x - b.x, a.y - b.y });
}

/*
 * Fast Forward Power Function Transform for two dimensional vector data.
 *
 * Special cases of "power":
 * 0.0: square wave function - undefined
 * 0.5: cosinus wave function(s) (also called Fast Fourier Transform)
 * 1.0: triangular wave function(s)
 */
void
mbin_fpt_fwd_cf(mbin_cf_t *ptr, uint8_t log2_size, float power)
{
	const size_t max = 1UL << log2_size;
	mbin_cf_t t[2];
	size_t y;
	size_t z;

	for (size_t step = max; (step /= 2);) {
		for (y = z = 0; y != max; y += 2 * step) {
			const float angle = (float)z / (float)max;

			/* do transform */
			for (size_t x = 0; x != step; x++) {
				t[0] = ptr[x + y];
				t[1] = mbin_angleadd_cf(ptr[x + y + step], angle, power);

				ptr[x + y] = mbin_fpt_add_cf(t[0], t[1]);
				ptr[x + y + step] = mbin_fpt_sub_cf(t[0], t[1]);
			}

			/* update index */
			z = mbin_fpt_add_bitreversed(z, max / 4);
		}
	}

	/* bitreverse */
	for (size_t x = 0; x != max; x++) {
#if __LP64__
		y = mbin_bitrev64(x << (64 - log2_size));
#else
		y = mbin_bitrev32(x << (32 - log2_size));
#endif
		if (y < x) {
			/* swap */
			t[0] = ptr[x];
			ptr[x] = ptr[y];
			ptr[y] = t[0];
		}
	}
}

/* Inverse transform of function above */

void
mbin_fpt_inv_cf(mbin_cf_t *ptr, uint8_t log2_size, float power)
{
	const size_t max = 1UL << log2_size;
	mbin_cf_t t[2];
	size_t y;
	size_t z;

	/* bitreverse */
	for (size_t x = 0; x != max; x++) {
#if __LP64__
		y = mbin_bitrev64(x << (64 - log2_size));
#else
		y = mbin_bitrev32(x << (32 - log2_size));
#endif
		if (y < x) {
			/* swap */
			t[0] = ptr[x];
			ptr[x] = ptr[y];
			ptr[y] = t[0];
		}
	}

	for (size_t step = 1; step != max; step *= 2) {
		for (y = z = 0; y != max; y += 2 * step) {
			const float angle = (float)(max - z) / (float)max;

			/* do transform */
			for (size_t x = 0; x != step; x++) {
				t[0] = mbin_fpt_add_cf(ptr[x + y], ptr[x + y + step]);
				t[1] = mbin_fpt_sub_cf(ptr[x + y], ptr[x + y + step]);

				ptr[x + y] = t[0];
				ptr[x + y + step] = mbin_angleadd_cf(t[1], angle, power);
			}

			/* update index */
			z = mbin_fpt_add_bitreversed(z, max / 4);
		}
	}
}

/* Multiply two triangular two dimensional transforms */

void
mbin_fpt_mul_cf(const mbin_cf_t *pa, const mbin_cf_t *pb, mbin_cf_t *pc, uint8_t log2_size, float power)
{
	const size_t max = 1UL << log2_size;

	/* output array indexes from transform are bitreversed */
	for (size_t x = 0; x != max; x++)
		pc[x] = mbin_powmul_cf(pa[x], pb[x], power);
}
