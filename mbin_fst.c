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
 * This file implements the fast square wave transform. This
 * transform has the advantage over the fast fourier transform, that
 * no precision is lost, and no irrational values are used. That means
 * the result is always exact.
 *
 * This transform only works in the 3-adic system.
 *
 * In the code below the two dimensional 3-adic vectors are encoded
 * into a single unsigned 8-bit variable to save memory, because only
 * values 0 to 2, inclusivly, are used.
 */

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <math.h>

#include "math_bin.h"

#include "math_bin_complex.h"

/* Helper function to add one bitreversed starting at "mask" */

static inline uint8_t
mbin_fst_add_bitreversed(uint8_t x, uint8_t mask)
{
	do {
		x ^= mask;
	} while ((x & mask) == 0 && (mask /= 2) != 0);

	return (x);
}

/* 3-adic mapping tables for two dimensional vectors */

uint8_t mbin_fst_angle_to_vector[33] = { 1,7,6,8,2,5,3,4, 1,7,6,8,2,5,3,4, /* rest is zero */ };
uint8_t mbin_fst_vector_to_angle[9] = { 16,0,4,6,7,5,2,1,3 };

/*
 * Two dimensional vector multiplication for square wave function.
 */
static inline uint8_t
mbin_fst_multiply_2d(uint8_t a, uint8_t b)
{
	/* convert from vector to angle */
	a = mbin_fst_vector_to_angle[a];
	b = mbin_fst_vector_to_angle[b];

	/* add angles and convert back again */
	return (mbin_fst_angle_to_vector[a + b]);
}

/* Two dimensional vector rotation by "angle" for square wave function. */

static inline uint8_t
mbin_fst_angleadd_2d(uint8_t a, uint8_t angle)
{
	/* convert from vector to angle */
	a = mbin_fst_vector_to_angle[a];

	/* add angles and convert back again */
	return (mbin_fst_angle_to_vector[a + angle]);
}

/* Two dimensional vector addition for square wave functions. */

static inline uint8_t
mbin_fst_add_2d(uint8_t a, uint8_t b)
{
	const uint8_t r_x = (a + b) % 3;
	const uint8_t r_y = ((a / 3) + (b / 3)) % 3;

	return (r_x + (3 * r_y));
}

/* Two dimensional vector subtraction for square wave functions. */

static inline uint8_t
mbin_fst_sub_2d(uint8_t a, uint8_t b)
{
	const uint8_t r_x = (9 + a - b) % 3;
	const uint8_t r_y = (3 + (a / 3) - (b / 3)) % 3;

	return (r_x + (3 * r_y));
}

/* Fast Forward Square Transform for two dimensional 3-adic vector data. */

