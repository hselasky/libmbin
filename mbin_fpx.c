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
 * This file implements the fast P-adic two dimensional wave transform.
 */

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <math.h>

#include "math_bin.h"

#include "math_bin_complex.h"

c32_t *mbin_fpx_wave_c32;

c32_t *
mbin_fpx_generate_table_c32(uint32_t _x, uint32_t _y, uint8_t log2_size)
{
	const size_t max = 1UL << log2_size;
	c32_t *ret = malloc(max * sizeof(ret[0]));
	c32_t k = {};
	c32_t a = {};

	/* find a suitable vector */
	for (uint64_t x = _x; x != MBIN_FPX_C32_PRIME; x++) {
		for (uint64_t y = _y; y != MBIN_FPX_C32_PRIME; y++) {
			/* check for unit vector */
			if (((x * x + y * y) % MBIN_FPX_C32_PRIME) != 1)
				goto next;

			k.x = x;
			k.y = y;
			a.x = 1;
			a.y = 0;

			ret[0] = a;
			mbin_fpx_mul_c32(&a, &k, &a, 0);

			for (size_t z = 1; z != max; z++) {
				if (a.x == 1 && a.y == 0)
					goto next;
				ret[z] = a;
				mbin_fpx_mul_c32(&a, &k, &a, 0);
			}
			if (a.x != 1 || a.y != 0)
				goto next;

			for (size_t z = 0; z != (max / 2); z++) {
				if (ret[z].x != ((MBIN_FPX_C32_PRIME - ret[z + (max / 2)].x) % MBIN_FPX_C32_PRIME) ||
				    ret[z].y != ((MBIN_FPX_C32_PRIME - ret[z + (max / 2)].y) % MBIN_FPX_C32_PRIME))
					goto next;
			}
			return (ret);
		next:;
		}
	}
	free(ret);
	return (NULL);
}

void
mbin_fpx_init_c32()
{
	mbin_fpx_wave_c32 =
	    mbin_fpx_generate_table_c32(4, 17573, 16);
	assert(mbin_fpx_wave_c32 != NULL);
}

/* Helper function to add one bitreversed starting at "mask" */

static inline size_t
mbin_fpx_add_bitreversed(size_t x, size_t mask)
{
	do {
		x ^= mask;
	} while ((x & mask) == 0 && (mask /= 2) != 0);

	return (x);
}

static inline c32_t
mbin_fpx_multiply_c32(c32_t a, c32_t b)
{
	return (c32_t){
		((uint64_t)MBIN_FPX_C32_PRIME * (uint64_t)MBIN_FPX_C32_PRIME +
		 (uint64_t)a.x * (uint64_t)b.x -
		 (uint64_t)a.y * (uint64_t)b.y) % MBIN_FPX_C32_PRIME,
		((uint64_t)a.x * (uint64_t)b.y +
		 (uint64_t)a.y * (uint64_t)b.x) % MBIN_FPX_C32_PRIME,
	};
}

static inline c32_t
mbin_fpx_add_c32(c32_t a, c32_t b)
{
	return (c32_t){
		(a.x + b.x) % MBIN_FPX_C32_PRIME,
		(a.y + b.y) % MBIN_FPX_C32_PRIME,
	};
}

static inline c32_t
mbin_fpx_sub_c32(c32_t a, c32_t b)
{
	return (c32_t){
		(MBIN_FPX_C32_PRIME + a.x - b.x) % MBIN_FPX_C32_PRIME,
		(MBIN_FPX_C32_PRIME + a.y - b.y) % MBIN_FPX_C32_PRIME,
	};
}

void
mbin_fpx_mul_c32(const c32_t *pa, const c32_t *pb, c32_t *pc, uint8_t log2_size)
{
	const size_t max = 1UL << log2_size;

	for (size_t x = 0; x != max; x++)
		pc[x] = mbin_fpx_multiply_c32(pa[x], pb[x]);
}

void
mbin_fpx_xform_c32(c32_t *ptr, uint8_t log2_size)
{
	const size_t max = 1UL << log2_size;
	c32_t t[2];
	size_t y;
	size_t z;

	assert(log2_size <= 16);

	for (size_t step = max; (step /= 2);) {
		for (y = z = 0; y != max; y += 2 * step) {
			/* do transform */
			for (size_t x = 0; x != step; x++) {
				t[0] = ptr[x + y];
				t[1] = mbin_fpx_multiply_c32(ptr[x + y + step], mbin_fpx_wave_c32[z]);

				ptr[x + y] = mbin_fpx_add_c32(t[0], t[1]);
				ptr[x + y + step] = mbin_fpx_sub_c32(t[0], t[1]);
			}

			/* update index */
			z = mbin_fpx_add_bitreversed(z, (1UL << (16 - 2)));
		}
	}
}

