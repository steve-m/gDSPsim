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

// This C file is the interface to a popup window that displays decoded assembly

/* needed prototypes */
/* Called when step button pressed */
void decode_step(void);

/* Returns an array of strings that should be freed by the caller */
gchar **get_decoded_text(unsigned long int start_index, unsigned long int end_index);


/*
 * This will be called by the main routine to create a popup window.
 * At the top there should be 2 spin buttons that controls the range
 * of memory that the scroll bars can access (and labeled as such). One for
 * the top index and one for the bottom index. For 
 * memory efficieny reasons, the user may only want part of the memory visiable.
 * There should also be a step button at the top that will give a call to 
 * decode_step when pressed.
 */

GtkWidget *create_decode_window()
{
}


/* 
 * If the user is simulating code and stepping through it, then the main
 * program will call this function to highlight the correct line
 */
void highlight_decode_index(unsigned long int index)
{
}
/* 
 * Used to set min and max allowable scrowable range
 */
void set_decode_index_max_range(unsigned long int lower_index, unsigned long int upper_index)
{
}
