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

#include <glib.h>
#include <stdio.h>
#include "core_def.h"
#include "string.h"
#include <stdlib.h>
#include "string_func.h"
#include "decode.h"
#include "hardware.h"
#include "memory.h"
#include "symbols.h"

gchar *find_operands(char *mask,gchar *info, Word *start_code);
int find_opcode(Word *start_code, Word **next_code, GPtrArray *decode_info);
gchar *process_s_operand(char *mask, char info, Word *start_code);
gchar *accumulator_decode(char *mask, char info, Word *start_code);
gchar *ptr4_decode(gchar *mask, char info, Word *start_code);
int get_bits(gchar *info, int *word);

Word dm[55];

union u_operands read_op(char info, struct _Registers *Registers, Word bits, Word lk, int *wait_state)
{
  union u_operands op;

  if ( info == 's' || info =='d' )
    {
      if ( bits )
	{
	  op.regP = &MMR->B;
	}
      else
	{
	  op.regP = &MMR->A;
	}
      return op;
    }
  else if ( info == 'n' )
    {
    }
  else if ( info == 'u' || info == 'z' )
    {
    }
  else if ( info == 'x' || info == 'y' )
    {
    }
  else if ( info == 'a' )
    {
      // See C54_Vol1_CPU_and_Peripherals.pdf -> spru131g.pdf
      // Chapter 5.5, pg 128
      Word mod,arf;
      Word *arfP;

      arf = bits & 0x7;

      // get the pointer register ARx
      arfP = get_pointer_reg(arf,Registers,0);

      mod = ( bits >> 3 ) & 0xf;

      if ( bits & 0x80 )
	{
	  switch ( mod )
	    {
	    case 0:
	      {
		op.op_sword = read_data_mem(*arfP,wait_state);
		return op;
	      }
	    case 1:
	      {
		op.op_sword = read_data_mem( (*arfP)--,wait_state);
		return op;
	      }
	    case 2:
	      {
		op.op_sword = read_data_mem( (*arfP)++,wait_state);
		return op;
	      }
	    case 3:
	      {
		op.op_sword = read_data_mem( ++(*arfP),wait_state);
		return op;
	      }
	    case 4:
	      {
		int index,index_b,bit_size,bitn;

		// bit reverse addressing
		op.op_sword = read_data_mem(*arfP,wait_state);

		index = *arfP & ( MMR->ar0 -1);

		// bit reverse index;
		bit_size = MMR->ar0 >> 1;
		index_b = 0;
		bitn = 1;
		while ( index )
		  {
		    if ( index & bit_size )
		      index_b |= bitn;
		    index = index << 1;
		    bitn = bitn << 1;
		  }

		index_b--;

		// now bit reverse it back
		index_b = index_b & ( MMR->ar0 -1); // incase of overflow
		index = 0;
		bitn = 1;
		while ( index_b )
		  {
		    if ( index_b & bit_size )
		      index |= bitn;
		    index = index << 1;
		    bitn = bitn << 1;
		  }

		*arfP = index;

		return op; 
	      }
	    case 5:
	      {
		op.op_sword = read_data_mem(*arfP,wait_state);
		*arfP -= MMR->ar0;
		return op; 
	      }
	    case 6:
	      {
		op.op_sword = read_data_mem(*arfP,wait_state);
		*arfP += MMR->ar0;
		return op; 
	      }
	    case 7:
	      {
		int index,index_b,bit_size,bitn;

		// bit reverse addressing
		op.op_sword = read_data_mem(*arfP,wait_state);

		index = *arfP & ( MMR->ar0 -1);

		// bit reverse index;
		bit_size = MMR->ar0 >> 1;
		index_b = 0;
		bitn = 1;
		while ( index )
		  {
		    if ( index & bit_size )
		      index_b |= bitn;
		    index = index << 1;
		    bitn = bitn << 1;
		  }

		index_b++;

		// now bit reverse it back
		index_b = index_b & ( MMR->ar0 -1); // incase of overflow
		index = 0;
		bitn = 1;
		while ( index_b )
		  {
		    if ( index_b & bit_size )
		      index |= bitn;
		    index = index << 1;
		    bitn = bitn << 1;
		  }

		*arfP = index;

		return op; 
	      }
	    case 8:
	      {
		// Circular buffer changed by -1
		int index,N;
		Word BKcopy;

		op.op_sword = read_data_mem(*arfP,wait_state);
		// find lowest N that satisfies 2^N > BK
		
		if ( MMR->BK == 0 )
		  {
		    printf("Warning!, don't know how to do circular addressing with a 0 length circular buffer! file=%s:%d\n",__FILE__,__LINE__);
		    return op;
		  }
		N=0;
		BKcopy = MMR->BK;
		while ( BKcopy != 0 )
		  {
		    N++;
		    BKcopy = BKcopy >> 1;
		  }
		index = *arfP & (N-1);
		index--;
		if ( index >=  MMR->BK )
		  index -= MMR->BK;
		*arfP += index;

		return op; 
	      }
	    case 9:
	      {
		// Circular buffer changed by -1
		int index,N;
		Word BKcopy;

		op.op_sword = read_data_mem(*arfP,wait_state);
		// find lowest N that satisfies 2^N > BK
		
		if ( MMR->BK == 0 )
		  {
		    printf("Warning!, don't know how to do circular addressing with a 0 length circular buffer! file=%s:%d\n",__FILE__,__LINE__);
		    return op;
		  }
		N=0;
		BKcopy = MMR->BK;
		while ( BKcopy != 0 )
		  {
		    N++;
		    BKcopy = BKcopy >> 1;
		  }
		index = *arfP & (N-1);
		index--;
		if ( index < 0 )
		  index += MMR->BK;
		*arfP += index;

		return op; 
	      }
	    case 10:
	      {
		// Circular buffer changed by +1
		int index,N;
		Word BKcopy;

		op.op_sword = read_data_mem(*arfP,wait_state);
		// find lowest N that satisfies 2^N > BK
		
		if ( MMR->BK == 0 )
		  {
		    printf("Warning!, don't know how to do circular addressing with a 0 length circular buffer! file=%s:%d\n",__FILE__,__LINE__);
		    return op;
		  }
		N=1;
		BKcopy = MMR->BK;
		while ( BKcopy != 0 )
		  {
		    N++;
		    BKcopy = BKcopy >> 1;
		  }
		index = *arfP & (N-1);
		index++;
		if ( index >=  MMR->BK )
		  index -= MMR->BK;
		*arfP -= index;

		return op; 
	      }
	    case 11:
	      {
		// Circular buffer changed by +ar0
		int index,N;
		Word BKcopy;

		op.op_sword = read_data_mem(*arfP,wait_state);
		// find lowest N that satisfies 2^N > BK
		
		if ( MMR->BK == 0 )
		  {
		    printf("Warning!, don't know how to do circular addressing with a 0 length circular buffer! file=%s:%d\n",__FILE__,__LINE__);
		    return op;
		  }
		N=1;
		BKcopy = MMR->BK;
		while ( BKcopy != 0 )
		  {
		    N++;
		    BKcopy = BKcopy >> 1;
		  }
		index = *arfP & (N-1);
		index += MMR->ar0;
		if ( index >=  MMR->BK )
		  index -= MMR->BK;
		*arfP += index;

		return op; 
	      }
	    case 12:
	      {
		// *ARx(lk)

		// Not allowed for memory-mapped register addressing
		// instructions LDM, MVDM, MVMD, MVMM, POPM, PSHM, STLM, STM

		op.op_sword = read_data_mem(*arfP+lk,wait_state);
		return op;
	      }
	    case 13:
	      {
		// *+ARx(lk)

		// Not allowed for memory-mapped register addressing
		// instructions LDM, MVDM, MVMD, MVMM, POPM, PSHM, STLM, STM
		*arfP += lk;
		op.op_sword = read_data_mem(*arfP,wait_state);
		return op;
	      }
	    case 14:
	      {
		// *+ARx(lk)% , circular buffer preincrement by lk

		// Not allowed for memory-mapped register addressing
		// instructions LDM, MVDM, MVMD, MVMM, POPM, PSHM, STLM, STM

		int index,N;
		Word BKcopy;

		// find lowest N that satisfies 2^N > BK		
		if ( MMR->BK == 0 )
		  {
		    printf("Warning!, don't know how to do circular addressing with a 0 length circular buffer! file=%s:%d\n",__FILE__,__LINE__);
		    return op;
		  }
		N=1;
		BKcopy = MMR->BK;
		while ( BKcopy != 0 )
		  {
		    N++;
		    BKcopy = BKcopy >> 1;
		  }

		index = *arfP & (N-1);
		index += lk;
		if ( index >=  MMR->BK )
		  index -= MMR->BK;

		*arfP += index;

		op.op_sword = read_data_mem(*arfP,wait_state);

		return op; 
	      }
	    case 15:
	      {
		// *(lk)

		// Not allowed for memory-mapped register addressing
		// instructions LDM, MVDM, MVMD, MVMM, POPM, PSHM, STLM, STM

		op.op_sword = read_data_mem(lk,wait_state);
		return op;
	      }
	    }
	  // Indirect mode
	  if ( mod == 3 )
	    {
	      //union u_operands opp;

	      
	      arfP++;
	      return (union u_operands)*++arfP;
	      
	    }
	  if ( 1 )
	    {
	      // ptr = arfP;
	    }

	}
    }
  else if ( info == 'm' || info == 'w' )
    {
    }
  else
    {
      printf("Error cannot decode %d file=%s:%d\n",info,__FILE__,__LINE__);
    }
  return op;
}


