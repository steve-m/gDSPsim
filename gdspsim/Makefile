# gDSPsim - GNU Digital Signal Processor Simulator
#
# Copyright (C) 2000, Kerry Keal, kerry@industrialmusic.com
#
# This program is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License
# version 2, as published by the Free Software Foundation.
# 
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
# 
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.


CC = gcc
PROGRAM	= gdspsim

#	c54_func/nop.o \
#	c54_func/add.o \
#	c54_func/find_opcode.o \
#	c54_func/dadd.o \

OBJS	= \
	abdst.o \
	abs.o \
	add.o \
	addc.o \
	addm.o \
	adds.o \
	and.o \
	andm.o \
	b.o \
	bacc.o \
	banz.o \
	bc.o \
	bit.o \
	bitf.o \
	bitt.o \
	cala.o \
	call.o \
	cc.o \
	cmpl.o \
	cmpm.o \
	cmps.o \
	dadd.o \
	dadst.o \
	delay.o \
	drsub.o \
	dld.o \
	dsadt.o \
	dst.o \
	dsub.o \
	dsubt.o \
	exp.o \
	fb.o \
	fbacc.o \
	fcala.o \
	fcall.o \
	firs.o \
	frame.o \
	fret.o \
	frete.o \
	idle.o \
	ld.o \
	ldr.o \
	ldu.o \
	ldm.o \
	ltd.o \
	mac.o \
	maca.o \
	macd.o \
	macp.o \
	macsu.o \
	mar.o \
	mas.o \
	masa.o \
	max.o \
	min.o \
	mpy.o \
	mpya.o \
	mpyu.o \
	mvdd.o \
	mvdp.o \
	mvdk.o \
	mvdm.o \
	mvmd.o \
	mvmm.o \
	neg.o \
	nop.o \
	norm.o \
	or.o \
	orm.o \
	popd.o \
	popm.o \
	pshd.o \
	pshm.o \
	reada.o \
	ret.o \
	rete.o \
	rnd.o \
	rol.o \
	roltc.o \
	ror.o \
	rpt.o \
	rptb.o \
	rsbx.o \
	sat.o \
	sfta.o \
	st.o \
	st_ld.o \
	sth.o \
	ssbx.o \
	stl.o \
	stlm.o \
	stm.o \
	strcd.o \
	sub.o \
	subb.o \
	subc.o \
	subs.o \
	xc.o \
	xor.o \
	xorm.o \
	alu.o \
	decode.o \
	decode_window.o \
	entryCB.o \
	fileIO.o \
	find_opcode.o \
	gtkbox_add.o \
	hardware.o \
	instruct_help.o \
	main.o \
	memory.o \
	memory_window.o \
	multiplier.o \
	pipeline.o \
	readfile.o \
	register_window.o \
	shifter.o \
	smem.o \
	string_func.o \
	symbols.o \
	xymem.o

SRCS	= $(OBJS:%.o=%.c)

#DEPS	= $(OBJS:%.o=%.c)
#include	$(SRCS:.c=.d)

#.c.o:
#$(OBJS):	$(SRCS) $(DEPS)
.c.o:
	$(CC) -Wall -ggdb -I. `pkg-config gtk+ --cflags`  -c $<

all:	$(PROGRAM)

# this might have worked if put on the first line
#	cd c54_func
#	./make

$(PROGRAM):	$(OBJS)
		$(CC) -rdynamic `pkg-config gtk+  --libs` -o $(PROGRAM) $(OBJS)
		@echo "done"

clean:		
		/bin/rm -f *.o $(PROGRAM)


