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

#include "smem.h"
#include "hardware.h"
#include <stdio.h>
#include "memory.h"

void mmem_set_EAB(struct _PipeLine *pipeP, struct _Registers *Reg)
{
  Word EAB;
  int num_words;

  // Only update if on last word of Smem
  if ( pipeP->current_opcode & 0x80 )
    {
      // Indirect Addressing
      
      num_words = num_words_for_smem(pipeP);

     if ( num_words > 1 )
	{
	  // word_number counts down from total_words
	  if ( (pipeP->total_words - pipeP->word_number) == 1 )
	    {
	      EAB = update_smem_2words(pipeP->current_opcode & 0xff , 
				       Reg->IR , Reg );
	      Reg->EAB = EAB;
	      Reg->EAB = EAB & 0x7f;
	    }
	}
      else if ( pipeP->total_words == pipeP->word_number )
	{
	  // This updates the auxillary registers 
	  EAB = update_smem(pipeP->current_opcode & 0xff , Reg, CPL(MMR));
	  Reg->EAB = EAB & 0x7f;
	}
    }
  else if ( pipeP->total_words == pipeP->word_number )
    {
      // Direct addressing. Only 1 word for due to Smem
      EAB = pipeP->current_opcode & 0x7f;
      Reg->EAB = EAB;
    }
  
}

void smem_set_EAB(struct _PipeLine *pipeP, struct _Registers *Reg)
{
  int num_words;

  num_words = num_words_for_smem(pipeP);

  if ( num_words > 1 )
    {
      // word_number counts down from total_words
      if ( (pipeP->total_words - pipeP->word_number) == 1 )
	{
	  Reg->EAB = update_smem_2words(pipeP->current_opcode & 0xff , 
					pipeP->storage1 , Reg);
	}
    }
  else if ( pipeP->word_number == 1 )
    {
      // This updates the auxillary registers 
      Reg->EAB = update_smem(pipeP->current_opcode & 0xff , Reg, CPL(MMR));
    }
}

// The basic read_stg1 for a smem. Handles 1 or 2 word smem. Places
// the read address onto DAB.
void smem_read_stg1(struct _PipeLine *pipeP, struct _Registers *Reg)
{
  Word DAB;
  int num_words;

  num_words = num_words_for_smem(pipeP);

  if ( num_words > 1 )
    {
      // word_number counts down from total_words
      if ( (pipeP->total_words - pipeP->word_number) == 1 )
	{
	  DAB = update_smem_2words(pipeP->current_opcode & 0xff , 
				   Reg->IR , Reg );
	  Reg->DAB = DAB;
	}
    }
  else if ( pipeP->total_words == pipeP->word_number )
    {
      // This updates the auxillary registers 
      DAB = update_smem(pipeP->current_opcode & 0xff , Reg, CPL(MMR));
      Reg->DAB = DAB;
    }
}

// The basic read_stg1 for a smem. Handles 1 or 2 word smem. Places
// the read address onto DAB.
void smem_read_stg2(struct _PipeLine *pipeP, struct _Registers *Reg)
{
  int wait_state;
  int num_words;

  num_words = num_words_for_smem(pipeP);

  if ( ( (num_words == 1) &&  ( pipeP->total_words == pipeP->word_number ) ) ||
       ( (pipeP->total_words - pipeP->word_number) == 1 ) )
    {
      Reg->DB = read_data_mem(Reg->DAB,&wait_state);
    }
}

void mmem_read_stg1(struct _PipeLine *pipeP, struct _Registers *Reg)
{
  WordA DAB;
  
  if ( pipeP->total_words > 1 )
    {
      // word_number counts down from total_words
      if ( pipeP->word_number == 1 )
	{
	  DAB = update_smem_2words(pipeP->current_opcode & 0xff , 
				   Reg->IR , Reg);
	  DAB = DAB & 0x7f; // Force it to page 0, needed for indirect addressing.
	  Reg->DAB = DAB;
	}
    }
  else
    {
      // This updates the auxillary registers 
      DAB = update_smem(pipeP->current_opcode & 0xff , Reg, 2);
      DAB = DAB & 0x7f; // Force it to page 0, needed for indirect addressing.
      Reg->DAB = DAB;
    }
}

// The basic read_stg1 for a smem. Handles 1 or 2 word smem. Places
// the read address onto DAB.
void mmem_read_stg2(struct _PipeLine *pipeP, struct _Registers *Reg)
{
  int wait_state;

  if ( pipeP->word_number == 1 )
    {
      Reg->DB = read_data_mem(Reg->DAB,&wait_state);
    }
}

