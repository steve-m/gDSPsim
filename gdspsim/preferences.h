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

#include <gtk/gtk.h>

#ifndef __PREFERENCES_H__
#define __PREFERENCES_H__
// globals
extern gboolean bound_program_mem_read;
extern gboolean bound_data_mem_read;
extern gboolean add_data_mem_on_write;
extern gboolean add_data_mem_on_read;

extern int gStopRun; // used to stop a run

// Prototypes
void edit_preferences(void);

#endif // __PREFERENCES_H__
