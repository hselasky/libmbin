/*-
 * Copyright (c) 2010 Hans Petter Selasky
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

/*
 * Function to compute the binary coefficient, which is similar to the
 * Sum of Sums.
 *
 * +---------------->  (x)
 * | 1
 * | 1   1
 * | 1   2   1
 * | 1   3   3   1
 * | 1   4   6   4   1
 * | 1   5  10  10   5   1
 * | 1   6  15  20  15   6   1
 * | 1   7  21  35  35  21   7   1
 * V
 *
 *(n)
 */

uint32_t
mbin_coeff_32(int32_t n, int32_t x)
{
	uint32_t shift = 1 << 16;
	uint32_t lsb;
	uint32_t y;
	uint32_t fa = 1;
	uint32_t fb = 1;

	/* check limits */
	if ((n < 0) || (x < 0))
		return (0);

	if (x > n)
		return (0);

	/* handle special case */
	if (x == n || x == 0)
		return (1);

	for (y = 0; y != x; y++) {
		lsb = (y - n) & (n - y);
		shift *= lsb;
		fa *= (n - y) / lsb;

		lsb = (-y - 1) & (y + 1);
		shift /= lsb;
		fb *= (y + 1) / lsb;
	}
	return (mbin_div_odd32(fa, fb) * (shift >> 16));
}
