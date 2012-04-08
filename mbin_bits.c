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
#include <sys/endian.h>

#include "math_bin.h"

uint32_t
mbin_get_bits32(const uint32_t *ptr, uint32_t *poff, uint32_t bits)
{
	uint32_t rem;
	uint32_t tmp;
	uint32_t offset = *poff;

	rem = 32 - (offset & 31);

	if (rem >= bits) {
		tmp = (le32toh(ptr[(offset / 32)]) >> (32 - rem));
	} else {
		tmp = ((le32toh(ptr[(offset / 32)]) >> (32 - rem)) |
		    (le32toh(ptr[(offset / 32) + 1]) << rem));
	}
	if (bits != 32)
		tmp &= (1 << bits) - 1;

	offset += bits;
	*poff = offset;

	return (tmp);
}

void
mbin_put_bits32(uint32_t *ptr, uint32_t *poff, uint32_t bits,
    uint32_t value)
{
	uint32_t rem;
	uint32_t tmp;
	uint32_t offset = *poff;

	if (bits < 32)
		value &= (1 << bits) - 1;

	rem = 32 - (offset & 31);

	if (rem >= bits) {
		tmp = value << (32 - rem);
		ptr[(offset / 32)] |= htole32(tmp);
	} else {
		tmp = value << (32 - rem);
		ptr[(offset / 32)] |= htole32(tmp);
		tmp = value >> rem;
		ptr[(offset / 32) + 1] |= htole32(tmp);
	}

	offset += bits;
	*poff = offset;
}

uint32_t
mbin_get_rev_bits32(const uint32_t *ptr, uint32_t *poff, uint32_t bits)
{
	uint32_t rem;
	uint32_t tmp;
	uint32_t offset = *poff;

	offset -= bits;

	rem = 32 - (offset & 31);

	if (rem >= bits) {
		tmp = (le32toh(ptr[(offset / 32)]) >> (32 - rem));
	} else {
		tmp = ((le32toh(ptr[(offset / 32)]) >> (32 - rem)) |
		    (le32toh(ptr[(offset / 32) + 1]) << rem));
	}

	if (bits < 32)
		tmp &= (1 << bits) - 1;

	*poff = offset;

	return (tmp);
}

void
mbin_put_rev_bits32(uint32_t *ptr, uint32_t *poff, uint32_t bits,
    uint32_t value)
{
	uint32_t rem;
	uint32_t tmp;
	uint32_t offset = *poff;

	if (bits < 32)
		value &= (1 << bits) - 1;

	offset += bits;

	rem = 32 - (offset & 31);

	if (rem >= bits) {
		tmp = value << (32 - rem);
		ptr[(offset / 32)] |= htole32(tmp);
	} else {
		tmp = value << (32 - rem);
		ptr[(offset / 32)] |= htole32(tmp);
		tmp = value >> rem;
		ptr[(offset / 32) + 1] |= htole32(tmp);
	}

	*poff = offset;
}