// returns 1, if this mask matches the mach_code and sets the
// length and other mach_codes if needed in decode_nfo. returns
// 0, if mask doesn't match and doesnt set anything in decode_nfo.
int check_mask(const char *mask, struct _decoded_opcode *decode_nfo )
{
  int bit,wait_state,length,smem;
  Word mach_code;
  WordA address;

  g_return_val_if_fail(*mask,-1);

  bit=BITS_PER_WORD-1;
  length=1;
  smem=0;
  mach_code = decode_nfo->mach_code[0];
  address = decode_nfo->address;
  decode_nfo->var_length = 0;

  while ( (*mask != '\0') )
    {
      if ( *mask=='1')
	{
	  if ( (mach_code & ( 1 << bit )) == 0 )
	    return 0;
	}
      else if ( *mask=='0')
	{
	  if ( (mach_code & ( 1 << bit )) != 0 ) 
	    return 0;
	}
      else if ( *mask=='a' )
        {
          // mark for special case
          smem=1;
        }
      if ( *mask != ' ' )
	{
          mask++;
	  bit--;
          if ( bit == -1 )
            {
              if ( smem )
                {
                  // take care of variable length word
                  if ( mach_code & 0x80 )
                    {
                      int mod;
                      mod = (mach_code & (8+16+32+64) ) >> 3;
                      if ( mod >= 12 )
                        {
                          mach_code = 
                            read_program_mem(address+length,&wait_state);
                          decode_nfo->mach_code[length] = mach_code;
                          length++;
                          decode_nfo->var_length = 1;
                        }
                    }
                  smem=0;
                }

              // Remove any spaces at the end of mask
              while ( (*mask != '\0') && (*mask == ' ') )
                mask++;

              if ( *mask == '\0' )
                {
                  decode_nfo->length = length;
                  return 1; // found a match
                }

              // must read another word
              mach_code = read_program_mem(address+length,&wait_state);
              decode_nfo->mach_code[length] = mach_code;
              length++;

              bit=BITS_PER_WORD-1;
            }
        }
      else
        {
          mask++;
        }

    }

    printf("Error, funny bit length for mask=%s file=%s:%d\n",mask,__FILE__,__LINE__);

    decode_nfo->length = length;
    return 1;
}

