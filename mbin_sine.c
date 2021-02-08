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

	/* Handle special cases, if any */
	switch (x) {
	case 0xFFFFFFFFU:
	case 0x00000000U:
		return (1.0f);
	case 0x3FFFFFFFU:
	case 0x40000000U:
	case 0xBFFFFFFFU:
	case 0xC0000000U:
		return (0.0f);
	case 0x7FFFFFFFU:
	case 0x80000000U:
		return (-1.0f);
	}

	/* Apply "grey" encoding */
	for (uint32_t mask = 1U << 31; mask != 1; mask /= 2) {
		if (x & mask)
			x ^= (mask - 1);
	}

	/* Find first set bit */
	for (num = 0; num != 30; num++) {
		if (x & (1U << num)) {
			num++;
			break;
		}
	}

	/* Initialize return value */
	retval = 0.0;

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

/*
 * Selected values of "power":
 * 0.0: square wave
 * 0.5: cosine wave
 * 1.0: triangle function
 *
 * Phase is given by "x" argument.
 */
float
mbin_cospowf_32(float _x, float _power)
{
	uint32_t x = (_x - floorf(_x)) * (1ULL << 32);
	double retval;
	uint8_t num;

	/* Handle special cases, if any */
	switch (x) {
	case 0xFFFFFFFFU:
	case 0x00000000U:
		return (1.0f);
	case 0x3FFFFFFFU:
	case 0x40000000U:
	case 0xBFFFFFFFU:
	case 0xC0000000U:
		return (0.0f);
	case 0x7FFFFFFFU:
	case 0x80000000U:
		return (-1.0f);
	}

	/* Apply "grey" encoding */
	for (uint32_t mask = 1U << 31; mask != 1; mask /= 2) {
		if (x & mask)
			x ^= (mask - 1);
	}

	/* Find first set bit */
	for (num = 0; num != 30; num++) {
		if (x & (1U << num)) {
			num++;
			break;
		}
	}

	/* Initialize return value */
	retval = 0.0;

	/* Compute the rest of the power series */
	for (; num != 30; num++) {
		if (x & (1U << num))
			retval = pow((1.0 - retval) / 2.0, _power);
		else
			retval = pow((1.0 + retval) / 2.0, _power);
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
mbin_acospowf_32(float _input, float _power)
{
	double input = _input;
	uint32_t retval = 0;

	/* Compute the rest of the power series */
	for (uint32_t m = 1U << 30; m != 0; m /= 2) {
		if (input <= 0) {
			retval ^= (2 * m - 1);
			/* Only want absolute values */
			input = -input;
		}
		input = pow(input, _power) * 2.0 - 1.0;
	}

	/* Apply sign and fixup value */
	return (float)retval / (float)(1ULL << 32);
}

float
mbin_sinpowf_32(float x, float power)
{
	return (mbin_cospowf_32(x + 0.75f, power));
}

float
mbin_asinpowf_32(float x, float power)
{
	return (0.25f - mbin_acospowf_32(x, power));
}

float
mbin_powgainf_2d(float x, float y, float power)
{
	const float invpower = 1.0f / power;

	return (powf(powf(fabs(x), invpower) +
		     powf(fabs(y), invpower), power));
}

float
mbin_powgainf_3d(float x, float y, float z, float power)
{
	const float invpower = 1.0f / power;

	return (powf(powf(fabs(x), invpower) +
		     powf(fabs(y), invpower) +
		     powf(fabs(z), invpower), power));
}

mbin_cf_t
mbin_powmul_cf(mbin_cf_t a, mbin_cf_t b, float power)
{
	float ga = mbin_powgainf_2d(a.x, a.y, power);
	float gb = mbin_powgainf_2d(b.x, b.y, power);

	if (ga != 0.0) {
		a.x /= ga;
		a.y /= ga;
	}

	if (gb != 0.0) {
		b.x /= gb;
		b.y /= gb;
	}

	float gr = ga * gb;

	uint8_t qa = (a.x < 0) + 2 * (a.y < 0);
	uint8_t qb = (b.x < 0) + 2 * (b.y < 0);

	float angle = 0;

	switch (qa) {
	case 0:
		angle += mbin_acospowf_32(fabs(a.x), power);
		break;
	case 1:
		angle += 0.5f - mbin_acospowf_32(fabs(a.x), power);
		break;
	case 2:
		angle += 1.0f - mbin_acospowf_32(fabs(a.x), power);
		break;
	case 3:
		angle += 0.5f + mbin_acospowf_32(fabs(a.x), power);
		break;
	default:
		break;
	}

	switch (qb) {
	case 0:
		angle += mbin_acospowf_32(fabs(b.x), power);
		break;
	case 1:
		angle += 0.5f - mbin_acospowf_32(fabs(b.x), power);
		break;
	case 2:
		angle += 1.0f - mbin_acospowf_32(fabs(b.x), power);
		break;
	case 3:
		angle += 0.5f + mbin_acospowf_32(fabs(b.x), power);
		break;
	default:
		break;
	}

	return (mbin_cf_t){
	    mbin_cospowf_32(angle, power) * gr,
	    mbin_sinpowf_32(angle, power) * gr
	};
}
