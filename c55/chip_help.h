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

#ifndef __CHIP_HELP_C55__
#define __CHIP_HELP_C55__

#include <chip_core.h>

#ifdef __cplusplus
extern "C" 
{
#endif


  // returns a register value AC[0-3], Temp Reg or Pointer Reg
  // sign_extend if true
  union _GP_Reg_Union get_register(int r, int sign_extend);

  // Set ACx (r=0-3). No flags or saturation set. Uses LSB
  // for 16 bit registers
  void set_register(union _GP_Reg_Union reg_union, int r);
  
  // Set ACOV[0-3] (r=0-3) if reg_union is saturated. Saturation
  // is 40 bits if M40 (read inside function) is set and 32 bits
  // if M40 is not set. Set register ACx (r=0-3) to saturated value if sat_bit
  // is non-zero or to reg_union if sat_bit is zero.
  void set_reg_saturate(union _GP_Reg_Union reg_union, int r, int sat_bit);

  // Set ACOV[0-3] (r=0-3) if reg_union is saturated. Saturation is
  // 0x80 0000 0000 and 0x7f ffff ffff if _M40=1 and 
  // 0xff 8000 0000 and 0x00 7fff ffff if _M40=0.
  // Saturated value is returned if _SATD=1,3 and reg_union is _SATD=0,2
  // ACx (r=0,3) is set to Saturated value is _SATD=2,3
  union _GP_Reg_Union saturate(union _GP_Reg_Union reg_union, int r, int _M40, int _SATD);

#ifdef __cplusplus
}
#endif

#endif // __CHIP_HELP_C55__



