/*
 * gDSPsim - GNU Digital Signal Processor Simulator
 *
 * Copyright (C) 2001-2002, Kerry Keal, kerry@industrialmusic.com
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

#ifndef __XYMEM_C55_H__
#define __XYMEM_C55_H__

#include <chip_core.h>

#ifdef __cplusplus
extern "C" {
#endif

  // Sets DAB
  void xymem_address_stg_b2(struct _PipeLine *pipeP, struct _Registers *Reg);
  
  // Sets DB
  void xymem_read_stg(struct _PipeLine *pipeP, struct _Registers *Reg);

#ifdef __cplusplus
}
#endif

#endif // __XYMEM_C55_H__
