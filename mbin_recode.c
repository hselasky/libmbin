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

uint32_t
mbin_recodeA_fwd32(uint32_t val, const uint8_t *premap)
{
	uint32_t temp = 0;

	while (val) {
		if (val & 1)
			temp |= 1 << premap[0];
		if (val & 2)
			temp |= 1 << premap[1];
		if (val & 4)
			temp |= 1 << premap[2];
		if (val & 8)
			temp |= 1 << premap[3];

		val >>= 4;
		premap += 4;
	}
	return (temp);
}

uint16_t
mbin_recodeA_fwd16(uint16_t val, const uint8_t *premap)
{
	uint16_t temp = 0;

	while (val) {
		if (val & 1)
			temp |= 1 << premap[0];
		if (val & 2)
			temp |= 1 << premap[1];
		if (val & 4)
			temp |= 1 << premap[2];
		if (val & 8)
			temp |= 1 << premap[3];

		val >>= 4;
		premap += 4;
	}
	return (temp);
}

uint8_t
mbin_recodeA_fwd8(uint8_t val, const uint8_t *premap)
{
	uint8_t temp = 0;

	while (val) {
		if (val & 1)
			temp |= 1 << premap[0];
		if (val & 2)
			temp |= 1 << premap[1];
		if (val & 4)
			temp |= 1 << premap[2];
		if (val & 8)
			temp |= 1 << premap[3];

		val >>= 4;
		premap += 4;
	}
	return (temp);
}

void
mbin_recodeA_def(uint8_t *premap, uint8_t start, uint8_t max)
{
	uint8_t x;
	uint8_t y;

	for (x = 0; x != max; x++) {
		for (y = 0; y != start; y++) {
			if (premap[y] == x) {
				break;
			}
		}
		if (y != start) {
			continue;
		}
		if (start == max) {
			break;
		}
		premap[start++] = x;
	}
	return;
}

void
mbin_recodeA_inv(const uint8_t *src, uint8_t *dst, uint8_t max)
{
	uint8_t x;
	uint8_t y;

	for (x = 0; x != max; x++) {
		for (y = 0;; y++) {
			if (y == max) {
				dst[x] = 0 - 1;
				break;
			} else if (src[y] == x) {
				dst[x] = y;
				break;
			}
		}
	}
	return;
}

uint32_t
mbin_recodeB_fwd32(uint32_t x, const uint32_t *ptr)
{
	uint32_t m = 1;
	uint32_t val = 0;

	while (m) {
	  val |= (x + ptr[0]) & m;
	  m *= 2;
	  ptr++;
	}
	return (val);
}

uint32_t
mbin_recodeB_inv32(uint32_t val, const uint32_t *ptr)
{
	uint32_t m = 1;
	uint32_t x = 0;

	while (m) {
	  if (((x + ptr[0]) ^ val) & m) {
		x |= m;
	  }
	  m *= 2;
	  ptr++;
	}
	return (x);
}

