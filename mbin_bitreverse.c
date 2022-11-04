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

/* Functions for CPU optimised bitreverse */

uint64_t
mbin_bitrev64(uint64_t a)
{
	a = ((a & 0x5555555555555555ULL) << 1) | ((a & 0xAAAAAAAAAAAAAAAAULL) >> 1);
	a = ((a & 0x3333333333333333ULL) << 2) | ((a & 0xCCCCCCCCCCCCCCCCULL) >> 2);
	a = ((a & 0x0F0F0F0F0F0F0F0FULL) << 4) | ((a & 0xF0F0F0F0F0F0F0F0ULL) >> 4);
	a = ((a & 0x00FF00FF00FF00FFULL) << 8) | ((a & 0xFF00FF00FF00FF00ULL) >> 8);
	a = ((a & 0x0000FFFF0000FFFFULL) << 16) | ((a & 0xFFFF0000FFFF0000ULL) >> 16);
	a = ((a & 0x00000000FFFFFFFFULL) << 32) | ((a & 0xFFFFFFFF00000000ULL) >> 32);
	return (a);
}

uint32_t
mbin_bitrev32(uint32_t a)
{
	a = ((a & 0x55555555) << 1) | ((a & 0xAAAAAAAA) >> 1);
	a = ((a & 0x33333333) << 2) | ((a & 0xCCCCCCCC) >> 2);
	a = ((a & 0x0F0F0F0F) << 4) | ((a & 0xF0F0F0F0) >> 4);
	a = ((a & 0x00FF00FF) << 8) | ((a & 0xFF00FF00) >> 8);
	a = ((a & 0x0000FFFF) << 16) | ((a & 0xFFFF0000) >> 16);
	return (a);
}

uint16_t
mbin_bitrev16(uint16_t a)
{
	a = ((a & 0x5555) << 1) | ((a & 0xAAAA) >> 1);
	a = ((a & 0x3333) << 2) | ((a & 0xCCCC) >> 2);
	a = ((a & 0x0F0F) << 4) | ((a & 0xF0F0) >> 4);
	a = ((a & 0x00FF) << 8) | ((a & 0xFF00) >> 8);
	return (a);
}

uint8_t
mbin_bitrev8(uint8_t a)
{
	a = ((a & 0x55) << 1) | ((a & 0xAA) >> 1);
	a = ((a & 0x33) << 2) | ((a & 0xCC) >> 2);
	a = ((a & 0x0F) << 4) | ((a & 0xF0) >> 4);
	return (a);
}
