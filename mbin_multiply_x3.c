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

/* Set logarithmic limit for switching to classic multiplication: */

#ifndef MBIN_X3_LOG2_COMBA
#define	MBIN_X3_LOG2_COMBA 6
#endif

/* Assert sane limit: */

#if (MBIN_X3_LOG2_COMBA < 1)
#error "MBIN_X3_LOG2_COMBA must be greater than 0"
#endif

/*
 * Helper structure to save some pointer passing. The structure size
 * is aligned to 32-bytes to avoid multiplication in array lookups.
 */
struct mbin_x3_mul_input_double {
	double	a;
	double	b;
} __aligned(16);

/*
 * This function take one input array input[0..stride-1] and accumulate
 * the resulting product into ptr_low[0..stride-1] and
 * ptr_high[0..stride-1] for the low and high parts respectivly.
 */
static void
mbin_x3_multiply_sub_double(struct mbin_x3_mul_input_double *input, double *ptr_low, double *ptr_high,
    const size_t stride, const uint8_t toggle)
{
	size_t x;
	size_t y;

	/*
	 * Check for small multiplications, because they run faster
	 * the classic way than by using the transform on the CPU:
	 */
	if (stride >= (1UL << MBIN_X3_LOG2_COMBA)) {
		const size_t strideh = stride >> 1;

		/*
		 * Optimise use of transforms to avoid having to
		 * balance the inverse and forward transforms before
		 * returning from this function:
		 */
		if (toggle) {

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

			mbin_x3_multiply_sub_double(input, ptr_low, ptr_low + strideh, strideh, toggle);

			for (x = 0; x != strideh; x++)
				ptr_low[x + strideh] = -ptr_low[x + strideh];

			mbin_x3_multiply_sub_double(input + strideh, ptr_low + strideh, ptr_high + strideh, strideh, toggle);

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

			mbin_x3_multiply_sub_double(input + strideh, ptr_low + strideh, ptr_high, strideh, !toggle);
		} else {
			mbin_x3_multiply_sub_double(input + strideh, ptr_low + strideh, ptr_high, strideh, !toggle);

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

			mbin_x3_multiply_sub_double(input + strideh, ptr_low + strideh, ptr_high + strideh, strideh, toggle);

			for (x = 0; x != strideh; x++)
				ptr_low[x + strideh] = -ptr_low[x + strideh];

			mbin_x3_multiply_sub_double(input, ptr_low, ptr_low + strideh, strideh, toggle);

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
#if (MBIN_X3_LOG2_COMBA == 1)
		ptr_low[0] += input[0].a * input[0].b;
#else
		for (x = 0; x != stride; x++) {
			double value = input[x].a;

			/* optimise multiplication by zero */
			if (value == 0.0)
				continue;
			/* compute low-part of product */
			for (y = 0; y != (stride - x); y++) {
				ptr_low[x + y] += input[y].b * value;
			}
			/* compute high-part of product */
			for (; y != stride; y++) {
				ptr_high[x + y - stride] += input[y].b * value;
			}
		}
#endif
	}
}

/*
 * This function take the two input arrays va[0..max-1] and
 * vb[0..max-1] and compute their product into pc[0..2max-1]:
 */
void
mbin_x3_multiply_double(const double *va, const double *vb, double *pc, const size_t max)
{
	struct mbin_x3_mul_input_double input[max];
	size_t x;

	/*
	 * The transform is only supported for powers of two.
	 * Return otherwise.
	 */
	if (max & (max - 1))
		return;

	/* setup input vector */
	for (x = 0; x != max; x++) {
		input[x].a = va[x];
		input[x].b = vb[x];
	}

	/* clear output vector */
	memset(pc, 0, (2 * sizeof(double)) * max);

	/* do multiplication */
	mbin_x3_multiply_sub_double(input, pc, pc + max, max, 1);
}

/*
 * Helper structure to save some pointer passing. The structure size
 * is aligned to 16-bytes to avoid multiplication in array lookups.
 */
struct mbin_x3_sqr_input_double {
	double	a;
} __aligned(8);

/*
 * This function take one input array input[0..stride-1] and accumulate
 * the resulting square product into ptr_low[0..stride-1] and
 * ptr_high[0..stride-1] for the low and high parts respectivly.
 */
static void
mbin_x3_square_sub_double(struct mbin_x3_sqr_input_double *input, double *ptr_low, double *ptr_high,
    const size_t stride, const uint8_t toggle)
{
	size_t x;
	size_t y;

	/*
	 * Check for small multiplications, because they run faster
	 * the classic way than by using the transform on the CPU:
	 */
	if (stride >= (1UL << MBIN_X3_LOG2_COMBA)) {
		const size_t strideh = stride >> 1;

		/*
		 * Optimise use of transforms to avoid having to
		 * balance the inverse and forward transforms before
		 * returning from this function:
		 */
		if (toggle) {

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

			mbin_x3_square_sub_double(input, ptr_low, ptr_low + strideh, strideh, toggle);

			for (x = 0; x != strideh; x++)
				ptr_low[x + strideh] = -ptr_low[x + strideh];

			mbin_x3_square_sub_double(input + strideh, ptr_low + strideh, ptr_high + strideh, strideh, toggle);

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
			}

			mbin_x3_square_sub_double(input + strideh, ptr_low + strideh, ptr_high, strideh, !toggle);
		} else {
			mbin_x3_square_sub_double(input + strideh, ptr_low + strideh, ptr_high, strideh, !toggle);

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
			}

			mbin_x3_square_sub_double(input + strideh, ptr_low + strideh, ptr_high + strideh, strideh, toggle);

			for (x = 0; x != strideh; x++)
				ptr_low[x + strideh] = -ptr_low[x + strideh];

			mbin_x3_square_sub_double(input, ptr_low, ptr_low + strideh, strideh, toggle);

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
#if (MBIN_X3_LOG2_COMBA == 1)
		ptr_low[0] += input[0].a * input[0].a;
#else
		for (x = 0; x != stride; x++) {
			double value = input[x].a;

			/* optimise multiplication by zero */
			if (value == 0.0)
				continue;
			/* compute low-part of product */
			for (y = 0; y != (stride - x); y++) {
				ptr_low[x + y] += input[y].a * value;
			}
			/* compute high-part of product */
			for (; y != stride; y++) {
				ptr_high[x + y - stride] += input[y].a * value;
			}
		}
#endif
	}
}

/*
 * This function take the input array va[0..max-1] and
 * compute the square product into pc[0..2max-1]:
 */
void
mbin_x3_square_double(const double *va, double *pc, const size_t max)
{
	struct mbin_x3_sqr_input_double input[max];
	size_t x;

	/*
	 * The transform is only supported for powers of two.
	 * Return otherwise.
	 */
	if (max & (max - 1))
		return;

	/* setup input vector */
	for (x = 0; x != max; x++)
		input[x].a = va[x];

	/* clear output vector */
	memset(pc, 0, (2 * sizeof(double)) * max);

	/* do multiplication */
	mbin_x3_square_sub_double(input, pc, pc + max, max, 1);
}
