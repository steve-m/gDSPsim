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
#include "smem.h"
#include "xymem.h"
#include "decode.h"

static void read_stg1(struct _PipeLine *pipeP, struct _Registers *Reg);
static void read_stg2(struct _PipeLine *pipeP, struct _Registers *Reg);
static void execute(struct _PipeLine *pipeP, struct _Registers *Reg);
static int number_words(struct _PipeLine *pipeP);

static GPtrArray *machine_code(gchar *opcode_text);

static gchar *mask[]= 
{
  "0000000s aaaaaaaa",
  "0000010s aaaaaaaa",
  "001111sd aaaaaaaa",
  "01101111 aaaaaaaa 000011sd 000nnnnn",
  "1001000s xxxxuuuu",
  "1010000d xxxxyyyy",
  "111100sd 0000uuuu nnnnnnnn nnnnnnnn",
  "111100sd 01100000 nnnnnnnn nnnnnnnn",
  "111101sd 000nnnnn",
  "111101sd 10000000" 
};

static gchar *opcode[] = 
{
"ADD a,s",
"ADD a,TS,s",
"ADD a,16,s,(opt=s)d",
"ADD s,(opt='0')n,s,(opt=d)d",
"ADD x,u,s",
"ADD x,y,d",
"ADD #n,(opt='0')u,s,(opt=s)d",
"ADD #n,16,s,(opt=s)d",
"ADD s,(opt='0')n,(opt=s)d",
"ADD s,_ASM,(opt=s)d"
};

static gchar *comment[]=  
{
"$s=$i$a<<TS+$s",
"$(s)=$(a)<<TS+$(s)",
"$(d)=$(a)<<16+$(s)",
"$(d)=$(a)<<$(n)+$(s)",
"$s)=$(x)<<$(u)+$(s)",
"$(d)=$(x)+$(y)<<16",
"$(d)=$(n)<<$(u)+$(s)",
"$(d)=$(n)<<$(n)#,_16,src[9],dst[8]",
"$(d)=$(d)+$(s)<<$(n)",
"$(d)=$(d)+$(s)<<ASM"
};

/* This definition is global because another routine will make have
 * an array that points to all the different instruction classes.
 */
Instruction_Class ADD_Obj =
{
  "ADD",
  NULL, // prefetch
  NULL, // fetch
  NULL, // decode
  read_stg1, // read_stg1 (access)
  read_stg2, // read_stg2 (read)
  execute, // execute
  number_words, // number_words 
  NULL, // set_cycle_number
  10,
  mask,
  opcode,
  comment,
  machine_code
};

static void read_stg1(struct _PipeLine *pipeP, struct _Registers *Reg)
{
  switch ( pipeP->opcode_subType )
    {
    case 0: // ADD Smem,src
    case 1: // ADD Smem,TS,src
    case 2: // ADD Smem,16,src,dst
      {
	smem_read_stg1(pipeP,Reg);
	break;
      }
    case 3: // ADD Smem,shift,src,dst
      {
	smem_read_stg1(pipeP,Reg);
	if ( pipeP->word_number == 1 )
	  {
	    pipeP->storage1 = Reg->IR;
	  }
	break;
      }
    case 4: // ADD Xmem,shift,src
      {
	xmem_read_stg1(pipeP,Reg);
	break;
      }
    case 5: // ADD Xmem,Ymem,dst
      {
	xymem_read_stg1(pipeP,Reg);
	break;
      }
    case 6: // ADD  #lk [, SHFT], src [, dst ]
    case 7: // ADD  #lk, 16, src [, dst ]
      {
	if ( pipeP->word_number == 1 )
	  {
	    pipeP->storage1 = Reg->IR;
	  }
	break;
      }
    }
  
  // ADD  src [, SHIFT], [, dst ]
  // ADD  src, ASM, [, dst ]
}