// Decodes 9 memory mapped registers MMRx (ARx,SP) MVMM AR2,SP
void vw_decode(gchar *ch, gchar *mask, char info, 
               struct _decoded_opcode *decode_nfo )
{
  unsigned int bits;

  bits = bit_extract(info,mask,decode_nfo,NULL);

  if ( bits > 7 )
    g_snprintf(ch,MAX_SUB_OP,"SP");
  else
    g_snprintf(ch,MAX_SUB_OP,"AR%d",bits);
}

// decodes condition codes
void c_decode(gchar *ch, gchar *mask, char info, 
	      struct _decoded_opcode *decode_nfo )
{
  unsigned int bits;

  bits = bit_extract(info,mask,decode_nfo,NULL);
  
  *ch='\0';

  if ( bits == 0x0 )
    {
      strcat(ch,"UNC");
      return;
    }

  if ( bits & 0x40 )
    {
      // Group 1
      if ( bits & 0x7 )
	{
	  // Group 1, Category A   EQ,NEQ,LT,LEQ,GT,GEQ
	  // Assume that Category A determines the Condition Register
	  if ( bits & 0x8 )
	    strcat(ch,"B");
	  else
	    strcat(ch,"A");

	  switch ( bits & 0x7 )
	    {
	    default: 
	    case 0:
	    case 1:
	      strcat(ch,"????");
	      break;
	    case 2:
	      strcat(ch,"GEQ");
	      break;
	    case 3:
	      strcat(ch,"LT");
	      break;
	    case 4:
	      strcat(ch,"NEQ");
	      break;
	    case 5: 
	      strcat(ch,"EQ");
	      break;
	    case 6:
	      strcat(ch,"GT");
	      break;
	    case 7:
	      strcat(ch,"LEQ");
	      break;
	    }
	}
      if ( bits & 0x20 )
	{
	  if ( bits & 0x7 )
	    // 2 conditions
	    strcat(ch,",");

	  // Group 1, Category B
	  if ( ((bits & 0x7) && (bits & 0x8) ) ||
	       ((bits & 0x7)==0x0 && (bits & 0x8)==0x0) )
	    strcat(ch,"A");
	  else
	    strcat(ch,"B");

	  if ( bits & 0x10 )
	    strcat(ch,"OV");
	  else
	    strcat(ch,"NOV");
	}
    }
  else
    {
      // Group 2

      // Category A, TC NTC
      if ( bits & 0x2 )
	{
	  if ( bits & 0x1 )
	    strcat(ch,"BIO");
	  else
	    strcat(ch,"NBIO");
	  if ( bits & 0x28 )
	    // Another condition follows
	    strcat(ch,",");
	}
      if ( bits & 0x8 )
	{
	  if ( bits & 0x4 )
	    strcat(ch,"C");
	  else
	    strcat(ch,"NC");
	  if ( bits & 0x20 )
	    // Another condition follows
	    strcat(ch,",");
	}
      if ( bits & 0x20 )
	{
	  if ( bits & 0x10 )
	    strcat(ch,"TC");
	  else
	    strcat(ch,"NTC");
	}
    }
  return;
}

