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

gchar *find_operands(char *mask,gchar *info, Word *start_code);
int find_opcode(Word *start_code, Word **next_code, GPtrArray *decode_info);
gchar *process_s_operand(char *mask, char info, Word *start_code);
gchar *accumulator_decode(char *mask, char info, Word *start_code);
gchar *ptr4_decode(gchar *mask, char info, Word *start_code);
Word bit_extract(char info, char *mask, Word mach_code, WordA *location);
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


// Returns the number of words 1,2,3 that match the mask
// 0, if it doesn't match. Will read program memory address+1 
// and address+2 only if needed
int check_mask(const char *mask, Word mach_code, WordA address )
{
  int bit,wait_state,length,smem;

  g_return_val_if_fail(*mask,-1);

  bit=BITS_PER_WORD-1;
  length=1;
  smem=0;
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
              while ( (*mask != '\0') && (*mask == ' ') )
                mask++;
              if ( *mask == '\0' )
                {
                  if ( smem )
                    {
                      // may need to ignore this next word
                      if ( mach_code & 0x80 )
                        {
                          int mod;
                          mod = (mach_code & (8+16+32+64) ) >> 3;
                          if ( mod >= 12 )
                            {
                              length++;
                            }
                        }
                    } 
                  return length; // found a match
                }
              // must read another word
              if ( smem )
                {
                  // may need to ignore this next word
                  if ( mach_code & 0x80 )
                    {
                      int mod;
                      mod = (mach_code & (8+16+32+64) ) >> 3;
                      if ( mod >= 12 )
                        {
                          address++;
                          mach_code = read_program_mem(address,&wait_state);
                          length++;
                        }
                    }
                  smem=0;
                }
              address++;
              mach_code = read_program_mem(address,&wait_state);
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

  return length;

}

gchar *vw_decode(gchar *mask, char info, Word start_code, WordA *location)
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