void
mbin_fst_fwd_2d(uint8_t *ptr, uint8_t log2_size)
{
	const size_t max = 1UL << log2_size;
	uint8_t t[2];
	uint8_t z;
	size_t y;

	for (size_t step = max; (step /= 2);) {
		for (y = 0, z = 0; y != max; y += 2 * step) {
			/* do transform */
			for (size_t x = 0; x != step; x++) {
				t[0] = ptr[x + y];
				t[1] = mbin_fst_angleadd_2d(ptr[x + y + step], z);

				ptr[x + y] = mbin_fst_add_2d(t[0], t[1]);
				ptr[x + y + step] = mbin_fst_sub_2d(t[0], t[1]);
			}

			/* update index */
			z = mbin_fst_add_bitreversed(z, 2);
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

/* Fast Inverse Square Transform for two dimensional 3-adic vector data. */

void
mbin_fst_inv_2d(uint8_t *ptr, uint8_t log2_size)
{
	const size_t max = 1UL << log2_size;
	uint8_t t[2];
	uint8_t z;
	size_t y;

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
		for (y = 0, z = 0; y != max; y += 2 * step) {
			/* do transform */
			for (size_t x = 0; x != step; x++) {
				t[0] = mbin_fst_add_2d(ptr[x + y], ptr[x + y + step]);
				t[1] = mbin_fst_sub_2d(ptr[x + y], ptr[x + y + step]);

				ptr[x + y] = t[0];
				ptr[x + y + step] = mbin_fst_angleadd_2d(t[1], (-z) & 7);
			}

			/* update index */
			z = mbin_fst_add_bitreversed(z, 2);
		}
	}
}

/* Multiply two square two dimensional transforms */

void
mbin_fst_mul_2d(const uint8_t *pa, const uint8_t *pb, uint8_t *pc, uint8_t log2_size)
{
	const size_t max = 1UL << log2_size;

	/* output array indexes from transform are bitreversed */
	for (size_t x = 0; x != max; x++)
		pc[x] = mbin_fst_multiply_2d(pa[x], pb[x]);
}

/* The derivative of a triangle function is either -1, 0 or +1 */

void
mbin_fst_diff_2d(uint8_t *pc, uint8_t log2_size)
{
	const size_t max = 1UL << log2_size;
	uint8_t prev = pc[max - 1];

	for (size_t x = 0; x != max; x++) {
		const uint8_t old = pc[x];
		pc[x] = mbin_fst_sub_2d(pc[x], prev);
		prev = old;
	}
}

/* The integrate of a square wave function is a triangle function */

void
mbin_fst_integ_2d(uint8_t *pc, uint8_t log2_size)
{
	const size_t max = 1UL << log2_size;
	uint8_t sum = 0;

	for (size_t x = 0; x != max; x++)
		pc[x] = sum = mbin_fst_add_2d(sum, pc[x]);
}

/*
 * Implementation of 3-adic multiplier in two dimensions:
 *
 * Handles small size multiplications.
 * Input has size "2**log2_size" (pa,pb)
 * Output has size "2**(log2_size + 1)" (pc)
 */

void
mbin_fst_multiply_3_adic_2d(const uint8_t *pa, const uint8_t *pb, uint8_t *pc, uint8_t log2_size)
{
	const size_t max = 1UL << log2_size;
	uint8_t ta[2 * max];
	uint8_t tb[2 * max];

	/*
	 * Store input vectors into transform arrays:
	 *
	 * 0: No value
	 * 1: positive (0 degrees angle)
	 * 2: negative (180 degrees angle)
	 */
	for (size_t x = 0; x != max; x++) {
		ta[x] = pa[x];
		tb[x] = pb[x];
		ta[x + max] = 0;
		tb[x + max] = 0;
	}

	/* Do standard fast transform with multiplication */
	mbin_fst_fwd_2d(ta, log2_size + 1);
	mbin_fst_fwd_2d(tb, log2_size + 1);
	mbin_fst_mul_2d(ta, tb, ta, log2_size + 1);
	mbin_fst_inv_2d(ta, log2_size + 1);

	/* Keep first dimension */
	for (size_t x = 0; x != (2 * max); x++)
		pc[x] = ta[x] % 3;
}

void
mbin_fst_angleadd_c32(c32_t *ptr, uint8_t num)
{
	if (num & 1) {
		int nx = ptr->x + ptr->y;
		int ny = ptr->y - ptr->x;

		if (!(nx % 2))
			nx /= 2;
		if (!(ny % 2))
			ny /= 2;

		ptr->x = nx;
		ptr->y = ny;
	}
	if (num & 2) {
		int nx = ptr->y;
		int ny = -ptr->x;

		ptr->x = nx;
		ptr->y = ny;
	}
	if (num & 4) {
		int nx = -ptr->x;
		int ny = -ptr->y;

		ptr->x = nx;
		ptr->y = ny;
	}
}

/* Fast Forward Square Transform for two dimensional complex 32-bit vector data. */

void
mbin_fst_fwd_c32(c32_t *ptr, uint8_t log2_size)
{
	const size_t max = 1UL << log2_size;
	c32_t t[2];
	uint8_t z;
	size_t y;

	for (size_t step = max; (step /= 2);) {
		for (y = 0, z = 0; y != max; y += 2 * step) {
			/* do transform */
			for (size_t x = 0; x != step; x++) {
				c32_t *p[2] = {
					ptr + x + y,
					ptr + x + y + step,
				};
				t[0] = p[0][0];
				t[1] = p[1][0];

				mbin_fst_angleadd_c32(t + 1, z);

				p[0][0] = t[0];
				p[0][0].x += t[1].x;
				p[0][0].y += t[1].y;

				p[1][0] = t[0];
				p[1][0].x -= t[1].x;
				p[1][0].y -= t[1].y;
			}

			/* update index */
			z = mbin_fst_add_bitreversed(z, 2);
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

/* Fast Inverse Square Transform for two dimensional complex 32-bit vector data. */

void
mbin_fst_inv_c32(c32_t *ptr, uint8_t log2_size)
{
	const size_t max = 1UL << log2_size;
	c32_t t[2];
	uint8_t z;
	size_t y;

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
		for (y = 0, z = 0; y != max; y += 2 * step) {
			/* do transform */
			for (size_t x = 0; x != step; x++) {
				c32_t *p[2] = {
					ptr + x + y,
					ptr + x + y + step,
				};

				t[0].x = (p[0][0].x + p[1][0].x) / 2;
				t[0].y = (p[0][0].y + p[1][0].y) / 2;

				t[1].x = (p[0][0].x - p[1][0].x) / 2;
				t[1].y = (p[0][0].y - p[1][0].y) / 2;

				mbin_fst_angleadd_c32(t + 1, (-z) & 7);

				p[0][0] = t[0];
				p[1][0] = t[1];
			}

			/* update index */
			z = mbin_fst_add_bitreversed(z, 2);
		}
	}
}
