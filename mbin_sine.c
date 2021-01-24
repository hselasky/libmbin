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

#include <math.h>

#include "math_bin.h"

/*
 * The following function generates the full sinus waveform from [0
 * to 360> degrees, using the floating point range [0..1.0>.
 *
 * The input and output noise floor for this function is 2**-32.
 */
float
mbin_sinf_32(float x)
{
	return (mbin_cosf_32(x + 0.75f));
}

/*
 * The following function computes the inverse of the function above.
 */
float
mbin_asinf_32(float x)
{
	return (0.25f - mbin_acosf_32(x));
}

/*
 * The following function generates the full cosinus waveform from [0
 * to 360> degrees, using the floating point range [0..1.0>.
 *
 * The input and output noise floor for this function is 2**-32.
 */
float
mbin_cosf_32(float _x)
{
	uint32_t x = (_x - floorf(_x)) * (1ULL << 32);
	double retval;
	uint8_t num;

	/* Apply "grey" encoding */
	for (uint32_t mask = 1U << 31; mask != 1; mask /= 2) {
		if (x & mask)
			x ^= (mask - 1);
	}

	/* Find first set bit */
	for (num = 0; num != 30; num++) {
		if (!(x & (1U << num)))
			break;
	}

	/* Initialize return value */
	retval = 0;

	/* Compute the rest of the square-root series */
	for (; num != 30; num++) {
		if (x & (1U << num))
			retval = sqrt((1.0 - retval) / 2.0);
		else
			retval = sqrt((1.0 + retval) / 2.0);
	}

	/* Check if halfway */
	if (x & (1ULL << 30))
		retval = -retval;

	return (retval);
}

/*
 * The following function computes the inverse of the function above.
 */
float
mbin_acosf_32(float _input)
{
	double input = _input;
	uint32_t retval = 0;

	/* Compute the rest of the squaring series */
	for (uint32_t m = 1U << 30; m != 0; m /= 2) {
		if (input <= 0)
			retval ^= (2 * m - 1);
		input = input * input * 2.0 - 1.0;
	}

	/* Apply sign and fixup value */
	return (float)retval / (float)(1ULL << 32);
}
