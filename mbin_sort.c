/*-
 * Copyright (c) 2016-2022 Hans Petter Selasky. All rights reserved.
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

inline int
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

static __always_inline bool
mbin_xsort_complete(void *ptr, const size_t lim, const size_t es, mbin_cmp_t *fn)
{
	for (size_t x = 1; x != lim; x++) {
		if (fn(ptr, (char *)ptr + es) > 0)
			return (false);
		ptr = (char *)ptr + es;
	}
	return (true);
}

static __always_inline void
mbin_xsort_xform(void *ptr, const size_t n, const size_t lim, const size_t es, mbin_cmp_t *fn)
{
#define	MBIN_XSORT_TABLE_MAX (1 << 4)
	size_t x, y, z;
	unsigned t, u, v;
	size_t p[MBIN_XSORT_TABLE_MAX];
	char *q[MBIN_XSORT_TABLE_MAX];
	uintptr_t end;

	end = (uintptr_t)ptr + lim * es;
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
		for (t = 0; t != v; t += 2) {
			p[t] = t * x;
			p[t + 1] = (t + 2) * x - 1;
		}

		/* bitonic sort */
		for (y = 0; y != n; y += (v * x)) {
			for (z = 0; z != x; z++) {
				const size_t w = y + z;

				q[0] = (char *)ptr + (w ^ p[0]) * es;

				/* insertion sort */
				for (t = 1; t != v; t++) {
					q[t] = (char *)ptr + (w ^ p[t]) * es;

					/* check for arrays which are not power of two */
					if ((uintptr_t)q[t] >= end)
						break;

					for (u = t; u--; ) {
						if (fn(q[u], q[u + 1]) > 0) {
							mbin_sort_swap(q[u], q[u + 1], es);
						} else {
							break;
						}
					}
				}
			}
		}
	}
}

__always_inline size_t
mbin_sort_index(size_t t)
{
	t ^= t >> 1;
	t ^= t >> 2;
	t ^= t >> 4;
	t ^= t >> 8;
	t ^= t >> 16;
	if (sizeof(t) >= 8)
		t ^= t >> (sizeof(t) >= 8 ? 32 : 0);
	return (t);
}

__always_inline void
mbin_sort_swap(char *pa, char *pb, const size_t es)
{
	if (__builtin_constant_p(es) && es == 8) {
		uint64_t tmp;

		/* swap */
		tmp = *(uint64_t *)pa;
		*(uint64_t *)pa = *(uint64_t *)pb;
		*(uint64_t *)pb = tmp;
	} else if (__builtin_constant_p(es) && es == 4) {
		uint32_t tmp;

		/* swap */
		tmp = *(uint32_t *)pa;
		*(uint32_t *)pa = *(uint32_t *)pb;
		*(uint32_t *)pb = tmp;
	} else if (__builtin_constant_p(es) && es == 2) {
		uint16_t tmp;

		/* swap */
		tmp = *(uint16_t *)pa;
		*(uint16_t *)pa = *(uint16_t *)pb;
		*(uint16_t *)pb = tmp;
	} else if (__builtin_constant_p(es) && es == 1) {
		uint8_t tmp;

		/* swap */
		tmp = *(uint8_t *)pa;
		*(uint8_t *)pa = *(uint8_t *)pb;
		*(uint8_t *)pb = tmp;
	} else {
	  	char tmp[es] __aligned(8);

		/* swap */
		memcpy(tmp, pa, es);
		memcpy(pa, pb, es);
		memcpy(pb, tmp, es);
	}
}

void
mbin_sort(void *ptr, const size_t n, const size_t es, mbin_cmp_t *fn)
{
	size_t max;

	if (n <= 1)
		return;

	for (max = 1; max < n; max <<= 1)
		;

	if (es == 8) {
		while (!mbin_xsort_complete(ptr, n, 8, fn))
			mbin_xsort_xform(ptr, max, n, 8, fn);
	} else if (es == 4) {
		while (!mbin_xsort_complete(ptr, n, 4, fn))
			mbin_xsort_xform(ptr, max, n, 4, fn);
	} else if (es == 2) {
		while (!mbin_xsort_complete(ptr, n, 2, fn))
			mbin_xsort_xform(ptr, max, n, 2, fn);
	} else if (es == 1) {
		while (!mbin_xsort_complete(ptr, n, 1, fn))
			mbin_xsort_xform(ptr, max, n, 1, fn);
	} else {
		while (!mbin_xsort_complete(ptr, n, es, fn))
			mbin_xsort_xform(ptr, max, n, es, fn);
	}
}
