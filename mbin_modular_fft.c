/*-
 * Copyright (c) 2021-2022 Hans Petter Selasky. All rights reserved.
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

#include "math_bin.h"

static uint32_t
mbin_mod_fft_add_bitreversed(uint32_t x, uint32_t mask)
{
	do {
		x ^= mask;
	} while ((x & mask) == 0 && (mask /= 2) != 0);

	return (x);
}

#if __LP64__
static uint64_t
mbin_mod_fft_bitrev(uint64_t a, uint8_t log2size)
{
	a <<= (64 - log2size);

	a = ((a & 0x5555555555555555ULL) << 1) | ((a & 0xAAAAAAAAAAAAAAAAULL) >> 1);
	a = ((a & 0x3333333333333333ULL) << 2) | ((a & 0xCCCCCCCCCCCCCCCCULL) >> 2);
	a = ((a & 0x0F0F0F0F0F0F0F0FULL) << 4) | ((a & 0xF0F0F0F0F0F0F0F0ULL) >> 4);
	a = ((a & 0x00FF00FF00FF00FFULL) << 8) | ((a & 0xFF00FF00FF00FF00ULL) >> 8);
	a = ((a & 0x0000FFFF0000FFFFULL) << 16) | ((a & 0xFFFF0000FFFF0000ULL) >> 16);
	a = ((a & 0x00000000FFFFFFFFULL) << 32) | ((a & 0xFFFFFFFF00000000ULL) >> 32);
	return (a);
}
#else
static uint32_t
mbin_mod_fft_bitrev(uint32_t a, uint8_t log2size)
{
	a <<= (32 - log2size);

	a = ((a & 0x55555555) << 1) | ((a & 0xAAAAAAAA) >> 1);
	a = ((a & 0x33333333) << 2) | ((a & 0xCCCCCCCC) >> 2);
	a = ((a & 0x0F0F0F0F) << 4) | ((a & 0xF0F0F0F0) >> 4);
	a = ((a & 0x00FF00FF) << 8) | ((a & 0xFF00FF00) >> 8);
	a = ((a & 0x0000FFFF) << 16) | ((a & 0xFFFF0000) >> 16);
	return (a);
}
#endif

/* Fast Forward Fourier Transform for modular vector data. */

void
mbin_mod_fft_fwd_32(int32_t *ptr, const int32_t *table, uint8_t log2_size, bool doBitreverse)
{
	const size_t max = 1UL << log2_size;
	const int32_t mod = max + 1;
	int32_t t[2];
	size_t y;
	size_t z;

	for (size_t step = max; (step /= 2);) {
		for (y = z = 0; y != max; y += 2 * step) {
			/* do transform */
			for (size_t x = 0; x != step; x++) {
				t[0] = ptr[x + y];
				t[1] = ptr[x + y + step] * table[z];

				ptr[x + y] = (t[0] + t[1]) % mod;
				ptr[x + y + step] = (t[0] - t[1]) % mod;
			}

			/* update index */
			z = mbin_mod_fft_add_bitreversed(z, 1UL << (log2_size - 2));
		}
	}

	if (doBitreverse) {
		/* bitreverse */
		for (size_t x = 0; x != max; x++) {
			y = mbin_mod_fft_bitrev(x, log2_size);
			if (y < x) {
				/* swap */
				t[0] = ptr[x];
				ptr[x] = ptr[y];
				ptr[y] = t[0];
			}
		}
	}
}

/* Fast Inverse Fourier Transform for modular vector data. */

void
mbin_mod_fft_inv_32(int32_t *ptr, const int32_t *table, uint8_t log2_size, bool doBitreverse)
{
	const size_t max = 1UL << log2_size;
	const int32_t mod = max + 1;
	int32_t t[2];
	size_t y;
	size_t z;

	if (doBitreverse) {
		/* bitreverse */
		for (size_t x = 0; x != max; x++) {
			y = mbin_mod_fft_bitrev(x, log2_size);
			if (y < x) {
				/* swap */
				t[0] = ptr[x];
				ptr[x] = ptr[y];
				ptr[y] = t[0];
			}
		}
	}

	for (size_t step = 1; step != max; step *= 2) {
		for (y = z = 0; y != max; y += 2 * step) {
			/* do transform */
			for (size_t x = 0; x != step; x++) {
				t[0] = ptr[x + y];
				t[1] = ptr[x + y + step];

				ptr[x + y] = (t[0] + t[1]) % mod;
				ptr[x + y + step] = ((t[0] - t[1]) * table[(-z) & (max - 1)]) % mod;
			}

			/* update index */
			z = mbin_mod_fft_add_bitreversed(z, 1UL << (log2_size - 2));
		}
	}
}

/* Multiply two Fourier two dimensional transforms */

void
mbin_mod_fft_mul_32(const int32_t *pa, const int32_t *pb, int32_t *pc, uint8_t log2_size)
{
	const size_t max = 1UL << log2_size;
	const int32_t mod = max + 1;

	/* output array indexes from transform are bitreversed */
	for (size_t x = 0; x != max; x++)
		pc[x] = (pa[x] * pb[x]) % mod;
}
