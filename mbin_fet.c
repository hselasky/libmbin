/*-
 * Copyright (c) 2010-2019 Hans Petter Selasky. All rights reserved.
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
 * This file implements a fast transform that can be used for fast
 * integer convolution.
 *
 * FET is short for fast exponential transform.
 */

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "math_bin.h"

#define	MBIN_FET_COMBA (1U << 16)	/* crossover point */

static void
mbin_fet_shift_32(const int32_t *pa, int32_t *ptr, uint32_t num, uint32_t size)
{
	uint32_t x;

	if (num == 0) {
		for (x = 0; x != size; x++)
			ptr[x] = pa[x];
	} else if (num == size) {
		for (x = 0; x != size; x++)
			ptr[x] = -pa[x];
	} else if (num > size) {
		num -= size;

		for (x = 0; x != (size - num); x++)
			ptr[x] = -pa[x + num];
		for ( ; x != size; x++)
			ptr[x] = pa[x + num - size];
	} else {
		for (x = 0; x != (size - num); x++)
			ptr[x] = pa[x + num];
		for ( ; x != size; x++)
			ptr[x] = -pa[x + num - size];
	}
}

static void
mbin_fet_mul_slow_32(const int32_t *pa, const int32_t *pb, int32_t *ptr, uint32_t size)
{
	int32_t upper[size];
	uint32_t x;

	mbin_x3_multiply_32(pa, pb, ptr, upper, size);

	for (x = 0; x != size; x++)
		ptr[x] -= upper[x];
}

static void
mbin_fet_mul_fast_32(const int32_t *pa, const int32_t *pb, int32_t *ptr, uint8_t power)
{
	const uint32_t size = 1U << power;
  	int32_t upper[size];
	uint32_t x;

	mbin_fet_multiply_32(pa, pb, ptr, upper, power);

	for (x = 0; x != size; x++)
		ptr[x] -= upper[x];
}

void
mbin_fet_multiply_32(const int32_t *pa, const int32_t *pb, int32_t *low, int32_t *high, uint8_t power)
{
	const uint8_t varpower = (power + 2) / 2;
	const uint8_t numpower = (power + 2) - varpower;
	const uint32_t varmax = 1U << varpower;
	const uint32_t nummax = 1U << numpower;
	const uint32_t size = 1U << power;

	int32_t *ta = malloc(sizeof(ta[0]) * 4 * size);
	int32_t *va = malloc(sizeof(va[0]) * 4 * size);
	int32_t *vb = malloc(sizeof(vb[0]) * 4 * size);

	uint32_t x,y,z,t,u;

	assert(numpower <= (varpower + 1));

	memset(va, 0, sizeof(va[0]) * 4 * size);
	memset(vb, 0, sizeof(vb[0]) * 4 * size);

	for (x = z = 0; x != (nummax / 2); x++) {
		t = x << varpower;
		u = ((-x) & (nummax - 1)) << varpower;
		for (y = 0; y != (varmax / 2); y++, z++) {
			va[t + y] = pa[z];
			vb[u + y] = pb[z];
		}
	}

	mbin_fet_xform_fwd_32(va, varpower, numpower);
	mbin_fet_xform_fwd_32(vb, varpower, numpower);
	mbin_fet_correlate_32(va, vb, ta, varpower, numpower);
	mbin_fet_xform_inv_32(ta, varpower, numpower);

	for (x = 0, z = 0; x != (nummax / 2); x++) {
		t = x << varpower;
		u = (x + (nummax / 2)) << varpower;
		for (y = 0; y != (varmax / 2); y++, z++) {
			low[z] = ta[t + y];
			high[z] = ta[u + y];
		}
	}

	for (x = 0, z = (varmax / 2); x != (nummax / 2); x++) {
		t = x << varpower;
		u = (x + (nummax / 2)) << varpower;
		if (z >= size) {
			for (y = (varmax / 2); y != varmax; y++, z++) {
				high[z - size] += ta[t + y];
			}
		} else {
			for (y = (varmax / 2); y != varmax; y++, z++) {
				low[z] += ta[t + y];
				high[z] += ta[u + y];
			}
		}
	}

	free(ta);
	free(va);
	free(vb);
}

