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

/* XOR - transform */

void
mbin_transform_xor_fwd_32x32(uint32_t *ptr, uint32_t mask,
    uint32_t f_slice, uint32_t t_slice)
{
	uint32_t x;

	/* cleanup "t_slice" */
	x = 0;
	while (1) {
		ptr[x] &= ~t_slice;
		if (x == mask)
			break;
		x++;
	}

	/* transform "f" */
	x = 0;
	while (1) {
		if (ptr[x] & f_slice)
			mbin_expand_xor_32x32(ptr, x, mask, t_slice);
		if (x == mask)
			break;
		x++;
	}
}

/* XOR - transform */

void
mbin_transform_multi_xor_fwd_32x32(uint32_t *ptr, uint32_t *temp,
    uint32_t mask)
{
	uint32_t x;
	uint32_t val;

	/* cleanup "t_slice" */
	x = 0;
	while (1) {
		temp[x] = ptr[x];
		if (x == mask)
			break;
		x++;
	}

	/* transform "f" */
	x = 0;
	while (1) {
		val = temp[x];
		if (val) {
			/* expand logic expression */
			mbin_expand_xor_32x32(temp, x, mask, val);
			/* restore original value */
			temp[x] = val;
		}
		if (x == mask)
			break;
		x++;
	}
}

/* XOR-ADD - transform */

void
mbin_transform_add_fwd_32x32(uint32_t *ptr, uint32_t *temp,
    uint32_t mask)
{
	uint32_t x;
	uint32_t val;

	/* cleanup "t_slice" */
	x = 0;
	while (1) {
		temp[x] = ptr[x];
		if (x == mask)
			break;
		x++;
	}

	/* transform "f" */
	x = 0;
	while (1) {
		val = temp[x];
		if (val) {
			/* expand logic expression */
			mbin_expand_add_32x32(temp, x, mask, val);
			/* restore original value */
			temp[x] = val;
		}
		if (x == mask)
			break;
		x++;
	}
}

/* Greater Equal, GTE - transform */

void
mbin_transform_gte_fwd_32x32(uint32_t *ptr, uint32_t *temp,
    uint32_t mask)
{
	uint32_t x;
	uint32_t val;

	/* cleanup "t_slice" */
	x = 0;
	while (1) {
		temp[x] = ptr[x];
		if (x == mask)
			break;
		x++;
	}

	/* transform "f" */
	x = 0;
	while (1) {
		val = temp[x];
		if (val) {
			/* expand logic expression */
			mbin_expand_gte_32x32(temp, x, mask, val);
			/* restore original value */
			temp[x] = val;
		}
		if (x == mask)
			break;
		x++;
	}
}