// dual pointer decodes
void xy_decode(gchar *ch, gchar *mask, char info,   
               struct _decoded_opcode *decode_nfo)
{
  // See C54_Vol1_CPU_and_Peripherals.pdf -> spru131g.pdf
  // pg 135 5-20
  unsigned int bits,mod,ar;

  bits = bit_extract(info,mask,decode_nfo,NULL);
  ar = (bits & 3)+2;
  mod = (bits & 12);

  if ( mod == 4)
    g_snprintf(ch,MAX_SUB_OP,"*AR%d-",ar);
  else if ( mod == 8 )
    g_snprintf(ch,MAX_SUB_OP,"*AR%d+",ar);
  else if ( mod == 0 )
    g_snprintf(ch,MAX_SUB_OP,"*AR%d",ar);
  else
    g_snprintf(ch,MAX_SUB_OP,"*AR%d+0%%",ar);

  return;
}

// extract unsigned decimal number
void u_decode(gchar *ch, gchar *mask, char info, 
              struct _decoded_opcode *decode_nfo)
{
  unsigned int bits;

  bits = bit_extract(info,mask,decode_nfo,NULL);

  g_snprintf(ch,MAX_SUB_OP,"%d",bits);

  return;
}

// same as u_decode but adds plus 1 to the number
void p_decode(gchar *ch, gchar *mask, char info, 
              struct _decoded_opcode *decode_nfo)
{
  unsigned int bits;

  bits = bit_extract(info,mask,decode_nfo,NULL);