static void
mbin_fet_add_32(const int32_t *pa, const int32_t *pb, int32_t *ptr, uint32_t size)
{
	uint32_t x;

	for (x = 0; x != size; x++)
		ptr[x] = pa[x] + pb[x];
}

static void
mbin_fet_sub_32(const int32_t *pa, const int32_t *pb, int32_t *ptr, uint32_t size)
{
	uint32_t x;

	for (x = 0; x != size; x++)
		ptr[x] = pa[x] - pb[x];
}

static void
mbin_fet_add_half_32(const int32_t *pa, const int32_t *pb, int32_t *ptr, uint32_t size)
{
	uint32_t x;

	for (x = 0; x != size; x++)
		ptr[x] = (pa[x] + pb[x]) / 2;
}

static void
mbin_fet_sub_half_32(const int32_t *pa, const int32_t *pb, int32_t *ptr, uint32_t size)
{
	uint32_t x;

	for (x = 0; x != size; x++)
		ptr[x] = (pa[x] - pb[x]) / 2;
}

void
mbin_fet_xform_fwd_32(int32_t *data, uint8_t varpower, uint8_t numpower)
{
	const uint32_t varmax = 1U << varpower;
	const uint32_t nummax = 1U << numpower;
	int32_t t[2][varmax];
	uint32_t step;
	uint32_t x;
	uint32_t y;
	uint32_t z;

	assert(numpower <= (varpower + 1));
	
	for (step = nummax; (step /= 2);) {
		for (y = z = 0; y != nummax; y += 2 * step) {
			const uint32_t shift = (-z) & ((2U << varpower) - 1U);

			/* do transform */
			for (x = 0; x != step; x++) {
				memcpy(t[0], data + ((y + x) << varpower), sizeof(t[0]));
				mbin_fet_shift_32(data + ((y + x + step) << varpower),
				    t[1], shift, varmax);
				mbin_fet_add_32(t[0], t[1], data + ((y + x) << varpower), varmax);
				mbin_fet_sub_32(t[0], t[1], data + ((y + x + step) << varpower), varmax);
			}

			/* add one bitreversed */
			for (x = varmax / 2; x; x /= 2) {
				z ^= x;
				if (z & x)
					break;
			}
		}
	}

	/* in-place data order bit-reversal */
	for (x = 0; x != nummax; x++) {
		y = mbin_bitrev32(x << (32 - numpower));
		if (y < x) {
			memcpy(t[0], data + (x << varpower), sizeof(t[0]));
			memcpy(data + (x << varpower), data + (y << varpower), sizeof(t[0]));
			memcpy(data + (y << varpower), t[0], sizeof(t[0]));
		}
	}
}

void
mbin_fet_xform_inv_32(int32_t *data, uint8_t varpower, uint8_t numpower)
{
	const uint32_t varmax = 1U << varpower;
	const uint32_t nummax = 1U << numpower;
	int32_t t[2][varmax];
	uint32_t step;
	uint32_t x;
	uint32_t y;
	uint32_t z;

	assert(numpower <= (varpower + 1));
	
	/* in-place data order bit-reversal */
	for (x = 0; x != nummax; x++) {
		y = mbin_bitrev32(x << (32 - numpower));
		if (y < x) {
			memcpy(t[0], data + (x << varpower), sizeof(t[0]));
			memcpy(data + (x << varpower), data + (y << varpower), sizeof(t[0]));
			memcpy(data + (y << varpower), t[0], sizeof(t[0]));
		}
	}

	for (step = 1; step != nummax; step *= 2) {
		for (y = z = 0; y != nummax; y += 2 * step) {
			const uint32_t shift = z;

			/* do transform */
			for (x = 0; x != step; x++) {
				memcpy(t[0], data + ((y + x) << varpower), sizeof(t[0]));
				memcpy(t[1], data + ((y + x + step) << varpower), sizeof(t[1]));

				mbin_fet_add_half_32(t[0], t[1], data + ((y + x) << varpower), varmax);
				mbin_fet_sub_half_32(t[0], t[1], t[0], varmax);

				mbin_fet_shift_32(t[0], data + ((y + x + step) << varpower), shift, varmax);
			}

			/* add one bitreversed */
			for (x = varmax / 2; x; x /= 2) {
				z ^= x;
				if (z & x)
					break;
			}
		}
	}
}

