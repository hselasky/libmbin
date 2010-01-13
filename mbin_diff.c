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

#include <stdint.h>

#include "math_bin.h"

uint32_t
mbin_integrate_32(uint32_t *ptr, uint32_t max)
{
	uint32_t sum = 0;
	uint32_t n;

	for (n = 0; n != max; n++) {
		sum += ptr[n];
		ptr[n] = sum;
	}
	return (sum);
}

void
mbin_derivate_32(uint32_t *ptr, uint32_t max)
{
	uint32_t last = 0;
	uint32_t temp;
	uint32_t n;

	for (n = 0; n != max; n++) {
		temp = ptr[n];
		ptr[n] -= last;
		last = temp;
	}
}

uint32_t
mbin_sum_32(uint32_t *ptr, uint32_t max, uint8_t sstep)
{
	uint32_t sum = 0;
	uint32_t n;
	uint8_t shift = 0;

	for (n = 0; n != max; n++) {
		sum += ptr[n] << shift;

		shift += sstep;
		if (shift >= 32)
			break;
	}
	return (sum);
}
