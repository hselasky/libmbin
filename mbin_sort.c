/*-
 * Copyright (c) 2016 Hans Petter Selasky. All rights reserved.
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

#include "math_bin.h"

#include <string.h>

/*
 * A variant of bitonic sorting
 *
 * Sorting complexity: log(N) * log(N) * N
 * Additional memory usage: none
 */

int
mbin_bsort_xform(void *ptr, size_t n, size_t es, mbin_cmp_t *fn)
{
	int retval = 0;
	size_t x, y, z;

	/* sort array */
	for (x = n / 2; x != 0; x /= 2) {
		for (y = 0; y != n; y += (2 * x)) {
			int tog = (mbin_sort_index(y) & 1) ? -1 : 1;

			for (z = 0; z != x; z++) {
				char *pa = (char *)ptr + (mbin_sort_index(y + z) * es);
				char *pb = (char *)ptr + (mbin_sort_index(y + x + z) * es);

				if ((tog * fn(pa, pb)) > 0) {
					mbin_sort_swap(pa, pb, es);
					retval = 1;
				}
			}
		}
	}
	return (retval);
}

int
mbin_qsort_xform(void *ptr, size_t n, size_t es, mbin_cmp_t *fn)
{
	size_t x, y, z, t, u;
	int retval = 0;

	/* sort array */
	for (x = n / 4; x != 0; x /= 4) {
		for (y = 0; y != n; y += (4 * x)) {
			int tog = (mbin_sort_index(y) & 1) ? -1 : 1;

			for (z = 0; z != x; z++) {
				char *p[4] = {
					(char *)ptr + (mbin_sort_index(y + z) * es),
					(char *)ptr + (mbin_sort_index(y + z + x) * es),
					(char *)ptr + (mbin_sort_index(y + z + 3 * x) * es),
					(char *)ptr + (mbin_sort_index(y + z + 2 * x) * es),
				};

				for (t = 1; t != 4; t++) {
					for (u = t; u != 0 && (tog * fn(p[u - 1], p[u])) > 0; u--) {
						mbin_sort_swap(p[u - 1], p[u], es);
						retval = 1;
					}
				}
			}
		}
	}
	return (retval);
}

int
mbin_osort_xform(void *ptr, size_t n, size_t es, mbin_cmp_t *fn)
{
	size_t x, y, z, t, u;
	int retval = 0;

	/* sort array */
	for (x = n / 8; x != 0; x /= 8) {
		for (y = 0; y != n; y += (8 * x)) {
			int tog = (mbin_sort_index(y) & 1) ? -1 : 1;

			for (z = 0; z != x; z++) {
				char *p[8] = {
					(char *)ptr + (mbin_sort_index(y + z) * es),
					(char *)ptr + (mbin_sort_index(y + z + x) * es),
					(char *)ptr + (mbin_sort_index(y + z + 3 * x) * es),
					(char *)ptr + (mbin_sort_index(y + z + 2 * x) * es),
					(char *)ptr + (mbin_sort_index(y + z + 6 * x) * es),
					(char *)ptr + (mbin_sort_index(y + z + 7 * x) * es),
					(char *)ptr + (mbin_sort_index(y + z + 5 * x) * es),
					(char *)ptr + (mbin_sort_index(y + z + 4 * x) * es),
				};

				for (t = 1; t != 8; t++) {
					for (u = t; u != 0 && (tog * fn(p[u - 1], p[u])) > 0; u--) {
						mbin_sort_swap(p[u - 1], p[u], es);
						retval = 1;
					}
				}
			}
		}
	}
	return (retval);
}

static int
mbin_xsort_xform(void *ptr, size_t n, size_t lim, size_t es, mbin_cmp_t *fn)
{
#define	MBIN_XSORT_TABLE_MAX (1 << 4)
	size_t x, y, z;
	unsigned t, u, v;
	size_t p[MBIN_XSORT_TABLE_MAX];
	int retval = 0;

	x = n;
	while (1) {
		/* optimise */
		if (x >= MBIN_XSORT_TABLE_MAX)
			v = MBIN_XSORT_TABLE_MAX;
		else if (x >= 2)
			v = x;
		else
			break;

		/* divide down */
		x /= v;

		/* generate ramp table */
		for (t = 0; t != v; t++)
			p[t] = mbin_sort_index(x * (t ^ (t / 2)));

		/* bitonic sort */
		for (y = 0; y != n; y += (v * x)) {
			for (z = 0; z != x; z++) {
				size_t zi = y ^ mbin_sort_index(z);

				/* insertion sort */
				for (t = 1; t != v; t++) {
					/* check for arrays which are not power of two */
					if ((zi ^ p[t]) >= lim)
						break;
					for (u = t; u != 0; u--) {
						char *pa = (char *)ptr + ((zi ^ p[u - 1]) * es);
						char *pb = (char *)ptr + ((zi ^ p[u]) * es);

						if (fn(pa, pb) > 0) {
							mbin_sort_swap(pa, pb, es);
							retval = 1;
						} else {
							break;
						}
					}
				}
			}
		}
	}
	return (retval);
}

size_t
mbin_sort_index(size_t t)
{
	t ^= t >> 1;
	t ^= t >> 2;
	t ^= t >> 4;
	t ^= t >> 8;
	t ^= t >> 16;
	if (sizeof(t) >= 8)
		t ^= t >> 32;
	return (t);
}

void
mbin_sort_swap(char *pa, char *pb, size_t es)
{
	char tmp[es] __aligned(8);

	if (es == 8) {
		/* swap */
		*(uint64_t *)tmp = *(uint64_t *)pa;
		*(uint64_t *)pa = *(uint64_t *)pb;
		*(uint64_t *)pb = *(uint64_t *)tmp;
	} else if (es == 4) {
		/* swap */
		*(uint32_t *)tmp = *(uint32_t *)pa;
		*(uint32_t *)pa = *(uint32_t *)pb;
		*(uint32_t *)pb = *(uint32_t *)tmp;
	} else if (es == 2) {
		/* swap */
		*(uint16_t *)tmp = *(uint16_t *)pa;
		*(uint16_t *)pa = *(uint16_t *)pb;
		*(uint16_t *)pb = *(uint16_t *)tmp;
	} else if (es == 1) {
		/* swap */
		*(uint8_t *)tmp = *(uint8_t *)pa;
		*(uint8_t *)pa = *(uint8_t *)pb;
		*(uint8_t *)pb = *(uint8_t *)tmp;
	} else {
		/* swap */
		memcpy(tmp, pa, es);
		memcpy(pa, pb, es);
		memcpy(pb, tmp, es);
	}
}

void
mbin_sort(void *ptr, size_t n, size_t es, mbin_cmp_t *fn)
{
	size_t max;

	if (n <= 1)
		return;

	for (max = 1; max < n; max <<= 1)
		;

	while (mbin_xsort_xform(ptr, max, n, es, fn))
		;
}
