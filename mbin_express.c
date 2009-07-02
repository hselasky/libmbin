/*-
 * Copyright (c) 2009 Hans Petter Selasky. All rights reserved.
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
#include <sys/queue.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>

#include "math_bin.h"

struct mbin_expr_and {
	TAILQ_ENTRY(mbin_expr_and) entry;
	int8_t	type;
	int8_t	subtype;
	int8_t	shift;
	int8_t	reserved;
};

struct mbin_expr_xor {
	TAILQ_ENTRY(mbin_expr_xor) entry;
	TAILQ_HEAD(, mbin_expr_and) head;
	uint32_t value;
};

struct mbin_expr {
	TAILQ_ENTRY(mbin_expr) entry;
	TAILQ_HEAD(, mbin_expr_xor) head;
};

uint32_t
mbin_expr_get_value_xor(struct mbin_expr_xor *pxor)
{
	return (pxor->value);
}

void
mbin_expr_set_value_xor(struct mbin_expr_xor *pxor, uint32_t value)
{
	pxor->value = value;
}

int8_t
mbin_expr_get_type_and(struct mbin_expr_and *pand)
{
	return (pand->type);
}

void
mbin_expr_set_type_and(struct mbin_expr_and *pand, int8_t type)
{
	pand->type = type;
}

int8_t
mbin_expr_get_subtype_and(struct mbin_expr_and *pand)
{
	return (pand->subtype);
}

void
mbin_expr_set_subtype_and(struct mbin_expr_and *pand, int8_t subtype)
{
	pand->subtype = subtype;
}

int8_t
mbin_expr_get_shift_and(struct mbin_expr_and *pand)
{
	return (pand->shift);
}

void
mbin_expr_set_shift_and(struct mbin_expr_and *pand, int8_t shift)
{
	pand->shift = shift;
}

void
mbin_expr_enqueue_and(struct mbin_expr_xor *pxor, struct mbin_expr_and *pand)
{
	if (pand->entry.tqe_prev == NULL)
		TAILQ_INSERT_TAIL(&pxor->head, pand, entry);
}

void
mbin_expr_dequeue_and(struct mbin_expr_xor *pxor, struct mbin_expr_and *pand)
{
	if (pand->entry.tqe_prev != NULL) {
		TAILQ_REMOVE(&pxor->head, pand, entry);
		pand->entry.tqe_prev = NULL;
	}
}

struct mbin_expr_and *
mbin_expr_foreach_and(struct mbin_expr_xor *pxor, struct mbin_expr_and *pand)
{
	if (pand == NULL)
		pand = TAILQ_FIRST(&pxor->head);
	else
		pand = TAILQ_NEXT(pand, entry);

	return (pand);
}

void
mbin_expr_enqueue_xor(struct mbin_expr *pexpr, struct mbin_expr_xor *pxor)
{
	if (pxor->entry.tqe_prev == NULL)
		TAILQ_INSERT_TAIL(&pexpr->head, pxor, entry);
}

void
mbin_expr_dequeue_xor(struct mbin_expr *pexpr, struct mbin_expr_xor *pxor)
{
	if (pxor->entry.tqe_prev != NULL) {
		TAILQ_REMOVE(&pexpr->head, pxor, entry);
		pxor->entry.tqe_prev = NULL;
	}
}

struct mbin_expr_xor *
mbin_expr_foreach_xor(struct mbin_expr *pexpr, struct mbin_expr_xor *pxor)
{
	if (pxor == NULL)
		pxor = TAILQ_FIRST(&pexpr->head);
	else
		pxor = TAILQ_NEXT(pxor, entry);

	return (pxor);
}

struct mbin_expr *
mbin_expr_substitute_and_full(struct mbin_expr *pexpr,
    struct mbin_expr *psubst, int8_t type, int8_t subtype)
{
	struct mbin_expr *pnexpr;
	struct mbin_expr_and *paa;
	struct mbin_expr_and *pab;
	struct mbin_expr_and *pac;

	struct mbin_expr_xor *pxa;
	struct mbin_expr_xor *pxb;
	struct mbin_expr_xor *pxc;

	uint8_t alloc_pexpr;
	uint8_t found;

	alloc_pexpr = 0;

repeat:

	pnexpr = mbin_expr_alloc();
	if (pnexpr == NULL)
		goto failure;

	found = 0;
	pxa = NULL;
	while ((pxa = mbin_expr_foreach_xor(pexpr, pxa))) {

		int8_t shift;

		paa = NULL;
		while ((paa = mbin_expr_foreach_and(pxa, paa))) {
			if ((paa->type == type) && (paa->subtype == subtype))
				goto do_expand;
		}

		/* duplicate XOR statement */
		pxc = mbin_expr_alloc_xor(pnexpr);
		if (pxc == NULL)
			goto failure;

		pxc->value = pxa->value;

		pab = NULL;
		while ((pab = mbin_expr_foreach_and(pxa, pab))) {
			pac = mbin_expr_dup_and(pab, pxc);
			if (pac == NULL)
				goto failure;
		}
		continue;