  g_snprintf(ch,MAX_SUB_OP,"%d",bits+1);

  return;
}

void l_decode(gchar *ch, gchar *mask, char info, 
                struct _decoded_opcode *decode_nfo)
{
  gchar *sym_name;
  unsigned int bits;

  bits = bit_extract(info,mask,decode_nfo,NULL);

  sym_name=get_symbol(bits);
  if (sym_name)
    g_snprintf(ch,MAX_SUB_OP,"%s",sym_name);
  else
    g_snprintf(ch,MAX_SUB_OP,"0x%x",bits);
}

// Unsigned hex number
void h_decode(gchar *ch, gchar *mask, char info, 
                struct _decoded_opcode *decode_nfo)
{
  unsigned int bits;

  bits = bit_extract(info,mask,decode_nfo,NULL);

  g_snprintf(ch,MAX_SUB_OP,"0x%x",bits);

  return;
}

// Signed number
void n_decode(gchar *ch, gchar *mask, char info,  
              struct _decoded_opcode *decode_nfo)
{
  int bits;

  bits = signed_bit_extract(info,mask,decode_nfo);

  g_snprintf(ch,MAX_SUB_OP,"%d",bits);

  return;
}

// decodes status bits
void t_decode(gchar *ch, gchar *mask, char info,  
              struct _decoded_opcode *decode_nfo)
{
  int bits;

  bits = bit_extract(info,mask,decode_nfo,NULL);

  switch ( bits )
    {
    case 9:
      g_snprintf(ch,MAX_SUB_OP,"OVB");
      break;
    case 10:
      g_snprintf(ch,MAX_SUB_OP,"OVA");
      break;
    case 11:
      g_snprintf(ch,MAX_SUB_OP,"C");
      break;
    case 12:
      g_snprintf(ch,MAX_SUB_OP,"TC");
      break;
    case 512+5:
      g_snprintf(ch,MAX_SUB_OP,"CMPT");
      break;
    case 512+6:
      g_snprintf(ch,MAX_SUB_OP,"FRCT");
      break;
    case 512+7:
      g_snprintf(ch,MAX_SUB_OP,"C16");
      break;
    case 512+8:
      g_snprintf(ch,MAX_SUB_OP,"SXM");
      break;
    case 512+9:
      g_snprintf(ch,MAX_SUB_OP,"OVM");
      break;
    case 512+10:
      g_snprintf(ch,MAX_SUB_OP,"O");
      break;
    case 512+11:
      g_snprintf(ch,MAX_SUB_OP,"INTM");
      break;
    case 512+12:
      g_snprintf(ch,MAX_SUB_OP,"HM");
      break;
    case 512+13:
      g_snprintf(ch,MAX_SUB_OP,"XF");
      break;
    case 512+14:
      g_snprintf(ch,MAX_SUB_OP,"CPL");
      break;
    case 512+15:
      g_snprintf(ch,MAX_SUB_OP,"BRAF");
      break;
    default:
      g_snprintf(ch,MAX_SUB_OP,"%d",bits);
    }
  return;
}

// decodes a delay
void z_decode(gchar *ch, gchar *mask, char info,  
              struct _decoded_opcode *decode_nfo )
{
  unsigned int bits;

  bits = bit_extract(info,mask,decode_nfo,NULL);

  if ( bits == 0 )
    ch[0] = '\0';
  else
    {
      ch[0] = 'D';
      ch[1] = '\0';
    }
  
  return;
}

// decodes a round
void r_decode(gchar *ch, gchar *mask, char info,  
              struct _decoded_opcode *decode_nfo )
{
  unsigned int bits;

  bits = bit_extract(info,mask,decode_nfo,NULL);

  if ( bits == 0 )
    ch[0] = '\0';
  else
    {
      ch[0] = 'R';
      ch[1] = '\0';
    }
  
  return;
}

void sd_decode(gchar *ch, gchar *mask, char info, 
                 struct _decoded_opcode *decode_nfo )
{
  unsigned int bits;

  bits = bit_extract(info,mask,decode_nfo,NULL);

  if ( bits == 0 )
    {
      ch[0] = 'A';
      ch[1] = '\0';
    }
  else
    {
      ch[0] = 'B';
      ch[1] = '\0';
    }
  
