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
#include "alu.h"
#include "shifter.h"
#include "decode.h"

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
  int regNumDst,regNumSrc;
  if ( pipeP->word_number == 1 )
    {

      switch ( pipeP->opcode_subType )
	{
	case 0: // SUB Smem,src
	  {
	    // src = src - Smem
	    regNumSrc = (pipeP->current_opcode&0x100)>>8;

	    alu(0,regNumSrc,regNumSrc,1,Reg);
	    break;
	  }
	case 1: // SUB Smem,TS,src
	  {
	    // src = src - (Smem<<TS)
	    regNumSrc = (pipeP->current_opcode&0x100)>>8;

	    shifter(2,Reg,0,0,4);
	    alu(1,regNumSrc,regNumSrc,1,Reg);
	    break;
	  }
	case 2: // SUB Smem,16,src,dst
	  {
	    // dst = src - (Smem<<16)
	    regNumDst = (pipeP->current_opcode&0x100)>>8;
	    regNumSrc = (pipeP->current_opcode&0x200)>>9;

	    shifter(2,Reg,2,16,4);
	    alu(1,regNumSrc,regNumDst,1|64,Reg);
	    break;
	  }
	case 3: // SUB Smem,shift,src,dst
	  {
	    // dst = src - (Smem<<shift)
	    int shift;

	    regNumDst = (pipeP->storage1&0x100)>>8;
	    regNumSrc = (pipeP->storage1&0x200)>>9;


	    shift = signed_5bit_extract(pipeP->storage1);
	    shifter(2,Reg,2,shift,4);
 	    alu(1,regNumSrc,regNumDst,1,Reg);
 	    break;
  	  }
	case 4: // SUB Xmem,shift,src
	  {
	    // src = src - (Xmem<<shift)
	    int shift;
	
	    regNumSrc = (pipeP->current_opcode&0x100)>>8;
		    
	    shift = pipeP->current_opcode & 0xf;
	    shifter(2,Reg,2,shift,4);
	    alu(1,regNumSrc,regNumSrc,1,Reg);
	    break;
     	  }
	case 5: // SUB Xmem,Ymem,dst
	  {
	    // dst = (Xmem<<16) - (Ymem<<16)
	    regNumDst = (pipeP->storage1&0x100)>>8;
	    // special flag for shift, bit=2^2
	    alu(0,3,regNumDst,5,Reg);
	    break;
	  }
	case 6: // SUB  #lk [, SHFT], src [, dst ]
	  {
	    // dst = src - (lk<<shift)
	    SWord shift;
	    int regNumDst,regNumSrc;
	    	    
	    shift = pipeP->current_opcode & 0xf;
	    regNumDst = (pipeP->current_opcode&0x100)>>8;
	    regNumSrc = (pipeP->current_opcode&0x200)>>9;
	    
	    // we're shifting a constant, not sure how the
	    // barrel shifter is suppose to do this,
	    // I'm going to throw the constant into CB.
	    // If this is the correct way, then this should
	    // have been done earlier.
	    Reg->CB = pipeP->storage1;

	    shifter(3,Reg,2,shift,4);
	    
	    alu(1,regNumSrc,regNumDst,1,Reg);
	    
	    break;
	  }
	case 7: // SUB  #lk, 16, src [, dst ]
	  {
	    // dst = src - (lk<<16)
	    int regNumDst,regNumSrc;

	    regNumDst = (pipeP->current_opcode&0x100)>>8;
	    regNumSrc = (pipeP->current_opcode&0x200)>>9;

	    // we're shifting a constant, not sure how the
	    // barrel shifter is suppose to do this,
	    // I'm going to throw the constant into CB.
	    // If this is the correct way, then this should
	    // have been done earlier.
	    Reg->CB = pipeP->storage1;

	    shifter(3,Reg,2,16,4);
	    
	    alu(1,regNumSrc,regNumDst,1|64,Reg);

	    break;
	  }
	case 8: // SUB  src [, SHIFT], [, dst ]
	  {
	    // dst = dst - ( src << shift)
	    int shift;
	    int regNumDst,regNumSrc;

	    regNumDst = (pipeP->current_opcode&0x100)>>8;
	    regNumSrc = (pipeP->current_opcode&0x200)>>9;

	    shift = signed_5bit_extract(pipeP->current_opcode);
	    
	    shifter(regNumSrc,Reg,2,shift,4);

	    alu(1,regNumDst,regNumDst,1,Reg);
	    break;
	  }
	case 9: // SUB  src, ASM, [, dst ]
	  {
	    // dst = dst - ( src << ASM)
	    int regNumDst,regNumSrc;

	    regNumDst = (pipeP->current_opcode&0x100)>>8;
	    regNumSrc = (pipeP->current_opcode&0x200)>>9;

	    shifter(regNumSrc,Reg,1,0,4);

	    alu(1,regNumDst,regNumDst,1,Reg);
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
