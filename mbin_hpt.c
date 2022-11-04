/*-
 * Copyright (c) 2019 Hans Petter Selasky
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
 * HPT is short for higher power transform.
 *
 * Vector modulus used:
 *   3    0    1
 * r[0] r[1] r[2]
 */

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "math_bin.h"

static hpt_double_t
mbin_hpt_mul_fwd_double(hpt_double_t a, hpt_double_t b)
{
	const double top = 3.0 * a.r[1] * b.r[1];

	return ((hpt_double_t){
		a.r[0] * b.r[0] - top,
		a.r[0] * b.r[1] + a.r[1] * b.r[0] });
}

static hpt_double_t
mbin_hpt_mul_inv_double(hpt_double_t a, hpt_double_t b)
{
	const double top = a.r[0] * b.r[0] / 3.0;

	return ((hpt_double_t){
		a.r[0] * b.r[1] + a.r[1] * b.r[0],
		a.r[1] * b.r[1] - top });
}

static hpt_double_t
mbin_hpt_exp_fwd_double(hpt_double_t base, uint64_t exp)
{
	hpt_double_t r = {1, 0};

	while (exp != 0) {
		if (exp & 1)
			r = mbin_hpt_mul_fwd_double(r, base);
		base = mbin_hpt_mul_fwd_double(base, base);
		exp /= 2;
	}
	return (r);
}

static hpt_double_t
mbin_hpt_exp_inv_double(hpt_double_t base, uint64_t exp)
{
	hpt_double_t r = {0, 1};

	while (exp != 0) {
		if (exp & 1)
			r = mbin_hpt_mul_inv_double(r, base);
		base = mbin_hpt_mul_inv_double(base, base);
		exp /= 2;
	}
	return (r);
}

static hpt_double_t
mbin_hpt_add_double(hpt_double_t a, hpt_double_t b)
{
	return ((hpt_double_t){
		a.r[0] + b.r[0], a.r[1] + b.r[1] });
}

static hpt_double_t
mbin_hpt_sub_double(hpt_double_t a, hpt_double_t b)
{
	return ((hpt_double_t){
		a.r[0] - b.r[0], a.r[1] - b.r[1] });
}

static uint32_t
mbin_hpt_add_bitreversed_32(uint32_t x, uint32_t mask)
{
	do {
		x ^= mask;
	} while ((x & mask) == 0 && (mask /= 2) != 0);
	return (x);
}

void
mbin_hpt_xform_fwd_double(hpt_double_t *data, uint8_t power)
{
	const uint32_t max = 1U << power;
	const hpt_double_t base = {0, 1};
	hpt_double_t t[3];
	uint32_t step;
	uint32_t x;
	uint32_t y;
	uint32_t z;
	uint32_t u;

	for (step = max; (step /= 2);) {
		for (y = z = u = 0; y != max; y += 2 * step) {
			u = mbin_hpt_add_bitreversed_32(z, step);

			const hpt_double_t k[2] = {
				mbin_hpt_exp_fwd_double(base, z),
				mbin_hpt_exp_fwd_double(base, u)
			};

			for (x = 0; x != step; x++) {
				t[0] = data[y + x];
				t[1] = mbin_hpt_mul_fwd_double(data[y + x + step], k[0]);
				t[2] = mbin_hpt_mul_fwd_double(data[y + x + step], k[1]);

				data[y + x] = mbin_hpt_add_double(t[0], t[1]);
				data[y + x + step] = mbin_hpt_add_double(t[0], t[2]);
			}

			z = mbin_hpt_add_bitreversed_32(z, max / 4);
		}
	}
}

void
mbin_hpt_xform_inv_double(hpt_double_t *data, uint8_t power)
{
	const uint32_t max = 1U << power;
	const hpt_double_t base = {1, 0};
	hpt_double_t t[2];
	uint32_t step;
	uint32_t x;
	uint32_t y;
	uint32_t z;

	for (step = 1; step != max; step *= 2) {
		for (y = z = 0; y != max; y += 2 * step) {
			const hpt_double_t k = mbin_hpt_exp_inv_double(base, z);

			for (x = 0; x != step; x++) {
				t[0] = mbin_hpt_add_double(data[y + x], data[y + x + step]);
				t[1] = mbin_hpt_sub_double(data[y + x], data[y + x + step]);

				data[y + x] = t[0];
				data[y + x + step] = mbin_hpt_mul_inv_double(t[1], k);
			}
			z = mbin_hpt_add_bitreversed_32(z, max / 4);
		}
	}
}