// This updates the auxillary registers and returns the read address
// Generally, Ref is 2, for memory mapped registers and the CPL flag otherwise.
// If Ref is used for direct addressing (Smem<0x80). If Ref = 0, then the
// DP (data page pointer) times 0x80 is added to the address. If Ref = 1, then
// SP (stack pointer) is added to the address. If Ref=2, then nothing is added. 
Word update_smem( int Smem, struct _Registers *Reg, int Ref)
{
  int mod;
  Word *arfP;
  Word ReadAddress;

  if ( (Smem & 0x80)==0 )
    {
      // direct or Memory Mapped
      ReadAddress = Smem & 0x7f;
      if ( Ref == 1 )
	ReadAddress = ReadAddress + MMR->SP;
      else if ( Ref == 0 )
	ReadAddress = ReadAddress + 0x80*(MMR->ST0 & 0x1ff);

      return ReadAddress;
    }
  else
    {
      g_return_val_if_fail( Smem & 0x80, (Word)0); // Must be an indirect operation

      mod = (Smem & (8+16+32+64) ) >> 3;
  
      g_return_val_if_fail( mod < 12 , (Word)0 ); // Must be a 1 word smem operation
  
      // Get the particular pointer register
      arfP=get_pointer_reg(Smem & 7,Reg, CMPT(MMR));
  
      switch (mod & 0xf)
	{
	case 0:
	  return *arfP;
	case 1:
	  ReadAddress = *arfP;
	  *arfP = *arfP - 1;
	  return ReadAddress;
	case 2:
	  ReadAddress = *arfP;
	  *arfP = *arfP + 1;
	  return ReadAddress;
	case 3:
	  *arfP = *arfP + 1;
	  return *arfP;
	case 4:
	  ReadAddress = *arfP;
	  *arfP = bit_reversal(*arfP,MMR->ar0,(SWord)-1);
	  return ReadAddress;
	case 5:
	  ReadAddress = *arfP;
	  *arfP = *arfP - MMR->ar0;
	  return ReadAddress;
	case 6:
	  ReadAddress = *arfP;
	  *arfP = *arfP + MMR->ar0;
	  return ReadAddress;
	case 7:
	  ReadAddress = *arfP;
	  *arfP = bit_reversal(*arfP,MMR->ar0,1);
	  return ReadAddress;
	case 8:
	  ReadAddress = *arfP;
	  *arfP = circular_update(*arfP,(SWord)-1,MMR->BK);
	  return ReadAddress;
	case 9:
	  ReadAddress = *arfP;
	  *arfP = circular_update(*arfP,-((SWord)MMR->ar0),MMR->BK);
	  return ReadAddress;
	case 10:
	  ReadAddress = *arfP;
	  *arfP = circular_update(*arfP,(SWord)1,MMR->BK);
	  return ReadAddress;
	case 11:
	  ReadAddress = *arfP;
	  *arfP = circular_update(*arfP,(SWord)MMR->ar0,MMR->BK);
	  return ReadAddress;

	  
	}
    }

  return -1;
  
}

Word update_smem_2words( int Smem, Word next_word, struct _Registers *Reg)
{
  Word *arfP;
   int mod;
  Word ReadAddress;

  // Get the particular pointer register
  arfP=get_pointer_reg(Smem & 7,Reg, CMPT(MMR));

  g_return_val_if_fail( Smem & 0x80, (Word)0 ); // Must be an indirect operation

  mod = (Smem & (8+16+32+64) ) >> 3;

  g_return_val_if_fail( mod >= 12, (Word)0 ); // Must be a 2 word smem operation

  arfP=get_pointer_reg(Smem & 7,Reg, CMPT(MMR));

  switch ( mod )
    {
    case 12:
      ReadAddress = *arfP + next_word;
      break;
    case 13:
      *arfP = circular_update(*arfP,next_word,MMR->BK);
      ReadAddress = *arfP;
      break;
    case 14:
      *arfP = *arfP +  next_word; // circular
      ReadAddress = *arfP;
      break;
    case 15:
      ReadAddress = next_word;
    }
  return ReadAddress;
}

Word circular_update(Word start, SWord step, Word BK)
{
  SWord index;
  Word N;

  // Determine the N LSBs of BK
  N=1;
  while (BK)
    {
      N = N << 1;
      BK = BK >> 1;
    }

  index = start & (N-1);

  if ( (index + step) < 0 )
    index = index + step + BK;
  else if ( (index + step) <= BK )
    index = index + step;
  else
    index = index + step - BK;

  start = (start & ~(N-1)) + index;

  return start;
}

int num_words_for_smem_plus1(struct _PipeLine *pipeP)
{
  return num_words_for_smem(pipeP)+1;
}

int num_words_for_smem(struct _PipeLine *pipeP)
{
  Word Smem;

  Smem = pipeP->current_opcode;
  if ( Smem & 128 )
    {
      // indirect mode
      int mod;
      mod = (Smem & (8+16+32+64) ) >> 3;

      if ( mod >= 12 )
	return 2;
      else
	return 1;
    }
  return 1;
}

