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
#include <stdio.h>

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

uint8_t
mbin_xor_factor_find(uint32_t *ptr, uint32_t *pa,
    uint32_t *pb, uint32_t *pc, uint8_t lmax)
{
	struct mbin_xor_factor_state fs;
	uint32_t score;
	uint32_t keys;
	uint32_t temp;
	uint32_t mka;
	uint32_t mkb;
	uint32_t c;
	uint32_t d;
	uint32_t e;
	uint32_t x;
	uint8_t any = 0;

	score = 0;
	keys = -1U;
	mka = 0;
	mkb = 0;

	mbin_xor_factor_init(&fs, ptr, lmax);

	while (mbin_xor_factor(&fs, pa, pb, pc)) {
		for (c = d = e = x = 0; x != fs.max; x++) {
			if (ptr[x]) {
				d++;
				if (pc[x])
					c++;
			}
			if (pc[x])
				e++;
		}

		temp = (c << lmax) / d;
		if (temp >= score) {
			if (temp > score)
				keys = e;

			score = temp;

			if (e <= keys) {
				keys = e;
				mka = fs.ka;
				mkb = fs.kb - 1;
				any = 1;
			}
		}
	}

	if (any == 0)
		return (0);

	fs.ka = mka;
	fs.kb = mkb;

	return (mbin_xor_factor(&fs, pa, pb, pc));
}

void
mbin_xor_factor_dump(const char *name, uint32_t *ptr, uint8_t lmax)
{
	uint32_t max = 1U << lmax;
	uint32_t table[max];
	uint32_t fa[max];
	uint32_t fb[max];
	uint32_t fc[max];
	uint32_t x;
	uint32_t y;
	uint8_t n;

	memcpy(table, ptr, sizeof(table));

	printf("static const struct "
	    "mbin_factor_stage_32 %s[%d] = {\n",
	    name, lmax);

	for (n = 0; n != lmax; n++) {

		if (mbin_xor_factor_find(table, fa, fb, fc, lmax)) {
			y = 0;
			for (x = 0; x != max; x++) {
				if (fa[x] != 0)
					y++;
			}

			for (x = 0; x != max; x++) {
				if (fb[x] != 0)
					y--;
			}

			if (y > max) {
				/* "fa[]" is smallest */

				printf("\t[%d].factlist = ((const uint32_t []){", lmax - 1 - n);
				y = 0;
				for (x = 0; x != max; x++) {
					if (fa[x] != 0) {
						printf("0x%08x, ", x);
						y++;
					}
				}
				printf("}),\n");
				printf("\t[%d].factlen = %d,\n", lmax - 1 - n, y);
			} else {
				/* "fb[]" is smallest */

				printf("\t[%d].factlist = ((const uint32_t []){", lmax - 1 - n);
				y = 0;
				for (x = 0; x != max; x++) {
					if (fb[x] != 0) {
						printf("0x%08x, ", x);
						y++;
					}
				}
				printf("}),\n");
				printf("\t[%d].factlen = %d,\n", lmax - 1 - n, y);
			}

			for (x = 0; x != max; x++) {
				fc[x] ^= table[x];
			}

			printf("\t[%d].remlist = ((const uint32_t []){", lmax - 1 - n);
			y = 0;
			for (x = 0; x != max; x++) {
				if (fc[x] != 0) {
					printf("0x%08x, ", x);
					y++;
				}
			}
			printf("}),\n");
			printf("\t[%d].remlen = %d,\n", lmax - 1 - n, y);
		} else {


			printf("\t[%d].remlist = ((const uint32_t []){", lmax - 1 - n);
			y = 0;
			for (x = 0; x != max; x++) {
				if (table[x] != 0) {
					printf("0x%08x, ", x);
					y++;
				}
			}
			printf("}),\n");
			printf("\t[%d].remlen = %d,\n", lmax - 1 - n, y);
			break;
		}
		for (x = 0; x != max; x++)
			table[x] = fa[x] ^ fb[x];
	}
	printf("};\n");
}
