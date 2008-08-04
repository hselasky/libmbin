/*-
 * Copyright (c) 2008 Hans Petter Selasky. All rights reserved.
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
 * NOTE: "mask" and "set_bit" will be remapped by "premap"!
 */
void
mbin_optimise_32x32(uint32_t *ptr, const uint8_t *premap,
    uint32_t mask, uint32_t set_bit,
    uint32_t def_slice, uint32_t work_slice /* destination */ )
{
	uint32_t last_x;		/* last defined "x" */
	uint32_t r_mask;
	uint32_t x;
	uint32_t y;
	uint32_t zs;
	uint32_t zc;

	/* Remove "set_bit" from mask */
	r_mask = mask & ~set_bit;

	/*
	 * Or "set_bit" with the complement of the "mask" before
	 * reordering the mask!
	 */
	set_bit |= (~mask);

	/* re-order mask bits */
	if (premap) {
		r_mask = mbin_recode32(r_mask, premap);
		mask = mbin_recode32(mask, premap);
	}
	/* cleanup "work" slice */
	x = set_bit;
	y = ~work_slice;
	while (1) {
		if (premap) {
			zs = mbin_recode32(x, premap);
			zc = mbin_recode32(x ^ set_bit, premap);
		} else {
			zs = x;
			zc = x ^ set_bit;
		}
		ptr[zs & mask] &= y;
		ptr[zc & mask] &= y;

		if (x == (0 - 1))
			break;
		x = mbin_inc32(x, set_bit);
	}

	/* do an optimised transform which can take some time */
	x = set_bit;
	last_x = x;
	while (1) {
		if (premap) {
			zs = mbin_recode32(x, premap);
			zc = mbin_recode32(x ^ set_bit, premap);
		} else {
			/* this variable has "set_bit" */
			zs = x;
			/* this variable does not have "set_bit" */
			zc = x ^ set_bit;
		}

		if (ptr[zs & mask] & def_slice) {

			mbin_expand_32x32(ptr, zc,
			    r_mask, work_slice);

			/*
			 * Check if we have a one here and move it
			 * further back, to reduce the logic required!
			 */
			y = mbin_msb32(x ^ last_x);
			y = ((-y) | set_bit) & x;

			if ((ptr[zc & mask] & work_slice) && (y != x)) {

				if (premap) {
					zc = mbin_recode32(y ^
					    set_bit, premap);
				} else {
					zc = y ^ set_bit;
				}

				if (!(ptr[zc & mask] & work_slice)) {
					/*
					 * Toggle expression to one so
					 * that we get a zero at the
					 * other place!
					 */
					mbin_expand_32x32(ptr, zc,
					    r_mask, work_slice);
				}
				while (1) {
					y = mbin_inc32(y, set_bit);
					if (y == x)
						break;	/* we are done */

					if (premap)
						zc = mbin_recode32(y ^
						    set_bit, premap);
					else
						zc = y ^ set_bit;

					if (ptr[zc & mask] & work_slice) {
						/*
					         * Toggle expression
					         * to zero.
					         */
						mbin_expand_32x32(ptr, zc,
						    r_mask, work_slice);
					}
				}
			}
			last_x = x;
		} else if (ptr[zc & mask] & def_slice) {
			/* The function is defined to "zero" */
			last_x = x;
		} else {
			/*
			 * Default action: Toggle value to zero on
			 * unused to reduce the expression.
			 */
			if (ptr[zc & mask] & work_slice) {
				mbin_expand_32x32(ptr, zc,
				    r_mask, work_slice);
			}
		}
		if (x == (uint32_t)(0 - 1))
			break;
		x = mbin_inc32(x, set_bit);
	}
	return;
}
