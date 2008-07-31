/*-
 * Copyright (c) 2008 Hans Petter Selasky. All rights reserved.
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

#ifndef _MATH_BIN_H_
#define	_MATH_BIN_H_

void	mbin_print8(uint8_t x);
void	mbin_print16(uint16_t x);
void	mbin_print32(uint32_t x);
void	mbin_print64(uint64_t x);

void	mbin_expand_32x32(uint32_t *ptr, uint32_t val, uint32_t mask, uint32_t slice);
void	mbin_expand_16x32(uint16_t *ptr, uint32_t val, uint32_t mask, uint16_t slice);
void	mbin_expand_8x32(uint8_t *ptr, uint32_t val, uint32_t mask, uint8_t slice);

uint32_t mbin_inc32(uint32_t val, uint32_t mask);
uint16_t mbin_inc16(uint16_t val, uint16_t mask);
uint8_t	mbin_inc8(uint8_t val, uint8_t mask);

uint32_t mbin_dec32(uint32_t val, uint32_t mask);
uint16_t mbin_dec16(uint16_t val, uint16_t mask);
uint8_t	mbin_dec8(uint8_t val, uint8_t mask);

uint8_t	mbin_optimise_32x32(uint32_t *ptr, uint32_t mask, uint32_t remove_bits, uint32_t def_slice, uint32_t work_slice, uint32_t temp_slice);

void	mbin_transform_fwd_32x32(uint32_t *ptr, uint32_t mask, uint32_t set_bits, uint32_t f_slice, uint32_t t_slice);

uint8_t	mbin_sumbits32(uint32_t val);
uint8_t	mbin_sumbits16(uint16_t val);
uint8_t	mbin_sumbits8(uint8_t val);

uint32_t mbin_msb32(uint32_t val);
uint16_t mbin_msb16(uint16_t val);
uint8_t	mbin_msb8(uint8_t val);

uint32_t mbin_grey_inv32(uint32_t t);
uint32_t mbin_grey_fwd32(uint32_t t);

#endif					/* _MATH_BIN_H_ */
