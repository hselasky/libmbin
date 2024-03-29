/*-
 * Copyright (c) 2008 Hans Petter Selasky
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

#include "math_bin.h"

uint64_t
mbin_msb64(uint64_t val)
{
	if (val & 0xFFFFFFFF00000000ULL)
		return (((uint64_t)mbin_msb32(val >> 32)) << 32);
	else
		return (mbin_msb32(val));
}

uint32_t
mbin_msb32(uint32_t val)
{
	uint32_t m;

	if (val & 0xFFFF0000) {
		if (val & 0xFF000000)
			m = (1 << 31);
		else
			m = (1 << 23);
	} else {
		if (val & 0xFF00)
			m = (1 << 15);
		else
			m = (1 << 7);
	}

	do {
		if (val & m)
			break;
	} while ((m /= 2));

	return (m);
}

uint16_t
mbin_msb16(uint16_t val)
{
	uint16_t m;

	if (val & 0xFF00)
		m = (1 << 15);
	else
		m = (1 << 7);

	do {
		if (val & m)
			break;
	} while ((m /= 2));

	return (m);
}

uint8_t
mbin_msb8(uint8_t val)
{
	uint8_t m;

	if (val & 0xF0)
		m = (1 << 7);
	else
		m = (1 << 3);

	do {
		if (val & m)
			break;
	} while ((m /= 2));

	return (m);
}