static void read_stg2(struct _PipeLine *pipeP, struct _Registers *Reg)
{
  switch ( pipeP->opcode_subType )
    {
    case 0: // ADD Smem,src
    case 1: // ADD Smem,TS,src
    case 2: // ADD Smem,16,src,dst
    case 3: // ADD Smem,shift,src,dst
      {
	smem_read_stg2(pipeP,Reg);
	break;
      }
    case 4: // ADD Xmem,shift,src
      {
	xmem_read_stg2(pipeP,Reg);
	break;
      }
    case 5: // ADD Xmem,Ymem,dst
      {
	xymem_read_stg2(pipeP,Reg);
	break;
      }
    }
  
  // ADD  #lk [, SHFT], src [, dst ]
  // ADD  #lk, 16, src [, dst ]
  // ADD  src [, SHIFT], [, dst ]
  // ADD  src, ASM, [, dst ]
}

/* Executes the instruction. The operands are passed as pointers to
 * Registers. These pointers point to Registers in the Reg variable.
 * Flags may also be affected or cause different execution results,
 * therefore the Register pointer is also passed. Many instructions
 * have slightly different implementations, therefore type is passed.
 */

static void execute(struct _PipeLine *pipeP, struct _Registers *Reg)
{
  union _GP_Reg_Union reg_union;
  union _GP_Reg_Union reg_union2;

  if ( pipeP->word_number == 1 )
    {

      switch ( pipeP->opcode_subType )
	{
	case 0: // ADD Smem,src
	  {
	    if ( pipeP->current_opcode & 0x100 )
	      reg_union.gp_reg = MMR->B;
	    else
	      reg_union.gp_reg = MMR->A;
	    
	    reg_union2.gp_reg = alu_add(reg_union.gp_reg,Reg->DB,0,SXM(MMR) );
	    
	    if ( pipeP->current_opcode & 0x100 )
	      MMR->B = reg_union2.gp_reg;
	    else
	      MMR->A = reg_union2.gp_reg;
	    break;
	  }
	case 1: // ADD Smem,TS,src
	  {
	    if ( pipeP->current_opcode & 0x100 )
	      reg_union.gp_reg = MMR->B;
	    else
	      reg_union.gp_reg = MMR->A;
	    
	    reg_union2.gp_reg = alu_add(reg_union.gp_reg,Reg->DB,MMR->T,SXM(MMR) );
	    
	    if ( pipeP->current_opcode & 0x100 )
	      MMR->B = reg_union2.gp_reg;
	    else
	      MMR->A = reg_union2.gp_reg;
	    break;
	  }
	case 2: // ADD Smem,16,src,dst
	  {
	    if ( pipeP->current_opcode & 0x200 )
	      reg_union.gp_reg = MMR->B;
	    else
	      reg_union.gp_reg = MMR->A;
	    
	    reg_union2.gp_reg = alu_add(reg_union.gp_reg,Reg->DB,16,SXM(MMR) );
	    
	    if ( pipeP->current_opcode & 0x100 )
	      MMR->B = reg_union2.gp_reg;
	    else
	      MMR->A = reg_union2.gp_reg;
	    break;
	  }
	case 3: // ADD Smem,shift,src,dst
	  {
	    int shift;
	    
	    if ( pipeP->storage1 & 0x200 )
	      reg_union.gp_reg = MMR->B;
	    else
	      reg_union.gp_reg = MMR->A;

	    shift = signed_5bit_extract(pipeP->storage1);
	    
	    reg_union2.gp_reg = alu_add(reg_union.gp_reg,Reg->DB,shift,SXM(MMR) );

	    if ( pipeP->storage1 & 0x100 )
	      MMR->B = reg_union2.gp_reg;
	    else
	      MMR->A = reg_union2.gp_reg;
	    break;
	  }
	case 4: // ADD Xmem,shift,src
	  {
	    int shift;
	    
	    if ( pipeP->current_opcode & 0x100 )
	      reg_union.gp_reg = MMR->B;
	    else
	      reg_union.gp_reg = MMR->A;
	    
	    shift = pipeP->current_opcode & 0xf;
	    
	    reg_union2.gp_reg = alu_add(reg_union.gp_reg,Reg->DB,shift,SXM(MMR) );
	    
	    if ( pipeP->current_opcode & 0x100 )
	      MMR->B = reg_union2.gp_reg;
	    else
	      MMR->A = reg_union2.gp_reg;
	    break;
	  }
	case 5: // ADD Xmem,Ymem,dst
	  {
	    if ( pipeP->current_opcode & 0x100 )
	      reg_union.gp_reg = MMR->B;
	    else
	      reg_union.gp_reg = MMR->A;
	    
	    reg_union2.gp_reg = alu_add(reg_union.gp_reg,Reg->DB,0,SXM(MMR) );
	    
	    // shift result by 16
	    reg_union2.guint64 = reg_union2.guint64 << 16;
	    
	    if ( pipeP->current_opcode & 0x100 )
	      MMR->B = reg_union2.gp_reg;
	    else
	      MMR->A = reg_union2.gp_reg;
	    break;
	  }
	case 6: // ADD  #lk [, SHFT], src [, dst ]
	  {
	    SWord shift;
	    
	    if ( (pipeP->current_opcode & 0x200) != 0 )
	      reg_union.gp_reg = MMR->B;
	    else
	      reg_union.gp_reg = MMR->A;
	    
	    shift = pipeP->current_opcode & 0xf;
	    
	    reg_union2.gp_reg = alu_add(reg_union.gp_reg,pipeP->storage1,shift,SXM(MMR) );
	    
	    if ( pipeP->current_opcode & 0x100 )
	      MMR->B = reg_union2.gp_reg;
	    else
	      MMR->A = reg_union2.gp_reg;
	    break;
	  }
	case 7: // ADD  #lk, 16, src [, dst ]
	  {
	    if ( pipeP->current_opcode & 0x200 )
	      reg_union.gp_reg = MMR->B;
	    else
	      reg_union.gp_reg = MMR->A;
	    
	    reg_union2.gp_reg = alu_add(reg_union.gp_reg,pipeP->storage1,16,SXM(MMR) );
	    
	    if ( pipeP->current_opcode & 0x100 )
	      MMR->B = reg_union2.gp_reg;
	    else
	      MMR->A = reg_union2.gp_reg;
	    break;
	  }
	case 8: // ADD  src [, SHIFT], [, dst ]
	  {
	    int shift;
	    
	    if ( pipeP->current_opcode & 0x200 )
	      reg_union.gp_reg = MMR->B;
	    else
	      reg_union.gp_reg = MMR->A;
	    
	    if ( pipeP->current_opcode & 0x100 )
	      reg_union2.gp_reg = MMR->B;
	    else
	      reg_union2.gp_reg = MMR->A;
	    
	    shift = signed_5bit_extract(pipeP->current_opcode);
	    
	    reg_union2.gp_reg = alu_add40(reg_union.gp_reg,reg_union2.gp_reg,shift,Reg );
	    
	    if ( pipeP->current_opcode & 0x100 )
	      MMR->B = reg_union2.gp_reg;
	    else
	      MMR->A = reg_union2.gp_reg;
	    break;
	    
	  }
	case 9: // ADD  src, ASM, [, dst ]
	  {
	    int shift;
	    
	    if ( pipeP->current_opcode & 0x200 )
	      reg_union.gp_reg = MMR->B;
	    else
	      reg_union.gp_reg = MMR->A;
	    
	    if ( pipeP->current_opcode & 0x100 )
	      reg_union2.gp_reg = MMR->B;
	    else
	      reg_union2.gp_reg = MMR->A;
	    
	    shift = ASM(MMR);
	    
	    reg_union2.gp_reg = alu_add40(reg_union.gp_reg,reg_union2.gp_reg,shift,Reg);
	    
	    if ( pipeP->current_opcode & 0x100 )
	      MMR->B = reg_union2.gp_reg;
	    else
	      MMR->A = reg_union2.gp_reg;
	    break;
	  }
	  
	}
    }
}

int number_words(struct _PipeLine *pipeP)
{
  switch ( pipeP->opcode_subType )
    {
    case 0: // ADD Smem,src
    case 1: // ADD Smem,TS,src
    case 2: // ADD Smem,16,src,dst
      {
	return num_words_for_smem(pipeP);
      }
    case 3: // ADD Smem,shift,src,dst
      {
	return num_words_for_smem_plus1(pipeP);
      }
    case 4: // ADD Xmem,shift,src
    case 5: // ADD Xmem,Ymem,dst
    case 8: // ADD  src [, SHIFT], [, dst ]
    case 9: // ADD  src, ASM, [, dst ]
    default:
      {
	return 1;
	
      }
    case 6: // ADD  #lk [, SHFT], src [, dst ]
    case 7: // ADD  #lk, 16, src [, dst ]
      {
	return 2;
	
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
