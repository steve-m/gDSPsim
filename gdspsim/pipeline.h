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

#ifndef __PIPELINE_H__
#define __PIPELINE_H__

// Should be called once to setup
struct _Registers *pipe_new();

// Return 1 if it hits a breakpoint
// Return 0, otherwise
// clear_old_changes = 1, unhighlight old changes
// clear_old_changes = 0, don't unhighlight changes from last call
int pipeline(struct _Registers *Registers);
void reset_view();
void update_view();

// Reset the registers
void default_registers(struct _Registers *Registers);

// Return 1 if breakpoint set, 0 if breakpoint removed
int toggle_breakpoint(WordA bp);

#endif //__PIPELINE_H__

