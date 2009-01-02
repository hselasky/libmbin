#
# Makefile for my Binary Math library
#

LIB=		mbin1
SHLIB_MAJOR=	1
SHLIB_MINOR=	0
CFLAGS+=	-Wall -O2 -O3

SRCS=
SRCS+=	mbin_base1999.c
SRCS+=	mbin_baseG.c
SRCS+=	mbin_baseM.c
SRCS+=	mbin_baseN.c
SRCS+=	mbin_bitreverse.c
SRCS+=	mbin_dec.c
SRCS+=	mbin_depolarise.c
SRCS+=	mbin_div.c
SRCS+=	mbin_expand.c
SRCS+=	mbin_inc.c
SRCS+=	mbin_optimise.c
SRCS+=	mbin_polarise.c
SRCS+=	mbin_print.c
SRCS+=  mbin_greycode.c
SRCS+=  mbin_logic.c
SRCS+=  mbin_lsb.c
SRCS+=  mbin_msb.c
SRCS+=  mbin_recode.c
SRCS+=  mbin_submits.c
SRCS+=  mbin_transform.c

INCS=	math_bin.h

MKLINT=		no

NOGCCERROR=

MAN=	# no manual pages at the moment

.include <bsd.lib.mk>