void
mbin_fet_correlate_32(const int32_t *a, const int32_t *b, int32_t *c,
    uint8_t varpower, uint8_t numpower)
{
	const uint32_t nummax = 1U << numpower;
	const uint32_t varmax = 1U << varpower;
	uint32_t x;

	assert(numpower <= (varpower + 1));

	if (varmax < MBIN_FET_COMBA) {
		for (x = 0; x != nummax; x++) {
			mbin_fet_mul_slow_32(a + (x << varpower),
					     b + (((-x) & (nummax - 1)) << varpower),
					     c + (x << varpower), varmax);
		}
	} else {
		for (x = 0; x != nummax; x++) {
			mbin_fet_mul_fast_32(a + (x << varpower),
					     b + (((-x) & (nummax - 1)) << varpower),
					     c + (x << varpower), varpower);
		}
	}
}

/* 64-bit version of functions above */

static void
mbin_fet_shift_64(const int64_t *pa, int64_t *ptr, uint32_t num, uint32_t size)
{
	uint32_t x;

	if (num == 0) {
		for (x = 0; x != size; x++)
			ptr[x] = pa[x];
	} else if (num == size) {
		for (x = 0; x != size; x++)
			ptr[x] = -pa[x];
	} else if (num > size) {
		num -= size;

		for (x = 0; x != (size - num); x++)
			ptr[x] = -pa[x + num];
		for ( ; x != size; x++)
			ptr[x] = pa[x + num - size];
	} else {
		for (x = 0; x != (size - num); x++)
			ptr[x] = pa[x + num];
		for ( ; x != size; x++)
			ptr[x] = -pa[x + num - size];
	}
}

static void
mbin_fet_mul_slow_64(const int64_t *pa, const int64_t *pb, int64_t *ptr, uint32_t size)
{
	int64_t upper[size];
	uint32_t x;

	mbin_x3_multiply_64(pa, pb, ptr, upper, size);

	for (x = 0; x != size; x++)
		ptr[x] -= upper[x];
}

static void
mbin_fet_mul_fast_64(const int64_t *pa, const int64_t *pb, int64_t *ptr, uint8_t power)
{
	const uint32_t size = 1U << power;
  	int64_t upper[size];
	uint32_t x;

	mbin_fet_multiply_64(pa, pb, ptr, upper, power);

	for (x = 0; x != size; x++)
		ptr[x] -= upper[x];
}