gchar *c_decode(gchar *mask, char info, Word start_code, WordA *location)
{
  unsigned int bits;
  gchar *ch;

  bits = bit_extract(info,mask,start_code,location);
  
  ch=g_new(gchar,13);
  *ch='\0';

  if ( bits == 0x0 )
    {
      g_strcat(ch,"UNC");
      return ch;
    }

  if ( bits & 0x40 )
    {
      // Group 1
      if ( bits & 0x7 )
	{
	  // Group 1, Category A   EQ,NEQ,LT,LEQ,GT,GEQ
	  // Assume that Category A determines the Condition Register
	  if ( bits & 0x8 )
	    g_strcat(ch,"B");
	  else
	    g_strcat(ch,"A");

	  switch ( bits & 0x7 )
	    {
	    default: 
	    case 0:
	    case 1:
	      g_strcat(ch,"????");
	      break;
	    case 2:
	      g_strcat(ch,"GEQ");
	      break;
	    case 3:
	      g_strcat(ch,"LT");
	      break;
	    case 4:
	      g_strcat(ch,"NEQ");
	      break;
	    case 5: 
	      g_strcat(ch,"EQ");
	      break;
	    case 6:
	      g_strcat(ch,"GT");
	      break;
	    case 7:
	      g_strcat(ch,"LEQ");
	      break;
	    }
	}
      if ( bits & 0x20 )
	{
	  if ( bits & 0x7 )
	    // 2 conditions
	    g_strcat(ch,",");

	  // Group 1, Category B
	  if ( ((bits & 0x7) && (bits & 0x8) ) ||
	       ((bits & 0x7)==0x0 && (bits & 0x8)==0x0) )
	    g_strcat(ch,"A");
	  else
	    g_strcat(ch,"B");

	  if ( bits & 0x10 )
	    g_strcat(ch,"OV");
	  else
	    g_strcat(ch,"NOV");
	}
    }
  else
    {
      // Group 2

      // Category A, TC NTC
      if ( bits & 0x2 )
	{
	  if ( bits & 0x1 )
	    g_strcat(ch,"BIO");
	  else
	    g_strcat(ch,"NBIO");
	  if ( bits & 0x28 )
	    // Another condition follows
	    g_strcat(ch,",");
	}
      if ( bits & 0x8 )
	{
	  if ( bits & 0x4 )
	    g_strcat(ch,"C");
	  else
	    g_strcat(ch,"NC");
	  if ( bits & 0x20 )
	    // Another condition follows
	    g_strcat(ch,",");
	}
      if ( bits & 0x20 )
	{
	  if ( bits & 0x10 )
	    g_strcat(ch,"TC");
	  else
	    g_strcat(ch,"NTC");
	}
    }
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

gchar *p_decode(gchar *mask, char info, Word start_code, WordA *location)
{
  unsigned int bits;
  gchar *ch;

  bits = bit_extract(info,mask,start_code,location);

  ch = g_new(gchar,8);

  g_snprintf(ch,8,"%d",bits+1);

  return ch;
}


gchar *h_decode(gchar *mask, char info, Word start_code, WordA *location)
{
  unsigned int bits;
  gchar *ch;

  bits = bit_extract(info,mask,start_code,location);

  ch = g_new(gchar,8);

  g_snprintf(ch,8,"0x%x",bits);

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

gchar *t_decode(gchar *mask, char info, Word start_code, WordA *location)
{
  gchar *ch;
  int bits;

  bits = bit_extract(info,mask,start_code,location);

  switch ( bits )
    {
    case 9:
      ch = g_strdup("OVB");
      break;
    case 10:
      ch = g_strdup("OVA");
      break;
    case 11:
      ch = g_strdup("C");
      break;
    case 12:
      ch = g_strdup("TC");
      break;
    case 512+5:
      ch = g_strdup("CMPT");
      break;
    case 512+6:
      ch = g_strdup("FRCT");
      break;
    case 512+7:
      ch = g_strdup("C16");
      break;
    case 512+8:
      ch = g_strdup("SXM");
      break;
    case 512+9:
      ch = g_strdup("OVM");
      break;
    case 512+10:
      ch = g_strdup("O");
      break;
    case 512+11:
      ch = g_strdup("INTM");
      break;
    case 512+12:
      ch = g_strdup("HM");
      break;
    case 512+13:
      ch = g_strdup("XF");
      break;
    case 512+14:
      ch = g_strdup("CPL");
      break;
    case 512+15:
      ch = g_strdup("BRAF");
      break;
    default:
      ch = g_new(gchar,8);
      g_snprintf(ch,8,"%d",bits);
    }
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

// Decodes memory mapped addressing
gchar *m_decode(gchar *mask, char info, Word start_code, WordA *location)
{
  unsigned int bits;
  gchar *ch;
  ch = g_new(gchar,4);

  bits = bit_extract(info,mask,start_code,location);

  switch (bits)
    {
    case 0x00:
      g_snprintf(ch,4,"IMR");
      return ch;
    case 0x01:
      g_snprintf(ch,4,"IFR");
      return ch;
    case 0x06:
      g_snprintf(ch,4,"ST0");
      return ch;
    case 0x07:
      g_snprintf(ch,4,"ST1");
      return ch;
    case 0x0e:
      g_snprintf(ch,4,"T");
      return ch;
    case 0x0f:
      g_snprintf(ch,4,"TRN");
      return ch;
    case 0x10:
      g_snprintf(ch,4,"AR0");
      return ch;
    case 0x11:
      g_snprintf(ch,4,"AR1");
      return ch;
    case 0x12:
      g_snprintf(ch,4,"AR2");
      return ch;
    case 0x13:
      g_snprintf(ch,4,"AR3");
      return ch;
    case 0x14:
      g_snprintf(ch,4,"AR4");
      return ch;
    case 0x15:
      g_snprintf(ch,4,"AR5");
      return ch;
    case 0x16:
      g_snprintf(ch,4,"AR6");
      return ch;
    case 0x17:
      g_snprintf(ch,4,"AR7");
      return ch;
    case 0x18:
      g_snprintf(ch,4,"SP");
      return ch;
    case 0x19:
      g_snprintf(ch,4,"BK");
      return ch;
    case 0x1a:
      g_snprintf(ch,4,"BRC");
      return ch;
    case 0x1b:
      g_snprintf(ch,4,"RSA");
      return ch;
    case 0x1c:
      g_snprintf(ch,4,"REA");
      return ch;
    case 0x1d:
      g_snprintf(ch,4,"XPC");
      return ch;
      
    }
  // Didn't use allocated string
  g_free(ch);
  return h_decode(mask,info,start_code,location);
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
#if 0
      // Decode direct addressing as ARx
      *ind='\0';
#else
      // Decode as an unsigned number
      return u_decode(mask,info,start_code,location);
#endif
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
      g_snprintf(ch,20,"%s(%s)",ind,m_decode("hhhhhhhh hhhhhhhh",'h',read_program_mem(next_loc,&wait_state),&next_loc) );
      break;
    }
  
  return ch;
}
gchar *b_decode(gchar *mask, char info, Word start_code, WordA *location)
{
  unsigned int mod,bits;

  // If it's in the form ARx just decode that and return
  bits = bit_extract(info,mask,start_code,location);
  mod = ( bits >> 3 ) & 0xf;

  if ( (bits & 0x80)==0 && mod==0 )
    return a_decode(mask,info,start_code,location);
  // This doesn't handle indirect. Return the unsigned integer
  return u_decode(mask,info,start_code,location);
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
	    }	  
	  else
	    {
	      // Check OVB
	      if ( (bits & 0x10) && !(MMR->ST0 & 0x200) )
		return 0;
	      else if ( !(bits & 0x10) && (MMR->ST0 & 0x200) )
		return 0;
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


Word bit_extract(char info, char *mask, Word mach_code, WordA *location)
{
  int bit,smallest;
  Word ans;
  int wait_state;
  WordA current_location;

  ans = 0;
  smallest = 0;
  bit = BITS_PER_WORD;
  if ( location )
    current_location = *location;
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
	      if ( location )
		current_location = current_location + 1;
	      else
		{
		  printf("Error! %s:%d\n",__FILE__,__LINE__);
		  return 0;
		}
	      mach_code = read_program_mem(current_location,&wait_state);
	      
	    }
	}
      if ( *mask == info )
	{
	  ans = ans | (mach_code & (1<<bit));
	  smallest = bit;
	  // Only want to increment location if actually used bits
	  // from that location, otherwise location might get incremented
	  // too often
	  if ( location )
	    *location = current_location;
	}
      mask++;
    }
  ans = ans >> smallest;

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
	      *location = *location + 1;
	      mach_code = read_program_mem(*location,&wait_state);
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
