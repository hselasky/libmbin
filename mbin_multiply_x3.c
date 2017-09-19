/*-
 * Copyright (c) 2017 Hans Petter Selasky. All rights reserved.
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
#include <string.h>

#include "math_bin.h"

#ifndef MBIN_X3_LOG2_COMBA
#define	MBIN_X3_LOG2_COMBA 6
#endif

#if (MBIN_X3_LOG2_COMBA < 2)
#error "MBIN_X3_LOG2_COMBA must be greater than 1"
#endif

struct mbin_x3_input_double {
	double	a;
	double	b;
	size_t	toggle;
} __aligned(32);

/*
 * <input size> = "stride"
 * <output size> = 2 * "stride"
 */
static void
mbin_x3_multiply_sub_double(struct mbin_x3_input_double *input, double *ptr_low, double *ptr_high, const size_t stride)
{
	size_t x;
	size_t y;

	if (stride >= (1UL << MBIN_X3_LOG2_COMBA)) {
		const size_t strideh = stride >> 1;

		input->toggle ^= stride;

		if (input->toggle & stride) {

			/* inverse step */
			for (x = 0; x != strideh; x++) {
				double a, b, c, d;

				a = ptr_low[x];
				b = ptr_low[x + strideh];
				c = ptr_high[x];
				d = ptr_high[x + strideh];

				ptr_low[x + strideh] = a + b;
				ptr_high[x] = a + b + c + d;
			}

			mbin_x3_multiply_sub_double(input, ptr_low, ptr_low + strideh, strideh);

			for (x = 0; x != strideh; x++)
				ptr_low[x + strideh] = -ptr_low[x + strideh];

			mbin_x3_multiply_sub_double(input + strideh, ptr_low + strideh, ptr_high + strideh, strideh);

			/* forward step */
			for (x = 0; x != strideh; x++) {
				double a, b, c, d;

				a = ptr_low[x];
				b = ptr_low[x + strideh];
				c = ptr_high[x];
				d = ptr_high[x + strideh];

				ptr_low[x + strideh] = -a - b;
				ptr_high[x] = c + b - d;

				input[x + strideh].a += input[x].a;
				input[x + strideh].b += input[x].b;
			}

			mbin_x3_multiply_sub_double(input + strideh, ptr_low + strideh, ptr_high, strideh);
		} else {
			mbin_x3_multiply_sub_double(input + strideh, ptr_low + strideh, ptr_high, strideh);

			/* inverse step */
			for (x = 0; x != strideh; x++) {
				double a, b, c, d;

				a = ptr_low[x];
				b = ptr_low[x + strideh];
				c = ptr_high[x];
				d = ptr_high[x + strideh];

				ptr_low[x + strideh] = -a - b;
				ptr_high[x] = a + b + c + d;

				input[x + strideh].a -= input[x].a;
				input[x + strideh].b -= input[x].b;
			}

			mbin_x3_multiply_sub_double(input + strideh, ptr_low + strideh, ptr_high + strideh, strideh);

			for (x = 0; x != strideh; x++)
				ptr_low[x + strideh] = -ptr_low[x + strideh];

			mbin_x3_multiply_sub_double(input, ptr_low, ptr_low + strideh, strideh);

			/* forward step */
			for (x = 0; x != strideh; x++) {
				double a, b, c, d;

				a = ptr_low[x];
				b = ptr_low[x + strideh];
				c = ptr_high[x];
				d = ptr_high[x + strideh];

				ptr_low[x + strideh] = b - a;
				ptr_high[x] = c - b - d;
			}
		}
	} else {
		for (x = 0; x != stride; x++) {
			double value = input[x].a;

			if (value == 0.0)
				continue;
			for (y = 0; y != (stride - x); y++) {
				ptr_low[x + y] += input[y].b * value;
			}
			for (; y != stride; y++) {
				ptr_high[x + y - stride] += input[y].b * value;
			}
		}
	}
}

/*
 * <input size> = "max"
 * <output size> = 2 * "max"
 */
void
mbin_x3_multiply_double(double *va, double *vb, double *pc, const size_t max)
{
	struct mbin_x3_input_double input[max];
	size_t x;

	/* check for non-power of two */
	if (max & (max - 1))
		return;

	/* setup input vector */
	for (x = 0; x != max; x++) {
		input[x].a = va[x];
		input[x].b = vb[x];
		input[x].toggle = 0;
	}

	/* clear output vector */
	memset(pc, 0, (2 * sizeof(double)) * max);

	/* do multiplication */
	mbin_x3_multiply_sub_double(input, pc, pc + max, max);
}
