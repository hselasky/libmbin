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
	const size_t size = sizeof(*ptr) + sizeof(ptr->data[0]) * num;

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
mbin_xform_puzzle_reverse(mbin_xform_puzzle_head_t *phead)
{
	struct mbin_xform_puzzle *ptr;
	size_t x;

	TAILQ_FOREACH(ptr, phead, entry) {
		ptr->start = -ptr->start - ptr->size + 1;

		for (x = 0; x != (ptr->size / 2); x++) {
			struct mbin_xform_var temp = ptr->data[x];

			ptr->data[x] = ptr->data[ptr->size - 1 - x];
			ptr->data[ptr->size - 1 - x] = temp;
		}
	}
}

int
mbin_xform_puzzle_simplify(mbin_xform_puzzle_head_t *phead)
{
	struct mbin_xform_puzzle **array;
	struct mbin_xform_puzzle *pother;
	struct mbin_xform_puzzle *ptr;
	int retval = 0;
	int any;
	size_t x;
	size_t y;

repeat:
	any = 0;
	TAILQ_FOREACH(ptr, phead, entry) {
		size_t half = ptr->size / 2;

		for (x = 0; x != half; x++) {
			/* single out mirrors */
			if (ptr->data[x].val != 0 &&
			    ptr->data[x].var == ptr->data[x + half].var &&
			    ptr->data[x].val == -ptr->data[x + half].val) {

				pother = mbin_xform_puzzle_new(phead, half);

				pother->start = ptr->start;
				pother->data[x] = ptr->data[x];

				ptr->data[x].var = ptr->data[x + half].var = 0;
				ptr->data[x].val = ptr->data[x + half].val = 0;
				any = 1;
			}
		}

		TAILQ_FOREACH(pother, phead, entry) {
			if (pother == ptr || pother->size < ptr->size)
				continue;
			for (x = 0; x != ptr->size; x++) {
				if (ptr->data[x].val == 0)
					continue;
				y = pother->size - ptr->size + x;

				/* try to move variable from ptr to pother */
				if ((ptr->start + ptr->size) == (pother->start + pother->size) &&
				    (pother->data[y].val == 0 ||
				    pother->data[y].var == ptr->data[x].var)) {
					pother->data[y].val += ptr->data[x].val;
					pother->data[y].var = ptr->data[x].var;
					ptr->data[x].val = 0;
					ptr->data[x].var = 0;
					if (ptr->size != pother->size) {
						any = 1;
						retval = 1;
					}
				}
			}
		}
	}

	TAILQ_FOREACH_SAFE(ptr, phead, entry, pother) {
retest:
		for (x = 0; x != ptr->size; x++) {
			if (ptr->data[x].val != 0)
				break;
		}
		if (x == ptr->size) {
			TAILQ_REMOVE(phead, ptr, entry);
			free(ptr);
		} else if (ptr->size >= 2 && x >= (ptr->size / 2)) {
			/* shrink */
			any = 1;
			retval = 1;
			ptr->size /= 2;
			ptr->start += ptr->size;
			for (x = 0; x != ptr->size; x++)
				ptr->data[x] = ptr->data[x + ptr->size];
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
		return (retval);

	x = 0;
	TAILQ_FOREACH(ptr, phead, entry)
	    array[x++] = ptr;

	mergesort(array, x, sizeof(array[0]), &mbin_xform_puzzle_compare);

	TAILQ_INIT(phead);
	for (y = 0; y != x; y++)
		TAILQ_INSERT_TAIL(phead, array[y], entry);

	free(array);
	return (retval);
}

void
mbin_xform_puzzle_print(mbin_xform_puzzle_head_t *phead)
{
	struct mbin_xform_puzzle *ptr;
	size_t total_cost = 0;
	size_t xform_cost;
	ssize_t min = 0;
	size_t x;
	size_t sb;
	size_t mask;

	printf("Transform set\n");

	TAILQ_FOREACH(ptr, phead, entry) {
		if (ptr->start < min)
			min = ptr->start;
	}

	TAILQ_FOREACH(ptr, phead, entry) {
		mask = 0;
		for (x = 0; x != ptr->size; x++) {
			if (ptr->data[x].val != 0)
				mask |= x;
		}

		sb = mbin_sumbits32(mask);

		xform_cost = ((size_t)sb << sb) / 2ULL;

		total_cost += xform_cost;

		printf("start=%-4lld size=%-4lld mask=%-4lld :: ",
		    (long long)ptr->start, (long long)ptr->size,
		    (long long)mask);

		for (x = min; x != ptr->start; x++)
			printf("        ");

		for (x = 0; x != ptr->size; x++) {
			if (ptr->data[x].val == 0) {
				printf("  *     ");
			} else {
				printf("%2lld*%-4lld ",
				    (long long)ptr->data[x].val,
				    (long long)ptr->data[x].var);
			}
		}

		printf("\n");
	}
	printf("Total cost: %lld\n", (long long)total_cost);
}
