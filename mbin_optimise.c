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

/* XOR optimisation */

void
mbin_optimise_xor_32x32(uint32_t *ptr, uint32_t mask,
    uint32_t func_slices /* 2-bits */ , uint32_t tmp_slices /* 3-bits */ )
{
	uint32_t fv_slice;		/* function value slice */
	uint32_t fd_slice;		/* function defined slice */
	uint32_t wv_slice;		/* work value slice */
	uint32_t wd_slice;		/* work defined slice */
	uint32_t wt_slice;		/* work temp slice (result) */
	uint32_t last_x;
	uint32_t last_y;
	uint32_t x;
	uint32_t y;
	uint8_t value;

	fv_slice = ~(func_slices - 1) & func_slices;
	func_slices ^= fv_slice;
	fd_slice = ~(func_slices - 1) & func_slices;
	func_slices ^= fd_slice;

	wv_slice = ~(tmp_slices - 1) & tmp_slices;
	tmp_slices ^= wv_slice;
	wd_slice = ~(tmp_slices - 1) & tmp_slices;
	tmp_slices ^= wd_slice;
	wt_slice = ~(tmp_slices - 1) & tmp_slices;
	tmp_slices ^= wt_slice;

	/* cleanup "work" slices */
	x = 0;
	y = ~(wv_slice | wd_slice | wt_slice);
	while (1) {
		ptr[x] &= y;
		if (x == mask)
			break;
		x++;
	}

	/* Optimise truth table */
	x = 0;
	last_x = x;
	last_y = x;
	value = 2;
	while (1) {
		if (ptr[x] & fd_slice) {

			/* got a defined value */
			/* check defined value */

			if (ptr[x] & fv_slice) {

				/* got a defined one */
				/* check for value difference */
				if (value != 1) {
					value = 1;
					last_x = last_y;
				}
				/* move the one back */
				y = mbin_msb32(x ^ last_x);
				y = ((-y) & x);

				/* put new one in the table */
				ptr[y] |= wd_slice | wv_slice;

				/* update last "x" value */
				last_y = x;

			} else {

				/* got a defined zero */
				/* check for value difference */
				if (value != 0) {
					value = 0;
					last_x = last_y;
				}
				/* move the zero back */
				y = mbin_msb32(x ^ last_x);
				y = ((-y) & x);

				/* put new zero in the table */
				ptr[y] |= wd_slice;

				/* update last "x" value */
				last_y = x;
			}
		}
		if (x == mask)
			break;
		x++;
	}

	/* Optimise statement table */
	x = 0;
	while (1) {
		if (ptr[x] & wd_slice) {
			if (ptr[x] & wv_slice) {
				/* XOR in a one */
				mbin_expand_xor_32x32(ptr,
				    x, mask, wt_slice);
			}
		} else {
			/*
			 * Default action: Toggle statement to zero on
			 * unused value to reduce the expression.
			 */
			if (ptr[x] & wt_slice) {
				mbin_expand_xor_32x32(ptr,
				    x, mask, wt_slice);
			}
		}
		if (x == mask)
			break;
		x++;
	}
}
