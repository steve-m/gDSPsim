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
//#include "opcode_def.h"
#include "core_def.h"
#include "string.h"
#include <stdlib.h>
#include "string_func.h"
#include "decode.h"
#include "hardware.h"
#include "memory.h"

gchar *find_operands(char *mask,gchar *info, Word *start_code);
int find_opcode(Word *start_code, Word **next_code, GPtrArray *decode_info);
gchar *process_s_operand(char *mask, char info, Word *start_code);
gchar *accumulator_decode(char *mask, char info, Word *start_code);
gchar *ptr4_decode(gchar *mask, char info, Word *start_code);
Word bit_extract(char info, char *mask, Word mach_code, WordA *location);
int get_bits(gchar *info, int *word);

Word dm[55];

/* Reads the mask to set the operands */
void default_pipe_read(struct _PipeLine *pipeP, struct _Registers *Reg)
{
  int SubType;
  Operand_List *operands;
  gchar *mask;

  SubType = pipeP->opcode_subType;
  operands = &pipeP->operands;
  mask = pipeP->opcode_object->mask[SubType];

  while ( *mask )
    {
      if ( *mask=='1' || *mask=='0' )
	{
	  mask++;
	}
      else if ( *mask != ' ' )
	{
	  
	  // OK, we have an operand marker
	  
	  mask++;
	}
    }
}
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


// Returns true if the mach_code matches the mask
int check_mask(const char *mask, Word mach_code )
{
  int bit;

  g_return_val_if_fail(*mask,-1);


  bit=BITS_PER_WORD-1;
  while ( (*mask != '\0') && (bit >= 0))
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
      if ( *mask != ' ' )
	{
	  bit--;
	}
      mask++;

    }

  if ( bit != -1 )
    printf("Error, funny bit length for mask=%s file=%s:%d\n",mask,__FILE__,__LINE__);

  return 1;

  // printf("len=%d  bytes=\n",len,bytes);
}

gchar *mw_decode(gchar *mask, char info, Word start_code, WordA *location)
{
  unsigned int bits;
  gchar *ch;

  bits = bit_extract(info,mask,start_code, location);
  ch = g_new(gchar,4);

  if ( bits > 7 )
    g_snprintf(ch,4,"SP");
  else
    g_snprintf(ch,4,"AR%d",bits);
  return ch;
}

gchar *xy_decode(gchar *mask, char info, Word start_code, WordA *location)
{
  // See C54_Vol1_CPU_and_Peripherals.pdf -> spru131g.pdf
  // pg 135 5-20
  unsigned int bits,mod,ar;
  gchar *ch;

  bits = bit_extract(info,mask,start_code,location);
  ar = (bits & 3)+2;
  mod = (bits & 12);
  ch = g_new(gchar,8);

  if ( mod == 4)
    g_snprintf(ch,8,"*AR%d-",ar);
  else if ( mod == 8 )
    g_snprintf(ch,8,"*AR%d+",ar);
  else if ( mod == 0 )
    g_snprintf(ch,8,"*AR%d",ar);
  else
    g_snprintf(ch,8,"*AR%d+0%%",ar);

  return ch;
}

gchar *u_decode(gchar *mask, char info, Word start_code, WordA *location)
{
  unsigned int bits;
  gchar *ch;

  bits = bit_extract(info,mask,start_code,location);

  ch = g_new(gchar,8);
  
  g_snprintf(ch,8,"%d",bits);

  return ch;
}

gchar *n_decode(gchar *mask, char info, Word start_code, WordA *location)
{
  gchar *ch;
  int bits;

  bits = signed_bit_extract(info,mask,start_code,location);

  ch = g_new(gchar,8);
  
  g_snprintf(ch,8,"%d",bits);

  return ch;
}

gchar *z_decode(gchar *mask, char info, Word start_code, WordA *location)
{
  unsigned int bits;
  gchar *ch;

  bits = bit_extract(info,mask,start_code,location);

  if ( bits == 0 )
    ch = g_strdup("");
  else
    ch = g_strdup("D");
  
  return ch;
}

gchar *r_decode(gchar *mask, char info, Word start_code, WordA *location)
{
  unsigned int bits;
  gchar *ch;

  bits = bit_extract(info,mask,start_code,location);

  if ( bits == 0 )
    ch = g_strdup("");
  else
    ch = g_strdup("R");
  
  return ch;
}

