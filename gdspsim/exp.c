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

#include "c54_core.h"
#include "hardware.h"
#include <stdio.h>
#include "xymem.h"
#include "multiplier.h"

static GPtrArray *machine_code(gchar *opcode_text);
static void execute(struct _PipeLine *pipeP, struct _Registers *Reg);

static gchar *mask[]=    { "1111010s 10001110" };
static gchar *opcode[] = { "EXP d" };
static gchar *comment[]= { "T=leading bits($(s))" };

/* This definition is global because another routine will make have
 * an array that points to all the different instruction classes.
 */
Instruction_Class EXP_Obj =
{
  "EXP",
  NULL, // prefetch
  NULL, // fetch
  NULL, // decode
  NULL, // read_stg1 (access)
  NULL, // read_stg2 (read)
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
  int AorB,bit;
  union _GP_Reg_Union regS;

  AorB = (pipeP->current_opcode & 0x100 ) >> 8;
  if ( AorB )
    regS.gp_reg = MMR->B;
  else
    regS.gp_reg = MMR->A;

  if ( regS.gint64 == 0 )
    MMR->T = 0;
  else
    {
      if ( regS.gint64 < 0 )
	{
	  // Negative
	  if ( regS.gint64 == -1 )
	    {
	      MMR->T = -31;
	    }
	  else
	    {
	      bit = 38;
	      MMR->T=-8;
	      while ( (regS.gint64 & (1<<bit)) == 1 )
		{
		  bit--;
		  (MMR->T)++;
		}
	    }
  	}
      else
	{
	  bit = 38;
	  MMR->T=-8;
	  while ( (regS.gint64 & (1<<bit)) == 0 )
	    {
	      bit--;
	      (MMR->T)++;
	    }
	}
    }
}


/* Generates an array of Words that this opcode text generates or NULL
 * if it doesn't. There should not be any comments in the string
 */
static GPtrArray *machine_code(gchar *opcode_text)
{
  return NULL;
}


