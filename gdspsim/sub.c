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
//#include "hardware.h"
#include <stdio.h>
#include "smem.h"
#include "xymem.h"
#include "decode.h"
#include "alu.h"
#include "shifter.h"

static void read_stg1(struct _PipeLine *pipeP, struct _Registers *Reg);
static void read_stg2(struct _PipeLine *pipeP, struct _Registers *Reg);
static void execute(struct _PipeLine *pipeP, struct _Registers *Reg);
static int number_words(struct _PipeLine *pipeP);

static GPtrArray *machine_code(gchar *opcode_text);

static gchar *mask[]= 
{
  "0000100s aaaaaaaa",
  "0000110s aaaaaaaa",
  "010000sd aaaaaaaa",
  "01101111 aaaaaaaa 000011sd 001nnnnn",
  "1001001s xxxxuuuu",
  "1010001d xxxxyyyy",
  "111100sd 0001uuuu nnnnnnnn nnnnnnnn",
  "111100sd 01100001 nnnnnnnn nnnnnnnn",
  "111101sd 001nnnnn",
  "111101sd 10000001" 
};

static gchar *opcode[] = 
{
"SUB a,s",
"SUB a,TS,s",
"SUB a,16,s,(opt=s)d",
"SUB s,(opt='0')n,s,(opt=d)d",
"SUB x,u,s",
"SUB x,y,d",
"SUB #n,(opt='0')u,s,(opt=s)d",
"SUB #n,16,s,(opt=s)d",
"SUB s,(opt='0')n,(opt=s)d",
"SUB s,_ASM,(opt=s)d"
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
Instruction_Class SUB_Obj =
{
  "SUB",
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
    case 0: // SUB Smem,src
    case 1: // SUB Smem,TS,src
    case 2: // SUB Smem,16,src,dst
      {
	smem_read_stg1(pipeP,Reg);
	break;
      }
    case 3: // SUB Smem,shift,src,dst
      {
	smem_read_stg1(pipeP,Reg);
	if ( pipeP->word_number == 1 )
	  {
	    pipeP->storage1 = Reg->IR;
	  }
	break;
      }
    case 4: // SUB Xmem,shift,src
      {
	xmem_read_stg1(pipeP,Reg);
	break;
      }
    case 5: // SUB Xmem,Ymem,dst
      {
	xymem_read_stg1(pipeP,Reg);
	break;
      }
    case 6: // SUB  #lk [, SHFT], src [, dst ]
    case 7: // SUB  #lk, 16, src [, dst ]
      {
	if ( pipeP->word_number == 1 )
	  {
	    pipeP->storage1 = Reg->IR;
	  }
	break;
      }
    }
  
  // SUB  src [, SHIFT], [, dst ]
  // SUB  src, ASM, [, dst ]
}