void
mbin_fet_multiply_64(const int64_t *pa, const int64_t *pb, int64_t *low, int64_t *high, uint8_t power)
{
	const uint8_t varpower = (power + 2) / 2;
	const uint8_t numpower = (power + 2) - varpower;
	const uint32_t varmax = 1U << varpower;
	const uint32_t nummax = 1U << numpower;
	const uint32_t size = 1U << power;

	int64_t *ta = malloc(sizeof(ta[0]) * 4 * size);
	int64_t *va = malloc(sizeof(va[0]) * 4 * size);
	int64_t *vb = malloc(sizeof(vb[0]) * 4 * size);

	uint32_t x,y,z,t,u;

	assert(numpower <= (varpower + 1));

	memset(va, 0, sizeof(va[0]) * 4 * size);
	memset(vb, 0, sizeof(vb[0]) * 4 * size);

	for (x = z = 0; x != (nummax / 2); x++) {
		t = x << varpower;
		u = ((-x) & (nummax - 1)) << varpower;
		for (y = 0; y != (varmax / 2); y++, z++) {
			va[t + y] = pa[z];
			vb[u + y] = pb[z];
		}
	}

	mbin_fet_xform_fwd_64(va, varpower, numpower);
	mbin_fet_xform_fwd_64(vb, varpower, numpower);
	mbin_fet_correlate_64(va, vb, ta, varpower, numpower);
	mbin_fet_xform_inv_64(ta, varpower, numpower);

	for (x = 0, z = 0; x != (nummax / 2); x++) {
		t = x << varpower;
		u = (x + (nummax / 2)) << varpower;
		for (y = 0; y != (varmax / 2); y++, z++) {
			low[z] = ta[t + y];
			high[z] = ta[u + y];
		}
	}

	for (x = 0, z = (varmax / 2); x != (nummax / 2); x++) {
		t = x << varpower;
		u = (x + (nummax / 2)) << varpower;
		if (z >= size) {
			for (y = (varmax / 2); y != varmax; y++, z++) {
				high[z - size] += ta[t + y];
			}
		} else {
			for (y = (varmax / 2); y != varmax; y++, z++) {
				low[z] += ta[t + y];
				high[z] += ta[u + y];
			}
		}
	}

	free(ta);
	free(va);
	free(vb);
}

static void
mbin_fet_add_64(const int64_t *pa, const int64_t *pb, int64_t *ptr, uint32_t size)
{
	uint32_t x;

	for (x = 0; x != size; x++)
		ptr[x] = pa[x] + pb[x];
}

static void
mbin_fet_sub_64(const int64_t *pa, const int64_t *pb, int64_t *ptr, uint32_t size)
{
	uint32_t x;

	for (x = 0; x != size; x++)
		ptr[x] = pa[x] - pb[x];
}

static void
mbin_fet_add_half_64(const int64_t *pa, const int64_t *pb, int64_t *ptr, uint32_t size)
{
	uint32_t x;

	for (x = 0; x != size; x++)
		ptr[x] = (pa[x] + pb[x]) / 2.0;
}

static void
mbin_fet_sub_half_64(const int64_t *pa, const int64_t *pb, int64_t *ptr, uint32_t size)
{
	uint32_t x;

	for (x = 0; x != size; x++)
		ptr[x] = (pa[x] - pb[x]) / 2.0;
}

void
mbin_fet_xform_fwd_64(int64_t *data, uint8_t varpower, uint8_t numpower)
{
	const uint32_t varmax = 1U << varpower;
	const uint32_t nummax = 1U << numpower;
	int64_t t[2][varmax];
	uint32_t step;
	uint32_t x;
	uint32_t y;
	uint32_t z;

	assert(numpower <= (varpower + 1));

	for (step = nummax; (step /= 2);) {
		for (y = z = 0; y != nummax; y += 2 * step) {
			const uint32_t shift = (-z) & ((2U << varpower) - 1U);

			/* do transform */
			for (x = 0; x != step; x++) {
				memcpy(t[0], data + ((y + x) << varpower), sizeof(t[0]));
				mbin_fet_shift_64(data + ((y + x + step) << varpower),
				    t[1], shift, varmax);

				mbin_fet_add_64(t[0], t[1], data + ((y + x) << varpower), varmax);
				mbin_fet_sub_64(t[0], t[1], data + ((y + x + step) << varpower), varmax);
			}

			/* add one bitreversed */
			for (x = varmax / 2; x; x /= 2) {
				z ^= x;
				if (z & x)
					break;
			}
		}
	}

	/* in-place data order bit-reversal */
	for (x = 0; x != nummax; x++) {
		y = mbin_bitrev32(x << (32 - numpower));
		if (y < x) {
			memcpy(t[0], data + (x << varpower), sizeof(t[0]));
			memcpy(data + (x << varpower), data + (y << varpower), sizeof(t[0]));
			memcpy(data + (y << varpower), t[0], sizeof(t[0]));
		}
	}
}

