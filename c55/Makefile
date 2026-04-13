# gDSPsim - GNU Digital Signal Processor Simulator
#
# Copyright (C) 2001-2002, Kerry Keal, kerry@industrialmusic.com
#
# This program is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License
# version 2, as published by the Free Software Foundation.

CC ?= gcc
PROGRAM	= gdspsim-c55
PREFIX ?= /usr
VPATH = .:../gdspsim

GTK_CFLAGS = $(shell pkg-config gtk+-3.0 --cflags)
GTK_LIBS   = $(shell pkg-config gtk+-3.0 --libs)

CFLAGS += -Wall -Wno-deprecated-declarations -Wno-pointer-to-int-cast \
          -Wno-int-to-pointer-cast -Wno-incompatible-pointer-types \
          -Wno-format -Wno-unused -fgnu89-inline -fcommon -ggdb \
          -I../c55 -I../gdspsim $(GTK_CFLAGS) -D_C55

SRCS	= \
	aadd.c \
	abs.c \
	add.c \
	addsub.c \
	amar.c \
	and.c \
	asub.c \
	b.c \
	band.c \
	bcc.c \
	bclr.c \
	bcnt.c \
	bfxpa.c \
	bfxtr.c \
	bnot.c \
	bset.c \
	btst.c \
	btstclr.c \
	btstnot.c \
	btstp.c \
	btstset.c \
	call.c \
	cmp.c \
	dual_multiply.c \
	exp.c \
	implied_parallel_instr.c \
	mac.c \
	mant.c \
	mas.c \
	max.c \
	min.c \
	mpy.c \
	mpyk.c \
	mpym.c \
	mov_Mem2Mem.c \
	mov_RegLoad.c \
	mov_RegMove.c \
	mov_RegStore.c \
	mov_SpecRegLoad.c \
	mov_SpecRegMove.c \
	mov_SpecRegSave.c \
	neg.c \
	nop.c \
	not.c \
	pop.c \
	psh.c \
	or.c \
	ret.c \
	rol.c \
	ror.c \
	rpt.c \
	rptb.c \
	rptblocal.c \
	sftl.c \
	sfts.c \
	sftsc.c	\
	sqa.c \
	sqr.c \
	sqrm.c \
	sub.c \
	subadd.c \
	xcc.c \
	xor.c \
	alu.c \
	chip_help.c \
	cmem.c \
	decode.c \
	find_opcode.c \
	main.c \
	memory.c \
	multiplier.c \
	pipeline.c \
	readfile.c \
	register_window.c \
	shifter.c \
	smem.c \
	xymem.c \
	decode_window.c \
	entryCB.c \
	fileIO.c \
	memory_window.c \
	plot_window.c \
	preferences.c \
	symbols.c


OBJS	= $(SRCS:%.c=%.o)

all:	$(PROGRAM)

$(PROGRAM):	$(OBJS)
	$(CC) -o $@ $(OBJS) $(GTK_LIBS) -lm

install:	$(PROGRAM)
	install -D --mode=755 $(PROGRAM) $(PREFIX)/bin/$(PROGRAM)

clean:
	/bin/rm -f *.o $(PROGRAM)

.PHONY: all install clean
