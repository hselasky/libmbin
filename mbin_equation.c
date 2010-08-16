/*-
 * Copyright (c) 2010 Hans Petter Selasky. All rights reserved.
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

#include <stdlib.h>

#include "math_bin.h"

uint32_t *
mbin_compress_tab_32(const uint32_t *ptr,
    uint32_t last, uint32_t slice)
{
	uint32_t *pcomp;
	uint32_t n;
	uint32_t c;

	c = 0;
	n = 0;

	while (1) {
		if (ptr[c] & slice)
			n++;

		if (c == last)
			break;

		c++;
	}

	n++;

	pcomp = malloc(sizeof(uint32_t) * n);

	if (pcomp == NULL)
		return (NULL);

	pcomp[0] = n - 1;

	c = 0;
	n = 1;

	while (1) {
		if (ptr[c] & slice) {
			pcomp[n] = c;
			n++;
		}
		if (c == last)
			break;
		c++;

	}

	return (pcomp);
}

void
mbin_expand_xor_tab_32(uint32_t *ptr, uint32_t *pcomp,
    uint32_t last, uint32_t slice)
{
	uint32_t n;
	uint32_t t;

	for (n = 0; n != pcomp[0]; n++) {
		t = pcomp[n + 1];
		if (t <= last)
			ptr[t] ^= slice;
	}
}

void
mbin_expand_add_tab_32(uint32_t *ptr, uint32_t *pcomp,
    uint32_t last, uint32_t slice)
{
	uint32_t n;
	uint32_t t;

	for (n = 0; n != pcomp[0]; n++) {
		t = pcomp[n + 1];
		if (t <= last)
			ptr[t] += slice;
	}
}

void
mbin_expand_sub_tab_32(uint32_t *ptr, uint32_t *pcomp,
    uint32_t last, uint32_t slice)
{
	uint32_t n;
	uint32_t t;

	for (n = 0; n != pcomp[0]; n++) {
		t = pcomp[n + 1];
		if (t <= last)
			ptr[t] -= slice;
	}
}

uint32_t *
mbin_foreach_tab_32(uint32_t *pcomp, uint32_t *ptr)
{
	uint32_t *end;

	end = pcomp + pcomp[0] + 1;

	if (ptr == NULL)
		ptr = pcomp + 1;
	else
		ptr++;

	if (ptr >= end)
		return (NULL);

	return (ptr);
}

void
mbin_free_tab_32(uint32_t *pcomp)
{
	if (pcomp == NULL)
		return;

	free(pcomp);
}

uint32_t
mbin_count_tab_32(uint32_t *pcomp)
{
	if (pcomp == NULL)
		return (0);

	return (pcomp[0]);
}
