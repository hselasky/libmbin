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

#include "math_bin.h"

struct mbin_expr_and {
	TAILQ_ENTRY(mbin_expr_and) entry;
	uint32_t value;
	int8_t	type;
	int8_t	subtype;
	int8_t	shift;
	int8_t	reserved;
};

struct mbin_expr_xor {
	TAILQ_ENTRY(mbin_expr_xor) entry;
	TAILQ_HEAD(, mbin_expr_and) head;
};

struct mbin_expr {
	TAILQ_ENTRY(mbin_expr) entry;
	TAILQ_HEAD(, mbin_expr_xor) head;
};

uint32_t
mbin_expr_get_value_and(struct mbin_expr_and *pand)
{
	if (pand->type == MBIN_EXPR_TYPE_CONST)
		return (pand->value);
	return (0);
}

void
mbin_expr_set_value_and(struct mbin_expr_and *pand, uint32_t value)
{
	if (pand->type == MBIN_EXPR_TYPE_CONST)
		pand->value = value;
}

int8_t
mbin_expr_get_subtype_and(struct mbin_expr_and *pand)
{
	if (pand->type != MBIN_EXPR_TYPE_CONST)
		return (pand->subtype);
	return (0);
}

void
mbin_expr_set_subtype_and(struct mbin_expr_and *pand, int8_t subtype)
{
	if (pand->type != MBIN_EXPR_TYPE_CONST)
		pand->subtype = subtype;
}

int8_t
mbin_expr_get_shift_and(struct mbin_expr_and *pand)
{
	if (pand->type != MBIN_EXPR_TYPE_CONST)
		return (pand->shift);
	return (0);
}

void
mbin_expr_set_shift_and(struct mbin_expr_and *pand, int8_t shift)
{
	if (pand->type != MBIN_EXPR_TYPE_CONST)
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

struct mbin_expr_and *
mbin_expr_alloc_and(uint8_t type)
{
	struct mbin_expr_and *pand = malloc(sizeof(*pand));

	if (pand == NULL)
		return (NULL);

	memset(pand, 0, sizeof(*pand));

	pand->type = type;

	return (pand);
}

struct mbin_expr_xor *
mbin_expr_alloc_xor(void)
{
	struct mbin_expr_xor *pxor = malloc(sizeof(*pxor));

	if (pxor == NULL)
		return (NULL);

	memset(pxor, 0, sizeof(*pxor));

	TAILQ_INIT(&pxor->head);

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