  return;
}

// Decodes memory mapped addressing
void m_decode(gchar *ch, gchar *mask, char info, 
              struct _decoded_opcode *decode_nfo )
{
  unsigned int bits;

  bits = bit_extract(info,mask,decode_nfo,NULL);

  switch (bits)
    {
    case 0x00:
      g_snprintf(ch,MAX_SUB_OP,"IMR");
      return;
    case 0x01:
      g_snprintf(ch,MAX_SUB_OP,"IFR");
      return;
    case 0x06:
      g_snprintf(ch,MAX_SUB_OP,"ST0");
      return;
    case 0x07:
      g_snprintf(ch,MAX_SUB_OP,"ST1");
      return;
    case 0x0e:
      g_snprintf(ch,MAX_SUB_OP,"T");
      return;
    case 0x0f:
      g_snprintf(ch,MAX_SUB_OP,"TRN");
      return;
    case 0x10:
      g_snprintf(ch,MAX_SUB_OP,"AR0");
      return;
    case 0x11:
      g_snprintf(ch,MAX_SUB_OP,"AR1");
      return;
    case 0x12:
      g_snprintf(ch,MAX_SUB_OP,"AR2");
      return;
    case 0x13:
      g_snprintf(ch,MAX_SUB_OP,"AR3");
      return;
    case 0x14:
      g_snprintf(ch,MAX_SUB_OP,"AR4");
      return;
    case 0x15:
      g_snprintf(ch,MAX_SUB_OP,"AR5");
      return;
    case 0x16:
      g_snprintf(ch,MAX_SUB_OP,"AR6");
      return;
    case 0x17:
      g_snprintf(ch,MAX_SUB_OP,"AR7");
      return;
    case 0x18:
      g_snprintf(ch,MAX_SUB_OP,"SP");
      return;
    case 0x19:
      g_snprintf(ch,MAX_SUB_OP,"BK");
      return;
    case 0x1a:
      g_snprintf(ch,MAX_SUB_OP,"BRC");
      return;
    case 0x1b:
      g_snprintf(ch,MAX_SUB_OP,"RSA");
      return;
    case 0x1c:
      g_snprintf(ch,MAX_SUB_OP,"REA");
      return;
    case 0x1d:
      g_snprintf(ch,MAX_SUB_OP,"XPC");
      return;
      
    }
  // Didn't use allocated string
  return h_decode(ch,mask,info,decode_nfo);
}

void a_decode(gchar *ch, gchar *mask, char info,  
              struct _decoded_opcode *decode_nfo)
{
  unsigned int mod,bits,arf;
  gchar ind[]="*";
  int word_num;

  // See C54_Vol1_CPU_and_Peripherals.pdf -> spru131g.pdf
  // Chapter 5.5, pg 128

  bits = bit_extract(info,mask,decode_nfo,&word_num);

  mod = ( bits >> 3 ) & 0xf;
  arf = bits & 7;

  if ( !(bits & 0x80) )
    {
      // Decode as an unsigned number
      return u_decode(ch,mask,info,decode_nfo);
    }

