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
/* 
 * This include is to define some of the basic properties of the DSP/CPU core
 */

#ifndef __CORE_DEF_H__
#define __CORE_DEF_H__

#define BYTES_PER_WORD 2
#define BYTES_PER_ADDRESS 2

#define BITS_PER_WORD 8*BYTES_PER_WORD
#define BITS_PER_ADDRESS 8*BYTES_PER_ADDRESS

#define MAX_WORD 0xffff

// We want to deal with the proper bit length words.
typedef guint16 Word;
typedef gint16 SWord;
typedef guint16 WordA; //type of variable used for addressing Words.

//typedef struct _Word Word;
struct _Word
{
  unsigned char byte0;
  unsigned char byte1;
};

// 40 bit General Purpose Register
struct _GP_Reg
{
  unsigned char byte0;
  unsigned char byte1;
  unsigned char byte2;
  unsigned char byte3;
  unsigned char byte4;
};

typedef struct _GP_Reg GP_Reg;

struct _GP_Words
{
  Word low;
  Word high;
};

union _GP_Reg_Union
{
  GP_Reg gp_reg;
  guint64 guint64;
  gint64 gint64;
  struct _GP_Words words;
};


// Used to tell if the opcode tag is used in the notation. 
#define INCLUDE_OPCODE_IN_DECODE 1
// Used to tell if commas are inserted between operands
#define INCLUDE_COMMA_IN_DECODE 1

/* For example:
 * #define INCLUDE_OPCODE_IN_DECODE 1
 * #define INCLUDE_COMMA_IN_DECODE 1
 * ADD A,*AR1,B
 *
 * #define INCLUDE_OPCODE_IN_DECODE 0
 * #define INCLUDE_COMMA_IN_DECODE 0
 * B = A + *AR1
 */

#define FIXME()(printf("FIXME %s:%d\n",__FILE__,__LINE__))

#endif // __CORE_DEF_H__