gchar *sd_decode(gchar *mask, char info, Word start_code, WordA *location)
{
  unsigned int bits;
  gchar *ch;

  bits = bit_extract(info,mask,start_code,location);

  if ( bits == 0 )
    ch = g_strdup("A");
  else
    ch = g_strdup("B");
  
  return ch;
}

gchar *a_decode(gchar *mask, char info, Word start_code, WordA *location)
{
  unsigned int mod,bits,arf;
  gchar *ch;
  gchar ind[]="*";
  int wait_state;
  WordA next_loc;

  // See C54_Vol1_CPU_and_Peripherals.pdf -> spru131g.pdf
  // Chapter 5.5, pg 128

  ch = g_new(gchar,20);

  bits = bit_extract(info,mask,start_code,location);

  mod = ( bits >> 3 ) & 0xf;
  arf = bits & 7;

  if ( !(bits & 0x80) )
    {
      *ind='\0';
    }

  switch ( mod )
    {
    case 0:
      g_snprintf(ch,20,"%sAR%d",ind,arf );
      break;
    case 1:
      g_snprintf(ch,20,"%sAR%d-",ind,arf );
      break;
    case 2:
      g_snprintf(ch,20,"%sAR%d+",ind,arf );
      break;
    case 3:
      g_snprintf(ch,20,"%s+AR%d",ind,arf );
      break;
    case 4:
      g_snprintf(ch,20,"%sAR%d-0B",ind,arf );
      break;
    case 5:
      g_snprintf(ch,20,"%sAR%d-0",ind,arf );
      break;
    case 6:
      g_snprintf(ch,20,"%sAR%d+0",ind,arf );
      break;
    case 7:
      g_snprintf(ch,20,"%sAR%d+0B",ind,arf );
      break;
    case 8:
      g_snprintf(ch,20,"%sAR%d-%%",ind,arf );
      break;
    case 9:
      g_snprintf(ch,20,"%sAR%d-0%%",ind,arf );
      break;
    case 10:
      g_snprintf(ch,20,"%sAR%d+%%",ind,arf );
      break;
    case 11:
      g_snprintf(ch,20,"%sAR%d+0%%",ind,arf );
      break;
    case 12:
      next_loc = *location;
      next_loc++;
      *location = next_loc;
      g_snprintf(ch,20,"%sAR%d(%d)",ind,arf,signed_bit_extract('n',"nnnnnnnn nnnnnnnn",read_program_mem(next_loc,&wait_state),location) );
      break;
    case 13:
      next_loc = *location;
      next_loc++;
      *location = next_loc;
      g_snprintf(ch,20,"%s+AR%d(%d)",ind,arf,signed_bit_extract('n',"nnnnnnnn nnnnnnnn", read_program_mem(next_loc,&wait_state),location) );
      break;
    case 14:
      next_loc = *location;
      next_loc++;
      *location = next_loc;
      g_snprintf(ch,20,"%s+AR%d(%d)%%",ind,arf,signed_bit_extract('n',"nnnnnnnn nnnnnnnn",read_program_mem(next_loc,&wait_state),location ) );
      //      g_snprintf(ch,20,"%s+AR%d(0x%x)%%",ind,arf,*(start_code +1) );
      break;
    case 15:
      next_loc = *location;
      next_loc++;
      *location = next_loc;
      g_snprintf(ch,20,"%s(0x%x)",ind,read_program_mem(next_loc,&wait_state) );
      break;
    }
  
  return ch;
}
Word bit_extract(char info, char *mask, Word mach_code, WordA *location)
{
  int bit,smallest;
  Word ans;
  int wait_state;

  ans = 0;
  smallest = 0;
  bit = BITS_PER_WORD;
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
	      mach_code = read_program_mem(*++location,&wait_state);
	    }
	}
      if ( *mask == info )
	{
	  ans = ans | (mach_code & (1<<bit));
	  smallest = bit;
	}
      mask++;
    }
  ans = ans >> smallest;
  return ans;
}
SWord signed_bit_extract(char info, char *mask, Word mach_code, WordA *location)
{
  int bit,smallest,largest_bit,sign_bit;
  guint32 ans;
  int wait_state;

  ans = 0;
  smallest = 0;
  bit = BITS_PER_WORD;
  largest_bit = -1;
  sign_bit = 0;

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
	      mach_code = read_program_mem(*++location,&wait_state);
	    }
	}
      if ( *mask == info )
	{
	  if ( largest_bit < 0 )
	    {
	      sign_bit=(mach_code & (1<<bit));
	      largest_bit = bit;
	    }
	  ans = ans | (mach_code & (1<<bit));
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