  switch ( mod )
    {
    case 0:
      g_snprintf(ch,MAX_SUB_OP,"%sAR%d",ind,arf );
      break;
    case 1:
      g_snprintf(ch,MAX_SUB_OP,"%sAR%d-",ind,arf );
      break;
    case 2:
      g_snprintf(ch,MAX_SUB_OP,"%sAR%d+",ind,arf );
      break;
    case 3:
      g_snprintf(ch,MAX_SUB_OP,"%s+AR%d",ind,arf );
      break;
    case 4:
      g_snprintf(ch,MAX_SUB_OP,"%sAR%d-0B",ind,arf );
      break;
    case 5:
      g_snprintf(ch,MAX_SUB_OP,"%sAR%d-0",ind,arf );
      break;
    case 6:
      g_snprintf(ch,MAX_SUB_OP,"%sAR%d+0",ind,arf );
      break;
    case 7:
      g_snprintf(ch,MAX_SUB_OP,"%sAR%d+0B",ind,arf );
      break;
    case 8:
      g_snprintf(ch,MAX_SUB_OP,"%sAR%d-%%",ind,arf );
      break;
    case 9:
      g_snprintf(ch,MAX_SUB_OP,"%sAR%d-0%%",ind,arf );
      break;
    case 10:
      g_snprintf(ch,MAX_SUB_OP,"%sAR%d+%%",ind,arf );
      break;
    case 11:
      g_snprintf(ch,MAX_SUB_OP,"%sAR%d+0%%",ind,arf );
      break;
    case 12:
      g_snprintf(ch,MAX_SUB_OP,"%sAR%d(%d)",ind,arf,decode_nfo->mach_code[word_num+1] );
      break;
    case 13:
      g_snprintf(ch,MAX_SUB_OP,"%s+AR%d(%d)",ind,arf,decode_nfo->mach_code[word_num+1] );
      break;
    case 14:
      g_snprintf(ch,MAX_SUB_OP,"%s+AR%d(%d)%%",ind,arf,decode_nfo->mach_code[word_num+1] );
      break;
    case 15:
      {
        gchar ch2[MAX_SUB_OP];
        Word mach_code_sav;

        mach_code_sav = decode_nfo->mach_code[0];
        decode_nfo->mach_code[0] = decode_nfo->mach_code[word_num+1];
        m_decode(ch2,"mmmmmmmm mmmmmmmm",'m',decode_nfo);
        decode_nfo->mach_code[0] = mach_code_sav;
        
        g_snprintf(ch,MAX_SUB_OP,"%s(%s)",ind,ch2 );
        break;
      }
    }
  
  return;
}
// decodes memory mapped bits
void b_decode(gchar *ch, gchar *mask, char info, 
              struct _decoded_opcode *decode_nfo )
{
  unsigned int mod,bits;

  // If it's in the form ARx just decode that and return
  bits = bit_extract(info,mask,decode_nfo,NULL);
  mod = ( bits >> 3 ) & 0xf;

  if ( (bits & 0x80)==0 && mod==0 )
    return a_decode(ch, mask,info,decode_nfo);

  // This doesn't handle indirect. Return the hex value
  return h_decode(ch, mask,info,decode_nfo);
}

// Returns 1 if condition is true
int check_condition(Word bits)
{

  if ( bits == 0x0 )
    {
      return 1;
    }

  if ( bits & 0x40 )
    {
      // Group 1
      if ( bits & 0x7 )
	{
	  union _GP_Reg_Union reg_union;

	  // Group 1, Category A   EQ,NEQ,LT,LEQ,GT,GEQ
	  // Assume that Category A determines the Condition Register
	  reg_union.gint64 = 0; // clear all bits
	  if ( bits & 0x8 )
	    reg_union.gp_reg = MMR->B;
	  else
	    reg_union.gp_reg = MMR->A;

	  switch ( bits & 0x7 )
	    {
	    default: 
	    case 0:
	    case 1:
	      printf("Condition is undefined %s:%d\n",__FILE__,__LINE__);
	      return 0;
	    case 2:
	      if ( reg_union.gint64 < 0 )
		return 0;
	      break;
	    case 3:
	      if ( reg_union.gint64 >= 0 )
		return 0;
	      break;
	    case 4:
	      if ( reg_union.gint64 == 0 )
		return 0;
	      break;
	    case 5: 
	      if ( reg_union.gint64 != 0 )
		return 0;
	      break;
	    case 6:
	      if ( reg_union.gint64 <= 0 )
		return 0;
	      break;
	    case 7:
	      if ( reg_union.gint64 > 0 )
		return 0;
	      break;
	    }
	}
      if ( bits & 0x20 )
	{
	  // Group 1, Category B
	  if ( ((bits & 0x7) && (bits & 0x8) ) ||
	       ((bits & 0x7)==0x0 && (bits & 0x8)==0x0) )
	    {
	      // Check OVA
	      if ( (bits & 0x10) && !(MMR->ST0 & 0x400) )
		return 0;
	      else if ( !(bits & 0x10) && (MMR->ST0 & 0x400) )
		return 0;
	      // Clear OVA
	      set_OVA(MMR,0);
	    }	  
	  else
	    {
	      // Check OVB
	      if ( (bits & 0x10) && !(MMR->ST0 & 0x200) )
		return 0;
	      else if ( !(bits & 0x10) && (MMR->ST0 & 0x200) )
		return 0;
	      // Clear OVB
	      set_OVB(MMR,0);
	    }
	}

      // Passed all the conditions, must be true
      return 1;
    }
  else
    {
      // Group 2

      // Category A, TC NTC
      if ( bits & 0x2 )
	{
	  if ( bits & 0x1 )
	    FIXME(); // Return 0 if BIO not set
	  else
	    FIXME(); // Return 0 if BIO set
	}
      if ( bits & 0x8 )
	{
	  // Check C
	  if ( bits & 0x4 )
	    {
	      if ( (MMR->ST0 & 0x400) == 0 )
		return 0;
	    }
	  else
	    {
	      if ( (MMR->ST0 & 0x400) != 0 )
		return 0;
	    }
	}
      if ( bits & 0x20 )
	{
	  // Check TC
	  if ( bits & 0x10 )
	    {
	      if ( (MMR->ST0 & 0x800) == 0 )
		return 0;
	    }
	  else
	    {
	      if ( (MMR->ST0 & 0x800) != 0 )
		return 0;
	    }
	}
    }
  // Must be true
  return 1;
}


