/*
 * gDSPsim - GNU Digital Signal Processor Simulator
 *
 * Copyright (C) 2001, Kerry Keal, kerry@industrialmusic.com
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * version 2, as published by the Free Software Foundation.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/

#ifndef __XYMEM_H__
#define __XYMEM_H__

#include "c54_core.h"

WordA update_xymem( Word XYmem, int Xdecode, struct _Registers *Reg);
void xymem_read_stg1(struct _PipeLine *pipeP, struct _Registers *Reg);
void xymem_read_stg2(struct _PipeLine *pipeP, struct _Registers *Reg);
void xmem_read_stg1(struct _PipeLine *pipeP, struct _Registers *Reg);
void xmem_read_stg2(struct _PipeLine *pipeP, struct _Registers *Reg);
void xmem_set_EAB(struct _PipeLine *pipeP, struct _Registers *Reg);
void ymem_set_EAB(struct _PipeLine *pipeP, struct _Registers *Reg);
#endif // __XYMEM_H__
