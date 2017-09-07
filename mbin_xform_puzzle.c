/*-
 * Copyright (c) 2017 Hans Petter Selasky. All rights reserved.
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "math_bin.h"

struct mbin_xform_puzzle *
mbin_xform_puzzle_new(mbin_xform_puzzle_head_t *phead, size_t num)
{
	struct mbin_xform_puzzle *ptr;
	const size_t size = sizeof(*ptr) + sizeof(ptr->vars[0]) * num;

	ptr = malloc(size);
	if (ptr != NULL) {
		memset(ptr, 0, size);
		ptr->size = num;
		TAILQ_INSERT_TAIL(phead, ptr, entry);
	}
	return (ptr);
}

static int
mbin_xform_puzzle_compare(const void *_pa, const void *_pb)
{
	struct mbin_xform_puzzle *const *pa = _pa;
	struct mbin_xform_puzzle *const *pb = _pb;

	if (pa[0]->start > pb[0]->start)
		return (1);
	else if (pa[0]->start < pb[0]->start)
		return (-1);
	else
		return (0);
}

void
mbin_xform_puzzle_free(mbin_xform_puzzle_head_t *phead)
{
	struct mbin_xform_puzzle *ptr;

	while ((ptr = TAILQ_FIRST(phead)) != NULL) {
		TAILQ_REMOVE(phead, ptr, entry);
		free(ptr);
	}
}

void
mbin_xform_puzzle_solve(mbin_xform_puzzle_head_t *phead)
{
	struct mbin_xform_puzzle **array;
	struct mbin_xform_puzzle *pother;
	struct mbin_xform_puzzle *ptr;
	int any;
	int x;
	int y;

repeat:
	any = 0;
	TAILQ_FOREACH(ptr, phead, entry) {
		TAILQ_FOREACH(pother, phead, entry) {
			if (pother == ptr || pother->size < ptr->size)
				continue;
			for (x = 0; x != ptr->size; x++) {
				if (ptr->vars[x] == 0)
					continue;

				/* try to move variable from ptr to pother */
				if ((ptr->start + ptr->size) == (pother->start + pother->size) &&
				    pother->vars[pother->size - ptr->size + x] == 0) {
					pother->vars[pother->size - ptr->size + x] = ptr->vars[x];
					ptr->vars[x] = 0;
					any = 1;
				}
			}
		}
	}

	TAILQ_FOREACH_SAFE(ptr, phead, entry, pother) {
retest:
		for (x = 0; x != ptr->size; x++) {
			if (ptr->vars[x] != 0)
				break;
		}
		if (x == ptr->size) {
			TAILQ_REMOVE(phead, ptr, entry);
			free(ptr);
		} else if (ptr->size >= 2 && x >= (ptr->size / 2)) {
			/* shrink */
			any = 1;
			ptr->size /= 2;
			ptr->start += ptr->size;
			for (x = 0; x != ptr->size; x++)
				ptr->vars[x] = ptr->vars[x + ptr->size];
			goto retest;
		}
	}
	if (any)
		goto repeat;

	x = 0;
	TAILQ_FOREACH(ptr, phead, entry)
	    x++;

	array = malloc(sizeof(array[0]) * x);
	if (array == NULL)
		return;

	x = 0;
	TAILQ_FOREACH(ptr, phead, entry)
	    array[x++] = ptr;

	mergesort(array, x, sizeof(array[0]), &mbin_xform_puzzle_compare);

	TAILQ_INIT(phead);
	for (y = 0; y != x; y++)
		TAILQ_INSERT_TAIL(phead, array[y], entry);

	free(array);
}

void
mbin_xform_puzzle_print(mbin_xform_puzzle_head_t *phead)
{
	struct mbin_xform_puzzle *ptr;
	size_t total_cost = 0;
	size_t xform_cost;
	int x;
	int sb;
	int mask;

	printf("Transform set\n");

	TAILQ_FOREACH(ptr, phead, entry) {

		printf("start=%4d size=%4d :: ", ptr->start, ptr->size);

		for (x = 0; x != ptr->start; x++)
			printf("     ");

		for (x = 0; x != ptr->size; x++)
			printf("%4d ", ptr->vars[x]);

		mask = 0;
		for (x = 0; x != ptr->size; x++) {
			if (ptr->vars[x] == 0)
				continue;
			mask |= x;
		}

		sb = mbin_sumbits32(mask);

		xform_cost = ((size_t)sb << sb) / 2ULL;

		printf(" cost=%lld\n", (long long)xform_cost);

		total_cost += xform_cost;
	}
	printf("Total cost: %lld\n", (long long)total_cost);
}
