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

/* a variant of bitonic sorting */

int
mbin_sort_xform(void *ptr, size_t n, size_t es, mbin_cmp_t *fn)
{
	int retval = 0;
	size_t x, y, z;
	char tmp[es];
	char *pa;
	char *pb;

	/* sort array in toggle order */
	for (x = 1; x != n; x *= 2) {
		for (y = 0; y != n; y += (2 * x)) {
			int tog = (mbin_sumbits64(y & -x) & 1) ? -1 : 1;

			pa = (char *)ptr + y * es;
			pb = pa + x * es;
			for (z = 0; z != x; z++, pa += es, pb += es) {
				if ((tog * fn(pa, pb)) > 0) {
					/* swap */
					memcpy(tmp, pa, es);
					memcpy(pa, pb, es);
					memcpy(pb, tmp, es);
					retval = 1;
				}
			}
		}
	}
	return (retval);
}

size_t
mbin_sort_index(size_t t, size_t n)
{
	size_t m;

	for (m = 1; m != n; m *= 2ULL) {
		if (t & m)
			t ^= m - 1ULL;
	}
	return (t);
}

void
mbin_sort_reorder(void *ptr, size_t n, size_t es)
{
#if 0
	size_t x, y;
	char copy[n][es];
	char *pa;

	memcpy(copy, ptr, sizeof(copy));

	/* reorder array */
	for (x = 0; x != n; x++) {
		y = mbin_sort_index(x, n);
		pa = (char *)ptr + y * es;
		memcpy(pa, copy[x], es);
	}
#else
	size_t x, y, z;
	char tmp[2][es];
	char bitmap[(n + 7) / 8];
	char *pa;
	char m;

	memset(bitmap, 0, sizeof(bitmap));

	/* reorder array */
	for (x = 0; x != n; x++) {
		if (bitmap[x / 8] & (1U << (x % 8)))
			continue;
		z = x;
		pa = (char *)ptr + z * es;
		memcpy(tmp[0], pa, es);
		for (y = 0;; y++) {
			z = mbin_sort_index(z, n);
			m = (1U << (z % 8));
			if (bitmap[z / 8] & m)
				break;
			bitmap[z / 8] |= m;
			pa = (char *)ptr + z * es;
			memcpy(tmp[~y & 1], pa, es);
			memcpy(pa, tmp[y & 1], es);
		}
	}
#endif
}

void
mbin_sort(void *ptr, size_t n, size_t es, mbin_cmp_t *fn)
{
	if (n <= 1)
		return;

	while (mbin_sort_xform(ptr, n, es, fn))
		;

	mbin_sort_reorder(ptr, n, es);
}
