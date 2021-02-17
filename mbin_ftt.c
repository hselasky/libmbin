/*-
 * Copyright (c) 2021 Hans Petter Selasky. All rights reserved.
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
 * This file implements the fast triangle wave transform. This
 * transform has the advantage over the fast fourier transform, that
 * no precision is lost, and no irrational values are used. That means
 * the result is always exact.
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
mbin_ftt_add_bitreversed(size_t x, size_t mask)
{
	do {
		x ^= mask;
	} while ((x & mask) == 0 && (mask /= 2) != 0);

	return (x);
}

/*
 * Special case of "mbin_acospowf_32(x,1.0)"
 * Returns the phase of a triangular function.
 */
static inline float
mbin_ftt_acosf(float x)
{
	x = fabsf(x);

	/* check for special case */
	if (x == 1.0f)
		return (0.0f);
	else if (x == 0.0f)
		return (0.25f);
	else
		return (ceilf(x) - x) * (1.0f / 4.0f);
}

/*
 * Special case of "mbin_cospowf_32(x,1.0)"
 * Returns a triangular wave function based on phase "x".
 */
static inline float
mbin_ftt_cosf(float x)
{
	x = x - floorf(x);

	/* check for special case */
	if (x == 0.0f)
		return (1.0f);
	else if (x == 0.5f)
		return (-1.0f);

	x = x * 4.0f;

	/* check quadrant */
	if (x < 1.0f)
		x = ceilf(x) - x;
	else if (x < 2.0f)
		x = floorf(x) - x;
	else if (x < 3.0f)
		x = x - ceilf(x);
	else
		x = x - floorf(x);
	return (x);
}

/*
 * Special case of "mbin_sinpowf_32(x,1.0)"
 * Returns a triangular wave function based on phase "x".
 */
static inline float
mbin_ftt_sinf(float x)
{
	return (mbin_ftt_cosf(x + 0.75f));
}

/*
 * Special case of "mbin_powmul_cf(a,b,1.0)"
 * Two dimensional vector multiplication for triangular wave functions.
 */
static inline mbin_cf_t
mbin_ftt_multiply_cf(mbin_cf_t a, mbin_cf_t b)
{
	/* Compute vector gain */
	const float ga = fabsf(a.x) + fabsf(a.y);
	const float gb = fabsf(b.x) + fabsf(b.y);

	/* Figure out quadrants */
	const uint8_t qa = (a.x < 0) + 2 * (a.y < 0);
	const uint8_t qb = (b.x < 0) + 2 * (b.y < 0);

	/* Normalize input vectors, "cosine" argument */
	if (ga != 0.0f)
		a.x /= ga;

	if (gb != 0.0f)
		b.x /= gb;

	/* Compute output vector gain */
	const float gr = ga * gb;

	float angle;

	/* Add the two angles, that's part of vector multiplication */
	switch (qa) {
	case 0:
		angle = mbin_ftt_acosf(a.x);
		break;
	case 1:
		angle = 0.5f - mbin_ftt_acosf(a.x);
		break;
	case 2:
		angle = 1.0f - mbin_ftt_acosf(a.x);
		break;
	case 3:
		angle = 0.5f + mbin_ftt_acosf(a.x);
		break;
	default:
		angle = 0.0f;
		break;
	}

	switch (qb) {
	case 0:
		angle += mbin_ftt_acosf(b.x);
		break;
	case 1:
		angle += 0.5f - mbin_ftt_acosf(b.x);
		break;
	case 2:
		angle += 1.0f - mbin_ftt_acosf(b.x);
		break;
	case 3:
		angle += 0.5f + mbin_ftt_acosf(b.x);
		break;
	default:
		break;
	}

	/* Restore output vector */
	return ((mbin_cf_t){
	    mbin_ftt_cosf(angle) * gr,
	    mbin_ftt_sinf(angle) * gr
	});
}

/*
 * Special case of "mbin_angleadd_cf(a,b,1.0)"
 * Two dimensional vector multiplication for triangular wave functions.
 */