void
mbin_fpx_bitreverse_c32(c32_t *ptr, uint8_t log2_size)
{
	const size_t max = 1UL << log2_size;
	c32_t t[1];
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
}

static c32_t
mbin_fpx_div_2(c32_t z, uint8_t num)
{
	while (num--) {
		if (z.x & 1)
			z.x += MBIN_FPX_C32_PRIME;
		z.x /= 2;

		if (z.y & 1)
			z.y += MBIN_FPX_C32_PRIME;
		z.y /= 2;
	}
	return (z);
}

void
mbin_fpx_multiply_8(const uint8_t *pa, const uint8_t *pb, uint8_t *pc, uint8_t log2_size)
{
	const size_t max = 1UL << log2_size;
	const size_t mask = (1UL << (log2_size + 4)) - 1UL;
	c32_t ta[2 * 8 * max];
	c32_t tb[2 * 8 * max];
	uint16_t temp;

	assert(log2_size + 4 <= 16);

	memset(ta, 0, sizeof(ta));
	memset(tb, 0, sizeof(tb));

	for (size_t x = 0; x != max; x++) {
		ta[8 * x + 0].x = (pa[x] & 1) ? 1 : 0;
		ta[8 * x + 1].x = (pa[x] & 2) ? 1 : 0;
		ta[8 * x + 2].x = (pa[x] & 4) ? 1 : 0;
		ta[8 * x + 3].x = (pa[x] & 8) ? 1 : 0;
		ta[8 * x + 4].x = (pa[x] & 16) ? 1 : 0;
		ta[8 * x + 5].x = (pa[x] & 32) ? 1 : 0;
		ta[8 * x + 6].x = (pa[x] & 64) ? 1 : 0;
		ta[8 * x + 7].x = (pa[x] & 128) ? 1 : 0;

		tb[8 * x + 0].x = (pb[x] & 1) ? 1 : 0;
		tb[8 * x + 1].x = (pb[x] & 2) ? 1 : 0;
		tb[8 * x + 2].x = (pb[x] & 4) ? 1 : 0;
		tb[8 * x + 3].x = (pb[x] & 8) ? 1 : 0;
		tb[8 * x + 4].x = (pb[x] & 16) ? 1 : 0;
		tb[8 * x + 5].x = (pb[x] & 32) ? 1 : 0;
		tb[8 * x + 6].x = (pb[x] & 64) ? 1 : 0;
		tb[8 * x + 7].x = (pb[x] & 128) ? 1 : 0;
	}

	mbin_fpx_xform_c32(ta, log2_size + 4);
	mbin_fpx_xform_c32(tb, log2_size + 4);
	mbin_fpx_mul_c32(ta, tb, ta, log2_size + 4);
	mbin_fpx_bitreverse_c32(ta, log2_size + 4);
	mbin_fpx_xform_c32(ta, log2_size + 4);
	mbin_fpx_bitreverse_c32(ta, log2_size + 4);

	temp = 0;

	for (size_t x = 0; x != 2 * max; x++) {
		temp += mbin_fpx_div_2(ta[(-8 * x - 0) & mask], log2_size + 4).x * 1;
		temp += mbin_fpx_div_2(ta[(-8 * x - 1) & mask], log2_size + 4).x * 2;
		temp += mbin_fpx_div_2(ta[(-8 * x - 2) & mask], log2_size + 4).x * 4;
		temp += mbin_fpx_div_2(ta[(-8 * x - 3) & mask], log2_size + 4).x * 8;
		temp += mbin_fpx_div_2(ta[(-8 * x - 4) & mask], log2_size + 4).x * 16;
		temp += mbin_fpx_div_2(ta[(-8 * x - 5) & mask], log2_size + 4).x * 32;
		temp += mbin_fpx_div_2(ta[(-8 * x - 6) & mask], log2_size + 4).x * 64;
		temp += mbin_fpx_div_2(ta[(-8 * x - 7) & mask], log2_size + 4).x * 128;
		pc[x] = (uint8_t)temp;
		temp >>= 8;
	}
}
