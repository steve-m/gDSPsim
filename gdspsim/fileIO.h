/*
 * gDSPsim - GNU Digital Signal Processor Simulator
 *
 * Copyright (C) 2002, Kerry Keal, kerry@industrialmusic.com
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

#ifndef __FILEIO_H__
#define __FILEIO_H__

#include <gtk/gtk.h>
#include "chip_core.h"
#include <stdio.h>

typedef enum { MEMORY_READ=0, MEMORY_WRITE=1, PIPELINE_EXECUTED=2 } FileIOReached;

struct _fileIO
{
  int mem_type_reached; // Data or Program mem
  WordA address_reached; // reached
  FileIOReached  reached_how;
  int put_get; // put or get
  int amount;
  gchar *filename;
  int type_access;
  int address_access;

  int valid;
  GtkWidget *connect_box;
  GtkWidget *removeB;
  GtkWidget *applyB;
  GtkWidget *closeB;
  int modified; // non-zero if things have been modified
  FILE *file;

  GtkWidget *mem_type_reachedW;
  GtkWidget *address_reachedW;
  GtkWidget *reached_howW;
  GtkWidget *put_getW;
  GtkWidget *amountW;
  GtkWidget *filenameW;
  GtkWidget *type_accessW;
  GtkWidget *address_accessW;
  
};

void create_fileIO(GtkWidget *widget, gpointer data);

// Function to call when a file IO condition is met
void fileIO_process(struct _fileIO *io);

#endif // __FILEIO_H__



