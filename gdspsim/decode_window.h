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

#ifndef __DECODE_WINDOW_H__
#define __DECODE_WINDOW_H__

#include <gtk/gtk.h>
#include "c54_core.h"

GtkWidget *create_decode_window();
void highlight_decode_index(unsigned long int index);
void set_decode_index_max_range(unsigned long int lower_index, unsigned long int upper_index);
void highlight_pipeline(WordA prefetch);


#endif // __DECODE_WINDOW_H__
