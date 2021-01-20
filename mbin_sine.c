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
 * The following function generates the sinus waveform from [0 to 90>
 * degrees, using the integer range [0 ... 0xFFFFFFFFU]. In this case
 * PI is 2 ** 32 !
 */
double
mbin_sin_32(uint32_t x)
{
	double retval = sqrt(0.5);

	if (x == 0x80000000U)
		return (retval);
	else if (x & 0x80000000U)
		x ^= 0x2AAAAAAAU;
	else
		x ^= 0x55555555;

	for (uint32_t mask = 1U << 31; (mask /= 2); ) {
		if (x & mask)
			x ^= (mask - 1);
	}

	for (uint8_t num = 32; num--; ) {
		if (x & 1)
			retval = sqrt((1.0 + retval) / 2.0);
		else
			retval = sqrt((1.0 - retval) / 2.0);
		x /= 2;
	}
	return (retval);
}

/*
 * The following function computes the inverse of the function above,
 * and returns a value in the range [0 ... 0xFFFFFFFFU].
 * The sign of the input value is ignored.
 */
uint32_t
mbin_asin_32(double input)
{
	uint32_t retval = 0x7FFFFFFF;

	for (uint32_t m = 1U << 31; m != 0; m /= 2) {
		input = input * input * 2.0 - 1.0;

		if (input > 0)
			retval ^= m;
		if (retval & m)
			retval ^= (m / 2);
	}
#if 1
	/* XXX check if we should round up XXX */
	if ((retval & 1) != 0 && retval != 0xFFFFFFFFU)
		retval++;
#endif
	return (retval);
}
