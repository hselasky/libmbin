#
# Makefile for my Binary Mathematics library
#

LIB=		mbin1
SHLIB_MAJOR=	1
SHLIB_MINOR=	0
CFLAGS+=	-Wall -O3

SRCS=
SRCS+=	mbin_base23.c
SRCS+=	mbin_base3.c
SRCS+=	mbin_base4.c
SRCS+=	mbin_base5.c
SRCS+=	mbin_base6.c
SRCS+=	mbin_base7.c
SRCS+=	mbin_baseG.c
SRCS+=	mbin_baseH.c
SRCS+=	mbin_baseL.c
SRCS+=	mbin_baseM.c
SRCS+=	mbin_baseN.c
SRCS+=	mbin_baseP.c
SRCS+=	mbin_baseT.c
SRCS+=	mbin_baseU.c
SRCS+=	mbin_baseV.c
SRCS+=	mbin_base_m3.c
SRCS+=	mbin_bits.c
SRCS+=	mbin_bitreverse.c
SRCS+=	mbin_coeff.c
SRCS+=	mbin_correlate.c
SRCS+=	mbin_dec.c
SRCS+=	mbin_depolarise.c
SRCS+=	mbin_div.c
SRCS+=	mbin_eq_mod.c
SRCS+=	mbin_equation.c
SRCS+=	mbin_equation_float.c
SRCS+=	mbin_equation_double.c
SRCS+=	mbin_expand.c
SRCS+=	mbin_express.c
SRCS+=	mbin_factor.c
SRCS+=	mbin_ftt.c
SRCS+=	mbin_fet.c
SRCS+=	mbin_filter.c
SRCS+=	mbin_fp.c
SRCS+=	mbin_fpt.c
SRCS+=	mbin_fpx.c
SRCS+=	mbin_fst.c
SRCS+=	mbin_hpt.c
SRCS+=	mbin_inc.c
SRCS+=	mbin_modular_fft.c
SRCS+=	mbin_noise.c
SRCS+=	mbin_optimise.c
SRCS+=	mbin_orthogonal.c
SRCS+=	mbin_parse.c
SRCS+=	mbin_polarise.c
SRCS+=	mbin_power.c
SRCS+=	mbin_print.c
SRCS+=  mbin_diff.c
SRCS+=  mbin_graycode.c
SRCS+=  mbin_logic.c
SRCS+=  mbin_lsb.c
SRCS+=  mbin_lucas.c
SRCS+=  mbin_msb.c
SRCS+=  mbin_mod_array.c
SRCS+=  mbin_mul.c
SRCS+=  mbin_multiply_x3.c
SRCS+=  mbin_quad.c
SRCS+=  mbin_recode.c
SRCS+=  mbin_sine.c
SRCS+=  mbin_sort.c
SRCS+=  mbin_sos.c
SRCS+=  mbin_sqrt.c
SRCS+=  mbin_sumbits.c
SRCS+=  mbin_sumdigit.c
SRCS+=  mbin_swm.c
SRCS+=  mbin_transform.c
SRCS+=  mbin_vector.c
SRCS+=  mbin_xform_puzzle.c
SRCS+=  mbin_xor.c

INCS=	math_bin.h \
	math_bin_complex.h \
	math_bin_wrapper.h

MAN=	# no manual pages at the moment

.include <bsd.lib.mk>