Word bit_extract(char info, char *mask, struct _decoded_opcode *decode_nfo,
                 int *word_num)
{
  int bit,smallest;
  Word ans;
  int current_location,on_word;
  int found_a;

  ans = 0;
  smallest = 0;
  on_word = -1;
  bit = BITS_PER_WORD;
  found_a = 0;
  current_location = 0;

  while ( *mask )
    {
      if ( *mask !=' ' )
	{
	  bit--;

          if ( *mask == 'a' )
            found_a = 1;

	  if ( bit < 0 )
	    {
	      if ( *mask == '\0' )
		{
		  printf("Error! bit description can't span multiple words file=%s:%d\n",
			 __FILE__,__LINE__);
		}
	      bit = BITS_PER_WORD-1;

              // skip over next word if the a decode takes up 2 words
              if ( found_a )
                current_location = current_location + 1 + decode_nfo->var_length;
              else
                current_location = current_location + 1;
	    }
	}
      if ( *mask == info )
	{
	  ans = ans | (decode_nfo->mach_code[current_location] & (1<<bit));
	  smallest = bit;
          if ( on_word < 0 )
            on_word=current_location;
	}
      mask++;
    }
  ans = ans >> smallest;

  if ( word_num )
    *word_num = on_word;

  return ans;
}

SWord signed_5bit_extract(Word mach_code)
{
  SWord bits;

  bits = mach_code & 0xf;
  if ( bits & 0x10 )
    {
      // Negative number
      bits = bits - 16;
    }
  return bits;
}

SWord signed_bit_extract(char info, char *mask, struct _decoded_opcode *decode_nfo)
{
  int bit,smallest,largest_bit,sign_bit;
  guint32 ans;
  int current_location;

  ans = 0;
  smallest = 0;
  bit = BITS_PER_WORD;
  largest_bit = -1;
  sign_bit = 0;
  current_location = 0;

  while ( *mask )
    {
      if ( *mask !=' ' )
	{
	  bit--;
	  if ( bit < 0 )
	    {
	      if ( *mask == '\0' )
		{
		  printf("Error! bit description can't span multiple words file=%s:%d\n",
			 __FILE__,__LINE__);
		}
	      bit = BITS_PER_WORD-1;
              
              current_location = current_location + 1;
	    }
	}
      if ( *mask == info )
	{
	  if ( largest_bit < 0 )
	    {
	      sign_bit=(decode_nfo->mach_code[current_location] & (1<<bit));
	      largest_bit = bit;
	    }
	  ans = ans | (decode_nfo->mach_code[current_location] & (1<<bit));
	  smallest = bit;
	}
      mask++;
    }
  ans = ans >> smallest;
  if ( sign_bit )
    {
      int k;
      for (k=largest_bit-smallest+1;k<32;k++)
	{
	  ans = ans ^ (1<<k);
	}
    }

  return (gint32)ans;
}