Word bit_reversal(Word start, Word bit_reversed_one, SWord adjustment)
{
  Word base;
  SWord index,br_index,add;

  bit_reversed_one = bit_reversed_one<<1;

  index = start & (bit_reversed_one-1);

  add=1;
  br_index = 0;
  while (index)
    {
      index=index<<1;
      if ( index & bit_reversed_one )
	{
	  br_index = br_index + add;
	}
      add = add << 1;
    }

  // now we can make the adjustment
  br_index = br_index + adjustment;

  // looped around
  if ( br_index == bit_reversed_one )
    br_index=0;
  if ( br_index < 0 )
    br_index=bit_reversed_one-1;

  // reverse again
  add=1;
  index = 0;
  while (br_index)
    {
      br_index=br_index<<1;
      if ( br_index & bit_reversed_one )
	{
	  index = index + add;
	}
      add = add << 1;
    }

  base = start & ~(bit_reversed_one-1);

  return base+index;
}

void lmem_read_stg1(struct _PipeLine *pipeP, struct _Registers *Reg)
{
  Word L1,L2;
  int num_words;

  num_words = num_words_for_smem(pipeP);

  if ( num_words > 1 )
    {
      // word_number counts down from total_words
      if ( (pipeP->total_words - pipeP->word_number) == 1 )
	{
	  L1 = update_smem_2words(pipeP->current_opcode & 0xff , 
				  Reg->IR , Reg );
	  Reg->Lmem1 = L1;
	  Reg->Lmem2 = L1 ^ 1;
	}
    }
  else
    {
      int mod,indirect;

      // This updates the auxillary registers 
      // mods 1,2,3,8,10 gets update_smem twice
      // mod 0 doesnt matter
      
      if ( pipeP->total_words == pipeP->word_number )
	{
	  indirect = ( pipeP->current_opcode & 0x80 );
	  mod = (pipeP->current_opcode & (8+16+32+64) ) >> 3;

	  L1 = update_smem(pipeP->current_opcode & 0xff , Reg, CPL(MMR));
	  Reg->Lmem1 = L1;

	  if ( (indirect) &&  ( (mod==1) || (mod==2) || (mod==3) ||
				(mod==8) || (mod=10) ) )
	    {
	      L2 = update_smem(pipeP->current_opcode & 0xff , Reg, CPL(MMR));
	      Reg->Lmem2 = L2;
	    }
	  else
	    {
	      Reg->Lmem2 = L1 ^ 1;
	    }
	}
    }
}

void lmem_read_stg2(struct _PipeLine *pipeP, struct _Registers *Reg)
{
  int wait_state;
  int num_words;
  Word L1,L2;

  num_words = num_words_for_smem(pipeP);

  if ( (num_words == 1) ||
       ( (pipeP->total_words - pipeP->word_number) == 1 ) )
    {
      union _GP_Reg_Union reg_union;
      L1 = read_data_mem(Reg->Lmem1,&wait_state);
      L2 = read_data_mem(Reg->Lmem2,&wait_state);
      reg_union.gint64 = 0;
      reg_union.words.low = L2;
      reg_union.words.high = L1;
      
      Reg->Shifter = reg_union.gp_reg;
    }
}

void lmem_set_EAB(struct _PipeLine *pipeP, struct _Registers *Reg)
{
  int num_words;

  // DST uses this

  num_words = num_words_for_smem(pipeP);

  if ( num_words > 1 )
    {
      if ( pipeP->cycles == 1 )
        {
	  Reg->EAB = update_smem_2words(pipeP->current_opcode & 0xff , 
					Reg->IR , Reg);
	}
      else if ( pipeP->cycles == 2 )
	{
	  Reg->EAB = Reg->EAB ^ 1;
	}
    }
  else if ( pipeP->word_number == 1 )
    {
      int mod,indirect;

      // This updates the auxillary registers 
      // mods 1,2,3,8,10 gets update_smem twice
      // mod 0 doesnt matter
      
      indirect = ( pipeP->current_opcode & 0x80 );
      mod = (pipeP->current_opcode & (8+16+32+64) ) >> 3;

      if ( (indirect) &&  ( (mod==1) || (mod==2) || (mod==3) ||
			    (mod==8) || (mod==10) ) )
	{
	  // mod==1 *ARx-  mod==2 *ARx+  mod==3 *+ARx
	  // mod==8 *ARx-% mod==10 *ARx+%

	  // Not sure if this should call update each time
	  // or just xor EAB. 
	  if ( pipeP->cycles == 0 )
	    Reg->EAB = update_smem(pipeP->current_opcode & 0xff , Reg, CPL(MMR));
	  else if ( pipeP->cycles == 1 )
	    Reg->EAB = update_smem(pipeP->current_opcode & 0xff , Reg, CPL(MMR));
	}
      else
	{
	  if ( pipeP->cycles == 0 )
	    Reg->EAB = update_smem(pipeP->current_opcode & 0xff , Reg, CPL(MMR));
	  else if ( pipeP->cycles == 1 )
	    Reg->EAB = Reg->EAB ^ 1;
	}
    }
}