do_expand:
		/* get shift level */
		shift = paa->shift;

		/* expand XOR statement */
		found = 1;
		pxb = NULL;
		while ((pxb = mbin_expr_foreach_xor(psubst, pxb))) {

			int32_t temp;

			pxc = mbin_expr_alloc_xor(pnexpr);
			if (pxc == NULL)
				goto failure;

			if ((shift < -31) || (shift > 31))
				temp = 0;
			else if (shift < 0)
				temp = (pxb->value >> -shift);
			else if (shift > 0)
				temp = (pxb->value << shift);
			else
				temp = pxb->value;

			pxc->value = pxa->value & temp;
			if (pxc->value == 0)
				continue;

			pab = NULL;
			while ((pab = mbin_expr_foreach_and(pxa, pab))) {
				if (pab == paa)
					continue;
				pac = mbin_expr_dup_and(pab, pxc);
				if (pac == NULL)
					goto failure;
			}

			pab = NULL;
			while ((pab = mbin_expr_foreach_and(pxb, pab))) {

				pac = mbin_expr_dup_and(pab, pxc);
				if (pac == NULL)
					goto failure;

				temp = pac->shift + shift;
				if (temp > MBIN_BITS_MAX)
					temp = MBIN_BITS_MAX;
				else if (temp < MBIN_BITS_MIN)
					temp = MBIN_BITS_MIN;

				pac->shift = temp;
			}

			/* TODO: check for duplicate ANDs */
		}
	}

	if (alloc_pexpr)
		mbin_expr_free(pexpr);

	if (found) {
		alloc_pexpr = 1;
		pexpr = pnexpr;
		goto repeat;
	}
	return (pnexpr);

failure:
	if (pnexpr)
		mbin_expr_free(pnexpr);
	if (alloc_pexpr)
		mbin_expr_free(pexpr);
	return (NULL);
}

int8_t
mbin_expr_max_shift_xor(struct mbin_expr_xor *pxa)
{
	struct mbin_expr_and *paa;
	int8_t max;

	for (max = 0; max != 32; max++) {
		if (pxa->value & (1 << max))
			break;
	}

	paa = NULL;
	while ((paa = mbin_expr_foreach_and(pxa, paa))) {
		if (paa->shift > max)
			max = paa->shift;
	}
	return (max);
}

void
mbin_expr_substitute_and_simple(struct mbin_expr *pexpr,
    int8_t from_type, int32_t delta)
{
	struct mbin_expr_and *paa;
	struct mbin_expr_xor *pxa;

	pxa = NULL;
	while ((pxa = mbin_expr_foreach_xor(pexpr, pxa))) {

		paa = NULL;
		while ((paa = mbin_expr_foreach_and(pxa, paa))) {
			if (paa->type == from_type) {
				paa->subtype += delta;
			}
		}
	}
}

void
mbin_expr_print_and(struct mbin_expr_and *paa)
{
	char cvar;

	cvar = paa->type - MBIN_EXPR_TYPE_VAR_A + 'a';
	if ((cvar < 'a') || (cvar > 'z')) {
		cvar = paa->type;
		if (!isalpha(cvar))
			cvar = '?';
	}
	printf("(%c%d", cvar, paa->subtype);
	if (paa->shift > 0)
		printf("<<%u", paa->shift);
	else if (paa->shift < 0)
		printf(">>%u", -paa->shift);
	printf(")");
}

void
mbin_expr_print_xor(struct mbin_expr_xor *pxa)
{
	struct mbin_expr_and *paa;
	uint8_t had_expr;

	printf("(");

	had_expr = 0;
	paa = NULL;
	while ((paa = mbin_expr_foreach_and(pxa, paa))) {
		if (had_expr)
			printf("&");
		else
			printf("0x%x&", pxa->value);

		mbin_expr_print_and(paa);
		had_expr = 1;
	}

	if (had_expr == 0)
		printf("0x%x", pxa->value);

	printf(") ^ \n");
}

void
mbin_expr_print(struct mbin_expr *pexpr)
{
	struct mbin_expr_xor *pxa;

	pxa = NULL;
	while ((pxa = mbin_expr_foreach_xor(pexpr, pxa))) {
		mbin_expr_print_xor(pxa);
	}
}

