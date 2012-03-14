/*-
 * Copyright (c) 2012 Hans Petter Selasky. All rights reserved.
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

void
mbin_xor_factor_init(struct mbin_xor_factor_state *st,
    uint32_t *ptr, uint8_t lmax)
{
	memset(st, 0, sizeof(*st));

	st->ptr = ptr;
	st->max = 1U << lmax;
	st->lmax = lmax;
}

uint8_t
mbin_xor_factor(struct mbin_xor_factor_state *st,
    uint32_t *pa, uint32_t *pb, uint32_t *pc)
{
	uint32_t a = st->ka;
	uint32_t b = st->kb;
	uint32_t max = st->max;
	uint32_t *ptr = st->ptr;
	uint32_t x;
	uint32_t na;
	uint32_t nb;
	uint32_t ns;

repeat:

	if (b >= max) {
		b = 0;
		a++;
	}
	if (b <= a) {
		b = a + 1;
		if (b >= max)
			goto done;
	}
	if (a & b) {
		b++;
		goto repeat;
	}
	na = 0;
	nb = 0;
	ns = 0;

	memset(pa, 0, sizeof(pa[0]) * max);
	memset(pb, 0, sizeof(pb[0]) * max);

	for (x = 0; x != max; x++) {

		if (ptr[x])
			ns++;

		if ((x & a) == a && ptr[x]) {
			na++;
			pa[x & ~a] = ptr[x];
		}
		if ((x & b) == b && ptr[x]) {
			nb++;
			pb[x & ~b] = ptr[x];
		}
	}

	pa[a | b] = 0;
	pb[a | b] = 0;

	if (na < 2 || na == ns) {
		a++;
		goto repeat;
	}
	if (nb < 2 || nb == ns) {
		b++;
		goto repeat;
	}
	mbin_xor_xform_32(pa, st->lmax);
	mbin_xor_xform_32(pb, st->lmax);

	for (x = 0; x != max; x++) {
		pc[x] = pa[x] & pb[x];
	}

	mbin_xor_xform_32(pa, st->lmax);
	mbin_xor_xform_32(pb, st->lmax);
	mbin_xor_xform_32(pc, st->lmax);

	for (x = ns = 0; x != max; x++) {
		if (pc[x])
			ns++;
	}

	b++;
	st->ka = a;
	st->kb = b;
	st->na = na;
	st->nb = nb;
	st->nc = ns;
	return (1);

done:
	st->ka = st->kb = 0;
	return (0);
}
