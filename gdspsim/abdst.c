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

// Audit: Feb 20,2002:1 wkk

#include "c54_core.h"
#include "xymem.h"

static GPtrArray *machine_code(gchar *opcode_text);
static void execute(struct _PipeLine *pipeP, struct _Registers *Reg);

static gchar *mask[]=    { "11100011 xxxxyyyy" };
static gchar *opcode[] = { "ABDST x,y" };
static gchar *comment[]= { "B=B + A[32:16]; A = ($(x) * $(y)) << 16" };


/* This definition is global because another routine will make have
 * an array that points to all the different instruction classes.
 */
Instruction_Class ABDST_Obj =
{
  "ABDST",
  NULL, // prefetch
  NULL, // fetch
  NULL, // decode
  xymem_read_stg1, // read_stg1 (access)
  xymem_read_stg2, // read_stg2 (read)
  execute, // execute
  NULL, // number_words 
  NULL, // set_cycle_number
  1,
  mask,
  opcode,
  comment,
  machine_code
};

static void execute(struct _PipeLine *pipeP, struct _Registers *Reg)
{
  // much taken from ALU, abs properties and FRCT reasons for embedding
  union _GP_Reg_Union X,Y;

  // B = B + |A[32-16]|
  X.gint64 = 0;
  Y.gint64 = 0;

  X.guint64 = GP_REG17_TO_UINT64(MMR->A);
  if ( X.gint64 < 0 )
    X.gint64 = -X.gint64;

  Y.guint64 = GP_REG_2_UINT64(MMR->B);

  X.gint64 = X.gint64 + Y.gint64;

  // Set C bit. FIXME? just guessing on how the C is set
  if ( X.guint64 & (guint64)0x100000000 )
    set_C(MMR,1);

  // Check for overflow and set overflow bit
  if ( X.gint64 > max_pos32 )
    {
      set_OVB(MMR,1);
      if ( OVM(MMR) )
	X.gint64 = max_pos32;
    }
  else if ( X.gint64 < max_neg32 )
    {
      set_OVB(MMR,1);
      if ( OVM(MMR) )
	X.gint64 = max_neg32;
    }

  MMR->B = X.gp_reg;

  // A=|Xmem-Ymem|<<16  

  X.gint64 = 0;
  Y.gint64 = 0;

  if ( SXM(MMR) )
    X.gint64 = (SWord)Reg->DB;
  else
    X.words.low = Reg->DB;
  
  if ( SXM(MMR) )
    Y.gint64 = (SWord)Reg->CB;
  else
    Y.words.low = Reg->CB;
  
  X.gint64 = X.gint64 - Y.gint64;
  
  if ( X.gint64 < 0 )
    X.gint64 = -X.gint64;

  X.guint64 = X.guint64 << 16;

  if ( FRCT(MMR) )
    X.guint64 = X.guint64 << 1;

  // Set C bit. FIXME? just guessing on how the C is set
  if ( X.guint64 & (guint64)0x100000000 )
    set_C(MMR,1);

  // Check for overflow and set overflow bit
  if ( X.gint64 > max_pos32 )
    {
      set_OVA(MMR,1);
      if ( OVM(MMR) )
	X.gint64 = max_pos32;
    }
  else if ( X.gint64 < max_neg32 )
    {
      set_OVA(MMR,1);
      if ( OVM(MMR) )
	X.gint64 = max_neg32;
    }

  MMR->A = X.gp_reg;

}


/* Generates an array of Words that this opcode text generates or NULL
 * if it doesn't. There should not be any comments in the string
 */
static GPtrArray *machine_code(gchar *opcode_text)
{
  return NULL;
}