struct mbin_expr_and *
mbin_expr_dup_and(struct mbin_expr_and *pand_old, struct mbin_expr_xor *pxor)
{
	struct mbin_expr_and *pand = malloc(sizeof(*pand));

	if (pand == NULL)
		return (NULL);

	*pand = *pand_old;

	memset(&pand->entry, 0, sizeof(pand->entry));

	if (pxor != NULL)
		mbin_expr_enqueue_and(pxor, pand);

	return (pand);
}

struct mbin_expr_and *
mbin_expr_alloc_and(struct mbin_expr_xor *pxor)
{
	struct mbin_expr_and *pand = malloc(sizeof(*pand));

	if (pand == NULL)
		return (NULL);

	memset(pand, 0, sizeof(*pand));

	if (pxor != NULL)
		mbin_expr_enqueue_and(pxor, pand);

	return (pand);
}

struct mbin_expr_xor *
mbin_expr_alloc_xor(struct mbin_expr *pexpr)
{
	struct mbin_expr_xor *pxor = malloc(sizeof(*pxor));

	if (pxor == NULL)
		return (NULL);

	memset(pxor, 0, sizeof(*pxor));

	TAILQ_INIT(&pxor->head);

	pxor->value = -1UL;

	if (pexpr != NULL)
		mbin_expr_enqueue_xor(pexpr, pxor);

	return (pxor);
}

struct mbin_expr *
mbin_expr_alloc(void)
{
	struct mbin_expr *pexpr = malloc(sizeof(*pexpr));

	if (pexpr == NULL)
		return (NULL);

	memset(pexpr, 0, sizeof(*pexpr));

	TAILQ_INIT(&pexpr->head);

	return (pexpr);
}

void
mbin_expr_free_and(struct mbin_expr_xor *pxor, struct mbin_expr_and *pand)
{
	mbin_expr_dequeue_and(pxor, pand);
	free(pand);
}

void
mbin_expr_free_xor(struct mbin_expr *pexpr, struct mbin_expr_xor *pxor)
{
	struct mbin_expr_and *pand;

	mbin_expr_dequeue_xor(pexpr, pxor);

	while ((pand = TAILQ_FIRST(&pxor->head)))
		mbin_expr_free_and(pxor, pand);

	free(pxor);
}

void
mbin_expr_free(struct mbin_expr *pexpr)
{
	struct mbin_expr_xor *pxor;

	while ((pxor = TAILQ_FIRST(&pexpr->head)))
		mbin_expr_free_xor(pexpr, pxor);

	free(pexpr);
}

void
mbin_expr_optimise(struct mbin_expr *pexpr, uint32_t mask)
{
	struct mbin_expr_xor *pxa;
	struct mbin_expr_xor *pxb;

	struct mbin_expr_and *paa;
	struct mbin_expr_and *pab;

	uint32_t alen;
	uint32_t blen;

	uint8_t found;

	pxa = NULL;
	while ((pxa = mbin_expr_foreach_xor(pexpr, pxa))) {

		/* remove all zero values */
		while ((pxa->value & mask) == 0) {
	delete:
			/* get next element */
			pxb = mbin_expr_foreach_xor(pexpr, pxa);
			mbin_expr_free_xor(pexpr, pxa);
			pxa = pxb;
			if (pxa == NULL)
				break;
		}
		if (pxa == NULL)
			break;

		alen = 0;

		paa = NULL;
		while ((paa = mbin_expr_foreach_and(pxa, paa))) {
			if (paa->shift > 0) {
				if (paa->shift > 31)
					goto delete;
				if (!(mask & (1 << paa->shift)))
					goto delete;
			} else if (paa->shift < 0) {
				if (paa->shift < -31)
					goto delete;
			}
			alen++;
		}

repeat:

		pxb = pxa;
		while ((pxb = mbin_expr_foreach_xor(pexpr, pxb))) {

			blen = 0;

			pab = NULL;
			while ((pab = mbin_expr_foreach_and(pxb, pab))) {
				blen++;
			}

			if (alen != blen)
				continue;

			found = 0;

			paa = NULL;
			while ((paa = mbin_expr_foreach_and(pxa, paa))) {

				found = 0;

				pab = NULL;
				while ((pab = mbin_expr_foreach_and(pxb, pab))) {

					if ((paa->type == pab->type) &&
					    (paa->subtype == pab->subtype) &&
					    (paa->shift == pab->shift)) {
						found = 1;
						break;
					}
				}

				if (!found)
					break;
			}

			if (found || (alen == 0)) {
				pxa->value ^= pxb->value;
				mbin_expr_free_xor(pexpr, pxb);
				goto repeat;
			}
		}
	}
}