void
mbin_fet_xform_inv_64(int64_t *data, uint8_t varpower, uint8_t numpower)
{
	const uint32_t varmax = 1U << varpower;
	const uint32_t nummax = 1U << numpower;
	int64_t t[2][varmax];
	uint32_t step;
	uint32_t x;
	uint32_t y;
	uint32_t z;

	assert(numpower <= (varpower + 1));

	/* in-place data order bit-reversal */
	for (x = 0; x != nummax; x++) {
		y = mbin_bitrev32(x << (32 - numpower));
		if (y < x) {
			memcpy(t[0], data + (x << varpower), sizeof(t[0]));
			memcpy(data + (x << varpower), data + (y << varpower), sizeof(t[0]));
			memcpy(data + (y << varpower), t[0], sizeof(t[0]));
		}
	}

	for (step = 1; step != nummax; step *= 2) {
		for (y = z = 0; y != nummax; y += 2 * step) {
			const uint32_t shift = z;

			/* do transform */
			for (x = 0; x != step; x++) {
				memcpy(t[0], data + ((y + x) << varpower), sizeof(t[0]));
				memcpy(t[1], data + ((y + x + step) << varpower), sizeof(t[1]));

				mbin_fet_add_half_64(t[0], t[1], data + ((y + x) << varpower), varmax);
				mbin_fet_sub_half_64(t[0], t[1], t[0], varmax);

				mbin_fet_shift_64(t[0], data + ((y + x + step) << varpower), shift, varmax);
			}

			/* add one bitreversed */
			for (x = varmax / 2; x; x /= 2) {
				z ^= x;
				if (z & x)
					break;
			}
		}
	}
}

void
mbin_fet_correlate_64(const int64_t *a, const int64_t *b, int64_t *c,
    uint8_t varpower, uint8_t numpower)
{
	const uint32_t nummax = 1U << numpower;
	const uint32_t varmax = 1U << varpower;
	uint32_t x;

	assert(numpower <= (varpower + 1));

	if (varmax < MBIN_FET_COMBA) {
		for (x = 0; x != nummax; x++) {
			mbin_fet_mul_slow_64(a + (x << varpower),
					     b + (((-x) & (nummax - 1)) << varpower),
					     c + (x << varpower), varmax);
		}
	} else {
		for (x = 0; x != nummax; x++) {
			mbin_fet_mul_fast_64(a + (x << varpower),
					     b + (((-x) & (nummax - 1)) << varpower),
					     c + (x << varpower), varpower);
		}
	}
}

/* double version of functions above */

static void
mbin_fet_shift_double(const double *pa, double *ptr, uint32_t num, uint32_t size)
{
	uint32_t x;

	if (num == 0) {
		for (x = 0; x != size; x++)
			ptr[x] = pa[x];
	} else if (num == size) {
		for (x = 0; x != size; x++)
			ptr[x] = -pa[x];
	} else if (num > size) {
		num -= size;

		for (x = 0; x != (size - num); x++)
			ptr[x] = -pa[x + num];
		for ( ; x != size; x++)
			ptr[x] = pa[x + num - size];
	} else {
		for (x = 0; x != (size - num); x++)
			ptr[x] = pa[x + num];
		for ( ; x != size; x++)
			ptr[x] = -pa[x + num - size];
	}
}

static void
mbin_fet_mul_slow_double(const double *pa, const double *pb, double *ptr, uint32_t size)
{
	double upper[size];
	uint32_t x;

	mbin_x3_multiply_double(pa, pb, ptr, upper, size);

	for (x = 0; x != size; x++)
		ptr[x] -= upper[x];
}