static void read_stg2(struct _PipeLine *pipeP, struct _Registers *Reg)
{
  switch ( pipeP->opcode_subType )
    {
    case 0: // SUB Smem,src
    case 1: // SUB Smem,TS,src
    case 2: // SUB Smem,16,src,dst
    case 3: // SUB Smem,shift,src,dst
      {
	smem_read_stg2(pipeP,Reg);
	break;
      }
    case 4: // SUB Xmem,shift,src
      {
	xmem_read_stg2(pipeP,Reg);
	break;
      }
    case 5: // SUB Xmem,Ymem,dst
      {
	xymem_read_stg2(pipeP,Reg);
	break;
      }
    }
  
  // SUB  #lk [, SHFT], src [, dst ]
  // SUB  #lk, 16, src [, dst ]
  // SUB  src [, SHIFT], [, dst ]
  // SUB  src, ASM, [, dst ]
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
	case 0: // SUB Smem,src
	  {
	    alu(0,(pipeP->current_opcode & 0x100)>>8,(pipeP->current_opcode & 0x100)>>8,1,Reg);
	  }
	case 1: // SUB Smem,TS,src
	  {
	    shifter((pipeP->current_opcode & 0x100)>>8,Reg,0,0,4);
	    alu(1,(pipeP->current_opcode & 0x100)>>8,(pipeP->current_opcode & 0x100)>>8,1,Reg);
	  }
	case 2: // SUB Smem,16,src,dst
	  {
	    shifter(2,Reg,2,16,4);
	    alu(0,(pipeP->current_opcode & 0x200)>>9,(pipeP->current_opcode & 0x100)>>8,1,Reg);
	  }
	case 3: // SUB Smem,shift,src,dst
	  {
	    int shift;

	    shift = signed_5bit_extract(pipeP->storage1);

	    alu(0,(pipeP->storage1 & 0x200)>>9,(pipeP->storage1 & 0x100)>>8,1,Reg);
    	  }
	case 4: // SUB Xmem,shift,src
	  {
	    int shift;
			    
	    shift = pipeP->current_opcode & 0xf;
	    
	    alu(0,(pipeP->current_opcode & 0x100)>>8,(pipeP->current_opcode & 0x100)>>8,1,Reg);
     	  }
	case 5: // SUB Xmem,Ymem,dst
	  {
	    alu(0,3,(pipeP->current_opcode & 0x100)>>8,3,Reg);
	  }
	case 6: // SUB  #lk [, SHFT], src [, dst ]
	  {
	    SWord shift;
	    int regNum;
	    	    
	    shift = pipeP->current_opcode & 0xf;
	    regNum = (pipeP->current_opcode&0x100)>>8;
	    
	    reg_union.gint64 = (SWord)pipeP->storage1;
	    if (regNum)
	      MMR->B = reg_union.gp_reg;
	    else
	      MMR->A = reg_union.gp_reg;

	    shifter(regNum,Reg,2,shift,regNum);

	    
	    alu(1,(pipeP->current_opcode & 0x200)>>9,(pipeP->current_opcode & 0x100)>>8,1,Reg);
	    
	    //	    reg_union2.gp_reg = alu_add(reg_union.gp_reg,pipeP->storage1,shift,SXM(MMR) );
	    
	    if ( pipeP->current_opcode & 0x100 )
	      MMR->B = reg_union2.gp_reg;
	    else
	      MMR->A = reg_union2.gp_reg;
	    break;
	  }
	case 7: // SUB  #lk, 16, src [, dst ]
	  {
	    int regNum;
	    regNum = (pipeP->current_opcode&0x100)>>8;
	    
	    reg_union.gint64 = (SWord)pipeP->storage1;
	    if (regNum)
	      MMR->B = reg_union.gp_reg;
	    else
	      MMR->A = reg_union.gp_reg;

	    shifter(regNum,Reg,2,16,regNum);


	    if ( pipeP->current_opcode & 0x200 )
	      reg_union.gp_reg = MMR->B;
	    else
	      reg_union.gp_reg = MMR->A;
	    
	    //	    reg_union2.gp_reg = alu_add(reg_union.gp_reg,pipeP->storage1,16,SXM(MMR) );
	    
	    if ( pipeP->current_opcode & 0x100 )
	      MMR->B = reg_union2.gp_reg;
	    else
	      MMR->A = reg_union2.gp_reg;
	    break;
	  }
	case 8: // SUB  src [, SHIFT], [, dst ]
	  {
	    int shift;

	    shift = signed_5bit_extract(pipeP->current_opcode);
	    
	    shifter((pipeP->current_opcode & 0x200)>>9,Reg,2,shift,4);
	    
	    //	    reg_union2.gp_reg = alu_add40(reg_union.gp_reg,reg_union2.gp_reg,shift,Reg );
	    
	    if ( pipeP->current_opcode & 0x100 )
	      MMR->B = reg_union2.gp_reg;
	    else
	      MMR->A = reg_union2.gp_reg;
	    break;
	    
	  }
	case 9: // SUB  src, ASM, [, dst ]
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
	    
	    //	    reg_union2.gp_reg = alu_add40(reg_union.gp_reg,reg_union2.gp_reg,shift,Reg);
	    
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
    case 0: // SUB Smem,src
    case 1: // SUB Smem,TS,src
    case 2: // SUB Smem,16,src,dst
      {
	return num_words_for_smem(pipeP);
      }
    case 3: // SUB Smem,shift,src,dst
      {
	return num_words_for_smem_plus1(pipeP);
      }
    case 4: // SUB Xmem,shift,src
    case 5: // SUB Xmem,Ymem,dst
    case 8: // SUB  src [, SHIFT], [, dst ]
    case 9: // SUB  src, ASM, [, dst ]
    default:
      {
	return 1;
	
      }
    case 6: // SUB  #lk [, SHFT], src [, dst ]
    case 7: // SUB  #lk, 16, src [, dst ]
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
