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

#ifndef __MEMORY_H__
#define __MEMORY_H__

#include "chip_core.h"
#include "fileIO.h"

typedef enum { PROGRAM_MEM_TYPE=1,  DATA_MEM_TYPE=2 } MemType;

#ifdef __cplusplus
extern "C" {
#endif

/* Read data memory and determine it's wait state. Generally on chip RAM has
 * a wait state of 0 and external RAM has a wait state of 1 or more. This
 * procedure takes into account all the memory mapping needed to get the
 * correct value */

// read data mem using DP for page
Word read_data_mem(Word offset, int *wait_state);

// read program mem using DP for page
PWord read_program_mem(Word offset, int *wait_state);

// read data mem ignoring DP
Word read_data_mem_long(WordP offset, int *wait_state);

// read program mem ignoring DP
Word read_program_mem_long(WordP offset, int *wait_state);

// available = 1, if it is available
// available = 0, if it is not (not been written too)
Word read_mem(WordP offset, int *wait_state, MemType type, int *available);
// returns wait state
int write_mem(WordP offset, Word value, MemType type);

/* Write's to memory. Return's wait state or -1 if unsucessfull */

// write to program mem using DP for page
int write_program_mem(Word offset, Word value);

// write to data mem using DP for page
int write_data_mem(Word offset, Word value);

// write to program mem using offset
int write_program_mem_long(WordP offset, Word value);

// write to data mem using offset
int write_data_mem_long(WordP offset, Word value);

Word read_port_mem(WordP offset, int *wait_state);
int write_port_mem(WordP offset, Word value);

  /* Creates a chunk of DSP memory and fills it, if data is not NULL.
   * Normally this called by the function the reads in and processes the
   * COFF file being analyzed. It is also called to generate memory-mapped
   * registers and if any memory creation is specified if DSP memory is
   * accessed that doesn't exist. */
void create_mem(Word *data, WordP start, long int size, MemType type);

// Gets the min and max of all defined program memory, there may
// be holes inbetween
void get_prog_mem_start_end(WordP *start, WordP *end);
// Usually set to the .text section
void set_prog_mem_start_end(WordP start, WordP end);

void set_MMR_ptr();

// Debug function to print defined memory
void print_mem_list(void);


// Functions used for file IO
  void set_fileIO_break_on_memory(struct _fileIO *io);
  void update_fileIO_break_on_memory(struct _fileIO *io);
  void remove_fileIO_break_on_memory(struct _fileIO *io);

#ifdef __cplusplus
}
#endif

#endif // __MEMORY_H__
