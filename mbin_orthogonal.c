/*-
 * Copyright (c) 2012 Hans Petter Selasky
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
#include <string.h>

#include <sys/queue.h>
#include <sys/ctype.h>

#include "math_bin.h"

struct mbin_okey {
	STAILQ_ENTRY(mbin_okey) entry;
	uint32_t key;
};

static uint8_t
mbin_okey_compress_8(uint32_t val, uint32_t kmask)
{
	uint8_t retval = 0;

	while (kmask) {
		if (kmask & 1) {
			retval *= 2;
			retval |= (val & 1);
		}
		kmask /= 2;
		val /= 2;
	}
	return (retval);
}

void
mbin_find_orthogonal_key_32(const uint32_t *ptr, uint32_t max,
    uint32_t kmask, uint32_t **pres)
{
	uint32_t n;
	uint32_t mask;
	uint32_t x;
	uint32_t y;
	uint8_t *ptmp;
	uint8_t *pcmp;
	struct mbin_okey *pkey;
	STAILQ_HEAD(, mbin_okey) head = STAILQ_HEAD_INITIALIZER(head);

	*pres = NULL;

	mask = -1U;

	for (x = 0; x != max; x++)
		mask &= ~(ptr[x] ^ ptr[0]);

	/* mask should now contain all non-changing bits */

	mask = ~mask & ~kmask;

	n = mbin_msb32(mask) * 2;
	if (n == 0)
		return;

	ptmp = malloc(n);
	if (ptmp == NULL)
		return;

	memset(ptmp, 0, n);

	pcmp = malloc(max);
	if (pcmp == NULL) {
		free(ptmp);
		return;
	}
	for (x = 0; x != max; x++) {
		pcmp[x] = mbin_okey_compress_8(ptr[x], kmask) + 1;
	}

	for (y = 0; y <= mask; y++) {
		while (y & ~mask)
			y += (y & ~mask);
		if (y > mask)
			break;
		STAILQ_FOREACH(pkey, &head, entry) {
			if ((pkey->key & y) == pkey->key)
				break;
		}
		if (pkey != NULL)
			continue;

		for (x = 0; x != max; x++) {

			uint8_t val;

			val = pcmp[x];

			if (ptmp[y & ptr[x]] != 0 &&
			    ptmp[y & ptr[x]] != val) {
				break;
			} else {
				ptmp[y & ptr[x]] = val;
			}
		}

		if (x == max) {
			pkey = alloca(sizeof(*pkey));
			pkey->key = y;
			STAILQ_INSERT_TAIL(&head, pkey, entry);
		}
		/* cleanup */
		while (x--) {
			ptmp[y & ptr[x]] = 0;
		}
	}

	free(pcmp);
	free(ptmp);

	n = 1;
	STAILQ_FOREACH(pkey, &head, entry)
	    n++;

	*pres = malloc(n * sizeof(**pres));

	if (*pres != NULL) {
		n = 0;

		STAILQ_FOREACH(pkey, &head, entry)
		    (*pres)[n++] = pkey->key;

		(*pres)[n++] = -1U;
	}
}
