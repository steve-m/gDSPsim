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

#ifndef __CMEM_C55_H__
#define __CMEM_C55_H__

#include <chip_core.h>

#ifdef __cplusplus
extern "C" 
{
#endif
  
  // decode CDP encoding, update pointer and return value of CDP.
  Word cmem_decode(int mod);

  // Sets EAB based upon Cmem (CDP). Reads the last 2 bits of the
  // 3rd byte to figure out how to modify CDP
  void cmem_set_EAB_b3(struct _PipeLine *pipeP, struct _Registers *Reg);

  // Sets the BAB bus for CDP by decoding last 2 bits of 3rd byte.
  void cmem_address_stg_b3(struct _PipeLine *pipeP, struct _Registers *Reg);

#ifdef __cplusplus
}
#endif

#endif // __CMEM_C55_H__
