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

void
mbin_sort_xform(void *ptr, size_t n, size_t es, int tog, int tt, mbin_cmp_t *fn)
{
	size_t x, y, z;
	char tmp[es];
	char *pa;
	char *pb;

	/* sort array in toggle order */
	for (x = 1; x != n; x *= 2) {
		for (y = 0; y != n; y += (2 * x), tog *= tt) {
			pa = (char *)ptr + y * es;
			pb = pa + x * es;
			for (z = 0; z != x; z++, pa += es, pb += es) {
				if ((tog * fn(pa, pb)) > 0) {
					/* swap */
					memcpy(tmp, pa, sizeof(tmp));
					memcpy(pa, pb, sizeof(tmp));
					memcpy(pb, tmp, sizeof(tmp));
				}
			}
		}
	}
}

void
mbin_sort(void *ptr, size_t n, size_t es, mbin_cmp_t *fn)
{
	if (n <= 1)
		return;

	while (1) {
		size_t x;

		for (x = 1; x != n; x++) {
			if (fn((char *)ptr + x * es,
			    (char *)ptr + x * es - es) < 0)
				break;
		}
		if (x == n)
			break;
		mbin_sort_xform(ptr, n, es, 1, -1, fn);
		mbin_sort_xform(ptr, n, es, 1, 1, fn);
	}
}
