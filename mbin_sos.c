/*-
 * Copyright (c) 2009 Hans Petter Selasky. All rights reserved.
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
 * Function to compute the Sum of Sums, SoS:
 *
 *  +-------------->  (y)
 *  | 1   0   0   0   0   0
 *  | 1   1   1   1   1   1
 *  | 1   2   3   4   5   6
 *  | 1   3   6  10  15  21
 *  | 1   4  10  20  35  56
 *  | 1   5  15  35  70 126
 *  | 1   6  21  56 126 252
 *  V
 *
 * (x)
 */

#include <stdint.h>

#include "math_bin.h"

uint32_t
mbin_sos_32(int32_t x, int32_t y)
{
	uint32_t rem;
	uint32_t div;
	uint32_t n;
	uint32_t temp;
	uint32_t fact;

	/* handle some special cases */

	if ((x < 0) || (y < 0))
		return (0);

	if (y == 0)
		return (1);

	if (x == 0)
		return (0);

	/* try to optimise */

	if (x < (y + 1)) {
		/* swap X and Y due to symmetry */
		temp = (x - 1);
		x = y + 1;
		y = temp;
	}

	rem = 1;
	div = 1;

	for (n = 0; n != y; n++) {
		/* compute scaling factor */
		temp = (~n) & (n + 1);
		div *= (n + 1) / temp;
		fact = (x + n);

		/* remove common divisor */
		while ((~fact) & (~temp) & 1) {
			fact /= 2;
			temp /= 2;
		}

		/* compute Sum of Sum */
		rem *= fact;
		rem /= temp;
	}

	/* for better accuracy modulo (2**n) division is used */

	rem = mbin_div_odd32(rem, div);

	return (rem);
}