static void
mbin_fet_mul_fast_double(const double *pa, const double *pb, double *ptr, uint8_t power)
{
	const uint32_t size = 1U << power;
  	double upper[size];
	uint32_t x;

	mbin_fet_multiply_double(pa, pb, ptr, upper, power);

	for (x = 0; x != size; x++)
		ptr[x] -= upper[x];
}

void
mbin_fet_multiply_double(const double *pa, const double *pb, double *low, double *high, uint8_t power)
{
	const uint8_t varpower = (power + 2) / 2;
	const uint8_t numpower = (power + 2) - varpower;
	const uint32_t varmax = 1U << varpower;
	const uint32_t nummax = 1U << numpower;
	const uint32_t size = 1U << power;

	double *ta = malloc(sizeof(ta[0]) * 4 * size);
	double *va = malloc(sizeof(va[0]) * 4 * size);
	double *vb = malloc(sizeof(vb[0]) * 4 * size);

	uint32_t x,y,z,t,u;

	assert(numpower <= (varpower + 1));

	memset(va, 0, sizeof(va[0]) * 4 * size);
	memset(vb, 0, sizeof(vb[0]) * 4 * size);

	for (x = z = 0; x != (nummax / 2); x++) {
		t = x << varpower;
		u = ((-x) & (nummax - 1)) << varpower;
		for (y = 0; y != (varmax / 2); y++, z++) {
			va[t + y] = pa[z];
			vb[u + y] = pb[z];
		}
	}

	mbin_fet_xform_fwd_double(va, varpower, numpower);
	mbin_fet_xform_fwd_double(vb, varpower, numpower);
	mbin_fet_correlate_double(va, vb, ta, varpower, numpower);
	mbin_fet_xform_inv_double(ta, varpower, numpower);

	for (x = 0, z = 0; x != (nummax / 2); x++) {
		t = x << varpower;
		u = (x + (nummax / 2)) << varpower;
		for (y = 0; y != (varmax / 2); y++, z++) {
			low[z] = ta[t + y];
			high[z] = ta[u + y];
		}
	}

	for (x = 0, z = (varmax / 2); x != (nummax / 2); x++) {
		t = x << varpower;
		u = (x + (nummax / 2)) << varpower;
		if (z >= size) {
			for (y = (varmax / 2); y != varmax; y++, z++) {
				high[z - size] += ta[t + y];
			}
		} else {
			for (y = (varmax / 2); y != varmax; y++, z++) {
				low[z] += ta[t + y];
				high[z] += ta[u + y];
			}
		}
	}

	free(ta);
	free(va);
	free(vb);
}

static void
mbin_fet_add_double(const double *pa, const double *pb, double *ptr, uint32_t size)
{
	uint32_t x;

	for (x = 0; x != size; x++)
		ptr[x] = pa[x] + pb[x];
}

static void
mbin_fet_sub_double(const double *pa, const double *pb, double *ptr, uint32_t size)
{
	uint32_t x;

	for (x = 0; x != size; x++)
		ptr[x] = pa[x] - pb[x];
}

static void
mbin_fet_add_half_double(const double *pa, const double *pb, double *ptr, uint32_t size)
{
	uint32_t x;

	for (x = 0; x != size; x++)
		ptr[x] = (pa[x] + pb[x]) / 2.0;
}

static void
mbin_fet_sub_half_double(const double *pa, const double *pb, double *ptr, uint32_t size)
{
	uint32_t x;

	for (x = 0; x != size; x++)
		ptr[x] = (pa[x] - pb[x]) / 2.0;
}