static inline mbin_cf_t
mbin_ftt_angleadd_cf(mbin_cf_t a, float angle)
{
	/* Compute vector gain */
	const float ga = fabsf(a.x) + fabsf(a.y);

	/* Figure out quadrants */
	const uint8_t qa = (a.x < 0) + 2 * (a.y < 0);

	/* Normalize input vectors, "cosine" argument */
	if (ga != 0.0f)
		a.x /= ga;

	/* Add the two angles, that's part of vector multiplication */
	switch (qa) {
	case 0:
		angle += mbin_ftt_acosf(a.x);
		break;
	case 1:
		angle += 0.5f - mbin_ftt_acosf(a.x);
		break;
	case 2:
		angle += 1.0f - mbin_ftt_acosf(a.x);
		break;
	case 3:
		angle += 0.5f + mbin_ftt_acosf(a.x);
		break;
	default:
		break;
	}

	/* Restore output vector */
	return ((mbin_cf_t){
	    mbin_ftt_cosf(angle) * ga,
	    mbin_ftt_sinf(angle) * ga
	});
}

/* Two dimensional vector addition for triangular wave functions. */

static inline mbin_cf_t
mbin_ftt_add_cf(mbin_cf_t a, mbin_cf_t b)
{
	return ((mbin_cf_t){ a.x + b.x, a.y + b.y });
}

/* Two dimensional vector subtraction for triangular wave functions. */

static inline mbin_cf_t
mbin_ftt_sub_cf(mbin_cf_t a, mbin_cf_t b)
{
	return ((mbin_cf_t){ a.x - b.x, a.y - b.y });
}

/* Fast Forward Triangular Transform for two dimensional vector data. */

void
mbin_ftt_fwd_cf(mbin_cf_t *ptr, uint8_t log2_size)
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
				t[1] = mbin_ftt_angleadd_cf(ptr[x + y + step], angle);

				ptr[x + y] = mbin_ftt_add_cf(t[0], t[1]);
				ptr[x + y + step] = mbin_ftt_sub_cf(t[0], t[1]);
			}

			/* update index */
			z = mbin_ftt_add_bitreversed(z, max / 4);
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

/* Fast Inverse Triangular Transform for two dimensional vector data. */

void
mbin_ftt_inv_cf(mbin_cf_t *ptr, uint8_t log2_size)
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
				t[0] = mbin_ftt_add_cf(ptr[x + y], ptr[x + y + step]);
				t[1] = mbin_ftt_sub_cf(ptr[x + y], ptr[x + y + step]);

				ptr[x + y] = t[0];
				ptr[x + y + step] = mbin_ftt_angleadd_cf(t[1], angle);
			}

			/* update index */
			z = mbin_ftt_add_bitreversed(z, max / 4);
		}
	}
}

/* Multiply two triangular two dimensional transforms */

void
mbin_ftt_mul_cf(const mbin_cf_t *pa, const mbin_cf_t *pb, mbin_cf_t *pc, uint8_t log2_size)
{
	const size_t max = 1UL << log2_size;

	/* output array indexes from transform are bitreversed */
	for (size_t x = 0; x != max; x++)
		pc[x] = mbin_ftt_multiply_cf(pa[x], pb[x]);
}

/* The derivative of a triangle function is either -1, 0 or +1 */

void
mbin_ftt_diff_cf(mbin_cf_t *pc, uint8_t log2_size)
{
	const size_t max = 1UL << log2_size;
	mbin_cf_t prev = pc[max - 1];

	for (size_t x = 0; x != max; x++) {
		const mbin_cf_t old = pc[x];
		pc[x] = mbin_ftt_sub_cf(pc[x], prev);
		prev = old;
	}
}

/* The integrate of a square wave function is a triangle function */

void
mbin_ftt_integ_cf(mbin_cf_t *pc, uint8_t log2_size)
{
	const size_t max = 1UL << log2_size;
	mbin_cf_t sum = {};

	for (size_t x = 0; x != max; x++)
		pc[x] = sum = mbin_ftt_add_cf(sum, pc[x]);
}
