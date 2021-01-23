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
 * The following function generates the full sinus waveform from [0 to
 * 360> degrees, using the floating point range [0..1.0>.
 *
 * The input and output noise floor for this function is 2**-32.
 */
float
mbin_sinf_32(float _x)
{
	uint32_t x = (_x - floorf(_x)) * (1ULL << 32);
	double retval;
	uint8_t num;
	uint8_t quadrant;

	/* Apply "grey" encoding */
	for (uint32_t mask = 1U << 29; mask != 1; mask /= 2) {
		if (x & mask)
			x ^= (mask - 1);
	}

	quadrant = (x >> 30);

	/* Figure out quadrant */
	switch (quadrant) {
	case 0:
		if (x == 0)
			return (0.0);
		break;
	case 1:
		if (x == 0x40000000U)
			return (1.0);
		break;
	case 2:
		if (x == 0x80000000U)
			return (0.0);
		break;
	case 3:
		if (x == 0xC0000000U)
			return (-1.0);
		break;
	default:
		break;
	}

	/* Find first cleared bit, that is where it starts */
	for (num = 0; num != 30; num++) {
		if (!(x & (1U << num))) {
			num++;
			break;
		}
	}

	/* Initialize return value */
	retval = sqrt(0.5);

	/* Compute the rest of the square-root series */
	for (; num != 30; num++) {
		if (x & (1U << num))
			retval = sqrt((1.0 - retval) / 2.0);
		else
			retval = sqrt((1.0 + retval) / 2.0);
	}

	/* Use Pythagoras to fixup final value, if required */
	switch (quadrant) {
	case 0:
		retval = sqrt(1.0 - retval * retval);
		break;
	case 2:
		retval = -sqrt(1.0 - retval * retval);
		break;
	case 3:
		retval = -retval;
		break;
	default:
		break;
	}
	return (retval);
}

/*
 * The following function computes the inverse of the function above.
 */
float
mbin_asinf_32(float _input)
{
	double input = _input;
	double limit;
	uint32_t retval;

	/* Check for some well known values */
	if (input == 0.0)
		return (0);
	else if (input == -1.0)
		return (-0.25f);
	else if (input == 1.0)
		return (0.25f);

	/* Detect which quadrant the value is in. */
	limit = sqrt(0.5);
	if (input < 0.0) {
		if (input < -limit) {
			input = -input;
			retval = 0xBFFFFFFFU;
		} else {
			input = sqrt(1.0 - input * input);
			retval = 0x80000000U;
		}
	} else {
		if (input < limit) {
			input = sqrt(1.0 - input * input);
			retval = 0;
		} else {
			retval = 0x3FFFFFFFU;
		}
	}

	/* Compute the rest of the squaring series */
	for (uint32_t m = 1U << 29; m != 0; m /= 2) {
		input = input * input * 2.0 - 1.0;
		if (input <= 0)
			retval ^= (2 * m - 1);
	}

	/* Apply sign and fixup value */
	if (retval & 0x80000000U)
		return -(float)(retval & 0x7FFFFFFFU) / (float)(1ULL << 32);
	else
		return (float)(retval & 0x7FFFFFFFU) / (float)(1ULL << 32);
}
