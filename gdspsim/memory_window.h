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

#ifndef __MEMORY_WINDOW_H__
#define __MEMORY_WINDOW_H__

#include <gtk/gtk.h>

void create_memory_window();
// highlight = 1, to highlight changed memory
// highlight = 0, to unhighlight changed memory
void update_all_memory_windows(int highlight);

// Used to clear what memory has changed
void clear_mem_changed(void);

// Used to mark what memory has changed
// type==0 for written, type==1 for read
void set_mem_changed(WordA address, int type);


#endif // __MEMORY_WINDOW_H__
