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

#include "chip_core.h"
#include "memory.h"
#include "decode.h"

// returns 1, if this mask matches the mach_code and sets the
// length and other mach_codes if needed in decode_nfo. returns
// 0, if mask doesn't match and doesnt set anything in decode_nfo.
int check_mask(const char *mask, struct _decoded_opcode *decode_nfo )
{
  int bit,length,smem;
  unsigned char mach_code;
  WordA address;

  g_return_val_if_fail(*mask,-1);

  bit=BITS_PER_PROGRAM_ACCESS-1;
  length=1;
  smem=0;
  mach_code = decode_nfo->mach_code.bop[0];
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
      else if ( *mask=='p' )
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
#if 0
                      int mod;
                      mod = (mach_code & (8+16+32+64) ) >> 3;
                      if ( mod >= 12 )
                        {
                          mach_code = 
                            read_program_mem(address+length,&wait_state);
                          decode_nfo->mach_code.bop[length] = mach_code;
                          length++;
                          decode_nfo->var_length = 1;
                        }
#endif
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
              mach_code = decode_nfo->mach_code.bop[length];
              length++;

	      bit=BITS_PER_PROGRAM_ACCESS-1;
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



// extract unsigned decimal number
void u_decode(gchar *ch, gchar *mask, char info, 
              struct _decoded_opcode *decode_nfo)
{
  unsigned int bits;
  int num_mask;
  int length;

  bits = bit_extract(info,mask,decode_nfo,NULL,&num_mask,&length);
  
  g_snprintf(ch,MAX_SUB_OP,"%d",bits);

  return;
}



// extract unsigned hexedecimal number
void h_decode(gchar *ch, gchar *mask, char info, 
              struct _decoded_opcode *decode_nfo)
{
  unsigned int bits;
  int num_mask;
  int length;

  bits = bit_extract(info,mask,decode_nfo,NULL,&num_mask,&length);
  
  g_snprintf(ch,MAX_SUB_OP,"%xh",bits);

  return;
}

// extract signed decimal number where the sign bit
// is assumed
void m_decode(gchar *ch, gchar *mask, char info, 
              struct _decoded_opcode *decode_nfo)
{
  Word bits;
  int word_num,num_mask;

  bits = xor_bit_extract(info,mask,decode_nfo,&word_num,&num_mask);
  bits = bits + 1;

  g_snprintf(ch,MAX_SUB_OP,"-%d",bits);

  return;
}

// extract signed decimal number
void n_decode(gchar *ch, gchar *mask, char info, 
              struct _decoded_opcode *decode_nfo)
{
  SWord bits;

  bits = signed_bit_extract(info,mask,decode_nfo);

  g_snprintf(ch,MAX_SUB_OP,"%d",bits);

  return;
}

// extract unsigned decimal number
// q_decode uses this for 2nd register
void rR_decode(gchar *ch, gchar *mask, char info, 
              struct _decoded_opcode *decode_nfo)
{
  unsigned int bits;
  int num_mask;
  int length;

  bits = bit_extract(info,mask,decode_nfo,NULL,&num_mask,&length);

  switch ( bits )
    {
    case 0:
      g_snprintf(ch,MAX_SUB_OP,"AC0" );
      break;
    case 1:
      g_snprintf(ch,MAX_SUB_OP,"AC1" );
      break;
    case 2:
      g_snprintf(ch,MAX_SUB_OP,"AC2" );
      break;
    case 3:
      g_snprintf(ch,MAX_SUB_OP,"AC3" );
      break;
    case 4:
      g_snprintf(ch,MAX_SUB_OP,"T0" );
      break;
    case 5:
      g_snprintf(ch,MAX_SUB_OP,"T1" );
      break;
    case 6:
      g_snprintf(ch,MAX_SUB_OP,"T2" );
      break;
    case 7:
      g_snprintf(ch,MAX_SUB_OP,"T3" );
      break;
    case 8:
      g_snprintf(ch,MAX_SUB_OP,"AR0" );
      break;
    case 9:
      g_snprintf(ch,MAX_SUB_OP,"AR1" );
      break;
    case 10:
      g_snprintf(ch,MAX_SUB_OP,"AR2" );
      break;
    case 11:
      g_snprintf(ch,MAX_SUB_OP,"AR3" );
      break;
    case 12:
      g_snprintf(ch,MAX_SUB_OP,"AR4" );
      break;
    case 13:
      g_snprintf(ch,MAX_SUB_OP,"AR5" );
      break;
    case 14:
      g_snprintf(ch,MAX_SUB_OP,"AR6" );
      break;
    case 15:
      g_snprintf(ch,MAX_SUB_OP,"AR7" );
      break;


    default:
      g_snprintf(ch,MAX_SUB_OP,"?" );
      g_warning("r decode has too many characters");
      break;
    }

  return;
}

// Used to decode Tx registers
void t_decode(gchar *ch, gchar *mask, char info, 
              struct _decoded_opcode *decode_nfo)
{
  unsigned int bits;
  int num_mask;
  int length;

  bits = bit_extract(info,mask,decode_nfo,NULL,&num_mask,&length);

  g_snprintf(ch,MAX_SUB_OP,"T%d",bits);

  return;
}

// TC0,TC1 flag 1 bit
void C_decode(gchar *ch, gchar *mask, char info, 
              struct _decoded_opcode *decode_nfo)
{
  unsigned int bits;
  int num_mask;
  int length;

  bits = bit_extract(info,mask,decode_nfo,NULL,&num_mask,&length);

  g_snprintf(ch,MAX_SUB_OP,"TC%d",bits+1);

  return;
}


// decodes Smem, usually pppp ppp1
// automatically adds 16 bits of unsigned if
// (pppp ppp1 == ppp0 1101) or (pppp ppp1 == ppp0 1111)
// autmatically looks for 1001 1101 following if
// (pppp ppp1 == ppp0 0011) or (pppp ppp1 == ppp0 0101)
void s_decode(gchar *ch, gchar *mask, char info, 
              struct _decoded_opcode *decode_nfo )
{
  unsigned int mod,bits,arf;
  int word_num;
  int num_mask;
  unsigned char tag;
  int length;
  int extend; // opcode is extended

  // See C54_Vol1_CPU_and_Peripherals.pdf -> spru131g.pdf
  // Chapter 5.5, pg 128

  bits = bit_extract(info,mask,decode_nfo,&word_num,&num_mask,&length);
  
  if ( num_mask == 3 )
    {
      g_snprintf(ch,MAX_SUB_OP,"AR%d",bits );
      return;
    }

  if ( (bits & 1)==0 )
    {
      // SP offset
      g_snprintf(ch,MAX_SUB_OP,"*SP(#%d)",(bits>>1)&0x7f );
      return;
    }

  arf = bits>>5;
  mod = (bits>>1) & 0xf;
  
  tag = decode_nfo->mach_code.bop[length+1];
  
  if ( tag == 0x9d )
    {
      extend = 1; // Circular addressing
    }

  switch ( mod )
    {
    case 0:
      g_snprintf(ch,MAX_SUB_OP,"*AR%d",arf );
      break;
    case 1:
      if ( extend == 1 )
	{
	  g_snprintf(ch,MAX_SUB_OP,"*AR%d+%%",arf );
	  decode_nfo->length += 1;
	}
      else
	{
	  g_snprintf(ch,MAX_SUB_OP,"*AR%d+",arf );
	}
      break;
    case 2:
      if ( extend == 1 )
	{
	  g_snprintf(ch,MAX_SUB_OP,"*AR%d-%%",arf );
	  decode_nfo->length += 1;
	}
      else
	{
	  g_snprintf(ch,MAX_SUB_OP,"*AR%d-",arf );
	}
      break;
    case 3:
      g_snprintf(ch,MAX_SUB_OP,"*(AR%d+T0)",arf );
      break;
    case 4:
      g_snprintf(ch,MAX_SUB_OP,"*(AR%d-T0)",arf );
      break;
    case 5:
      g_snprintf(ch,MAX_SUB_OP,"*AR%d(T0)",arf );
      break;
    case 6:
      g_snprintf(ch,MAX_SUB_OP,"*AR%d(#%2.2x%2.2xh)",arf,
		 decode_nfo->mach_code.bop[word_num+1],
		 decode_nfo->mach_code.bop[word_num+2] );
      decode_nfo->length += 2;
      break;
    case 7:
      if ( ARLC(MMR,arf) )
	{
	  g_snprintf(ch,MAX_SUB_OP,"*+AR%d(#%2.2x%2.2xh)%%",arf,
		     decode_nfo->mach_code.bop[word_num+1],
		     decode_nfo->mach_code.bop[word_num+2] );
	}
      else
	{
	  g_snprintf(ch,MAX_SUB_OP,"*+AR%d(#%2.2x%2.2xh)",arf,
		     decode_nfo->mach_code.bop[word_num+1],
		     decode_nfo->mach_code.bop[word_num+2] );
	}
      decode_nfo->length += 2;
      break;
    case 8:
      switch (arf)
	{
	case 0:
	  g_snprintf(ch,MAX_SUB_OP,"ABS16(#%2.2x%2.2xh)",
		     decode_nfo->mach_code.bop[length+1],
		     decode_nfo->mach_code.bop[length+2] );
	  decode_nfo->length += 2;
	  break;
	case 1:
	  g_snprintf(ch,MAX_SUB_OP,"*(#%2.2x%2.2x%2.2xh)",
		     decode_nfo->mach_code.bop[length+1],
		     decode_nfo->mach_code.bop[length+2],
		     decode_nfo->mach_code.bop[length+3] );
	  decode_nfo->length += 3;
	case 2:
	  g_snprintf(ch,MAX_SUB_OP,"port(#%2.2x%2.2xh)",
		     decode_nfo->mach_code.bop[length+1],
		     decode_nfo->mach_code.bop[length+2] );
	  decode_nfo->length += 2;
	  break;
	case 3:
	  g_snprintf(ch,MAX_SUB_OP,"*CDP");
	  break;
	case 4:
	  g_snprintf(ch,MAX_SUB_OP,"*CDP+");
	  break;
	case 5:
	  g_snprintf(ch,MAX_SUB_OP,"*CDP-");
	  break;
	case 6:
	  g_snprintf(ch,MAX_SUB_OP,"*CDP(#%2.2x%2.2xh)",
		     decode_nfo->mach_code.bop[length+1],
		     decode_nfo->mach_code.bop[length+2] );
	  decode_nfo->length += 2;
	  break;
	case 7:
	  g_snprintf(ch,MAX_SUB_OP,"*+CDP(#%2.2x%2.2xh)",
		     decode_nfo->mach_code.bop[length+1],
		     decode_nfo->mach_code.bop[length+2] );
	  decode_nfo->length += 2;
	  break;
	}
      break;
    case 9:
      if ( ARMS(MMR) )
	g_snprintf(ch,MAX_SUB_OP,"*AR%d(short(#1))",arf );
      else
	g_snprintf(ch,MAX_SUB_OP,"*(AR%d+T1)",arf );
      break;
    case 10:
      if ( ARMS(MMR) )
	g_snprintf(ch,MAX_SUB_OP,"*AR%d(short(#2))",arf );
      else
	g_snprintf(ch,MAX_SUB_OP,"*(AR%d-T1)",arf );
      break;
    case 11:
      if ( ARMS(MMR) )
	g_snprintf(ch,MAX_SUB_OP,"*AR%d(short(#3))",arf );
      else
	g_snprintf(ch,MAX_SUB_OP,"*AR%d(T1)",arf );
      break;
    case 12:
      if ( ARMS(MMR) )
	g_snprintf(ch,MAX_SUB_OP,"*AR%d(short(#4))",arf );
      else
	g_snprintf(ch,MAX_SUB_OP,"*+AR%d",arf );
      break;
    case 13:
      if ( ARMS(MMR) )
	g_snprintf(ch,MAX_SUB_OP,"*AR%d(short(#5))",arf );
      else
	g_snprintf(ch,MAX_SUB_OP,"*-AR%d",arf );
      break;
    case 14:
      if ( ARMS(MMR) )
	g_snprintf(ch,MAX_SUB_OP,"*AR%d(short(#6))",arf );
      else
	g_snprintf(ch,MAX_SUB_OP,"*(AR%d+T0B)",arf );
      break;
    case 15:
      if ( ARMS(MMR) )
	g_snprintf(ch,MAX_SUB_OP,"*AR%d(short(#7))",arf );
      else
	g_snprintf(ch,MAX_SUB_OP,"*(AR%d-T0B)",arf );
      break;
    }
}
// Used to decode TRNx registers
void T_decode(gchar *ch, gchar *mask, char info, 
              struct _decoded_opcode *decode_nfo)
{
  gboolean bit;

  bit = one_bit_extract(info,mask,decode_nfo);

  if ( bit )
    g_snprintf(ch,MAX_SUB_OP,"TRN1");
  else
    g_snprintf(ch,MAX_SUB_OP,"TRN0");

  return;
}

  // f Rounding (f is for floor, and letters were running out)
void f_decode(gchar *ch, gchar *mask, char info, 
              struct _decoded_opcode *decode_nfo)
{
  gboolean bit;

  bit = one_bit_extract(info,mask,decode_nfo);

  if ( bit )
    g_snprintf(ch,MAX_SUB_OP,"R");

  return;
}

// F Rounding (F is for floor, and letters were running out)
// this round is rnd()
void F_decode(gchar *ch, gchar *mask, char info, 
              struct _decoded_opcode *decode_nfo)
{
  gboolean bit;

  bit = one_bit_extract(info,mask,decode_nfo);

  if ( bit )
    g_snprintf(ch,MAX_SUB_OP,"rnd(");

  return;
}

// G close up F Rounding (F is for floor, and letters were running out)
void G_decode(gchar *ch, gchar *mask, char info, 
              struct _decoded_opcode *decode_nfo)
{
  gboolean bit;

  bit = one_bit_extract('F',mask,decode_nfo);

  if ( bit )
    g_snprintf(ch,MAX_SUB_OP,")");

  return;
}

  // 3 applies T3=
void t3_decode(gchar *ch, gchar *mask, char info, 
              struct _decoded_opcode *decode_nfo)
{
  gboolean bit;

  bit = one_bit_extract(info,mask,decode_nfo);

  if ( bit )
    g_snprintf(ch,MAX_SUB_OP,"T3=");

  return;
}

// 1=TC2 0=CARRY (used for rol,ror)
void zZ_decode(gchar *ch, gchar *mask, char info, 
              struct _decoded_opcode *decode_nfo)
{
  gboolean bit;

  bit = one_bit_extract(info,mask,decode_nfo);

  if ( bit )
    g_snprintf(ch,MAX_SUB_OP,"TC2");
  else
    g_snprintf(ch,MAX_SUB_OP,"CARRY");

  return;
}

// A unsigned flag, think Abs
void A_decode(gchar *ch, gchar *mask, char info, 
              struct _decoded_opcode *decode_nfo)
{
  gboolean bit;

  bit = one_bit_extract(info,mask,decode_nfo);

  if ( bit )
    g_snprintf(ch,MAX_SUB_OP,"U");

  return;
}
  // U unsigned wrapper start uns(
void U_decode(gchar *ch, gchar *mask, char info, 
              struct _decoded_opcode *decode_nfo)
{
  gboolean bit;

  bit = one_bit_extract(info,mask,decode_nfo);

  if ( bit )
    g_snprintf(ch,MAX_SUB_OP,"uns(");

  return;
}
  // V unsigned wrapper end )
void V_decode(gchar *ch, gchar *mask, char info, 
              struct _decoded_opcode *decode_nfo)
{
  gboolean bit;

  bit = one_bit_extract('U',mask,decode_nfo);

  if ( bit )
    g_snprintf(ch,MAX_SUB_OP,")");

  return;
}

// v reserved
void v_decode(gchar *ch, gchar *mask, char info, 
              struct _decoded_opcode *decode_nfo)
{
}

// p parallel
void p_decode(gchar *ch, gchar *mask, char info, 
              struct _decoded_opcode *decode_nfo)
{
  FIXME();
}

  // c Cmem addressing
void c_decode(gchar *ch, gchar *mask, char info, 
	      struct _decoded_opcode *decode_nfo )
{
  unsigned int bits;
  int num_mask;
  int length;

  bits = bit_extract(info,mask,decode_nfo,NULL,&num_mask,&length);

  switch (bits)
    {
    case 0:
      g_snprintf(ch,MAX_SUB_OP,"*CDP");
      return;
    case 1:
      g_snprintf(ch,MAX_SUB_OP,"*CDP+");
      return;
    case 2:
      g_snprintf(ch,MAX_SUB_OP,"*CDP-");
      return;
    case 3:
      g_snprintf(ch,MAX_SUB_OP,"*(CDP+T0)");
      return;
    default:
      printf("Error in mask %s\n",mask);
    }

  return;
}

void xy_decode(gchar *ch, gchar *mask, char info, 
	       struct _decoded_opcode *decode_nfo )
{
  unsigned int bits;
  int num_mask;
  int length;
  int mod,arf,c54cm;

  bits = bit_extract(info,mask,decode_nfo,NULL,&num_mask,&length);

  mod = bits & 7;
  arf = bits >> 3;
  c54cm = C54CM(MMR);

  switch (mod)
    {
    case 0:
      g_snprintf(ch,MAX_SUB_OP,"*AR%d",arf );
      break;
    case 1:
      g_snprintf(ch,MAX_SUB_OP,"*AR%d+",arf );
      break;
    case 2:
      g_snprintf(ch,MAX_SUB_OP,"*AR%d-",arf );
      break;
    case 3:
      if ( c54cm )
	g_snprintf(ch,MAX_SUB_OP,"*(AR%d+AR0)",arf );
      else
	g_snprintf(ch,MAX_SUB_OP,"*(AR%d+T0)",arf );
      break;
    case 4:
      g_snprintf(ch,MAX_SUB_OP,"*(AR%d+T1)",arf );
      break;
    case 5:
      if ( c54cm )
	g_snprintf(ch,MAX_SUB_OP,"*(AR%d-AR0)",arf );
      else
	g_snprintf(ch,MAX_SUB_OP,"*(AR%d-T0)",arf );
      break;
    case 6:
      g_snprintf(ch,MAX_SUB_OP,"*(AR%d-T1)",arf );
      break;
    case 7:
      if ( c54cm )
	g_snprintf(ch,MAX_SUB_OP,"*AR%d(AR0)",arf );
      else
	g_snprintf(ch,MAX_SUB_OP,"*AR%d(T0)",arf );
      break;
    }
}

gboolean one_bit_extract(char info, char *mask, 
			 struct _decoded_opcode *decode_nfo)
{
  int num_mask,length;
  Word bits;

  bits = bit_extract(info, mask, decode_nfo, NULL, &num_mask, &length);

  if ( bits )
    return TRUE;
  else
    return FALSE;
}

  
Word bit_extract(char info, char *mask, struct _decoded_opcode *decode_nfo,
                 int *word_num, int *num_mask, int *length)
{
  int bit,smallest;
  Word ans;
  int current_location,on_word;
//  int found_a;

  ans = 0;
  smallest = 0;
  on_word = -1;
  bit = BITS_PER_PROGRAM_ACCESS;
//  found_a = 0;
  current_location = 0;
  *num_mask = 0;

  while ( *mask )
    {
      if ( *mask !=' ' )
	{
	 bit--;

	 //          if ( *mask == 'a' )
	 //            found_a = 1;

	  if ( bit < 0 )
	    {
	      if ( *mask == '\0' )
		{
		  printf("Error! bit description can't span multiple words file=%s:%d\n",
			 __FILE__,__LINE__);
		}
	      bit = BITS_PER_PROGRAM_ACCESS-1;

              // skip over next word if the a decode takes up 2 words
//              if ( found_a )
//                current_location = current_location + 1 + decode_nfo->var_length;
//              else
                current_location = current_location + 1;
	    }
	}
      if ( *mask == info )
	{
	  ans = ans << 1;
          if ( (decode_nfo->mach_code.bop[current_location] & (1<<bit)) )
	    ans = ans | 1;
	  smallest = bit;

	  //	  *num_mask = *num_mask + 1;
	  //	  ans = ans | (decode_nfo->mach_code.bop[current_location] & (1<<bit));
	  //	  smallest = bit;
	  //          if ( on_word < 0 )
            on_word=current_location;
	}
      mask++;
    }

  *length = current_location;
  if ( bit != 0 )
    g_warning("Error in mask");

  //  ans = ans >> smallest;

  if ( word_num )
    *word_num = on_word;

  return ans;
}


Word xor_bit_extract(char info, char *mask, struct _decoded_opcode *decode_nfo,
		     int *word_num, int *num_mask)
{
  int bit,smallest;
  Word ans;
  int current_location,on_word;
//  int found_a;

  ans = 0;
  smallest = 0;
  on_word = -1;
  bit = BITS_PER_PROGRAM_ACCESS;
//  found_a = 0;
  current_location = 0;
  *num_mask = 0;

  while ( *mask )
    {
      if ( *mask !=' ' )
	{
	 bit--;

	 //          if ( *mask == 'a' )
	 //            found_a = 1;

	  if ( bit < 0 )
	    {
	      if ( *mask == '\0' )
		{
		  printf("Error! bit description can't span multiple words file=%s:%d\n",
			 __FILE__,__LINE__);
		}
	      bit = BITS_PER_PROGRAM_ACCESS-1;

              // skip over next word if the a decode takes up 2 words
//              if ( found_a )
//                current_location = current_location + 1 + decode_nfo->var_length;
//              else
                current_location = current_location + 1;
	    }
	}
      if ( *mask == info )
	{
	  ans = ans << 1;
	  //  largest_bit += 1;
          if ( (decode_nfo->mach_code.bop[current_location] & (1<<bit)) )
	    ans = ans | 1;
	  smallest = bit;

	  *num_mask = *num_mask + 1;

	  //  ans = ans | ((decode_nfo->mach_code.bop[current_location] & (1<<bit)) ^ (1<<bit));
	  //smallest = bit;
	  // if ( on_word < 0 )
            on_word=current_location;
	}
      mask++;
    }
  //ans = ans >> smallest;

  if ( word_num )
    *word_num = on_word;

  return ans;
}

SWord signed_bit_extract(char info, char *mask, struct _decoded_opcode *decode_nfo)
{
  int bit,smallest,largest_bit,sign_bit;
  guint32 ans;
  int current_location;

  ans = 0;
  smallest = 0;
  bit = BITS_PER_PROGRAM_ACCESS;
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
	      bit = BITS_PER_PROGRAM_ACCESS-1;
              
              current_location = current_location + 1;
 	    }
	}
      if ( *mask == info )
	{
	  if ( largest_bit < 0 )
	    {
	      sign_bit=(decode_nfo->mach_code.bop[current_location] & (1<<bit));
	      largest_bit = 0;
	    }
	  ans = ans << 1;
	  largest_bit += 1;
          if ( (decode_nfo->mach_code.bop[current_location] & (1<<bit)) )
	    ans = ans | 1;
	  smallest = bit;
	}
      mask++;
    }
  //  ans = ans >> smallest;
  if ( sign_bit )
    {
      int k;
      for (k=largest_bit+1;k<32;k++)
	{
	  ans = ans | (1<<k);
	}
    }

  return (gint32)ans;
}
