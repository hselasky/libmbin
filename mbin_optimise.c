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
 * Return values:
 * 0: Success
 * Else: Failure
 */

uint8_t
mbin_optimise_32x32(uint32_t *ptr, uint32_t mask,
    uint32_t remove_bits, uint32_t def_slice,
    uint32_t work_slice, uint32_t temp_slice)
{
	uint32_t x;
	uint32_t y;

	/* basic cleanup */
	x = 0;
	while (1) {
		if ((ptr[x] & work_slice) &&
		    (!(ptr[x] & def_slice))) {
			mbin_expand_32x32(ptr, x, mask, work_slice);
		}
		if (x == mask)
			break;
		x++;
	}

	if (remove_bits == 0)
		return (0);		/* nothing more to do */

	y = 0;
	while (1) {

		if ((y & remove_bits) &&
		    (ptr[y] & work_slice)) {
			/*
			 * We found an expression with the bits we
			 * want to remove !
			 */
		} else {
			if (y == mask)
				break;
			y++;
			continue;
		}

		/* cleanup the temporary slice */
		x = y;
		while (1) {
			ptr[x] &= ~temp_slice;
			if (x == mask)
				break;
			x = mbin_inc32(x, y);
		}

		/* compute the substitution mask */
		x = y;
		while (1) {
			/*
			 * Check if "x" is defined
			 *
			 * Then check if the "temp_slice" is defined,
			 * but the entry it represents is not defined,
			 * so that it can be removed.
			 */
			if ((ptr[x] & def_slice) ||
			    ((ptr[x] & temp_slice) &&
			    (!(ptr[x & ~y] & def_slice)))) {
				mbin_expand_32x32(ptr, x, mask, temp_slice);
			}
			/*
			 * Check if there is an expression with the
			 * remove bits:
			 */
			if ((ptr[x] & temp_slice) &&
			    (x & ~y & remove_bits)) {
				/* not possible */
				return (1);
			}
			if (x == mask)
				break;
			x = mbin_inc32(x, y);
		}

		/* do the substitution */
		ptr[y] ^= work_slice;

		/* insert replacement */
		x = y;
		while (1) {
			if (ptr[x] & temp_slice) {
				ptr[x & ~y] ^= work_slice;
			}
			if (x == mask)
				break;
			x = mbin_inc32(x, y);
		}
	}
	return (0);
}
