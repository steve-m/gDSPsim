# gDSPsim - GNU Digital Signal Processor Simulator
#
# Copyright (C) 2001-2002, Kerry Keal, kerry@industrialmusic.com
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
PROGRAM	= gdspsim-c55
PREFIX = /usr
VPATH = .:../gdspsim

SRCS	= \
	aadd.c \
	add.c \
	bclr.c \
	bset.c \
	mov.c \
	neg.c \
	sfts.c \
	sftsc.c	\
	sub.c \
	decode.c \
	find_opcode.c \
	main.c \
	memory.c \
	pipeline.c \
	readfile.c \
	register_window.c \
	shifter.c \
	smem.c \
	decode_window.c \
	entryCB.c \
	fileIO.c \
	memory_window.c \
	plot_window.c \
	preferences.c \
	symbols.c


OBJS	= $(SRCS:%.c=%.o)

.c.o:
	$(CC) -Wall -ggdb -I../c55 -I../gdspsim `pkg-config gtk+ --cflags` -D_C55  -c $<

all:	$(PROGRAM)

$(PROGRAM):	$(OBJS)
		$(CC) -rdynamic `pkg-config gtk+  --libs` -lgtkextra -o $(PROGRAM) $(OBJS)
		@echo "done"

install:	$(PROGRAM)
		install -D --mode=755 $(PROGRAM) $(PREFIX)/bin/$(PROGRAM)
		install -D --mode=644 stop.xpm $(PREFIX)/share/$(PROGRAM)/stop.xpm

clean:		
		/bin/rm -f *.o $(PROGRAM)