void
mbin_fet_xform_fwd_double(double *data, uint8_t varpower, uint8_t numpower)
{
	const uint32_t varmax = 1U << varpower;
	const uint32_t nummax = 1U << numpower;
	double t[2][varmax];
	uint32_t step;
	uint32_t x;
	uint32_t y;
	uint32_t z;

	assert(numpower <= (varpower + 1));

	for (step = nummax; (step /= 2);) {
		for (y = z = 0; y != nummax; y += 2 * step) {
			const uint32_t shift = (-z) & ((2U << varpower) - 1U);

			/* do transform */
			for (x = 0; x != step; x++) {
				memcpy(t[0], data + ((y + x) << varpower), sizeof(t[0]));
				mbin_fet_shift_double(data + ((y + x + step) << varpower),
				    t[1], shift, varmax);

				mbin_fet_add_double(t[0], t[1], data + ((y + x) << varpower), varmax);
				mbin_fet_sub_double(t[0], t[1], data + ((y + x + step) << varpower), varmax);
			}

			/* add one bitreversed */
			for (x = varmax / 2; x; x /= 2) {
				z ^= x;
				if (z & x)
					break;
			}
		}
	}

	/* in-place data order bit-reversal */
	for (x = 0; x != nummax; x++) {
		y = mbin_bitrev32(x << (32 - numpower));
		if (y < x) {
			memcpy(t[0], data + (x << varpower), sizeof(t[0]));
			memcpy(data + (x << varpower), data + (y << varpower), sizeof(t[0]));
			memcpy(data + (y << varpower), t[0], sizeof(t[0]));
		}
	}
}

void
mbin_fet_xform_inv_double(double *data, uint8_t varpower, uint8_t numpower)
{
	const uint32_t varmax = 1U << varpower;
	const uint32_t nummax = 1U << numpower;
	double t[2][varmax];
	uint32_t step;
	uint32_t x;
	uint32_t y;
	uint32_t z;

	assert(numpower <= (varpower + 1));

	/* in-place data order bit-reversal */
	for (x = 0; x != nummax; x++) {
		y = mbin_bitrev32(x << (32 - numpower));
		if (y < x) {
			memcpy(t[0], data + (x << varpower), sizeof(t[0]));
			memcpy(data + (x << varpower), data + (y << varpower), sizeof(t[0]));
			memcpy(data + (y << varpower), t[0], sizeof(t[0]));
		}
	}

	for (step = 1; step != nummax; step *= 2) {
		for (y = z = 0; y != nummax; y += 2 * step) {
			const uint32_t shift = z;

			/* do transform */
			for (x = 0; x != step; x++) {
				memcpy(t[0], data + ((y + x) << varpower), sizeof(t[0]));
				memcpy(t[1], data + ((y + x + step) << varpower), sizeof(t[1]));

				mbin_fet_add_half_double(t[0], t[1], data + ((y + x) << varpower), varmax);
				mbin_fet_sub_half_double(t[0], t[1], t[0], varmax);

				mbin_fet_shift_double(t[0], data + ((y + x + step) << varpower), shift, varmax);
			}

			/* add one bitreversed */
			for (x = varmax / 2; x; x /= 2) {
				z ^= x;
				if (z & x)
					break;
			}
		}
	}
}

void
mbin_fet_correlate_double(const double *a, const double *b, double *c,
    uint8_t varpower, uint8_t numpower)
{
	const uint32_t nummax = 1U << numpower;
	const uint32_t varmax = 1U << varpower;
	uint32_t x;

	assert(numpower <= (varpower + 1));

	if (varmax < MBIN_FET_COMBA) {
		for (x = 0; x != nummax; x++) {
			mbin_fet_mul_slow_double(a + (x << varpower),
						 b + (((-x) & (nummax - 1)) << varpower),
						 c + (x << varpower), varmax);
		}
	} else {
		for (x = 0; x != nummax; x++) {
			mbin_fet_mul_fast_double(a + (x << varpower),
						 b + (((-x) & (nummax - 1)) << varpower),
						 c + (x << varpower), varpower);
		}
	}
}
