# gDSPsim - GNU Digital Signal Processor Simulator
#
# Copyright (C) 2001, Kerry Keal, kerry@industrialmusic.com
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
PREFIX = /usr
VPATH = .:../gdspsim

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
	cmpr.o \
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
	intr.o \
	ld.o \
	ld_mac.o \
	ld_mas.o \
	ldr.o \
	ldu.o \
	ldm.o \
	lms.o \
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
	mvkd.o \
	mvpd.o \
	mvmd.o \
	mvmm.o \
	neg.o \
	nop.o \
	norm.o \
	or.o \
	orm.o \
	poly.o \
	popd.o \
	popm.o \
	portr.o \
	portw.o \
	pshd.o \
	pshm.o \
	rc.o \
	reada.o \
	reset.o \
	ret.o \
	rete.o \
	retf.o \
	rnd.o \
	rol.o \
	roltc.o \
	ror.o \
	rpt.o \
	rptz.o \
	rptb.o \
	rsbx.o \
	saccd.o \
	sat.o \
	sfta.o \
	sftc.o \
	sftl.o \
	sqdst.o \
	squr.o \
	squra.o \
	squrs.o \
	srccd.o \
	st.o \
	st_add.o \
	st_ld.o \
	st_mac.o \
	st_mas.o \
	st_mpy.o \
	st_sub.o \
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
	trap.o \
	writa.o \
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
	plot_window.o \
	preferences.o \
	readfile.o \
	register_window.o \
	shifter.o \
	smem.o \
	string_func.o \
	symbols.o \
	xymem.o \

SRCS	= $(OBJS:%.o=%.c)

#$(OBJS):	$(SRCS) $(DEPS)
.c.o:
	$(CC) -Wall -ggdb -I../c54 -I../gdspsim -I. -I.. `pkg-config gnome gtk+ --cflags`   -c -D_C54 $<

all:	$(PROGRAM)

$(PROGRAM):	$(OBJS)
		$(CC) -rdynamic `pkg-config gnome gtk+ --libs` -lgtkextra -o $(PROGRAM) $(OBJS)
		@echo "done"
#		$(CC) -rdynamic `pkg-config gtk+  --libs`  -lgtkextra -o $(PROGRAM) $(OBJS)

install:	$(PROGRAM)
		install -D --mode=755 $(PROGRAM) $(PREFIX)/bin/$(PROGRAM)

clean:		
		/bin/rm -f *.o $(PROGRAM)


